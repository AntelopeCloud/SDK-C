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

//相关宏在Makefile中定义
#include "transportMedia.h"

//头文件包含
#ifdef QSTP
#include "LYQstpAPI.h"
#else
#include "cJSON.h"
#include "LYPlatformAPI.h"
#include "lyUtils.h"
#endif

char* videoName = NULL;
char* audioName = NULL;


#ifdef LY_PLATFORM
/*********************平台相关**************************/

typedef struct _lyWorkFlag
{
	int working;
	int waitMessage;
	int msgid;
	pthread_t pidWork;
	pthread_t pidMsgProc;
}lyWorkFlag;
lyWorkFlag workFlag = {1,1,-1,NULL,NULL};

typedef struct _tranportConfig 
{
	int publicLiveFlag;//直播开关，device设置为直播则为1，否则为0
	int isRecording;//云存储开关，device打开云存储为1，否则为0
	int qstpOpenFlag;//qstp开关，打开云存储或者直播为1，否则为0
}tranportConfig;

typedef struct _config
{
	int type;//设备端配置类型。0:qsup协议，2:qstp协议直播，3:云存储，4:qstp直播带云存储,//5:qstp协议非公众不带云存储
	int audioFlag;
	int bitrate;//传输码率。1:1M,2:500K,3:300K。
	int typeChanged;//标识设备端设置是否改变，在手机端改变配置后，会在popMessage回调函数中设置为1；
	int fds[MAX_CHANNEL];
	int curWorkChannelCount;
	int pushing;
	tranportConfig trans_config;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}deviceConfig;
deviceConfig endPointConfig;
/*********************平台相关—end**************************/
//初始化相关参数
static int deviceConfigInit(deviceConfig* config)
{
	int i = 0;
	config->type = 0;
	config->audioFlag = 0;
	config->bitrate = 0;
	config->trans_config.publicLiveFlag = 0;
	config->trans_config.isRecording = 0;
	config->trans_config.qstpOpenFlag = 0;
	for(;i<MAX_CHANNEL;i++)
		config->fds[i] = -1;
	config->curWorkChannelCount = 0;
	config->typeChanged = 0;
	config->pushing = 0;
	pthread_cond_init(&config->cond,NULL);
	pthread_mutex_init(&config->mutex,NULL);
}

//手机端与设备端的连接建立
int connectEstablished()
{
	int fd = -1;
	if(endPointConfig.curWorkChannelCount <= MAX_CHANNEL)
	{
		if(!endPointConfig.pushing)
		{
			endPointConfig.fds[0] = 0;//QSUP协议使用默认使用0
			if(startPushMedia(endPointConfig.fds[0],endPointConfig.audioFlag)==0)
				endPointConfig.pushing = 1;
		}
		return 0;
	}
	else
	{
		printf("already start max channel count,can't start a channel any more\n");
		return -1;
	}
}

//手机端与设备端的连接关闭。
void connectClose()
{
	if(endPointConfig.pushing == 1 && !endPointConfig.trans_config.qstpOpenFlag)
	{
		stopPushMedia(endPointConfig.fds[0]);
		endPointConfig.pushing = 0;
	}
}


int startLyCloudService(char* token,char* configStr)
{
	int ret = -1;
	if(!token || !configStr)
	{
		printf("token or configstring is NULL\n");
		return -1;
	}

	if(workFlag.msgid == -1)//防止多次调用
	{
		deviceConfigInit(&endPointConfig);//初始化配置相关结构体

		//创建消息队列处理以平台发过来的消息
		if((workFlag.msgid = createMsgId('h')) != -1)
		{
			if(pthread_create(&workFlag.pidMsgProc,NULL,msgProc,NULL) != 0)
			{
				//失败则不能响应平台的消息，应该退出程序
				printf("create msgProc thread failed\n");
				return -1;
			}
		}

		//创建工作模式更改处理线程
		ret = pthread_create(&workFlag.pidWork,NULL,configChangeProc,NULL);
		if(ret != 0)
		{
			printf("start configChangeProc thread failed\n");//创建线程失败将不能响应工作模式改变处理
		}

		while(1)
		{
			//开启云平台服务，并注册消息回调函数,调用此函数之后才能调用云平台的其他函数，第四个参数为对接厂商自定义数据，没有则不填
			ret = LY_startCloudService(token,configStr,popMessage,NULL);
			if(ret == 0)//返回0表示开启成功
			{
				break;
			} 
			printf("StartCloudService failed!  ret = %d  retry...\n",ret );
			sleep(1); 
		}
		printf("start lingyang cloud service success\n");
	}
	return 0;
}


