CC = gcc
CFLAGS = -Wall -Wextra -O2

ARM_CC = arm-none-eabi-gcc
ARM_CFLAGS = -mcpu=cortex-m3 -mthumb -O2 -Wall -Wextra --specs=nosys.specs

SRC = ascon128.c test_vectors.c
OBJ = $(SRC:.c=.o)
ARM_OBJ = $(SRC:.c=.arm.o)

TARGET = ascon_test
ARM_TARGET = ascon128.elf

all: $(TARGET) $(ARM_TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(ARM_TARGET): $(ARM_OBJ)
	$(ARM_CC) $(ARM_CFLAGS) -o $@ $^

%.o: %.c ascon128.h
	$(CC) $(CFLAGS) -c $< -o $@

%.arm.o: %.c ascon128.h
	$(ARM_CC) $(ARM_CFLAGS) -c $< -o $@

clean:
	rm -f *.o *.arm.o $(TARGET) $(ARM_TARGET)

.PHONY: all clean
