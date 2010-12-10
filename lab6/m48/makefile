PRG            =lab6_m48

OBJ            = $(PRG).o twi.o

MCU_TARGET     = atmega48
PROGRAMMER_TARGET     = m48

#agressive optimization
OPTIMIZE       = -O2    # options are 1, 2, 3, s
#optimize for small code
#OPTIMIZE       = -Os    # options are 1, 2, 3, s

DEFS           =
LIBS           =

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: $(PRG).elf lst text eeprom

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

#prevent confusion with any file named "clean"
#"-" prevents erroring out with file not found
.PHONY	: clean
clean: 
	-rm -rf $(PRG).o $(PRG).elf 
	-rm -rf $(PRG).lst $(PRG).map 
	-rm -rf $(PRG).srec $(PRG)*.bin $(PRG).hex 
	-rm -rf $(PRG)_eeprom.srec $(PRG)_eeprom*.bin $(PRG)_eeprom.hex 

cleanall:
	rm -rf *.o *.elf *.lst *.map *.srec *.bin *.hex

#setup for usb programmer
#hacked the permissions for stinkin' Redhat box
program: $(PRG).hex
	chmod 644 $(PRG).hex
	sudo avrdude -B 10 -p $(PROGRAMMER_TARGET) -c osuisp2 -e -U flash:w:$(PRG).hex 

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@ \
	|| { echo empty $@ not generated; exit 0; }

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@ \
	|| { echo empty $@ not generated; exit 0; }
