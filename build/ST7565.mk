
ST7565_CPP_SRC = $(wildcard include/ST7565/*.cpp)
ST7565_CPP_OBJS := $(patsubst %.cpp,%.o,$(ST7565_CPP_SRC))

$(STATIC):	arduino $(ST7565_CPP_OBJS)
	@echo "[Link (Static)]"
	@$(AR) rcs $(STATIC) $(OBJ)
	@$(RL) $(STATIC)
	@$(SZ)   $(STATIC)

$(DYNAMIC): arduino $(ST7565_CPP_OBJS)
	$Q echo "[Link (Dynamic)] libst7565.so"
	$Q $(CC) -shared -Wl,-soname,libst7565.so -o libst7565.so  $(LDFLAGS) $(LIBS) $(ST7565_CPP_OBJS) $(ARDUINO_C_OBJS) $(ARDUINO_CPP_OBJS)
