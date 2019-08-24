/*************************************************************************
    > File Name: yuv_overlay.c
    > Author: zhongjihao
    > Mail: zhongjihao100@163.com 
    > Created Time: 2019年08月18日 19时45分22秒 CST
 ************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FORMAT_NV21 1
#define FORMAT_NV12 2
#define FORMAT_YV12 3
#define FORMAT_I420 4

/**
 * yuvType yuv类型
 * startX,startY 需要添加水印的位置
 * waterMarkData 水印YUV数据，可以通过读取水印文件获取
 * waterMarkW,waterMarkH 水印数据的分辨率
 * yuvData 源YUV图像数据
 * yuvW,yuvH 源YUV的分辨率
 **/
void yuvAddWaterMark(int yuvType, int startX, int startY, unsigned char *waterMarkData,
                 int waterMarkW, int waterMarkH, unsigned char *yuvData, int yuvW, int yuvH) {
   switch(yuvType) {
     case FORMAT_NV21:
     case FORMAT_NV12:{
         int i = 0;
         int j = 0;
         int k = 0;
         for(i = startY; i < waterMarkH+startY; i++) {
            //逐行拷贝Y分量
            memcpy(yuvData+startX+i*yuvW, waterMarkData+j*waterMarkW, waterMarkW);
            j++;
          }

          for(i = startY/2; i < (waterMarkH+startY)/2; i++) {
            //UV分量高度是Y分量的一半,逐行拷贝UV分量
            memcpy(yuvData+startX+yuvW*yuvH+i*yuvW, waterMarkData+waterMarkW*waterMarkH+k*waterMarkW, waterMarkW);
            k++;
          }
 
          #ifdef DUMP_OUTPUT
          char output[256];
          memset(output,0,sizeof(output));
          sprintf(output,"nv21_%dx%d.yuv",yuvW,yuvH);
          FILE *outPutFp = fopen(output, "w+");
          fwrite(yuvData, 1, yuvW*yuvH*3/2, outPutFp);
          fclose(outPutFp);
          #endif
         break;
     }
     case FORMAT_YV12:
     case FORMAT_I420:{
          char* y1 = waterMarkData;
          char* u1 = y1 + waterMarkW * waterMarkH;
          char* v1 = u1 + waterMarkW * waterMarkH / 4;

          char* y2 = yuvData;
          char* u2 = y2 + yuvW*yuvH;
          char* v2 = u2 + yuvW * yuvH / 4;

         //从图像左上角开始进行yuv覆盖
         for (int i = 0; i < waterMarkH; i++){ //每次循环一次，扫描一行数据

           memcpy(y2, y1, waterMarkW); //y值覆盖
           memcpy(u2, u1, waterMarkW / 2); //注意：根据采样比例，每一行的Y是每一行的U的两倍
           memcpy(v2, v1, waterMarkW/2);

        //换行，指针偏移到行首
        y2 += yuvW;
        u2 += yuvW/2;
        v2 += yuvW/2;

        y1 += waterMarkW;
        u1 += waterMarkW / 2;
        v1 += waterMarkW / 2;

    }
      
         #ifdef DUMP_OUTPUT
         char output[256];
         memset(output,0,sizeof(output));
         sprintf(output,"i420_%dx%d.yuv",yuvW,yuvH);
         FILE *outPutFp = fopen(output, "w+");
         fwrite(yuvData, 1, yuvW*yuvH*3/2, outPutFp);
         fclose(outPutFp);
         #endif
         break;
     }
     default:
     //Not FInished
        break;
   }
 }


int main(int argc, char **argv)
{
    int sw = 256, sh = 256;
    int bw = 640, bh = 480;
    FILE* fp1 = NULL;
    fp1 = fopen("lena_256x256_yuv420p.yuv","rb");
    FILE* fp2 = NULL;
    fp2 = fopen("i420_640x480_channel_1_index_9.yuv", "rb");
    char yuv1[sw*sh*3/2];
    char yuv2[bw*bh*3/2];

    fread(yuv1, sw * sh * 3/2,1,fp1);
    fread(yuv2, bw * bh * 3/2, 1, fp2);

    yuvAddWaterMark(FORMAT_I420, 10, 20,yuv1,sw, sh, yuv2, bw, bh);

    fclose(fp1);
    fclose(fp2);
    return 0;
}