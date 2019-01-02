##################################################################### 
## liblshell.a                        
## 鸣谢博客：https://blog.csdn.net/li_wen01/article/details/65627086      
## 鸣谢博客：https://blog.csdn.net/yudingding6197/article/details/2831638   
## ##################################################################
LIB_NAME = lshell                                                                                 
STATIC_NAME = lib$(LIB_NAME)

SRC_DIR := src/
INCLUDE_DIR := include/
BUILD_DIR := build/
LIBS := -lreadline -ltermcap 
CFLAGS = -g -Wall

SRCS := $(wildcard $(SRC_DIR)*.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))
INCS := -I$(INCLUDE_DIR)

ifeq ($(VERBOSE),)
SILENCE=@echo "building: "$@;
else
SILENCE=
endif

all:$(STATIC_NAME)
 
%.o:%.c
	$(CC) -c $< -o $@ $(INCS) $(LIBS) $(CFLAGS)
 
$(STATIC_NAME):$(OBJS)
	$(SILENCE)if [ ! -d $(BUILD_DIR) ]; then mkdir -p $(BUILD_DIR); fi;
	ar -cr $(BUILD_DIR)$@.a $^
 
clean:
	rm -rf $(OBJS) $(BUILD_DIR)*
