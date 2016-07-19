#include "h264Util.h"

#ifdef __cplusplus
extern "C"{
#endif


int h264HelperInit(h264Helper* helper,char* fileName)
{
	if(!helper || !fileName)
	{
		printf("param invlaid,helper is NULL or fileName is NULL\n");
		return -1;
	}
	memset(helper,0,sizeof(h264Helper));
	helper->readFileEof = 0;
	helper->offset = 0;
	helper->frameSize = 0;
	helper->bufferSize = READ_ONCE;
	helper->startCodeLen = 0;
	helper->naluType = 0;
	helper->fp = fopen(fileName,"rb");
	if(helper->fp == NULL)
	{
		perror("open h264 file failed");
		return -1;
	}
	helper->buffer = (char*) malloc(helper->bufferSize);
	if(!helper->buffer)
	{
		perror("helper buffer malloc failed\n");
		if(helper->fp)
			fclose(helper->fp);
		return -1;
	}
	helper->sendDoneFlag = 0;
	helper->data = NULL;
	helper->readFileFlag = 1;
	memset(helper->buffer,0,helper->bufferSize);
	return 0;
}

int h264HelperFree(h264Helper* helper)
{
	if(!helper)
		return 1;
	if(helper->buffer)
	{
		free(helper->buffer);
		helper->buffer = NULL;
	}
	if(helper->fp)
	{
		fclose(helper->fp);
	}
	helper->data = NULL;
	return 0;
}

static int readH264File(h264Helper* helper)
{
	FILE* recordH264 = helper->fp;
	int ret = -1;
	int space = 0;
	int readSize = 0;
	if(!recordH264 || !helper)
		return -1;
	if(!helper->buffer)
		return -1;
	space = helper->bufferSize - helper->offset;
	if(space > 0 && space < helper->bufferSize)
	{
		memcpy(helper->buffer,helper->buffer+helper->offset,space);
		memset(helper->buffer+space,0,helper->bufferSize - space);
		readSize = helper->bufferSize - space;
		ret = fread(helper->buffer + space,1,helper->bufferSize - space,recordH264);
	}
	else
	{
		//fread的返回值不是读取的字节数
		readSize = helper->bufferSize;
		ret = fread(helper->buffer,1,helper->bufferSize,recordH264);
	}
	if(ret != readSize)
	{
		/*
		if( feof(recordH264) == 1)
		{
			printf("feof == 1\n");
			ret = 1;//读到文件尾。
			helper->readFileEof = 1;
		}
		else if(ferror(recordH264))
		{
		perror("fread record file error");
		ret = -1;//读文件发生错误，正常返回1
		}
		else
		{
		printf("vidoe read file other error,ret=%d,readSize=%d\n",ret,readSize);
		ret = -1;
		}
		*/
		ret = 1;//读到文件尾或者发生错误
		helper->readFileEof = 1;
		
	}
	else
	{
		ret = 0;
	}
	helper->offset = 0;
	return ret;
}

static int findStartCode(char* buffer,int bufferLen,int* startCodeLen)
{
	int i=0;
	if (!buffer && bufferLen < 3)
	{
		printf("findStartCode param invalid,buffer is NULL or bufferLen < 3\n");
		return -1;
	}
	for(i=0;i+1 < bufferLen;i+=2)
	{
		if(buffer[i])
			continue;
		if(i>0 && buffer[i-1] == 0)
			i--;
		if(i+2 < bufferLen && buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 1)
		{
			*startCodeLen = 3;
			return i;
		}
		if(i+3 < bufferLen && buffer[i] == 0 && buffer[i+1] == 0 && buffer[i+2] == 0 && buffer[i+3] == 1)
		{
			*startCodeLen = 4;
			return i;
		}
	}
	return -1;
}

static int getFrame(h264Helper* helper)
{
	int len = 0;
	int findStart = 0;
	int startCodeLen;
	unsigned char naluHeader = 0;
	char* bufferStart = helper->buffer + helper->offset;
	int checkLen = helper->bufferSize - helper->offset;
	if(!helper)
		return -1;
	if(!helper->buffer)
		return -1;
	if ((findStart = findStartCode(bufferStart,checkLen,&startCodeLen)) == -1)
	{
		helper->frameSize = 0;
		//helper->offset = helper->bufferSize;
		helper->naluType = 0;
		return 1;
	}
	helper->data = findStart + bufferStart;
	naluHeader = helper->data[startCodeLen];
	helper->startCodeLen = startCodeLen;
	switch (naluHeader & 0x1f)
	{
	case 6://sps
		helper->naluType = 6;
		break;
	case 7:
		helper->naluType = 7;
		break;
	case 5:
		helper->naluType = 5;
		break;
	case 1:
		helper->naluType = 1;
		break;
	case 8:
		helper->naluType = 8;
		break;
	default:
		break;
	}
	if((findStart = findStartCode(bufferStart+findStart+startCodeLen,checkLen - findStart - startCodeLen,&startCodeLen)) == -1)
	{
		helper->frameSize = 0;
		helper->naluType = 0;
		return 1;
	}
	helper->frameSize = findStart + startCodeLen;
	helper->offset += findStart + startCodeLen;
	return 0;
}

int getH264Frame(h264Helper* helper)
{
	int ret = 0;
	int left = 0;
	if(!helper)
	{
		return -1;
	}
	left = helper->bufferSize - helper->offset;
	//printf("left = %d,offset=%d\n",left,helper->offset);
	if(left < 200 || left == helper->bufferSize)
	{
		helper->readFileFlag = 1;
	}
	if(helper->readFileFlag == 1)
	{
		ret = readH264File(helper);
		helper->readFileFlag = 0;
	}
	ret = getFrame(helper);
	if(ret == 1)
	{
		if(helper->readFileEof)
		{
			helper->sendDoneFlag = 1;
		}
		else
		{
			helper->readFileFlag = 1;
		}
	}
	return ret;
}

int getFps(char* buffer,int bufferLen)
{
	return 15;
}


#ifdef __cplusplus
}
#endif