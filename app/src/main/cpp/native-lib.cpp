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
//extern "C"
//JNIEXPORT void JNICALL
//Java_ups_vision_proyectovision_MainActivity_imagenGris(JNIEnv *env, jobject instance, jobject bitmap) {
//    AndroidBitmapInfo info;
//    void* pixels;
//
//
//    // Crea un objeto cv::Mat desde el bitmap
//    Mat src(info.height, info.width, CV_8UC4, pixels);
//    Mat gray;
//
//    // Convierte a escala de grises
//    cvtColor(src, gray, COLOR_RGBA2GRAY);
//    cvtColor(gray, src, COLOR_GRAY2RGBA);
//
//    AndroidBitmap_unlockPixels(env, bitmap);
//}

extern "C"
JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_imagenGris(JNIEnv *env, jobject instance, jobject bitmap) {
    AndroidBitmapInfo info;
    void* pixels;

    // Obtén la información del bitmap
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return;
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) return;
    if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return;

    // Crea un objeto cv::Mat desde el bitmap
    Mat src(info.height, info.width, CV_8UC4, pixels);
    Mat gray;

    // Convierte a escala de grises
    cvtColor(src, gray, COLOR_RGBA2GRAY);
    cvtColor(gray, src, COLOR_GRAY2RGBA);

    AndroidBitmap_unlockPixels(env, bitmap);
}

