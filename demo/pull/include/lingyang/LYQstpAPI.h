#ifndef __LY_QSTP_API_H__
#define __LY_QSTP_API_H__

/*******************************************************************************************************
*												私有QSTP(quick streaming tcp  protocol)协议模块
*
*		该模块主要提供录像云存储以及直播服务（录像存储，公众直播，公众直播带云存储，私有直播）   
*			
*******************************************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

    #include "common_define.h"

    

	/***********************************************************************
	*    函数描述：QSTP数据回调处理函数
	*    输入参数：    
	*               	apUserData         用户传入到回调函数的参数，没有时可以传NULL进去
	*					aFd                数据流传输通道操作句柄（fd）
	*					aDataType          数据类型
	*                	aDataBuffer        数据指针
	*                	aDataLength        数据长度
	*    输出参数：    
	*									无
	*????返 回 值：????
	*									无
	*
	*????函数说明：回调函数定义，函数名作为参数传入，由SDK回调处理；数据回调函数只用于推流端，拉流端不需要该回调。
	***********************************************************************/
	typedef void (*QSTPMessageCallBack)(const void* const apUserData, const char *aMessage);
		
	
	
	/***********************************************************************
	*    函数描述： 连接到转发服务器 
	*    输入参数：    
	*								aUrl:topvdn://relay_ip:relay_port?protocolType=%d&connectType=%d&token=%s&cid=%lu;
    *								    server_ip:服务器ip; port:服务器端口; protocolType:传输协议（1-QSUP;2-QSTP）
    *								    ; connectType:连接类型（1-推流端，取数据; 2-拉流端）;token:token串; cid:cid号;
	*    输出参数：    
	*								无;   
	*    返 回 值：   
	*		 						=0 表示连接成功,返回连接通道句柄(fd)；
	*								<0 表示连接失败 
	*                
	*    函数说明：  连接到转发服务器，开启转发服务。使用前需要调用PlatformAPI
	*		  					中的LY_connectToServer方法连接到服务器，请求到私有协议转发地
	*                     址，转化成Url，再调用此方法来开启服务。
	***********************************************************************/
	int 	LY_connectToRelayServer(const char *aUrl);
	
	
	
	/***********************************************************************
	*    函数描述：  停止转发服务，断开连接 
	*    输入参数：    
	*									aFd 数据传输通道句柄
	*    输出参数：    
	*									无    
	*  	返 回 值：    
	*									无
	*                
	*    函数说明: 与LY_connectToRelayServer配对使用。            
	***********************************************************************/
	void		LY_disconnectRelayServer(const int aFd);


    
	/***********************************************************************
	*    函数描述：  停止转发服务，断开连接 
	*    输入参数：    
	*               aQSTPMessageCallBack    回调函数指针
	*               apData                回调函数的传入参数
	*  	返 回 值：    
	*									无
	*                
	*    函数说明: 函数只用于推流端，拉流端不需要设置回调函数。            
	***********************************************************************/
    void LY_setQSTPCallBack(const QSTPMessageCallBack aQstpMassageCallBack, const void *apUserData);



	/***********************************************************************
	*    函数描述：发送媒体数据
	*    输入参数：   
	*									aFd 数据传输通道句柄;	
	*                	apMediaFrame        待发送帧数据结构体            
	*    输出参数：    
	*									无
	*    返 回 值：   
	*		 							=0 表示推数据成功；
	*									<0 其它返回值都表示出现错误
	*
	*    函数说明：发送媒体数据,通过QSTP私有的协议
	*		 转发。            
	*                
	***********************************************************************/
	int	    LY_sendQSTPMediaFrame(const int aFd, MediaFrame_t *apMediaFrame);
	
	
	
	/***********************************************************************
	*    函数描述:  接收媒体数据
	*    输入参数：    
	*									aFd 数据传输通道句柄;	           
	*    输出参数：    
	*									apMediaFrame        接收帧数据结构体（结构体定义见common_define.h）
	*    返 回 值：   
	*		 							=0 表示接收数据成功；
	*									<0 其它返回值都表示出现错误
	*                
	*    函数说明：    其中参数aFd表示传输通道Handler,
	***********************************************************************/
	int		LY_recvQSTPMediaFrame(const int aFd, MediaFrame_t * apMediaFrame);
	
	
	
	
	
	/***********************************************************************
	*    函数描述：设置QSTP音视频流信息
	*    输入参数：    
	*									aFd 数据传输通道句柄;	
	*									aMediaInfo 	流媒体数据信息（结构体定义见common_define.h）	
	*    输出参数：    
	*									无    
	*    返 回 值：   
	*		 							=0 表示连接成功；
	*									<0 表示连接失败 
	*                
	*    函数说明：               
	***********************************************************************/
	int 	LY_setQSTPMediaInfo(const int aFd, const MediaInfo_t aMediaInfo);
	
	
	/***********************************************************************
	*    函数描述：设置QSTP私有协议加密密钥 
	*    输入参数：    
	*					aEncryptType            加密类型（1:enable encrypt; 0:disable encrypt）
	*                	aEncryptKeyValue        加密密钥内容指针,自定义密钥
	*                	aEncryptKeyLength        加密密钥内容长度
	*    输出参数：    
	*									无    
	*    返 回 值：   
	*		 							=0 表示连接成功；
	*									<0 表示连接失败 
	*                
	*    函数说明：	设置QSTP私有协议加密密钥，
	*								若不进行设置则使用SDK内部默认密钥。            
	***********************************************************************/
	int		LY_setQSTPEncryptKey(const unsigned int aEncryptType, const char * const aEncryptKeyValue, const unsigned int aEncryptKeyLength);
	
	
	
	/***********************************************************************
	*    函数描述：设置缓冲时长
	*    输入参数：   
	*									aFd 数据传输通道句柄;	
	*									aBufferTime      缓冲时长，单位为秒
	*    输出参数：    
	*									无    
	*    返 回 值：    
	*									无
	*                
	*    函数说明：    设置内部重连时缓存数据时长，以当前重连成功时间为准，若有缓
	*		   			存旧数据则丢弃所设定时长之前的数据，以秒为单位（param: seconds）;
    *		   			函数只用于推流端，拉流端不需要设置。            
	***********************************************************************/
	void 	LY_setBufferTime(const int aFd, const int aBufferTime);

    int LY_setQSTPLocalIP(const char *ip1, const char *ip2);

    void LY_setQSTPWorkingMode(int aFd, int aMode);
    
    unsigned long LY_getQSTPSocketSendCache(int aFd);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* End of #ifndef __LY_QSTP_API_H__ */
