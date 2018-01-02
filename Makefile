# Config target
ARCH                     = arm
LINARO_TOOLCHAIN_VERSION = 7.2.1-2017.11
NANOPI_IP_ADDRESS        = 192.168.1.146

# Path
LIB_PATH          = $(abspath ./lib)
INCLUDES_SRC      = $(abspath ./include)

include build/Common.mk