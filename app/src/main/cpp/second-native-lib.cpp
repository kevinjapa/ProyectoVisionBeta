#include <jni.h>
#include <string>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/video.hpp>
#include <android/log.h>
#include <fstream>


#include "android/bitmap.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <opencv2/ml.hpp>
#include <opencv2/opencv.hpp>

#define LOG_TAG "native-lib"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)


using namespace cv;
using namespace std;
using namespace cv::ml;
using namespace ml;

AAssetManager* assetManager = nullptr;

void bitmapToMat(JNIEnv * env, jobject bitmap, cv::Mat &dst, jboolean needUnPremultiplyAlpha){
    AndroidBitmapInfo info;
    void* pixels = 0;
    try {
        CV_Assert( AndroidBitmap_getInfo(env, bitmap, &info) >= 0 );
        CV_Assert( info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                   info.format == ANDROID_BITMAP_FORMAT_RGB_565 );
        CV_Assert( AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0 );
        CV_Assert( pixels );
        dst.create(info.height, info.width, CV_8UC4);
        if( info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 )
        {
            cv::Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if(needUnPremultiplyAlpha) cvtColor(tmp, dst, cv::COLOR_mRGBA2RGBA);
            else tmp.copyTo(dst);
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            cv::Mat tmp(info.height, info.width, CV_8UC2, pixels);
            cvtColor(tmp, dst, cv::COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch(const cv::Exception& e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        //jclass je = env->FindClass("org/opencv/core/CvException");
        jclass je = env->FindClass("java/lang/Exception");
        //if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nBitmapToMat}");
        return;
    }
}

void matToBitmap(JNIEnv * env, cv::Mat src, jobject bitmap, jboolean needPremultiplyAlpha) {
    AndroidBitmapInfo info;
    void *pixels = 0;
    try {
        CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
        CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
                  info.format == ANDROID_BITMAP_FORMAT_RGB_565);
        CV_Assert(src.dims == 2 && info.height == (uint32_t) src.rows &&
                  info.width == (uint32_t) src.cols);
        CV_Assert(src.type() == CV_8UC1 || src.type() == CV_8UC3 || src.type() == CV_8UC4);
        CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
        CV_Assert(pixels);
        if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            cv::Mat tmp(info.height, info.width, CV_8UC4, pixels);
            if (src.type() == CV_8UC1) {
                cvtColor(src, tmp, cv::COLOR_GRAY2RGBA);
            } else if (src.type() == CV_8UC3) {
                cvtColor(src, tmp, cv::COLOR_RGB2RGBA);
            } else if (src.type() == CV_8UC4) {
                if (needPremultiplyAlpha) cvtColor(src, tmp, cv::COLOR_RGBA2mRGBA);
                else src.copyTo(tmp);
            }
        } else {
            // info.format == ANDROID_BITMAP_FORMAT_RGB_565
            cv::Mat tmp(info.height, info.width, CV_8UC2, pixels);
            if (src.type() == CV_8UC1) {
                cvtColor(src, tmp, cv::COLOR_GRAY2BGR565);
            } else if (src.type() == CV_8UC3) {
                cvtColor(src, tmp, cv::COLOR_RGB2BGR565);
            } else if (src.type() == CV_8UC4) {
                cvtColor(src, tmp, cv::COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, bitmap);
        return;
    } catch (const cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, bitmap);
        //jclass je = env->FindClass("org/opencv/core/CvException");
        jclass je = env->FindClass("java/lang/Exception");
        //if(!je) je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, bitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "Unknown exception in JNI code {nMatToBitmap}");
        return;
    }
}

bool loadFileFromAssets(const char* filename, vector<char>& buffer) {
    if (!assetManager) {
        LOGE("AssetManager is null");
        return false;
    }

    AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open asset: %s", filename);
        return false;
    }

    size_t size = AAsset_getLength(asset);
    buffer.resize(size);

    int readBytes = AAsset_read(asset, buffer.data(), size);
    if (readBytes != size) {
        LOGE("Failed to read asset: %s", filename);
        AAsset_close(asset);
        return false;
    }

    AAsset_close(asset);
    LOGI("Loaded asset file %s successfully, size: %zu bytes", filename, size);
    return true;
}

bool loadCascade(CascadeClassifier& cascade, const char* filename) {
    vector<char> buffer;
    if (!loadFileFromAssets(filename, buffer)) {
        LOGE("Failed to load file from assets: %s", filename);
        return false;
    }

    // Crear un archivo temporal
    string tempFilePath = "/data/data/ups.vision.proyectovision/files/temp_cascade.xml";
    FILE* tempFile = fopen(tempFilePath.c_str(), "wb");
    if (!tempFile) {
        LOGE("Failed to create temporary file");
        return false;
    }
    fwrite(buffer.data(), 1, buffer.size(), tempFile);
    fclose(tempFile);

    // Cargar el archivo temporal en CascadeClassifier
    if (!cascade.load(tempFilePath)) {
        LOGE("Failed to load cascade from temporary file: %s", filename);
        remove(tempFilePath.c_str()); // Eliminar el archivo temporal
        return false;
    }

    remove(tempFilePath.c_str()); // Eliminar el archivo temporal
    return true;
}

//Inicializa los Assets
extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_SecondActivity_initAssetManager(JNIEnv* env, jobject, jobject mgr) {
    assetManager = AAssetManager_fromJava(env, mgr);
    if (assetManager == nullptr) {
        LOGE("Failed to initialize AssetManager");
    } else {
        LOGI("AssetManager initialized successfully");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_SecondActivity_reconocimiento(JNIEnv* env, jobject instance, jobject bitmapIn, jobject bitmapOut, jstring path) {
    const char *pathStr = env->GetStringUTFChars(path, 0);

    // Verificar si el AssetManager está inicializado
    if (!assetManager) {
        LOGE("AssetManager is null, cannot load cascades");
        return;
    }

    // Inicializar clasificadores Haar
    CascadeClassifier face_cascade;
    CascadeClassifier eyes_cascade;
    CascadeClassifier nose_cascade;
    CascadeClassifier mouth_cascade;

    if (!loadCascade(face_cascade, "haarcascade_frontalface_default.xml")) {
        LOGE("Failed to load face cascade");
        return;
    } else {
        LOGI("Loaded face cascade successfully");
    }

    if (!loadCascade(eyes_cascade, "haarcascade_eye.xml")) {
        LOGE("Failed to load eye cascade");
        return;
    } else {
        LOGI("Loaded eye cascade successfully");
    }

    if (!loadCascade(nose_cascade, "haarcascade_mcs_nose.xml")) {
        LOGE("Failed to load nose cascade");
        return;
    } else {
        LOGI("Loaded nose cascade successfully");
    }

    if (!loadCascade(mouth_cascade, "haarcascade_mcs_mouth.xml")) {
        LOGE("Failed to load mouth cascade");
        return;
    } else {
        LOGI("Loaded mouth cascade successfully");
    }

    Mat src;
    bitmapToMat(env, bitmapIn, src, false);

    Mat gray;
    vector<Rect> faces, eyes, noses, mouths;

    cvtColor(src, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);

    // Abrir archivo para guardar los puntos
    ofstream outFile(pathStr);
    outFile << "type,x,y,width,height\n"; // Encabezado del CSV

    // Detectar rostros
    face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

    for (size_t i = 0; i < faces.size(); i++) {
        rectangle(src, faces[i], Scalar(255, 0, 255), 4);
        outFile << "face," << faces[i].x << "," << faces[i].y << "," << faces[i].width << "," << faces[i].height << "\n";

        Mat faceROI = gray(faces[i]);
        Mat faceColorROI = src(faces[i]);

        eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < eyes.size(); j++) {
            Rect eye_rect = Rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
            rectangle(src, eye_rect, Scalar(255, 0, 0), 4);
            outFile << "eye," << eye_rect.x << "," << eye_rect.y << "," << eye_rect.width << "," << eye_rect.height << "\n";
        }

        nose_cascade.detectMultiScale(faceROI, noses, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < noses.size(); j++) {
            Rect nose_rect = Rect(faces[i].x + noses[j].x, faces[i].y + noses[j].y, noses[j].width, noses[j].height);
            rectangle(src, nose_rect, Scalar(0, 255, 0), 4);
            outFile << "nose," << nose_rect.x << "," << nose_rect.y << "," << nose_rect.width << "," << nose_rect.height << "\n";
        }

        mouth_cascade.detectMultiScale(faceROI, mouths, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < mouths.size(); j++) {
            Rect mouth_rect = Rect(faces[i].x + mouths[j].x, faces[i].y + mouths[j].y, mouths[j].width, mouths[j].height);
            rectangle(src, mouth_rect, Scalar(0, 0, 255), 4);
            outFile << "mouth," << mouth_rect.x << "," << mouth_rect.y << "," << mouth_rect.width << "," << mouth_rect.height << "\n";
        }
    }

    // Cerrar el archivo
    outFile.close();

    // Liberar la cadena de caracteres
    env->ReleaseStringUTFChars(path, pathStr);

    // Convertir Mat a bitmap (usando funciones que ya tienes implementadas)
    matToBitmap(env, src, bitmapOut, false);
}


//FUNCIONES PARA PRDECIR

bool readAssetToBuffer(const char* filename, vector<char>& buffer) {
    if (!assetManager) {
        LOGE("AssetManager is null");
        return false;
    }

    AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);
    if (!asset) {
        LOGE("Failed to open asset: %s", filename);
        return false;
    }

    size_t size = AAsset_getLength(asset);
    buffer.resize(size);

    int readBytes = AAsset_read(asset, buffer.data(), size);
    if (readBytes != size) {
        LOGE("Failed to read asset: %s", filename);
        AAsset_close(asset);
        return false;
    }

    AAsset_close(asset);
    LOGI("Loaded asset file %s successfully, size: %zu bytes", filename, size);
    return true;
}

bool loadANNModel(Ptr<ml::ANN_MLP>& model, const char* filename) {
    vector<char> buffer;
    if (!readAssetToBuffer(filename, buffer)) {
        LOGE("Failed to load file from assets: %s", filename);
        return false;
    }

    string tempFilePath = "/data/data/ups.vision.proyectovision/files/temp_model.yml";
    FILE* tempFile = fopen(tempFilePath.c_str(), "wb");
    if (!tempFile) {
        LOGE("Failed to create temporary file");
        return false;
    }
    fwrite(buffer.data(), 1, buffer.size(), tempFile);
    fclose(tempFile);

    model = ml::ANN_MLP::load(tempFilePath);
    if (model.empty()) {
        LOGE("Failed to load model from temporary file: %s", filename);
        remove(tempFilePath.c_str());
        return false;
    }

    remove(tempFilePath.c_str());
    return true;
}

bool loadClassLabels(map<int, string>& labelMap, const char* filename) {
    vector<char> buffer;
    if (!readAssetToBuffer(filename, buffer)) {
        LOGE("Failed to load file from assets: %s", filename);
        return false;
    }

    istringstream inputStream(string(buffer.begin(), buffer.end()));
    int label;
    string className;
    while (inputStream >> label >> className) {
        labelMap[label] = className;
    }

    return true;
}



//extern "C"
//JNIEXPORT void JNICALL
//Java_ups_vision_proyectovision_SecondActivity_Predecir(JNIEnv* env, jobject bitmapIn, jobject bitmapOut) {
//
//    Ptr<ml::ANN_MLP> modelo;
//    map<int, string> etiquetas;
//
//    // Verificar si el AssetManager está inicializado
//    if (!assetManager) {
//        LOGE("AssetManager is null, cannot load assets");
//        return;
//    }
//
//    if (!loadANNModel(modelo, "mlp_mnist_model.yml")) {
//        LOGE("Error al cargar el modelo");
//        return;
//    } else {
//        LOGI("Modelo cargado");
//    }
//
//    if (!loadClassLabels(etiquetas, "label_map.txt")) {
//        LOGE("Error al cargar etiquetas");
//        return;
//    } else {
//        LOGI("Etiquetas cargadas");
//    }
//
//    Mat src;
//    bitmapToMat(env, bitmapIn, src, false);
//
//    // Redimensionar la imagen al tamaño esperado
//    Mat resizedImage;
//    resize(src, resizedImage, Size(28, 28)); // Cambiar el tamaño a 28x28 para MNIST
//
//    // Definir los parámetros del HOGDescriptor
//    HOGDescriptor hog(
//            Size(28, 28),   // winSize (tamaño de las imágenes MNIST)
//            Size(14, 14),   // blockSize
//            Size(7, 7),     // blockStride
//            Size(7, 7),     // cellSize
//            9               // nbins
//    );
//
//    // Extraer características usando HOG
//    vector<float> descriptor;
//    hog.compute(resizedImage, descriptor);
//
//    // Convertir el descriptor a Mat y asegurar que sea de tipo CV_32F
//    Mat descriptorMat = Mat(descriptor).reshape(1, 1);  // Reshape para tener una fila por descriptor
//    descriptorMat.convertTo(descriptorMat, CV_32F);     // Convertir a tipo CV_32F
//
//    // Predecir la clase usando el modelo MLP
//    Mat response;
//    modelo->predict(descriptorMat, response);
//    Point maxLoc;
//    minMaxLoc(response, 0, 0, 0, &maxLoc);
//    int predictedLabel = maxLoc.x;
//
//    // Mostrar el nombre de la clase en lugar del número de etiqueta
//    string predictedClassName = etiquetas[predictedLabel];
//    LOGI("La imagen ha sido clasificada como: %s", predictedClassName.c_str());
//
//    // Convertir Mat a bitmap (usando funciones que ya tienes implementadas)
//    matToBitmap(env, src, bitmapOut, false);
//}

extern "C"
JNIEXPORT jstring JNICALL
Java_ups_vision_proyectovision_SecondActivity_Predecir(JNIEnv* env, jobject obj, jobject bitmapIn, jobject bitmapOut) {

    Ptr<ml::ANN_MLP> modelo;
    map<int, string> etiquetas;

    // Verificar si el AssetManager está inicializado
    if (!assetManager) {
        LOGE("AssetManager is null, cannot load assets");
//        return;
    }

    if (!loadANNModel(modelo, "mlp_mnist_model.yml")) {
        LOGE("Error al cargar el modelo");
//        return;
    } else {
        LOGI("Modelo cargado");
    }

    if (!loadClassLabels(etiquetas, "label_map.txt")) {
        LOGE("Error al cargar etiquetas");
//        return;
    } else {
        LOGI("Etiquetas cargadas");
    }

    string resultadoStr;
    Mat src;
    bitmapToMat(env, bitmapIn, src, false);
    // Convertir la imagen a escala de grises si es necesario
    cvtColor(src, src, COLOR_BGR2GRAY);
    // Redimensionar la imagen al tamaño esperado
    Mat resizedImage;
    resize(src, resizedImage, Size(28, 28)); // Cambiar el tamaño a 28x28 para MNIST

    // Definir los parámetros del HOGDescriptor
    HOGDescriptor hog(
            Size(28, 28),   // winSize (tamaño de las imágenes MNIST)
            Size(14, 14),   // blockSize
            Size(7, 7),     // blockStride
            Size(7, 7),     // cellSize
            9               // nbins
    );

    // Extraer características usando HOG
    vector<float> descriptor;
    hog.compute(resizedImage, descriptor);

    // Convertir el descriptor a Mat y asegurar que sea de tipo CV_32F
    Mat descriptorMat = Mat(descriptor).reshape(1, 1);  // Reshape para tener una fila por descriptor
    descriptorMat.convertTo(descriptorMat, CV_32F);     // Convertir a tipo CV_32F

    // Predecir la clase usando el modelo MLP
    Mat response;
    modelo->predict(descriptorMat, response);
    Point maxLoc;
    minMaxLoc(response, 0, 0, 0, &maxLoc);
    int predictedLabel = maxLoc.x;

    // Mostrar el nombre de la clase en lugar del número de etiqueta
    string predictedClassName = etiquetas[predictedLabel];
    LOGI("La imagen ha sido clasificada como: %s", predictedClassName.c_str());
    resultadoStr=("Es el Nro : "+ predictedClassName);

    // Convertir Mat a bitmap (usando funciones que ya tienes implementadas)
    matToBitmap(env, src, bitmapOut, false);
    return env->NewStringUTF(resultadoStr.c_str());
}

