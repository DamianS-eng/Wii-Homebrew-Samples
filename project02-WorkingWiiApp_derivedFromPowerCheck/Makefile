#---------------------------------------------------------------------------------
# Clear implicit rules
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>/devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# Project settings
#---------------------------------------------------------------------------------
TARGET      := boot
BUILD       := build
SOURCES     := source
DATA        := data
INCLUDES    :=

#---------------------------------------------------------------------------------
# Compiler flags
#---------------------------------------------------------------------------------
CFLAGS      := -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS    := $(CFLAGS)
LDFLAGS     := -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# Libraries
#---------------------------------------------------------------------------------
LIBS        := -lgrrlib -lfreetype -lbz2 -lfat -ljpeg -lpngu -lpng -lz \
               -lwiiuse -lbte -logc -lm

LIBDIRS     := $(PORTLIBS)

#---------------------------------------------------------------------------------
# Build system
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)

export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                $(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR := $(CURDIR)/$(BUILD)

# Source files
CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES    := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

# Linker
ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

# Object list
export OFILES := $(addsuffix .o,$(BINFILES)) \
                 $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
                 $(sFILES:.s=.o) $(SFILES:.S=.o)

# Include paths
export INCLUDE := $(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD) \
                  -I$(LIBOGC_INC)

# Library paths
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
                   -L$(LIBOGC_LIB)

.PHONY: $(BUILD) clean run

$(BUILD):
    @[ -d $@ ] || mkdir -p $@
    @$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
    @echo clean ...
    @rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

run:
    $(MAKE)
    wiiload $(TARGET).dol

else

#---------------------------------------------------------------------------------
# Inside build directory
#---------------------------------------------------------------------------------

# Dependency files (only for C/C++)
DEPENDS := $(filter %.d,$(OFILES:.o=.d))

# Main targets
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# Binary data rules (each extension needs its own rule)
#---------------------------------------------------------------------------------
define BIN2O_RULE
$1.o: $1
    @echo $$(notdir $$<)
    $$(bin2o)
endef

$(foreach ext, jpg png ttf bmf dol wad certs sys dat tmd tik bin, \
  $(eval $(call BIN2O_RULE,%.$(ext))) )

#---------------------------------------------------------------------------------
# Prevent .d generation for binary resources
#---------------------------------------------------------------------------------
%.jpg.d %.png.d %.ttf.d %.bmf.d %.dol.d %.wad.d %.certs.d %.sys.d %.dat.d %.tmd.d %.tik.d %.bin.d:
    @:

# Include dependency files
-include $(DEPENDS)

endif
