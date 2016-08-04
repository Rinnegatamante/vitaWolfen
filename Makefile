TARGET		:= vitaWolfen
WMODE		:= 3

LIBS = -lvita2d -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub -lm -lSceNet_stub \
	-lSceNetCtl_stub -lScePgf_stub -ljpeg -lfreetype -lc \
	-lScePower_stub -lSceCommonDialog_stub -lpng16 -lz

COMMON_OBJS = source/fmopl.o \
			source/ff_vita.o \
			source/id_ca.o \
			source/id_us.o \
			source/id_vh.o \
			source/misc.o \
			source/objs.o \
			source/sd_comm.o \
			source/sd_null.o \
			source/vi_vita.o \
			source/sys_vita.o \
			source/wl_act1.o \
			source/wl_act2.o \
			source/wl_act3.o \
			source/wl_agent.o \
			source/wl_debug.o \
			source/wl_draw.o \
			source/wl_game.o \
			source/wl_inter.o \
			source/wl_main.o \
			source/wl_menu.o \
			source/wl_play.o \
			source/wl_state.o \
			source/wl_text.o \
			source/font_data.o \
			source/font.o \
			source/automap.o 

CFILES		:=	$(COMMON_OBJS)
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
			
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3 -DSKIPFADE -DHAVE_FFBLK -DDOSISM -DWMODE=$(WMODE)
CXXFLAGS  = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself $< .\release\eboot$(WMODE).bin
	
%.velf: %.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin