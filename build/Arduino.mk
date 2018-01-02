ARDUINO_C_SRC = $(wildcard include/*.c)
ARDUINO_C_OBJS := $(patsubst %.c,%.o,$(ARDUINO_C_SRC))

ARDUINO_CPP_SRC = $(wildcard include/*.cpp)
ARDUINO_CPP_OBJS := $(patsubst %.cpp,%.o,$(ARDUINO_CPP_SRC))

%.o: %.c $(ARDUINO_C_SRC)
	@echo [Compile] $<
	$Q $(CC) $(INCLUDES) -c $(CFLAGS) $< -o $@

%.o: %.cpp $(ARDUINO_CPP_SRC)
	@echo [Compile] $<
	$Q $(CC) $(INCLUDES) -c $(CFLAGS) $< -o $@

arduino: $(ARDUINO_C_OBJS) $(ARDUINO_CPP_OBJS)
