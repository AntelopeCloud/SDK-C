#ifndef __LY_PLATFORM_API_H
#define __LY_PLATFORM_API_H

#ifdef __cplusplus
extern "C" {
#endif

    #include "common_define.h"

	/***********************************************************************
	*    函数描述: 启动云服务
	*    输入参数：    
	*									apToken							登录token值（token格式见该模块附3）
	*                	aConfig 						配置字符串
	*                	apMessageCallBack		平台消息回调函数，该函数用来处理服务
	*																				器返回的消息(定义见该模块附1)
	*                	apUserData            作为参数传入apMessageCallBack函数
	*    输出参数：     
	*									无
	*    返 回 值：    
	*									=0 表示成功
	*									<0 表示失败
	*
	*    函数说明: 验证身份，开始云服务，与LY_stopCloudService配套使用。            
	*
	***********************************************************************/
    int  LY_startCloudService(const char* aToken, const char* aConfig, PlatformMessageCallBack apMessageCallBack, void *apUserData);

	/***********************************************************************
	*    函数描述: 停止云服务
	*    输入参数：    
	*								无
	*    输出参数：    
	*								无
	*    返 回 值：   
	*		 						无
	*
	*    函数说明: 停止云服务，即停止与服务器端的通信，与LY_startCloudService
	*		 配套使用               
	***********************************************************************/
    void LY_stopCloudService();

	/***********************************************************************
	*    函数描述: 获取在线状态
	*    输入参数:    
	*									无
	*    输出参数:   
	*									无
	*    返 回 值:    
	*		 							在线状态（0：离线；1：在线）
	*
	*    函数说明: 获取设备在线状态（由于网络传输存在延迟，可能出现返回状态与
	*		 实际状态不同步的现象。）                
	***********************************************************************/
    int LY_getOnlineStatus();

	/***********************************************************************
	*    函数描述:  更新token信息
	*    输入参数：    
	*								        aToken token串;	 
    *								        aTokenSize token长度        
	*    输出参数：    
	*										无
	*    返 回 值：   
	*		 								=0 表示接收数据成功；
	*										<0 其它返回值都表示出现错误
	*                
	*    函数说明:.
	***********************************************************************/
    int LY_updateToken(const char *aToken, const int aTokenSize);

	/***********************************************************************
	*    函数描述: 建立连接 
	*    输入参数：    
	*										aUrl    
    *										aDataSourceInfo  请求录像下载信息,录像下载使用
	*    输出参数：    
	*										无
	*    返 回 值：    
	*										>=0 表示连接成功,返回fd数据传输通道句柄； 
	*										<0 表示出现错误
	*
	*    函数说明：   该函数与 LY_disconnect配套使用。           
	***********************************************************************/
    int LY_connect(const char *aUrl, const char *aDataSourceInfo);

	/***********************************************************************
	*    函数描述: 断开当前通道（fd）的连接 
	*    输入参数：    
	*										aFd 数据传输通道句柄;
	*    输出参数：    
	*										无    
	*    返回值：    
	*										无
	*                
	*    函数说明：断开当前的连接。 该函数与 LY_connect配套使用。           
	***********************************************************************/
    void LY_disconnect(const int aFd);

	/***********************************************************************
	*    函数描述: 发送媒体帧数据
	*    输入参数：   
	*		 			aFd 数据传输通道句柄;	
	*                	apMediaFrame        待发送帧数据结构体（结构体定义见common_define.h）
	*    输出参数:
	*									无
	*    返 回 值：    
	*									=0 表示发送数据成功；
	*									<0 其它返回值都表示出现错误
	*
	*    函数说明：其中参数aFd表示传输通道.
	***********************************************************************/
    int LY_sendMediaFrame(const int aFd, MediaFrame_t *apMediaFrame);

	/***********************************************************************
	*    函数描述:  接收媒体数据
	*    输入参数：    
	*										aFd 数据传输通道句柄;	         
	*    输出参数：    
	*										apMediaFrame        接收帧数据结构体
	*    返 回 值：   
	*		 								=0 表示接收数据成功；
	*										<0 其它返回值都表示出现错误
	*                
	*    函数说明：其中参数aFd表示传输通道。输入参数apMediaFrame由外部管理内存空间,
    *               需要输入有效的内存空间地址存放接收数据.
	***********************************************************************/  
    int LY_recvMediaFrame(const int aFd, MediaFrame_t *apMediaFrame);

	/***********************************************************************
	*    函数描述: 跳转到指定录像时间点获取录像数据 
	*    输入参数：    
	*										aFd 数据传输通道句柄;	      
    *										aCurrentTime 跳转到数据的时间点    
	*    输出参数：    
	*										
	*    返 回 值：   
	*		 								=0 表示接收数据成功；
	*										<0 其它返回值都表示出现错误
	*                
	*    函数说明：其中参数aFd表示传输通道。
    *           指定调转的时间点范围在打开该录像通道的开始时间和结束时间内
	***********************************************************************/  
    int  LY_seek(const int aFd, const unsigned int aCurrentTime);

    
    //extern API
    int LY_setLocalIP(const char *aFirstIP, const int aFirstIpLength, const char *aSecondIP, const int aSecondIpLength); //use for qstp 

    int LY_setMediaInfo(const int aFd, const MediaInfo_t aMediaInfo);   //use for qstp

    int LY_setEncryptKey(unsigned int aEncryptType, char *aEncryptKeyValue, int aEncryptKeyLength);


#ifdef __cplusplus
}
#endif

#endif
