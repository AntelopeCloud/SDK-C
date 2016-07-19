#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include "cJSON.h"
#include "LYPlatformAPI.h"
#include "common_define.h"

#define WRITE_FILE 1
pthread_t pidRecv = NULL; 
int fd = -1;
int recving = 1;

void* recvProc(void*arg)
{
	int ret;
#if WRITE_FILE
	FILE* h264File = fopen("recvH264.h264","wb");
	if(h264File == NULL)
	{
		printf("open recv file failed\n");
		return NULL;
	}
	FILE* aacFile = fopen("recvAAC.aac","wb");
	if(aacFile == NULL)
	{
		printf("open aacFile file failed\n");
		return NULL;
	}
#endif
	MediaFrame_t frame;
	frame.frameBuffer = (unsigned char*)malloc(512*1024);
	frame.capacitySize = 512*1024;//这个必要设置，否则无法拷贝数据
	int bits = 0;
	struct timeval s,e;
	unsigned long long curTime = 0;
	recving = 1;
	while(recving)
	{

		ret = LY_recvMediaFrame(fd,&frame);
		if(ret != 0)
		{
			printf("recv media frame ret=%d\n",ret);
			break;
		}
		curTime = e.tv_sec*1000*1000+e.tv_usec;

		printf("1111recv data frameType=%d,len=%d,time=%llu,curTime=%llu\n",frame.frameType,frame.frameLength,frame.frameTime,curTime);
#if WRITE_FILE
		if(frame.frameType != 0x09 && h264File)
		{
			fwrite(frame.frameBuffer,1,frame.frameLength,h264File);
			fflush(h264File);
		}
		else if(aacFile)
		{
			fwrite(frame.frameBuffer,1,frame.frameLength,aacFile);
			fflush(aacFile);
		}
#endif
	}
#if WRITE_FILE
	if(h264File)
		fclose(h264File);
	if(aacFile)
		fclose(aacFile);
#endif
}


void popMessage(void* arg,const char* msg)
{
	printf("lingyang cloud popMessage=%s\n",msg);
}



int main(int argc,char** argv)
{
	int ret = 0;
	char* token = "2147550094_3222536192_1493481600_601ef4914c7d6fc322f9709ef5f3d0b4";//测试token
	char* configStr = "[Config]\r\nIsDebug=0\r\nLocalBasePort=8200\r\nIsCaptureDev=1\r\nIsPlayDev=1\r\nU \
					  dpSendInterval=2\r\nConnectTimeout=10000\r\nTransferTimeout=10000\r\n[Tracker]\r\nCount=3\r\nIP1=121.42.156.148\r\nPort1=80 \
					  \r\nIP2=182.254.149.39\r\nPort2=80\r\nIP3=203.195.157.248\r\nPort3=80\r\n[LogServer]\r\nCount=1\r\nIP1=120.26.74.53\r\nPort1=80\r\n";
	//此示例适用于qstp协议
	//推流地址connectType=2表示拉流，protocolType=2表示使用qstp协议
	char* urlQstp = "topvdn://183.57.151.139:1935?protocolType=2&connectType=2&token=1003791_3222536192_1493481600_b118f1066b417b6751a4804253d6d236";

	while(1)
	{
		ret = LY_startCloudService(token,configStr,popMessage,NULL);
		if(ret != 0)
		{
			usleep(20*1000);
		}
		break;
	}

	fd = LY_connect(urlQstp,NULL);
	if(fd < 0)
	{
		printf("connect to peer failed,ret=%d\n",ret);
		return 0;
	}
	printf("connect  success\n");
	if(pidRecv == NULL)
	{
		if(pthread_create(&pidRecv,NULL,recvProc,NULL) != 0)
		{
			printf("create push video thread failed\n");
		}
	}

	while(1)
	{
		sleep(1);
	}
}
