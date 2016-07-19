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
#include "lyUtils.h"
#include "aacUtil.h"
#include "h264Util.h"
#include "common_define.h"

pthread_t pidPushVideo = NULL;
pthread_t pidPushAudio = NULL;
pthread_t pidRecvData = NULL;
int fd = 0;//默认为0
int loopPush =1;
int pushVideoing = 1;
int pushAudioing = 1;
int recvDataing = 1;


void* pushVideoFlowProc(void* arg)
{
	char* videoName = "video.h264";
	int ret;
	unsigned long long time_gap = 66000;//66毫秒间隔，15帧
	unsigned long long last_time = 0;
	unsigned long long now = 0;
	unsigned long long timeStamp = 0;
	unsigned long long st = 0;
	int lastTimeCost = 0;
	int frameNum = 0;
	h264Helper helper;
	if(h264HelperInit(&helper,videoName) != 0)
	{
		printf("init h264 helper faield\n");
		return NULL;
	}
	printf("start to push video,videoName=%s,bufferSize=%d,fd=%d\n",videoName,helper.bufferSize,fd);
	timeStamp = getMSec();
	MediaFrame_t frame = {0};
	while (pushVideoing)
	{
		if(helper.sendDoneFlag == 0)
		{
			now = getMSec();
			if(getH264Frame(&helper) == 0)
			{
				timeStamp = now;
				frame.frameType = helper.naluType;//帧类型参考common_define.h
				frame.frameBuffer = (unsigned char*)helper.data;
				frame.frameLength = helper.frameSize;
				frame.frameTime = timeStamp/1000;//毫秒单位
				ret = LY_sendMediaFrame(fd,&frame);
				if(ret != 0)
					printf("send video failed fd=%d nal %d frame ret:%d,frameSize=%d\n",fd,helper.naluType,ret,helper.frameSize);
				else
					printf("send video success fd=%d nal %d frame ret:%d,frameSize=%d\n",fd,helper.naluType,ret,helper.frameSize);
			}
			last_time = getMSec();
			st = (last_time - now);
			if(st < time_gap)
				usleep(time_gap - st);//实时流出来一帧推一帧即可，无需sleep
		}
		else if(helper.readFileEof)
		{
			if(loopPush)//循环推录像
			{
				printf("push video file again\n");
				fseek(helper.fp,0,SEEK_SET);
				helper.frameSize = 0;
				helper.offset = 0;
				helper.readFileEof = 0;
				helper.sendDoneFlag = 0;
				helper.naluType = 0;
			}
			else
			{
				pushVideoing = 0;
			}
		}
	}
	h264HelperFree(&helper);
	printf("stop push video,thread exit\n");
}

void* pushAudioFlowProc(void* arg)
{
	char* audioName = "audio.aac";
	int ret;
	AACHelper aacHelper;
	char aacData[AAC_BUFFER] = {0};
	int aacLen = AAC_BUFFER;

	unsigned long long time_gap = 142000;
	unsigned long long last_time = 0;
	unsigned long long now = 0;
	unsigned long long st = 0;

	if(initAACHelper(&aacHelper,audioName) != 0)
	{
		printf("init aac helper failed\n");
		return NULL;
	}
	MediaFrame_t frame = {0};
	printf("start to push audio,audioName=%s,bufferSize=%d,fd=%d\n",audioName,aacHelper.buffer_size,fd);
	pushAudioing = 1;
	while (pushAudioing)
	{
		aacLen = AAC_BUFFER;
		ret = getAACFrame(&aacHelper,aacData,&aacLen);
		if(ret == 0)
		{
			now = getMSec();
			frame.frameBuffer = (unsigned char*)aacData;
			frame.frameLength = aacLen;
			frame.frameTime = now/1000;
			frame.frameType = AAC_TYPE_SAMPLE;//音频数据
			ret = LY_sendMediaFrame(fd,&frame);
			if(ret != 0)
				printf("send audio failed fd=%d ret:%d\n",fd,ret);
			else
				printf("send audio success fd=%d ret:%d\n",fd,ret);
			last_time = getMSec();
			st = (last_time - now);
			if(st < time_gap)
				usleep(time_gap - st);//实时流出来一帧推一帧即可，无需sleep
		}
		else if(aacHelper.readFileEof)
		{
			if(loopPush)
			{
				printf("push audio file again,ret=%d\n",ret);
				fseek(aacHelper.fp,0,SEEK_SET);
				aacHelper.offset = 0;
				aacHelper.readFileEof = 0;
			}
			else
			{
				pushAudioing = 0;
			}
		}
	}
	freeAACHelper(&aacHelper);
	printf("stop push audio\n");
	return NULL;
}

