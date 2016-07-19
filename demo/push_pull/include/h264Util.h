#ifndef __RECORD_UTIL__
#define __RECORD_UTIL__

/*
这里h264文件的存储方式是sps,pps,sei,i,p...p,sps,pps.......每个nalu序列前面有四个字节的起始码。
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define READ_ONCE 128*1024 //应该大于最大的视频帧的大小

typedef struct _h264Helper
{
	char* buffer;
	char* data;
	int   bufferSize;
	int   frameSize;
	int   offset;
	int   startCodeLen;
	int	  naluType;
	int   sendDoneFlag;
	FILE* fp;
	char  readFileEof;
	char  readFileFlag;
}h264Helper;

#ifdef __cplusplus
extern "C"{
#endif

int h264HelperInit(h264Helper* helper,char* fileName);

int h264HelperFree(h264Helper* helper);

int getH264Frame(h264Helper* helper);

#ifdef __cplusplus
}
#endif

#endif