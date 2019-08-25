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
 * Nv21(分辨率8x4):
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * VUVUVUVU
 * VUVUVUVU
 *
 * Nv12(分辨率8x4):
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * UVUVUVUV
 * UVUVUVUV
 *
 * 分辨率8x4
 * YUV420P(I420):
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * UUUU
 * UUUU
 * VVVV
 * VVVV
 *
 * 分辨率8x4
 * YV12:
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * YYYYYYYY
 * VVVV
 * VVVV
 * UUUU
 * UUUU
 **/

/**
 *YV12 -> I420 
*/
void Yv12ToI420(unsigned char* pYv12, unsigned char* pI420, int width, int height)
{
	if(pYv12 == NULL || pI420 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	//拷贝Y分量
	memcpy(pI420,pYv12,frameSize);
	//拷贝U分量
	memcpy(pI420+frameSize,pYv12+frameSize*5/4,frameSize/4);
	//拷贝V分量
	memcpy(pI420+frameSize*5/4,pYv12+frameSize,frameSize/4);
}

/**
 *I420 -> YV12
*/
void I420ToYv12(unsigned char* pI420, unsigned char* pYv12, int width, int height)
{
	if(pI420 == NULL || pYv12 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	//拷贝Y分量
	memcpy(pYv12,pI420,frameSize);
	//拷贝V分量
	memcpy(pYv12+frameSize,pI420+frameSize*5/4,frameSize/4);
	//拷贝U分量
	memcpy(pYv12+frameSize*5/4,pI420+frameSize,frameSize/4);
}

/*
 * NV21 -> I420
*/
void Nv21ToI420(unsigned char* pNv21,unsigned char* pI420,int width,int height)
{
	if(pNv21 == NULL || pI420 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	int i = 0;
	//拷贝Y分量
	memcpy(pI420,pNv21,frameSize);
	
	for (i = 0; i < frameSize / 2; i += 2) {
		//U分量
		pI420[frameSize + i/2] = pNv21[frameSize + i + 1];
		//V分量
		pI420[frameSize + i/2 + frameSize / 4] = pNv21[frameSize + i];
	}
}

/*
 * I420 -> NV21
*/
void I420ToNv21(unsigned char* pI420,unsigned char* pNv21,int width,int height)
{
	if(pI420 == NULL || pNv21 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	int i = 0;
	//拷贝Y分量
	memcpy(pNv21,pI420,frameSize);
	
	for (i = 0; i < frameSize / 2; i += 2) {
		//V分量
		pNv21[frameSize + i] = pI420[frameSize + frameSize/4 + i/2];
		//U分量
		pNv21[frameSize + i + 1] = pI420[frameSize + i/2];
	}
}

/*
 * NV21 -> NV12
*/
void Nv21ToNv12(unsigned char* pNv21,unsigned char* pNv12,int width,int height)
{
	if(pNv21 == NULL || pNv12 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	//拷贝Y分量
	memcpy(pNv12,pNv21,frameSize);

	int i = 0;
	for (i = 0; i < frameSize / 4; i++) {
		pNv12[frameSize + i * 2] = pNv21[frameSize + i * 2 + 1]; //U
		pNv12[frameSize + i * 2 + 1] = pNv21[frameSize + i * 2]; //V
	}
}

/**
 *NV12 -> NV21
*/
void Nv12ToNv21(unsigned char* pNv12,unsigned char* pNv21,int width,int height)
{
	if(pNv12 == NULL || pNv21 == NULL)
		return;

	int frameSize = width * height;
	if(frameSize <= 0)
		return;

	//拷贝Y分量
	memcpy(pNv21,pNv12,frameSize);
    
	int i = 0;
	for (i = 0; i < frameSize / 4; i++) {
		pNv21[frameSize + i * 2] = pNv12[frameSize + i * 2 + 1]; //V
		pNv21[frameSize + i * 2 + 1] = pNv12[frameSize + i * 2]; //U
	}
}



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
   if(waterMarkData == NULL || yuvData == NULL)
	   return;
   if(yuvW < startX + waterMarkW || (yuvH < startY + waterMarkH))
	   return;

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
          sprintf(output,"water_nv21_%dx%d.yuv",yuvW,yuvH);
          FILE *outPutFp = fopen(output, "w+");
          fwrite(yuvData, 1, yuvW*yuvH*3/2, outPutFp);
          fclose(outPutFp);
          #endif
         break;
     }
     case FORMAT_I420:{
		  int i = 0;
          char* waterY = waterMarkData;
          char* waterU = waterY + waterMarkW * waterMarkH;
          char* waterV = waterU + waterMarkW * waterMarkH / 4;

          char* destY = yuvData;
          char* destU = destY + yuvW*yuvH;
          char* destV = destU + yuvW * yuvH / 4;

		  //拷贝Y分量
		  for (i = 0; i < waterMarkH; i++){ //每次循环一次，扫描一行数据
			  memcpy(destY+startX+(i+startY)*yuvW, waterY+i*waterMarkW, waterMarkW); //y值覆盖
		  }

		  for (i = 0; i < waterMarkH/2; i++){ //每次循环一次，扫描一行数据
			  //拷贝U分量
			  memcpy(destU+(i+startY/2)*(yuvW/2)+startX/2, waterU+i*(waterMarkW/2), waterMarkW/2);
			  //拷贝V分量
			  memcpy(destV+(i+startY/2)*(yuvW/2)+startX/2, waterV+i*(waterMarkW/2), waterMarkW/2);
		  }
      
         #ifdef DUMP_OUTPUT
         char output[256];
         memset(output,0,sizeof(output));
         sprintf(output,"water_i420_%dx%d.yuv",yuvW,yuvH);
         FILE *outPutFp = fopen(output, "w+");
         fwrite(yuvData, 1, yuvW*yuvH*3/2, outPutFp);
         fclose(outPutFp);
         #endif
         break;
     } 
     case FORMAT_YV12:{
		  int i = 0;
          char* waterY = waterMarkData;
          char* waterV = waterY + waterMarkW * waterMarkH;
          char* waterU = waterV + waterMarkW * waterMarkH / 4;

          char* destY = yuvData;
          char* destV = destY + yuvW*yuvH;
          char* destU = destV + yuvW * yuvH / 4;

		  //拷贝Y分量
		  for (i = 0; i < waterMarkH; i++){ //每次循环一次，扫描一行数据
			  memcpy(destY+startX+(i+startY)*yuvW, waterY+i*waterMarkW, waterMarkW); //y值覆盖
		  }

		  for (i = 0; i < waterMarkH/2; i++){ //每次循环一次，扫描一行数据
			  //拷贝V分量
			  memcpy(destV+(i+startY/2)*(yuvW/2)+startX/2, waterV+i*(waterMarkW/2), waterMarkW/2);
			  //拷贝U分量
			  memcpy(destU+(i+startY/2)*(yuvW/2)+startX/2, waterU+i*(waterMarkW/2), waterMarkW/2);
		  }
      
         #ifdef DUMP_OUTPUT
         char output[256];
         memset(output,0,sizeof(output));
         sprintf(output,"water_yv12_%dx%d.yuv",yuvW,yuvH);
         FILE *outPutFp = fopen(output, "w+");
         fwrite(yuvData, 1, yuvW*yuvH*3/2, outPutFp);
         fclose(outPutFp);
         #endif
         break;
     }
     default:
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

    //fp2 = fopen("test_nv12_640x480.yuv", "rb");
    char yuv1[sw*sh*3/2];
    char yuv2[bw*bh*3/2];

    fread(yuv1, sw * sh * 3/2,1,fp1);
    fread(yuv2, bw * bh * 3/2, 1, fp2);

    yuvAddWaterMark(FORMAT_I420, 70,100,yuv1,sw, sh, yuv2, bw, bh);

    fclose(fp1);
    fclose(fp2);
    return 0;
}
