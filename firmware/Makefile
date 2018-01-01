CC=sdcc
LD=sdld

LIBSTM8_PATH=$(PWD)/../libstm8

CFLAGS += -I$(LIBSTM8_PATH)/include
OBJECTS := blinky.rel \
	$(addprefix $(LIBSTM8_PATH)/lib/l/, \
		tim.rel \
		gpio.rel \
		clk.rel \
)
BINARY := snowflake_fw.ihx

.PHONY: all clean flash

all: $(BINARY)

clean:
	rm -f $(OBJECTS) $(BINARY)
	rm -f *.asm *.cdb *.lk *.lst *.map *.rel *.rst *.sym

flash: $(BINARY)
	stm8flash -cstlinkv2 -pstm8l051f3 -w $(BINARY)

%.rel: %.c
	$(CC) --std-c99 -lstm8 -mstm8 --out-fmt-ihx $(CFLAGS) $(LDFLAGS) -c -o $@ $<

$(BINARY): $(OBJECTS)
	$(CC) -mstm8 $(LDFLAGS) -o $@ $^
