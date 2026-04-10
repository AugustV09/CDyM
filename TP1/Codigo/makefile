# ======================================
# MICROCONTROLADOR
# ======================================
MCU      = atmega328p
F_CPU    = 16000000UL
PROGRAMMER = arduino
PORT     = /dev/ttyACM0
BAUD     = 115200

# ======================================
# HERRAMIENTAS
# ======================================
CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size
DUDE    = avrdude

# ======================================
# ARCHIVOS (RECURSIVO)
# ======================================
TARGET  = main

# Busca todos los .c en el directorio raíz y subdirectorios
SRC     = $(shell find . -name "*.c")

# Genera los .o respetando la ruta de cada fuente
OBJ     = $(SRC:.c=.o)

# Busca todos los directorios que contienen .h para pasarlos como -I
INC_DIRS = $(sort $(dir $(shell find . -name "*.h")))
INCLUDES = $(addprefix -I, $(INC_DIRS))

# ======================================
# FLAGS
# ======================================
CFLAGS  = -mmcu=$(MCU)   \
          -DF_CPU=$(F_CPU) \
          -Wall            \
          -Os              \
          -g               \
          -std=gnu11       \
          $(INCLUDES)

# ======================================
# DEFAULT
# ======================================
all: $(TARGET).elf size

# ======================================
# COMPILACION
# ======================================
$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ======================================
# TAMANO DEL PROGRAMA
# ======================================
size: $(TARGET).elf
	$(SIZE) --mcu=$(MCU) --format=avr $<

# ======================================
# HEX
# ======================================
$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# ======================================
# SUBIR AL MICRO
# ======================================
upload: $(TARGET).hex
	$(DUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$<

# ======================================
# ASM PARA ANALISIS (C + ASM)
# ======================================
asm: $(TARGET).elf
	$(OBJDUMP) -d -S $< > $(TARGET).lst

# ======================================
# LIMPIAR
# ======================================
clean:
	find . -name "*.o" -delete
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).lst

# ======================================
.PHONY: all upload asm clean size