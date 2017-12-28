ARCH := arm
LINARO_TOOLCHAIN_VERSION := 7.2.1-2017.11
NANOPI_IP_ADDRESS := 192.168.1.146

#Compilers
ifeq ($(ARCH),arm)
CC := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
GCC := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
RE := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-readelf
else
CC := g++
GCC := gcc
RE := readelf
endif

ifneq ($V,1)
Q ?= @
endif

LIB_PATH := $(abspath ./lib)
INCLUDES_SRC := $(abspath ./include)
OUTPUT := $(abspath ./out)

CFLAGS  := -g -Wall -Werror -Winline -pipe -std=c++11 -O -fPIC
INCLUDES := -I$(INCLUDES_SRC)/WiringNP/wiringPi -I$(INCLUDES_SRC)
LFLAGS := -L$(LIB_PATH) -L$(OUTPUT)/lib -lwiringPi -lpthread

.PHONY: default
default: main

libst7565:
	@echo '-> libst7565.so'
	$Q mkdir -p $(OUTPUT)/lib/
	$Q $(CC) $(CFLAGS) $(INCLUDES) $(LFLAGS) -c include/ST7565/ST7565.cpp -o $(OUTPUT)/ST7565.o
	$Q $(CC) -shared -Wl,--no-whole-archive -o $(OUTPUT)/lib/libst7565.so $(OUTPUT)/ST7565.o $(LFLAGS)

.PHONY: main
main: libst7565
	@echo '-> main'
	$Q $(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUT)/main src/main.cpp $(LFLAGS) -lst7565
	$Q $(RE) -a $(OUTPUT)/main | grep "Shared library:"

.PHONY: sync
sync: main
	@echo '-> push to device'
	$Q rsync -avz --exclude toolchain --exclude dl ./ root@$(NANOPI_IP_ADDRESS):/root/ST7565
	$Q ssh root@$(NANOPI_IP_ADDRESS) 'cp -f /root/ST7565/out/lib/libst7565.so /usr/lib/libst7565.so'

.PHONY: test
test:
	$Q ssh root@$(NANOPI_IP_ADDRESS) '/root/ST7565/out/main'

download: download_toolchain download_wiringpi

download_toolchain:
	$Q mkdir -p dl
	$Q mkdir -p toolchain

	@echo '-> Fetch linaro toolchain'
	$Q aria2c -x8 -c -o dl/gcc-linaro-x86_64_arm-linux-gnueabihf.tar.xz https://releases.linaro.org/components/toolchain/binaries/latest/arm-linux-gnueabihf/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf.tar.xz
	@echo '-> Extract toolchain'
	$Q tar xf dl/gcc-linaro-x86_64_arm-linux-gnueabihf.tar.xz -C toolchain

download_wiringpi:
	@echo '-> Fetch WiringNP'
	$Q git submodule init
	$Q git submodule update

clean: 
	$Q $(RM) -rf $(OUTPUT)
