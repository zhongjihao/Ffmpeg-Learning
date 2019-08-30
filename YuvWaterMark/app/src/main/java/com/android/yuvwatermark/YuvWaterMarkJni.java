package com.android.yuvwatermark;

/**
 * Created by zhongjihao on 18-4-25.
 */
public class YuvWaterMarkJni {
    static {
        System.loadLibrary("yuvWaterMark");
    }

    public native static long startYuvWaterEngine();
    public native static void Yv12ToI420(long cPtr,byte[] pYv12, byte[] pI420, int width, int height);
    public native static void I420ToYv12(long cPtr,byte[] pI420, byte[] pYv12, int width, int height);
    public native static void Nv21ToI420(long cPtr,byte[] pNv21,byte[] pI420,int width,int height);
    public native static void I420ToNv21(long cPtr,byte[] pI420,byte[] pNv21,int width,int height);
    public native static void Nv21ToNv12(long cPtr,byte[] pNv21,byte[] pNv12,int width,int height);
    public native static void Nv12ToNv21(long cPtr,byte[] pNv12,byte[] pNv21,int width,int height);
    public native static void yuvAddWaterMark(long cPtr,int yuvType, int startX, int startY, byte[] waterMarkData,
                                              int waterMarkW, int waterMarkH,byte[] yuvData, int yuvW, int yuvH);
    public native static void stopYuvWaterEngine(long cPtr);
}
