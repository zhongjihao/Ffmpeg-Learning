//
// Created by SPREADTRUM\jihao.zhong on 18-4-13.
//

#ifndef YUVWATERMARK_LOG_H
#define YUVWATERMARK_LOG_H


#include <android/log.h>


#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


#endif //YUVWATERMARK_LOG_H
