$(EXECUTABLE): libst7565.so src/main.o
	@echo "[bin]" $@
	$Q $(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBS) -lst7565 -o $(EXECUTABLE) src/main.o
	$Q $(RE) -a $(EXECUTABLE) | grep "Shared library:"

sync: $(EXECUTABLE)
	@echo '-> push to device'
	$Q rsync -avz --exclude toolchain --exclude dl ./ root@$(NANOPI_IP_ADDRESS):/root/ST7565
	$Q ssh root@$(NANOPI_IP_ADDRESS) 'cp -f /root/ST7565/out/lib/libst7565.so /usr/lib/libst7565.so'

test:
	$Q ssh root@$(NANOPI_IP_ADDRESS) '/root/ST7565/$(EXECUTABLE)'
