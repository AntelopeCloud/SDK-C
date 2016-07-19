#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C"{
#endif


int createMsgId(unsigned char sub_id)
{
	int msgid = -1;
	key_t key;
	if((key = ftok("/etc/init.d",sub_id)) == -1)
	{
		printf("create key failed\n");
		return -1;
	}
	if((msgid = msgget(key,IPC_CREAT|0666)) == -1)
	{
		printf("create msgid failed\n");
		return -1;
	}
	return msgid;
}

static int getMac(char* mac,char* dev_name)
{
	int fd;
	int ret;
	int i = 0;
	struct ifreq buf;

	if(!mac || !dev_name)
		return;
	fd = socket(AF_INET,SOCK_DGRAM,0);
	if(fd == -1)
	{
		perror("get_local_ip create socket failed\n");
		return -1;
	}
	strcpy(buf.ifr_name,dev_name);
	if(ioctl(fd, SIOCGIFHWADDR,&buf) < 0)
	{
		perror("error ioctl");
		close(fd);
		return -1;
	}
	for(i=0;i<6;i++)
	{
		sprintf(mac+2*i, "%02X", (unsigned char)buf.ifr_hwaddr.sa_data[i]);
	}
	mac[0] = 'S';
	mac[strlen(mac)] = 0;
	close(fd);
	return 0;
}

int checkNetStatus(char* net)
{
	if(net == NULL)
		return -1;
	int fd;
	struct ifreq ifr;
	int ret = 0;

	if (( fd = socket( AF_INET, SOCK_DGRAM, 0 )) <= 0)
	{
		perror(" checkNetStatus create socket");
		return -1;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, net, sizeof(ifr.ifr_name) - 1);
	if (ioctl(fd, SIOCGIFFLAGS, (char *)&ifr) < 0)
	{
		close(fd);
		return -1;
	}
	if ((ifr.ifr_flags & IFF_RUNNING) == 0)
	{
		ret = 1;//连上外网
	}
	else
	{
		ret = 0;//网络断开
	}
	close(fd);
	return ret;
}

