PWD     	:= $(shell pwd)
BIN			:= lyDeviceMultiDemo
TARGET      ?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
STRIP		:= $(CROSS_COMPILE)strip
SRC_DIR		:= ./src
INCLUDE_DIR	:= ./include
LIB_DIR		:= ./lib/arm-linux
INCLUDE		+= -I$(INCLUDE_DIR)

#公共部分
INCLUDE		+= -I$(INCLUDE_DIR)/lingyang
LDFLAG_COMMON	:= -lpthread -lm -lz -ldl
LDFLAG_LY	:=  -lTopvdnSDK -Wl,-rpath=. -lPusher -lssl -lcrypto


#功能开关
#FUNC := QSTP
FUNC := LY_PLATFORM

ifeq ($(FUNC),QSTP)
MACRO		+= -DQSTP
else
	ifeq ($(FUNC),LY_PLATFORM)
MACRO		+= -DLY_PLATFORM -DPLAYBACK
	endif
endif

DENP		:= $(SRC_DIR)/transportMedia.o $(SRC_DIR)/pushFile.o $(SRC_DIR)/aacUtil.o $(SRC_DIR)/h264Util.o $(SRC_DIR)/lyUtils.o

#如果是在linux环境下，目前只支持QSTP模块
ifeq ($(CC),gcc)
LDFLAG_LY	:= -lqstpclient -lssl -lcrypto
MACRO		:= -DQSTP -DLINUX
LIB_DIR		:=./lib/linux
endif


CFLAGS		+= -Wall -g -O2 $(INCLUDE) $(MACRO)
LDFLAGS		+= -Wl,--as-needed -L$(LIB_DIR) $(LDFLAG_LY) $(LDFLAG_COMMON)

$(BIN) : $(SRC_DIR)/main.o $(DENP)
	#if use $(CC),please link libstdc++.so
	$(CPP) $^ -o $@ $(LDFLAGS)
	$(STRIP) $(BIN)
	$(POST_BIN)

main.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/main.c -o  main.o
transportMedia.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/transportMedia.c -o  transportMedia.o
pushFile.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/pushFile.c -o  pushFile.o
aacUtil.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/aacUtil.c -o  aacUtil.o
h264Util.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/h264Util.c -o  h264Util.o
lyUtils.o : 
	$(CC) $(CFLAGS) $(SRC_DIR)/lyUtils.c -o  lyUtils.o
	
clean:
	rm -f ./src/*.o
	rm $(BIN)
