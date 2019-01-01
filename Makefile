##################################################################### 
## liblshell.a                        
## 鸣谢博客：http://blog.chinaunix.net/uid-25838286-id-3204219.html      
## ##################################################################
CC = gcc
RM = rm -rf

SRC_DIR := src/
INCLUDE_DIR := include/
BUILD_DIR := build/
LIBS_DIR := libs/

SRCS := $(wildcard $(SRC_DIR)*.c)
OBJS := $(SRCS: .c = .o)
LIBS := -lreadline -ltermcap
INCLUDES := -I$(INCLUDE_DIR)
CFLAGS = -g -Wall

lshell : $(OBJS)
	ar -crv build/liblshell.a $(OBJS)
%.o : %.c
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS) $(LIBS)

clean: 
	$(RM) $(OBJS) build/*
	
.PHONY:clean
