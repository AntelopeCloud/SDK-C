#include <pthread.h>
#include "LYQstpAPI.h"
#include "transportMedia.h"
#include "pushFile.h"


#ifdef __cplusplus
extern "C"{
#endif

	//此文件中实现一个应用羚羊云C接口SDK的多通道传输数据的框架，便于第三方友商快速对接羚羊云，第三方友商可根据自己的需求进行修改。
	//一般第三方友商只需按照自己的逻辑推视频流函数，推音频流函数，播放音频函数这三个线程执行函数即可快速借助羚羊云传输音视频数据。
#ifndef QSTP_BASE_FD_VALUE
#define QSTP_BASE_FD_VALUE 200 //QSTP协议使用的fd为200开始的
#endif
#define RECV_AUDIO_BUF_SIZE 128*1024 //接收音频数据缓存区大小
static transportChannel channels[MAX_CHANNEL] = {0};//传输通道数组
static curWorkChannelCount = 0;//记录当前使用通道数
typedef void (*threadProc)(void* arg);//线程函数指针

transportChannel* getChannel(int fd)
{
	int i = 0;
	for(;i<MAX_CHANNEL;i++)
	{
		if(channels[i].fd == fd)
		{
			return &channels[i];
		}
	}
	return NULL;
}

//实现开始推流逻辑
int startPushMedia(int fd,int isPushAudio)
{
	if(fd < 0)
	{
		printf("fd must >= 0\n");
		return -1;
	}
	if(curWorkChannelCount >= 10)
	{
		printf("already max channel\n");
		return -1;
	}
	transportChannel* ch = &channels[curWorkChannelCount];
	channels[curWorkChannelCount++].fd = fd;
	if(!ch->pushVideoing)
	{
		ch->fd = fd;
		ch->isAudioOpen = isPushAudio;
		threadProc video,audio;

		video = pushVideoFlowProc; //此处替换自己的推流线程函数
		audio = pushAudioFlowProc; //此处替换自己的推流线程函数

		ch->pushVideoing = 1;
		if(pthread_create(&ch->pidPushVideo,NULL,video,&ch->fd) != 0)
		{
			printf("\033[31mstart audio thread failed\n\033[0m");
			return -1;
		}
		if(isPushAudio)
		{
			ch->pushAudioing = 1;
			if(pthread_create(&ch->pidPushAudio,NULL,audio,&ch->fd) != 0)
			{
				printf("\033[31mstart audio thread failed\n\033[0m");
				return -1;
			}
		}

	}
	return 0;
}


//实现停止推流逻辑(停止推流后会不能继续接受数据)
void stopPushMedia(int fd)
{
	if(fd < 0)
	{
		printf("fd must >= 0\n");
		return -1;
	}
	transportChannel* ch = getChannel(fd);
	if(NULL != ch)
	{
		if(ch->pushVideoing == 1)
		{
			if(ch->pidPushVideo != NULL)
			{
				ch->pushVideoing = 0;
				pthread_join(ch->pidPushVideo,NULL);
				ch->pidPushVideo = NULL;
			}
			if(ch->pushAudioing != NULL)
			{
				ch->pushAudioing = 0;
				pthread_join(ch->pidPushAudio,NULL);
				ch->pidPushAudio = NULL;
			}
			ch->fd = -1;
			ch->isAudioOpen = 0;
		}
		curWorkChannelCount--;
	}
}

//开启推音频流，这个函数是设备刚开始只推视频，想推音频流的时候，通过app配置打开音频开关，然后开始推音频流
int startPushAudio(int fd)
{
	if(fd < 0)
	{
		printf("fd must >= 0\n");
		return -1;
	}
	transportChannel* ch = getChannel(fd);
	if(NULL != ch)
	{
		if(ch->pushVideoing && ch->pushAudioing == 0)
		{
			ch->isAudioOpen = 1;
			threadProc audio;
			audio = pushAudioFlowProc; //此处替换自己的推流线程函数
			ch->pushAudioing = 1;
			if(pthread_create(&ch->pidPushAudio,NULL,audio,&ch->fd) != 0)
			{
				printf("\033[31mstart audio thread failed\n\033[0m");
				return -1;
			}
		}
	}
	return 0;
}

//停止推音频流，一般通过APP端的音频开关配置，可以关闭音频流的推送
void stopPushAudio(int fd)
{
	if(fd < 0)
	{
		printf("fd must >= 0\n");
		return -1;
	}
	transportChannel* ch = getChannel(fd);
	if(NULL != ch)
	{
		if(ch->pushAudioing != NULL || ch->pushAudioing == 1)
		{
			ch->pushAudioing = 0;
			pthread_join(ch->pidPushAudio,NULL);
			ch->pidPushAudio = NULL;
			ch->isAudioOpen = 0;
		}
	}
}

//接收羚羊云回传的数据
static void* recvDataFromLyCloud(void* arg)
{
	if(arg == NULL)
		return;
	int fd = *(int*)arg;
	int ret = 0;
	char* buf;
	MediaFrame_t frame = {0};
	transportChannel* ch = getChannel(fd);
	if(NULL == ch)
	{
		printf("get channel failed by fd=%d\n",fd);
		return;
	}
	ch->recvAudioing = 1;
	frame.frameBuffer = (char*)malloc(RECV_AUDIO_BUF_SIZE);
	if(frame.frameBuffer == NULL)
	{
		printf("malloc frame buffer failed by fd=%d\n",fd);
		return;
	}
	frame.capacitySize = RECV_AUDIO_BUF_SIZE;//必要设置，否则无法接收数据
	while(ch->recvAudioing)
	{
#ifdef QSTP
		ret = LY_recvQSTPMediaFrame(fd,&frame);
#elif defined(LY_PLATFORM)
		ret = LY_recvMediaFrame(fd,&frame);
#endif
		if(ret != 0)//如果返回-1，则表示已经调用了LY_disconnect获取发生错误，退出线程
		{
			printf("LY_recvMediaFrame failed\n");
			ch->recvAudioing = 0;
			break;
		}
#ifdef PLAYBACK//如果有回放，在这里将数据压到播放队列中
		if(ret == 0 && pushFrameToQue(ch->playBackBufHandle,frame.frameBuffer,frame.frameLength,frame.frameTime,AAC_TYPE_SAMPLE) != 0)
		{
			printf("push audio frame to playback failed\n");
		}
#else
		printf("recv audio frame,len=%d\n",frame.frameLength);
#endif
	}
#ifdef PLAYBACK
	//清理工作
	if(ch->pidPlayBacking != NULL || ch->playBacking)
	{
		ch->playBacking = 0;
		cancelQue(ch->playBackBufHandle);//需要先取消队列，否则会阻塞住
		pthread_join(ch->pidPlayBacking,NULL);
		ch->pidPlayBacking = NULL;
	}
	destroyQue(ch->playBackBufHandle);
	ch->playBackBufHandle = NULL;
#endif
	printf("recv audio data exit fd=%d\n",fd);
}

int startRecvAudio(int fd)
{
	int ret = 0;
	if(fd < 0)
	{
		printf("fd must >= 0\n");
		return -1;
	}
	transportChannel* ch = getChannel(fd);
	if(NULL != ch)
	{
		if(ch->pidRecvAudioing == NULL)
		{
			if(pthread_create(&ch->pidRecvAudioing,NULL,recvDataFromLyCloud,&ch->fd) != 0)
			{
				printf("\033[31mstart recv data thread failed\n\033[0m");
				return -1;
			}
		}
#ifdef PLAYBACK//如果需要回放
		if(ch->playBackBufHandle == NULL)
		{
			//创建接收音频数据的队列，（此队列已由羚羊云SDK实现，第三方友商直接使用即可）
			ch->playBackBufHandle = createQue("playBack",NULL,RECV_AUDIO_BUF_SIZE);
			if(ch->playBackBufHandle != NULL)
			{
				threadProc playBackProc = playBackAudioProc;//此处替换自己的回放线程函数
				ch->playBacking = 1;
				//创建播放数据的线程
				if(pthread_create(&ch->pidPlayBacking,NULL,playBackProc,&ch->fd) != 0)
				{
					printf("create play back thread failed\n");
					ret = -1;
				}
			}
		}
#endif
	}
	return ret;
}

#ifdef __cplusplus
}
#endif