void* recvData(void* arg)
{
	int ret = -1;
	MediaFrame_t frame;
	frame.frameBuffer = (unsigned char*)malloc(512 * 1024);
	frame.capacitySize = 512 * 1024;//这个必要设置，否则无法拷贝数据
	int bits = 0;
	struct timeval s, e;
	unsigned long long curTime = 0;
	recvDataing = 1;
	while (recvDataing)
	{
		ret = LY_recvMediaFrame(fd, &frame);
		if (ret != 0)
		{
			printf("recv media frame ret=%d\n", ret);
			break;
		}
		printf("1111recv data frameType=%d,len=%d,time=%llu,curTime=%llu\n", frame.frameType, frame.frameLength, frame.frameTime, curTime);
	}
}
 

void popMessage(void* arg, const char* mess)
{
	if (!mess)
		return;
	cJSON* root = cJSON_Parse(mess);
	cJSON* name = cJSON_GetObjectItem(root, "name");
	cJSON* message;
	char buf[256] = { 0 };
	if (memcmp("ConnectionAcceptted", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage ConnectionAcceptted\n");
		if (pidPushVideo == NULL)
		{
			if (pthread_create(&pidPushVideo, NULL, pushVideoFlowProc, NULL) != 0)
			{
				printf("create push video thread failed\n");
			}
		}
		if (pidPushAudio == NULL)
		{
			if (pthread_create(&pidPushAudio, NULL, pushAudioFlowProc, NULL) != 0)
			{
				printf("create push audio thread failed\n");
			}
		}
	}
	else if (memcmp("ConnectionClosed", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage ConnectionClosed\n");
		if (pidPushVideo)
		{
			pushVideoing = 0;
			pthread_join(pidPushVideo, NULL);
		}
		if (pidPushAudio)
		{
			pushAudioing = 0;
			pthread_join(pidPushAudio, NULL);
		}
		if (pidRecvData)
		{
			recvDataing = 0;
			pthread_join(pidRecvData, NULL);
		}
	}
	else if (memcmp("StartPopData", name->valuestring, strlen(name->valuestring)) == 0)
	{
		printf("PopMessage StartPopData\n");
		if (pidRecvData == NULL)
		{
			if (pthread_create(&pidRecvData, NULL, recvData, NULL) != 0)
			{
				printf("create recv data thread failed\n");
			}
		}
	}
	else
	{
		printf("others msg\n");
	}
	if (root)
		cJSON_Delete(root);
	return;
}



int main(int argc,char** argv)
{
	int ret = 0;
	char* token = "1003791_3222536192_1493481600_b118f1066b417b6751a4804253d6d236";
	char* configStr = "[Config]\r\nIsDebug=0\r\nLocalBasePort=8200\r\nIsCaptureDev=1\r\nIsPlayDev=1\r\nU \
							dpSendInterval=2\r\nConnectTimeout=10000\r\nTransferTimeout=10000\r\n[Tracker]\r\nCount=3\r\nIP1=121.42.156.148\r\nPort1=80 \
							\r\nIP2=182.254.149.39\r\nPort2=80\r\nIP3=203.195.157.248\r\nPort3=80\r\n[LogServer]\r\nCount=1\r\nIP1=120.26.74.53\r\nPort1=80\r\n";

	while(1)
	{
		ret = LY_startCloudService(token,configStr,popMessage,NULL);
		if(ret != 0)
		{
			usleep(20*1000);
		}
		break;
	}
	printf("start cloud service success,wait for connected\n");
	while(1)
	{
		sleep(1);
	}
}
