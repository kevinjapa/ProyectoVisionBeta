#include <jni.h>
#include <string>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/video.hpp>
#include <android/log.h>

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
//
//CascadeClassifier face_cascade;
//CascadeClassifier eyes_cascade;
//CascadeClassifier nose_cascade;
//CascadeClassifier mouth_cascade;
//
//void loadCascade(CascadeClassifier &cascade, AAssetManager *mgr, const char *filename) {
//    AAsset *asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
//    if (!asset) {
//        LOGE("Error al abrir el archivo %s", filename);
//        return;
//    }
//
//    size_t size = AAsset_getLength(asset);
//    char *buffer = new char[size];
//    int bytesRead = AAsset_read(asset, buffer, size);
//    AAsset_close(asset);
//
//    if (bytesRead <= 0) {
//        LOGE("Error al leer el archivo %s", filename);
//        delete[] buffer;
//        return;
//    }
//
//    // Write buffer to a temporary file
//    std::string tmpFilePath = "/data/data/com.your.package.name/cache/tmp_" + std::string(filename);
//    FILE *tmpFile = fopen(tmpFilePath.c_str(), "wb");
//    if (!tmpFile) {
//        LOGE("Error al crear el archivo temporal %s", tmpFilePath.c_str());
//        delete[] buffer;
//        return;
//    }
//
//    size_t bytesWritten = fwrite(buffer, sizeof(char), size, tmpFile);
//    fclose(tmpFile);
//    delete[] buffer;
//
//    if (bytesWritten != size) {
//        LOGE("Error al escribir en el archivo temporal %s", tmpFilePath.c_str());
//        return;
//    }
//
//    if (!cascade.load(tmpFilePath)) {
//        LOGE("Error al cargar el cascade classifier desde %s", tmpFilePath.c_str());
//    }
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_ups_vision_proyectovision_SecondActivity_loadCascade(JNIEnv *env, jobject instance, jobject assetManager) {
//    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
//
//    loadCascade(face_cascade, mgr, "haarcascade_frontalface_default.xml");
//    loadCascade(eyes_cascade, mgr, "haarcascade_eye.xml");
//    loadCascade(nose_cascade, mgr, "haarcascade_mcs_nose.xml");
//    loadCascade(mouth_cascade, mgr, "haarcascade_mcs_mouth.xml");
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_ups_vision_proyectovision_SecondActivity_reconocimiento
//        (JNIEnv* env,
//         jobject /*this*/,
//         jobject bitmapIn,
//         jobject bitmapOut)
//{
//    Mat src;
//    bitmapToMat(env, bitmapIn, src, false);
//
//    Mat gray;
//    vector<Rect> faces, eyes, noses, mouths;
//
//    // Convertir la imagen a escala de grises
//    cvtColor(src, gray, COLOR_BGR2GRAY);
//    equalizeHist(gray, gray);
//
//    // Detectar rostros
//    face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
//
//    for (size_t i = 0; i < faces.size(); i++) {
//        rectangle(src, faces[i], Scalar(255, 0, 255), 4);
//
//        Mat faceROI = gray(faces[i]);
//
//        // Detectar ojos en la región del rostro
//        eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
//        for (size_t j = 0; j < eyes.size(); j++) {
//            Rect eye_rect = Rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
//            rectangle(src, eye_rect, Scalar(255, 0, 0), 4);
//        }
//
//        // Detectar nariz en la región del rostro
//        nose_cascade.detectMultiScale(faceROI, noses, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
//        for (size_t j = 0; j < noses.size(); j++) {
//            Rect nose_rect = Rect(faces[i].x + noses[j].x, faces[i].y + noses[j].y, noses[j].width, noses[j].height);
//            rectangle(src, nose_rect, Scalar(0, 255, 0), 4);
//        }
//
//        // Detectar boca en la región del rostro
//        mouth_cascade.detectMultiScale(faceROI, mouths, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
//        for (size_t j = 0; j < mouths.size(); j++) {
//            Rect mouth_rect = Rect(faces[i].x + mouths[j].x, faces[i].y + mouths[j].y, mouths[j].width, mouths[j].height);
//            rectangle(src, mouth_rect, Scalar(0, 0, 255), 4);
//        }
//    }
//
//    matToBitmap(env, src, bitmapOut, false);
//}

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
Java_ups_vision_proyectovision_SecondActivity_reconocimiento(JNIEnv* env, jobject, jobject bitmapIn, jobject bitmapOut) {
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

    // Convertir bitmap a Mat (usando funciones que ya tienes implementadas)
    Mat src;
    bitmapToMat(env, bitmapIn, src, false);

    Mat gray;
    vector<Rect> faces, eyes, noses, mouths;

    cvtColor(src, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);

    // Detectar rostros
    face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

    for (size_t i = 0; i < faces.size(); i++) {
        rectangle(src, faces[i], Scalar(255, 0, 255), 4);

        Mat faceROI = gray(faces[i]);
        Mat faceColorROI = src(faces[i]);

        eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < eyes.size(); j++) {
            Rect eye_rect = Rect(faces[i].x + eyes[j].x, faces[i].y + eyes[j].y, eyes[j].width, eyes[j].height);
            rectangle(src, eye_rect, Scalar(255, 0, 0), 4);
        }

        nose_cascade.detectMultiScale(faceROI, noses, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < noses.size(); j++) {
            Rect nose_rect = Rect(faces[i].x + noses[j].x, faces[i].y + noses[j].y, noses[j].width, noses[j].height);
            rectangle(src, nose_rect, Scalar(0, 255, 0), 4);
        }

        mouth_cascade.detectMultiScale(faceROI, mouths, 1.1, 5, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
        for (size_t j = 0; j < mouths.size(); j++) {
            Rect mouth_rect = Rect(faces[i].x + mouths[j].x, faces[i].y + mouths[j].y, mouths[j].width, mouths[j].height);
            rectangle(src, mouth_rect, Scalar(0, 0, 255), 4);
        }
    }

    // Convertir Mat a bitmap (usando funciones que ya tienes implementadas)
    matToBitmap(env, src, bitmapOut, false);
}
