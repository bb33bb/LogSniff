TARGET := log.sniff
CC = gcc
CXX = g++
CFLAGS := 
CXX_FLAGS :=  -g -Wall -D VERSION=\"1.2.3.5\" -I ./common -I ../LogLib
LDFLAGS :=-lcurl -lm -luuid -lpthread

CPP_SOURCES := $(wildcard ./*.cpp)
CPP_SOURCES += $(wildcard ./common/*.cpp)
CPP_SOURCES += $(wildcard ./common/json/*.cpp)
CPP_SOURCES += $(wildcard ../LogLib/LogProtocol.cpp)
CPP_SOURCES += $(wildcard ../LogLib/LogUtil.cpp)
CPP_SOURCES += $(wildcard ../LogLib/tcpserv.cpp)
CPP_OBJS = $(CPP_SOURCES:.cpp=.o)

C_SOURCES := $(wildcard ./*.c)
C_SOURCES += $(wildcard ../hlib/*.c)
C_SOURCES += $(wildcard ../../cplib/*.c)
C_OBJS = $(C_SOURCES:.c=.o)

#链接成目标文件
$(TARGET): $(CPP_OBJS) $(C_OBJS)
	echo "test1"
	g++ $(LDFLAGS) -o $(TARGET) $(CPP_OBJS) $(C_OBJS)
	cp ./$(TARGET) ../bin
	
#*********************************************
# 编译c代码文件
%.o: %.c
	gcc $(CC_FLAGS) -c $< -o $@

#*********************************************
# 编译c++代码文件
%.o: %.cpp
	g++ $(CXX_FLAGS) -c $< -o $@

clean:
	rm -f ./*.o;rm -f ./robin

clean-all:
	rm -f ./*.o;rm -f ./robin;rm -f ../../cplib/*.o