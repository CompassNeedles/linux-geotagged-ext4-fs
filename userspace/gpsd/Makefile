CC := arm-none-linux-gnueabi-gcc
LD := arm-none-linux-gnueabi-gcc
CFLAGS := -g -Wall -Werror -static
INCLUDE = -I. -I../../flo-kernel/usr/include -I../../flo-kernel/include -I../../flo-kernel/arch/arm/include
LDFLAGS := -static $(INCLUDE)

OBJECTS := gpsd.o

all: gpsd 

: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm -f gpsd 
	rm -f gpsd.o

.PHONY: clean
