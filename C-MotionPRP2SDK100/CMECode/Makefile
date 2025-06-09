#
# PMD CME template makefile
#
# This makefile is invoked by the build.bat file in each example project
#

PATH_GNU_ARM = ..\..\GNU
TCHAIN   = arm-none-eabi
MCU      = cortex-m7

CMOTIONDIR = $(OFFSETDIR)../c-motion
# List C source files here. (C dependencies are automatically generated.)
# use file-extension c for "c-only"-files
SRC  = $(TARGET).c
SRC += $(PROJECTFILES)

SRC += $(CMOTIONDIR)/C/C-Motion.c
SRC += $(CMOTIONDIR)/C/PMDtrans.c
SRC += $(CMOTIONDIR)/C/PMDdiag.c
SRC += $(CMOTIONDIR)/C/PMDcommon.c
SRC += $(CMOTIONDIR)/C/PMDutil.c
SRC += $(OFFSETDIR)startup.c
SRC += $(OFFSETDIR)syscalls.c

# List Assembler source files here.
ASRC = $(OFFSETDIR)UserImport.S

# List directories in which to search for source files.
# With this feature source file names must be unique.
# Note that pathnames with embedded spaces will have problems.
VPATH = $(OFFSETDIR) $(CMOTIONDIR)/c

# CME preprocessor definition
CDEFS =  -DCME

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS += $(OFFSETDIR) 
EXTRAINCDIRS += $(CMOTIONDIR)/include

# Path to Linker-Scripts
LINKERSCRIPTPATH = $(OFFSETDIR)

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
OPT = 0

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MD -MP -MF dep/$(@F).d

# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS += -O$(OPT)
CFLAGS += -mcpu=$(MCU)
#CFLAGS += -v
CFLAGS += $(CDEFS) $(CINCS) -I.
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS)) -I.
CFLAGS += -ffunction-sections -fdata-sections -fno-toplevel-reorder
CFLAGS += -Wall -Wcast-align
CFLAGS += -Wpointer-arith
CFLAGS += -Wunused-function
CFLAGS += $(GENDEPFLAGS)
CFLAGS += -Wcast-qual
CFLAGS += -Wno-unused-variable
CFLAGS += -mhard-float

# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS)
ASFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))


# Additional libraries.

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -nostartfiles 
LDFLAGS += -Wl,-Map=$(TARGET).map,--gc-sections,--print-memory-usage
#LDFLAGS += -Wl,--cref		#Output a cross reference table to the map file
#LDFLAGS += -Wl,--exclude-libs,ALL
#LDFLAGS += -nostdlib
LDFLAGS += -lm	#include math library
#LDFLAGS += -v	#verbose mode
LDFLAGS += -lc
LDFLAGS += -T$(LINKERSCRIPTPATH)/linkPMD.ld

# Listing file options
LSSFLAGS = -h -S -C -t

# Define directories, if needed.
DIRARM = $(PATH_GNU_ARM)
DIRARMBIN = $(DIRARM)/bin/
DIRARMUTILS = ../../utils/bin/

# Define programs and commands.
CC        = "$(DIRARMBIN)$(TCHAIN)-gcc"
AR        = "$(DIRARMBIN)$(TCHAIN)-ar"
OBJCOPY   = "$(DIRARMBIN)$(TCHAIN)-objcopy"
OBJDUMP   = "$(DIRARMBIN)$(TCHAIN)-objdump"
SIZE      = "$(DIRARMBIN)$(TCHAIN)-size"
NM        = "$(DIRARMBIN)$(TCHAIN)-nm"
REMOVE    = del /q
REMOVEDIR = rmdir /s /q
MAKEDIR   = mkdir
CRC       = $(DIRARMUTILS)crc
NEWLINE   = @echo=
STORE     = ../../../HostCode/Release/StoreUserCode
# Set the interface used by the store command: COM1, COM2, CAN, TCP
#PORT      = COM4


# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = -------- begin --------
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size:
MSG_FLASH = Creating CME flash file:
MSG_CRC = Adding CRC to flash file:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = Linking:
MSG_LIBBING = Creating lib file:
MSG_COMPILING = Compiling C:
MSG_ASSEMBLING = Assembling:
MSG_CLEANING = Cleaning project:
MSG_STORE = Downloading bin file to CME device:
# Define all object files.  Note that these will be put in the directory "obj".
COBJ      = $(addprefix obj/, $(notdir $(SRC:.c=.o)))
AOBJ      = $(addprefix obj/, $(notdir $(ASRC:.S=.o)))
#OBJ       = $(wildcard *.o)

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst) 

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = 
ALL_ASFLAGS = -mcpu=$(MCU) -I. $(ASFLAGS)

# Default target.
all: begin obj build finished end
build: elf bin lss sym crc
bin: $(TARGET).bin
elf: $(TARGET).elf
lss: $(TARGET).lss 
sym: $(TARGET).sym
a: libPMD.a

# Ensure directory exists
obj:
	$(MAKEDIR) obj
	$(MAKEDIR) dep

# Messages.
begin:
	$(NEWLINE) 
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	$(NEWLINE) 

# Display compiler version information.
gccversion : 
	@$(CC) --version

# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	$(NEWLINE) 
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O binary $< $@

# Add the CRC to the .bin file.
crc:
	$(NEWLINE) 
	@echo $(MSG_CRC)
	$(CRC) $(TARGET).bin

# Store the user code bin file to the CME device
store:
	$(NEWLINE) 
	@echo $(MSG_STORE)
	$(STORE) $(TARGET).bin $(PORT) 

# Create extended listing file from ELF output file.
%.lss: %.elf
	$(NEWLINE) 
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) $(LSSFLAGS) $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NEWLINE) 
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJ) $(COBJ)
%.elf:  $(AOBJ) $(COBJ)
	$(NEWLINE) 
	@echo $(MSG_LINKING) $@
	$(CC) $(CFLAGS) $(AOBJ) $(COBJ)  --output $@ $(LDFLAGS)

%.a:    $(AOBJ) $(COBJ)
	$(NEWLINE) 
	@echo $(MSG_LIBBING) $@
	$(AR) rcs $@ $(AOBJ) $(COBJ)

# Compile: create object files from C source files.
$(COBJ): obj/%.o : %.c
	$(NEWLINE) 
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(CFLAGS) $(CONLYFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files.
$(AOBJ): obj/%.o : %.S
	$(NEWLINE) 
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

# Target: clean project.
clean: begin clean_list finished end
rebuild: begin clean_list obj build finished end

clean_list :
	$(NEWLINE) 
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).bin  
	$(REMOVE) $(TARGET).elf  
	$(REMOVE) $(TARGET).map  
	$(REMOVE) $(TARGET).sym  
	$(REMOVE) $(TARGET).lss  
#	$(REMOVE) $(TARGET).lnk  
#	$(REMOVE) $(COBJ)
#	$(REMOVE) $(AOBJ)
#	$(REMOVE) $(LST)
#	$(REMOVE) $(SRC:.c=.d)
	if exist obj $(REMOVEDIR) obj 
	if exist dep $(REMOVEDIR) dep 


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion build elf hex bin lss sym clean clean_list

