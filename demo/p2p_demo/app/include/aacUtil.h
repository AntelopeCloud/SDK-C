#ifndef __AAC_UTIL__
#define __AAC_UTIL__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef AAC_BUFFER
#define AAC_BUFFER 1024*30
#endif

typedef struct _AACHelper
{
	int				channels;
	int				simple_rate;
	int				profile;
	char*			buffer;
	int				buffer_size;
	int				offset;
	int				readFileEof;
	int				get_config;
	FILE*			fp;
	char			config[2];
	char			readAAC;
}AACHelper;

#ifdef __cplusplus
extern "C"{
#endif

int initAACHelper(AACHelper* helper,char* fileName);

int freeAACHelper(AACHelper* helper);

//获取成功返回0，否则-1
int getAACFrame(AACHelper* helper,char* data,int* len);

unsigned long long getMSec();

#ifdef __cplusplus
}
#endif

#endif