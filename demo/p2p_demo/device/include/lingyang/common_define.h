#ifndef __COMMON_DEFINE__
#define __COMMON_DEFINE__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define MAX_STREAM_NUM  10

#define LY_SUCCESS 0
#define LY_FAILED -1

	typedef struct ServerInfo
	{
		unsigned char server_ip[32];
		unsigned int  server_port;
		unsigned char extradata[256];
	}ServerInfo_t;

    /* 数据帧结构体 */
    typedef struct
    {
        unsigned long capacitySize;      //input buffer size
        unsigned char frameType;         /*数据帧类型*/
        unsigned char *frameBuffer;      /*数据帧地址*/
        unsigned long frameLength;       /*数据帧长度*/
        unsigned long long frameTime;    /*数据帧时间戳*/
    }MediaFrame_t;
    
    /*流媒体信息结构体（包含音视频数据的编解码信息）*/
    typedef struct 
    {
          double  duration;
	      double  width;
	      double  height;
	      double  videodatarate;
	      double  framerate;
	      double  videocodecid;
	      double  audiodatarate;
	      double  audiosamplerate;
	      double  audiosamplesize;
	      double  stereo;                         //0 for false
	      double  audiocodecid;
	}MediaInfo_t;

	/*数据帧类型*/
	typedef enum {
	    NALU_TYPE_SLICE = 1,
	    NALU_TYPE_DPA = 2,
	    NALU_TYPE_DPB = 3,
	    NALU_TYPE_DPC = 4,
	    NALU_TYPE_IDR = 5,
	    NALU_TYPE_SEI = 6,
	    NALU_TYPE_SPS = 7,
	    NALU_TYPE_PPS = 8,
	    NALU_TYPE_AUD = 9,
	    NALU_TYPE_EOSEQ = 10,
	    NALU_TYPE_EOSTREAM = 11,
	    NALU_TYPE_FILL = 12,
	    NALU_TYPE_PREFIX = 14,
	    NALU_TYPE_SUB_SPS = 15,
	    NALU_TYPE_SLC_EXT = 20,
	    NALU_TYPE_VDRD = 24,
	    OPUS_TYPE_SAMPLE = 127,
	    AAC_TYPE_INFO = 128,
	    AAC_TYPE_SAMPLE = 129
	} FrameSampleType_Em;

	/********* 连接模式枚举类型定义 *********/
	typedef enum
	{
		CONNECT_MODE_QSUP_STANDBY               = 0,					/*QSUP待命状态*/
		CONNECT_MODE_QSUP_BROADCAST		        = 1,					/*QSUP广播*/
		CONNECT_MODE_QSTP_BROADCAST		        = 2,					/*QSTP广播（公众直播）*/
		CONNECT_MODE_QSTP_RECORD                = 3,					/*QSTP私有存储（录像）*/
		CONNECT_MODE_QSTP_REOCRD_BROADCAST	    = 4,					/*QSTP广播+私有存储（公众+录像）*/
		CONNECT_MODE_QSTP_STANDBY               = 5						/*QSTP私有直播*/
	}ConnectMode_Em;															
	
    typedef enum
    {
        QSTP_CLIENT_TYPE_PUSHING = 1,  /*推流端，上传音视频数据源*/
        QSTP_CLIENT_TYPE_PULLING = 2   /*拉流端，读取音视频数据源*/
    }ConnectType_Em;

	/***********************************************************************
	*    函数描述：    平台消息回调处理函数
	*    输入参数：    
	*										aMessage    消息体内容
	*                   apUserData        用户传入到回调函数的参数，没有时可以传NULL进去
	*    输出参数：    
	*										无
	*    返 回 值：    
	*										无
	*
	*    函数说明：    回调函数定义，函数名作为参数传入，由SDK回调该函数处理相应的message消息
	***********************************************************************/
	typedef void (*PlatformMessageCallBack)(void* apUserData, const char* const aMessage);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // __COMMON_DEFINE__
