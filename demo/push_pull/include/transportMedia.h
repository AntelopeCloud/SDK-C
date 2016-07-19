#ifndef __TRANSPORT_MEDIA_H__
#define __TRANSPORT_MEDIA_H__

#ifdef __cplusplus
extern "C"{
#endif

#ifndef QSTP_BASE_FD_VALUE
#define QSTP_BASE_FD_VALUE 200 //QSTP协议使用的fd为200开始的
#endif
#define RECV_AUDIO_BUF_SIZE 128*1024 //接收音频数据缓存区大小

#define MAX_CHANNEL 10
typedef struct _transportChannel
{
	int fd;//对应传输通道的fd
	int isAudioOpen;
	int pushVideoing;//推视频流标志位
	int pushAudioing;//推音频流标志位
	int recvAudioing;//接收对讲音频标志位
	int playBacking;//播放音频线程标志位
	pthread_t pidPushVideo;//推视频流线程id
	pthread_t pidPushAudio;//推音频流线程id
	pthread_t pidRecvAudioing;//接收对讲音频流线程id
	pthread_t pidPlayBacking;//播放音频流线程id
	void*	playBackBufHandle;//播放音频缓存队列句柄
}transportChannel;

transportChannel* getChannel(int fd);

//开始一个通道的推流
int startPushMedia(int fd,int isPushAudio);

//停止一个通道的推流，停止推流会关闭通道，相应的接受数据部分也会关闭
void stopPushMedia(int fd);

//开启推音频流，一般是设备刚开始只推视频，然后通过app配置打开音频开关，然后开始推音频流
int startPushAudio(int fd);

//停止推音频流，一般通过APP端的音频开关配置，可以关闭音频流的推送
void stopPushAudio(int fd);

//开始接受平台发送过来的数据，包括音频、报警等
int startRecvAudio(int fd);



#ifdef __cplusplus
}
#endif

#endif