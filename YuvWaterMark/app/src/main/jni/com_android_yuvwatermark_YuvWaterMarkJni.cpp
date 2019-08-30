//
// Created by zhongjihao on 19-8-26.
//

#include "com_android_yuvwatermark_YuvWaterMarkJni.h"

#define LOG_TAG "YuvWaterMark-Jni"

#include "./log.h"
#include "yuvwater/yuvwater.h"

JNIEXPORT jlong JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_startYuvWaterEngine
  (JNIEnv *env, jclass jcls  __unused)
{
    YuvWater* pYuvWater =  new YuvWater;
    if(pYuvWater != NULL) {
        return reinterpret_cast<long> (pYuvWater);
    }
}


JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_Yv12ToI420
  (JNIEnv *env, jclass jcls __unused, jlong jcPtr, jbyteArray jyv12Data, jbyteArray jI420Data, jint jwidth, jint jheight)
{
    jbyte* jYv12 = env->GetByteArrayElements(jyv12Data, NULL);
    jbyte* jI420 = env->GetByteArrayElements(jI420Data, NULL);
    unsigned char* pYv12 = (unsigned char*)jYv12;
    unsigned char* pI420 = (unsigned char*)jI420;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->Yv12ToI420(pYv12,pI420, (int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jyv12Data, jYv12, 0);
    env->ReleaseByteArrayElements(jI420Data, jI420, 0);
}


JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_I420ToYv12
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jbyteArray jI420Data, jbyteArray jyv12Data, jint jwidth, jint jheight)
{
    jbyte* jI420 = env->GetByteArrayElements(jI420Data, NULL);
    jbyte* jYv12 = env->GetByteArrayElements(jyv12Data, NULL);

    unsigned char* pI420 = (unsigned char*)jI420;
    unsigned char* pYv12 = (unsigned char*)jYv12;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->I420ToYv12(pI420,pYv12,(int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jI420Data, jI420, 0);
    env->ReleaseByteArrayElements(jyv12Data, jYv12, 0);
}


JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_Nv21ToI420
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jbyteArray jNv21Data, jbyteArray jI420Data, jint jwidth, jint jheight)
{
    jbyte* jNv21 = env->GetByteArrayElements(jNv21Data, NULL);
    jbyte* jI420 = env->GetByteArrayElements(jI420Data, NULL);

    unsigned char* pNv21 = (unsigned char*)jNv21;
    unsigned char* pI420 = (unsigned char*)jI420;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->Nv21ToI420(pNv21,pI420,(int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jNv21Data, jNv21, 0);
    env->ReleaseByteArrayElements(jI420Data, jI420, 0);
}


JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_I420ToNv21
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jbyteArray jI420Data , jbyteArray jNv21Data, jint jwidth, jint jheight)
{
    jbyte* jI420 = env->GetByteArrayElements(jI420Data, NULL);
    jbyte* jNv21 = env->GetByteArrayElements(jNv21Data, NULL);

    unsigned char* pI420 = (unsigned char*)jI420;
    unsigned char* pNv21 = (unsigned char*)jNv21;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->I420ToNv21(pI420,pNv21,(int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jI420Data, jI420, 0);
    env->ReleaseByteArrayElements(jNv21Data, jNv21, 0);
}

JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_Nv21ToNv12
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jbyteArray jNv21Data, jbyteArray jNv12Data, jint jwidth, jint jheight)
{
    jbyte* jNv21 = env->GetByteArrayElements(jNv21Data, NULL);
    jbyte* jNv12 = env->GetByteArrayElements(jNv12Data, NULL);

    unsigned char* pNv21 = (unsigned char*)jNv21;
    unsigned char* pNv12 = (unsigned char*)jNv12;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->Nv21ToNv12(pNv21,pNv12,(int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jNv21Data, jNv21, 0);
    env->ReleaseByteArrayElements(jNv12Data, jNv12, 0);
}


JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_Nv12ToNv21
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jbyteArray jNv12Data, jbyteArray jNv21Data, jint jwidth, jint jheight)
{
    jbyte* jNv12 = env->GetByteArrayElements(jNv12Data, NULL);
    jbyte* jNv21 = env->GetByteArrayElements(jNv21Data, NULL);

    unsigned char* pNv12 = (unsigned char*)jNv12;
    unsigned char* pNv21 = (unsigned char*)jNv21;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->Nv12ToNv21(pNv12,pNv21,(int)jwidth, (int)jheight);
    env->ReleaseByteArrayElements(jNv12Data, jNv12, 0);
    env->ReleaseByteArrayElements(jNv21Data, jNv21, 0);
}

JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_yuvAddWaterMark
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr, jint jyuvType, jint jstartX, jint jstartY, jbyteArray jwaterMarkData, jint jwaterMarkW, jint jwaterMarkH, jbyteArray jyuvData, jint jyuvW, jint jyuvH)
{
    jbyte* jwaterMark = env->GetByteArrayElements(jwaterMarkData, NULL);
    jbyte* jyuv = env->GetByteArrayElements(jyuvData, NULL);

    unsigned char* pWaterMark = (unsigned char*)jwaterMark;
    unsigned char* pYuv = (unsigned char*)jyuv;

    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    pYuvWater->yuvAddWaterMark((int)jyuvType,(int)jstartX,(int)jstartY,pWaterMark,(int)jwaterMarkW,(int)jwaterMarkH,pYuv,(int)jyuvW, (int)jyuvH);
    env->ReleaseByteArrayElements(jwaterMarkData, jwaterMark, 0);
    env->ReleaseByteArrayElements(jyuvData, jyuv, 0);
}

JNIEXPORT void JNICALL Java_com_android_yuvwatermark_YuvWaterMarkJni_stopYuvWaterEngine
  (JNIEnv * env, jclass jcls __unused, jlong jcPtr)
{
    YuvWater* pYuvWater = reinterpret_cast<YuvWater*> (jcPtr);
    delete pYuvWater;
}
