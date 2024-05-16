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

extern "C" JNIEXPORT jstring

JNICALL
Java_ups_vision_proyectovision_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}


extern "C" JNIEXPORT void JNICALL
Java_ups_vision_proyectovision_MainActivity_imagenGris(JNIEnv *env, jobject object, jobject imagen) {
   Mat mGray= reinterpret_cast<Mat &&>(imagen);
   cvtColor(mGray, mGray, COLOR_RGBA2GRAY);
}
