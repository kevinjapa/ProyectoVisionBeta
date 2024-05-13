#include <jni.h>
#include <string>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/video.hpp>

#include "android/bitmap.h"



extern "C" JNIEXPORT jstring

JNICALL
Java_ups_vision_proyectovision_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_Camara(JNIEnv *env, jobject thiz, jlong matAddrGray){
    cv::Mat& frame = *(cv::Mat*)matAddrGray;
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY); // Convertir a escala de grises

}
