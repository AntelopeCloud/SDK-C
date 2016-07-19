#ifndef __PUSH_FILE_H__
#define __PUSH_FILE_H__
#endif

#ifdef __cplusplus
extern "C"{
#endif

//推视频流（H264压缩格式）
void* pushVideoFlowProc(void* arg);

//推音频流（AAC压缩格式）
void* pushAudioFlowProc(void* arg);

//回放对讲音频线程
void* playBackAudioProc(void* arg);

#ifdef __cplusplus
}
#endif