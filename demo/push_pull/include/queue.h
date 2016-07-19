#ifndef __QUEQUE_H_
#define __QUEQUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define OUT //输出参数

//创建缓冲队列，传入参数为队列的大小，可处理数据大小为q_size - 4;
//创建成果返回消息队列句柄,失败返回NULL
void* createQue(char *q_buffer, unsigned long q_size);
//销毁队列，传入参数为队列句柄，销毁成功返回0，失败返回-1
int destroyQue(void* handle);
//清除队列中的数据，传入参数为队列句柄，成功返回0，失败返回-1
int clearQue(void* handle);

void cancelQue(void* handle);
//往队列压入数据，参数为队列句柄，压入数据的地址，数据长度
int pushToQue(void* handle,char* data,unsigned long len);
//从消息队列中取出数据，参数为队列句柄，接收队列的地址（外部管理内存），最大接收数据长度（传入为最大buffer长度，返回为实际长度），是否阻塞
int popFromQue(void* handle,OUT char* data,OUT unsigned long* len,int isBlock);

int pushFrameToQue(void* handle,char* data,unsigned long dataLen,unsigned long long timeStamp,char framType);

int popFrameFromQue(void* handle,OUT char* data,OUT unsigned long* dataLen,OUT unsigned long long* timeStamp,OUT char* frameType,int isBlock);

#ifdef __cplusplus
}
#endif

#endif