int sync_time(char* srv_ip)
{
	if(srv_ip == NULL)
		return -1;
//************define*******************//
#define  int8      char
#define  uint8     unsigned char
#define  uint32    unsigned int
#define  ulong32   unsigned long
#define  long32    long
#define  int32     int
#define  long64    long long
#define  From00to70 0x83aa7e80U
#define  NTPPORT  123
typedef struct NTPPACKET
{
	uint8     li_vn_mode;
	uint8     stratum;
	uint8     poll;
	uint8     precision;
	ulong32   root_delay;
	ulong32   root_dispersion;
	int8      ref_id[4];
	ulong32   reftimestamphigh;
	ulong32   reftimestamplow;
	ulong32   oritimestamphigh;
	ulong32   oritimestamplow;
	ulong32   recvtimestamphigh;
	ulong32   recvtimestamplow;
	ulong32   trantimestamphigh;  
	ulong32   trantimestamplow;
}NTPPacket;
//************end define*******************//
	NTPPacket  ntppack,newpack;
	long64   firsttimestamp,finaltimestamp;
	long64   diftime,delaytime;

	fd_set  inset1;
	int32  sockfd;
	struct timeval tv,tv1;
	struct timezone tz;
	struct sockaddr_in addr;

	if((sockfd=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		perror("create socket error!\n");
		exit(1);
	}

	addr.sin_family=AF_INET;   //IPV4协议
	addr.sin_port =htons(NTPPORT);   //NTP专用的123端口
	addr.sin_addr.s_addr=inet_addr(srv_ip);   //校时服务器
	bzero(&(addr.sin_zero),8);   //清零

	//wait 5s
	tv.tv_sec=10;    //select等待时间为10S
	tv.tv_usec=0;

	FD_ZERO(&inset1);
	FD_SET(sockfd,&inset1);

	//初始化
	bzero(&ntppack,sizeof(ntppack));
	ntppack.li_vn_mode=0x1b;//0|(3<<2)|(3<<5);
	//获取初始时间戳T1
	firsttimestamp=time(NULL)+From00to70;//-8*3600;
	ntppack.oritimestamphigh=htonl(firsttimestamp);

	//发送数据请求包
	sendto(sockfd,&ntppack,sizeof(ntppack),0,(struct sockaddr *)&addr,sizeof(struct sockaddr));
	//select巡视
	if(select(sockfd+1,&inset1,NULL,NULL,&tv)<0)
	{
		perror("select error!\n");
		close(sockfd);
		return -1;
	}
	else
	{
		//printf("OK\n");
		if(FD_ISSET(sockfd,&inset1))
		{
			// printf("OK\n");
			if(recv(sockfd,&newpack,sizeof(newpack),0)<0)        //接收数据在newpack中。
			{
				perror("recv error!\n");
				close(sockfd);
				return -1;
			}
		}
		else
		{
			printf("sync time by ntp time out\n");
			return -1;
		}
	}

	finaltimestamp=time(NULL)+From00to70;
	//将网络上传送的大端数据改为小端形式。
	newpack.root_delay= ntohl(newpack.root_delay);
	newpack.root_dispersion= ntohl(newpack.root_dispersion);
	newpack.reftimestamphigh=ntohl(newpack.reftimestamphigh);
	newpack.reftimestamplow= ntohl(newpack.reftimestamplow);
	newpack.oritimestamphigh= ntohl(newpack.oritimestamphigh);
	newpack.oritimestamplow= ntohl(newpack.oritimestamplow);
	newpack.recvtimestamphigh= ntohl(newpack.recvtimestamphigh);
	newpack.recvtimestamplow= ntohl(newpack.recvtimestamplow);
	newpack.trantimestamphigh= ntohl(newpack.trantimestamphigh);
	newpack.trantimestamplow= ntohl(newpack.trantimestamplow);

	//求出客户机跟服务器的时间差=((T2-T1)+(T3-T4))/2
	diftime=((newpack.recvtimestamphigh-firsttimestamp)+(newpack.trantimestamphigh-finaltimestamp))>>1;
	//求出延时
	delaytime=((newpack.recvtimestamphigh-firsttimestamp)-(newpack.trantimestamphigh-finaltimestamp))>>1;
	//diftime=(5-9)>>1;

	//求出真正时间的时间戳
	tv1.tv_sec=time(NULL)+diftime+delaytime;
	tv1.tv_usec=0;

//#define ntp_debug 1

#ifdef ntp_debug
	printf("debug information ...\n\n");
	printf("time(NULL) is %llu\n",time(NULL));
	printf("different time is %ld\n",diftime);
	printf("delaytime is %ld\n",delaytime);
	printf("time(NULL)+diftime+delaytime=%ld\n",time(NULL)+diftime+delaytime);
	printf("tv1.tv_sec is %ld\n\n", tv1.tv_sec);
#endif

	settimeofday(&tv1,NULL);

#ifdef ntp_debug
	//printf("different time is %ld\n",diftime);
	printf("delay  time is %ld\n",delaytime);
	//printf("firsttimestamp is %x\n",time(NULL));
	printf("newpack.tran is %ld\n",newpack.trantimestamphigh);
	printf("newpack.recv is %ld\n",newpack.recvtimestamphigh);
	printf("firsttimestamp is %ld\n",firsttimestamp);
	printf("finaltimestamp is %ld\n",finaltimestamp);
	printf("newpack.recv-firsttimestamp is %ld\n",newpack.recvtimestamphigh-firsttimestamp);
	printf("newpack.tran-finaltimestamp is %ld\n",newpack.trantimestamphigh-finaltimestamp);
	printf("(recv-first)+(ftran-final) is %ld\n",(newpack.recvtimestamphigh-firsttimestamp)+(newpack.trantimestamphigh-finaltimestamp));
	printf("((recv-first)+(ftran-final))>>1 is %ld\n",((newpack.recvtimestamphigh-firsttimestamp)+(newpack.trantimestamphigh-finaltimestamp))>>1);
	printf("different time is %ld\n\n",diftime);
	printf("sizeof(long long)  is:%d\n",sizeof(long long));
	printf("Current time is...\n");
#endif
	//system("date");
	return 0;
}

 unsigned long long getMSec()
{
	struct timeval aika;
	gettimeofday(&aika,NULL);
	unsigned long long tmp_sec = aika.tv_sec;
	unsigned long long tmp_usec = aika.tv_usec;
	unsigned long long msecs = tmp_sec * 1000 *1000;
	msecs += tmp_usec;
	return msecs/1000;

}


#ifdef __cplusplus
}
#endif