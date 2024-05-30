#include <jni.h>
#include <string>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/video.hpp>
#include <android/log.h>

#include "android/bitmap.h"

using namespace cv;
using namespace std;

extern "C" JNIEXPORT jstring

JNICALL
Java_ups_vision_proyectovision_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
//extern "C" JNIEXPORT void JNICALL
//Java_ups_vision_proyectovision_MainActivity_imagenGris(JNIEnv *env, jobject object, jobject imagen) {
//   Mat mGray= reinterpret_cast<Mat &&>(imagen);
//   cvtColor(mGray, mGray, COLOR_RGBA2GRAY);
//}

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

extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_fondoVerdeCartoon
        (JNIEnv* env,
        jobject /*this*/,
        jobject bitmapIn,
        jobject bitmapOut){
    Mat src;

    bitmapToMat(env, bitmapIn, src, false);

    Mat hsv,src2;
    src2=src.clone();
    cvtColor(src, hsv, COLOR_BGR2HSV);

    Scalar verde_bajo(35, 50, 50);
    Scalar verde_alto(85, 255, 255);

    Mat mascara_verde;
    inRange(hsv, verde_bajo, verde_alto, mascara_verde);

    Mat mascara_fondo;
    bitwise_not(mascara_verde, mascara_fondo);

    Mat fondo;
    fondo.setTo(Scalar(255, 255, 255)); // Fondo blanco
    src.copyTo(fondo, mascara_fondo);

    //Filtro del cartoon
    Mat imgBN;
    int mascara=7;
    cvtColor(fondo, imgBN, COLOR_BGR2GRAY);
    medianBlur(imgBN, imgBN, mascara);
    Laplacian(imgBN,imgBN,CV_8U, mascara);
    threshold(imgBN,imgBN,80,255,THRESH_BINARY_INV);

    // Ejemplo del método usado para crear imágenes vacías
    Mat resultado = Mat::zeros(Size(imgBN.cols, imgBN.rows), CV_8UC4);
    for (int i=0;i<imgBN.rows;i++){
        for(int j=0;j<imgBN.cols;j++){
            Vec4b pixel1 = fondo.at<Vec4b>(i, j);
            uchar pixel2 = imgBN.at<uchar>(i, j);
            if(pixel1[0] == 0 && pixel1[1] == 0 && pixel1[2] == 0 && pixel1[3] == 0){
                resultado.at<Vec4b>(i,j) = Vec4b(0, 0, 0, 0);
            }else{
                resultado.at<Vec4b>(i,j) = Vec4b(pixel2, pixel2, pixel2, 255);
            }
        }
    }
    matToBitmap(env, resultado, bitmapOut, false);
//    matToBitmap(env, fondo, bitmapOut, false);
}
extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_medianBlur
(JNIEnv* env,
jobject /*this*/,
jobject bitmapIn,
        jobject bitmapOut){
    Mat src,filtro;
    int mascara=5;
    bitmapToMat(env, bitmapIn, src, false);
    medianBlur(src,filtro,mascara);
    matToBitmap(env, filtro, bitmapOut, false);
}


extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_iluminacion
        (JNIEnv* env,
         jobject /*this*/,
         jobject bitmapIn,
         jobject bitmapOut){
    Mat src, filtro,src2;
    bitmapToMat(env, bitmapIn, src, false);

    Mat hsv,src3;
    src3=src.clone();
    cvtColor(src, hsv, COLOR_BGR2HSV);

    Scalar verde_bajo(35, 50, 50);
    Scalar verde_alto(85, 255, 255);

    Mat mascara_verde;
    inRange(hsv, verde_bajo, verde_alto, mascara_verde);

    Mat mascara_fondo;
    bitwise_not(mascara_verde, mascara_fondo);

    Mat fondo;
    fondo.setTo(Scalar(255, 255, 255)); // Fondo blanco
    src.copyTo(fondo, mascara_fondo);

    src2=src.clone();
    cvtColor(src,src,COLOR_BGR2GRAY);
    Ptr<CLAHE> metodoClahe = createCLAHE();
    metodoClahe-> apply(src, filtro);
    metodoClahe-> setTilesGridSize(Size(5,5));
    
    Mat resultado = Mat::zeros(Size(filtro.cols, filtro.rows), CV_8UC4);
    for (int i=0;i<filtro.rows;i++){
        for(int j=0;j<filtro.cols;j++){
            Vec4b pixel1 = fondo.at<Vec4b>(i, j);
            uchar pixel2 = filtro.at<uchar>(i, j);
            if(pixel1[0] == 0 && pixel1[1] == 0 && pixel1[2] == 0 && pixel1[3] == 0){
                resultado.at<Vec4b>(i,j) = Vec4b(0, 0, 0, 0);
            }else{
                resultado.at<Vec4b>(i,j) = Vec4b(pixel2, pixel2, pixel2, 255);
            }
        }
    }

    matToBitmap(env, filtro, bitmapOut, false);

//    matToBitmap(env, filtro, bitmapOut, false);
}

extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_textoImagen
        (JNIEnv* env,
         jobject /*this*/,
         jobject bitmapIn,
         jobject bitmapOut,
         jstring texto){
    Mat src, imgTexto;
    const char *nativeTexto = env->GetStringUTFChars(texto, 0);
    std::string textoCpp(nativeTexto);
//    env->ReleaseStringUTFChars(texto, nativeTexto);
    bitmapToMat(env, bitmapIn, src, false);
//    putText(src,textoCpp,Point(src.cols/3,src.rows/3), FONT_HERSHEY_SIMPLEX, 2.0, Scalar(3,3,233));
    putText(src,textoCpp, Point(src.cols/4,src.rows-50),FONT_HERSHEY_SIMPLEX,1,Scalar(3,3,255),3);
    matToBitmap(env, src, bitmapOut, false);
}

extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_fondoVerde
        (JNIEnv* env,
         jobject /*this*/,
         jobject bitmapIn,
         jobject bitmapOut){
    Mat src;
    bitmapToMat(env, bitmapIn, src, false);

    // Convertir la imagen a HSV para trabajar con el canal de color verde
    Mat hsv;
    cvtColor(src, hsv, COLOR_BGR2HSV);

    // Definir los límites del color verde en HSV
    Scalar verde_bajo(35, 50, 50);
    Scalar verde_alto(85, 255, 255);

    // Crear una máscara para el rango de color verde
    Mat mascara_verde;
    inRange(hsv, verde_bajo, verde_alto, mascara_verde);

    // Invertir la máscara para obtener el fondo
    Mat mascara_fondo;
    bitwise_not(mascara_verde, mascara_fondo);

    // Aplicar la máscara al fondo
    Mat fondo;
    fondo.setTo(Scalar(255, 255, 255)); // Fondo blanco
    src.copyTo(fondo, mascara_fondo);
    matToBitmap(env, fondo, bitmapOut, false);
}