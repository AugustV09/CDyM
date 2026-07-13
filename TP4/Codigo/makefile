# ======================================
# MICROCONTROLADOR
# ======================================
MCU        = atmega328p
F_CPU      = 16000000UL
PROGRAMMER = arduino
PORT       = /dev/ttyACM0
BAUD       = 115200

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
TARGET = main
BUILD  = build

SRC      = $(shell find . -name "*.c" -not -path "./$(BUILD)/*")
OBJ      = $(patsubst ./%, $(BUILD)/%, $(SRC:.c=.o))
DEP      = $(OBJ:.o=.d)

INC_DIRS = $(sort $(dir $(shell find . -name "*.h" -not -path "./$(BUILD)/*")))
INCLUDES = $(addprefix -I, $(INC_DIRS))

# ======================================
# FLAGS
# ======================================
CFLAGS = -mmcu=$(MCU)        \
         -DF_CPU=$(F_CPU)    \
         -Wall               \
         -Os                 \
         -gdwarf-2           \
         -std=gnu11          \
         -ffunction-sections \
         -fdata-sections     \
         -flto               \
         -MMD -MP            \
         $(INCLUDES)

LDFLAGS = -mmcu=$(MCU) -flto -Wl,--gc-sections

# ======================================
# DEFAULT
# ======================================
all: $(TARGET).elf size

# ======================================
# COMPILACION
# ======================================
$(TARGET).elf: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD)/%.o: ./%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEP)

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

hex: $(TARGET).hex

# ======================================
# PROTEUS (ELF sin debug para avr-gcc >= 14)
# ======================================
sim: $(TARGET).elf
	$(OBJCOPY) --strip-debug $< $(TARGET)_sim.elf
	@echo "Cargá $(TARGET)_sim.elf en Proteus"

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
	rm -rf $(BUILD)
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).lst $(TARGET)_sim.elf

# ======================================
.PHONY: all upload asm clean size hex sim