void stopLyCloudService()
{
	printf("clean app\n");
	myMsg tmp = {100,"cancel"};
	workFlag.waitMessage = 0;
	if(msgsnd(workFlag.msgid,&tmp,sizeof(myMsg),0) != 0)//使用非阻塞方式发送消息
	{
		perror("send cancel msg failed");
	}
	pthread_join(workFlag.pidMsgProc,NULL);
	printf("join msgProc success\n");
	workFlag.working = 0;
	endPointConfig.typeChanged = 1;
	pthread_cond_broadcast(&endPointConfig.cond);
	pthread_join(workFlag.pidWork,NULL);
	printf("join configChanged success\n");
	if(msgctl(workFlag.msgid,IPC_RMID,0))//删除内核的消息队列
	{
		printf("remove msg_que failed\n");
	}
	//step4.调用此接口关闭云平台，至此，云平台其他接口调用失效。
	LY_stopCloudService();
}

#endif

static void signalproc(int signo)
{
	printf("signal num:%d\n",signo);
}


int main(int argc,char** argv)
{
	int ret;
	int fd = -1;
	signal(SIGPIPE,signalproc);//忽略SIGPIPE信号，必须要
#ifdef QSTP //===========================只包含QSTP部分=============================================================
	int isPushAudio = 0;
	//第三方友商首先需要向后台服务器获取token和推流服务器IP、端口，然后组成连接url；若需要指定推流服务器，则在url ip和端口字段指定推流服务器ip和端口
	//使用羚羊调度的推流服务器推流url示例：
	char* url = "topvdn://183.57.151.111:1935?protocolType=2&connectType=1&token=1003791_3222536192_1493481600_b118f1066b417b6751a4804253d6d236&mode=2";

	if(argc < 2)
	{
		printf("usage:./%s [videoName] [audioName]\n",argv[0]);
		return 0;
	}
	
	if(argc > 2)
	{
		isPushAudio = 1;
	}
	while(1)
	{
		fd = LY_connectToRelayServer(url);
		if(fd < 0)
		{
			printf("connect to lingyang cloud relay server failed,retry...\n");
			usleep(300*1000);
			continue;
		}
		break;
	}
	
	videoName = argv[1];
	if(argc > 2)
	{
		audioName = argv[2];
	}

	if(startPushMedia(fd,isPushAudio) != 0)
	{
		printf("start push failed\n");
		return -1;
	}
	while(1)
	{
		sleep(3);
	}
//======================================包含云平台====================================================//
#elif defined(LY_PLATFORM)
	if(argc < 2)
	{
		printf("Usage:%s <videoFileName> [audioFileName]\n",argv[0]);
		return -1;
	}
	videoName = argv[1];
	if(argc > 2)
		audioName = argv[2];
	//step1.向后台获取token和configString,第三方对接厂商，这里按照自己业务逻辑实现。
	//羚羊示例：
	char* deviceToken = "1003791_3222536192_1493481600_b118f1066b417b6751a4804253d6d236";
	char* configStr = "[Config]\r\nIsDebug=0\r\nLocalBasePort=8200\r\nIsCaptureDev=1\r\nIsPlayDev=1\r\nU \
					  dpSendInterval=2\r\nConnectTimeout=10000\r\nTransferTimeout=10000\r\n[Tracker]\r\nCount=3\r\nIP1=121.42.156.148\r\nPort1=80 \
					  \r\nIP2=182.254.149.39\r\nPort2=80\r\nIP3=203.195.157.248\r\nPort3=80\r\n[LogServer]\r\nCount=1\r\nIP1=120.26.74.53\r\nPort1=80\r\n";
	
	//step2.启动羚羊云服务
	ret = startLyCloudService(deviceToken,configStr);
	if(ret != 0)
	{
		printf("!!!start ly cloud service failed\n");
		return 0;
	}
	
	//step3.根据羚羊云的回调消息实现自己的业务逻辑
#if 1
	//这里是因为没有和后台服务器交互，模拟后台服务器的消息
	sleep(3);
	pthread_mutex_lock(&endPointConfig.mutex);
	endPointConfig.audioFlag = 1;//音频开关
	endPointConfig.type = 2;//工作模式
	endPointConfig.trans_config.qstpOpenFlag = 1;
	endPointConfig.typeChanged = 1;
	pthread_cond_broadcast(&endPointConfig.cond);
	pthread_mutex_unlock(&endPointConfig.mutex);
	sleep(3);
	//startPushAudio(endPointConfig.fds[0]);
	sleep(10);
	printf("main stop push audio\n");
	//stopPushAudio(endPointConfig.fds[0])
#endif
	while(1)
	{
		//keep alive
		sleep(1);
	}
	//step4.停止羚羊云服务
	stopLyCloudService();
#endif
	printf("app exit\n");
	return 0;
}
