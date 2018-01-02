# Compilers
AR := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ar
CC := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
GCC := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc
RE := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-readelf
RL := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-ranlib
SZ := toolchain/gcc-linaro-$(LINARO_TOOLCHAIN_VERSION)-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-size

ifneq ($V,1)
Q ?= @
endif

EXECUTABLE = test_st7565_lcd
STATIC     = libst7565.a
DYNAMIC    = libst7565.so

LDFLAGS  = -L$(LIB_PATH) -L.
CFLAGS = -g -Wall -Werror -Winline -pipe -std=c++11 -O -fPIC
LIBS     = -lwiringPi -lpthread

INCLUDES   = -I$(INCLUDES_SRC)/WiringNP/wiringPi -I$(INCLUDES_SRC)

default: $(EXECUTABLE)

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
	$Q find include/ src/ -type f -name '*.o' -delete
	$Q rm -f *.so
	$Q rm -f *.a

include build/Arduino.mk
include build/ST7565.mk
include build/Test.mk