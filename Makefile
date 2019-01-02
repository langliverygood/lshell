##################################################################### 
## liblshell.a                        
## 鸣谢博客：http://blog.chinaunix.net/uid-25838286-id-3204219.html      
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

all:$(STATIC_NAME)
 
%.o:%.c
	$(CC) -c $< -o $@ $(INCS) $(LIBS) $(CFLAGS)
 
$(STATIC_NAME):$(OBJS)
	ar -cr $(BUILD_DIR)$@.a $^
 
clean:
	rm -rf $(OBJS) $(BUILD_DIR)*
