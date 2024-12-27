VERSION = 1.00
CROSS_COMPILE = /home/obt/workspace/code_cache/tools/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc
DOS = -DOS_UBUNTU
DEBUG = -DUSE_DEBUG
SRC_DIR = ./src
STATIC_LIB = libcomm_driver.a
DYNAMIC_LIB = libcomm_driver.so
CFLAGS = -Wall -fPIC
TIMESTAMP = $(shell date +%Y%m%d%H%M)
$(info $(TIMESTAMP))
SOURCES_DIRS = $(shell find $(SRC_DIR) -maxdepth 3 -type d)
$(info src_dir = $(SOURCES_DIRS))
SOURCES = $(foreach dir, $(SOURCES_DIRS), $(wildcard $(dir)/*.c))
#SOURCES = $(wildcard ./src/*.c)
#SOURCES += $(wildcard ./src/log/*.c)
#SOURCES += $(wildcard ./src/refactor/*.c)
#SOURCES += $(wildcard ./src/queue/*.c)
#SOURCES += $(wildcard ./src/memory/*.c)
#SOURCES += $(wildcard ./src/list/*.c)
#ALL_FILES := $(shell ls -l $(SRC_DIR))
#FILES := $(notdir $(ALL_FILES))
#print:
#	@echo Files: $(FILES_TEST)
$(info src_list = $(SOURCES))
INCLUDES = -I ./inc
LIB_NAMES = -lpthread
LIB_PATH = -L ./lib
OBJ = $(patsubst %.c, %.o, $(SOURCES))
$(info obj_list = $(OBJ))
TARGET = softmanage

$(TARGET) : $(OBJ)
	@mkdir -p output
	ar rcs output/$(STATIC_LIB) $(OBJ)
	$(CC) -shared -o output/$(DYNAMIC_LIB) $(OBJ)
	$(CC)  $(OBJ)  $(LIB_PATH) $(LIB_NAMES) -o output/$(TARGET)$(VERSION)
	@rm -rf $(OBJ)

%.o : %.c
	$(CC) $(INCLUDES) $(DEBUG) $(DOS) -c $(CFLAGS) $< -o $@

.PHONY:clean
clean:
	@echo "Remove linked and compiled files ....."
	rm -rf $(OBJ) $(TARGET) output
