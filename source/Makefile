TARGET		:= vitaWolfen

#CFLAGS += -Wall
CFLAGS += -Wl,-q -g -O2
CFLAGS += -Wpointer-arith
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -Wcast-align
CFLAGS += -DNO_STDIO_REDIRECT
CFLAGS += -flto -DWMODE=$(WMODE)
CFLAGS += -mtune=cortex-a9 -mfpu=neon

CCFLAGS += $(CFLAGS)
CCFLAGS += -std=gnu99
CCFLAGS += -Werror-implicit-function-declaration
CCFLAGS += -Wimplicit-int
CCFLAGS += -Wsequence-point

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++

CXXFLAGS += $(CFLAGS) -I$(VITASDK)/$(PREFIX)/include/SDL -fpermissive -fno-exceptions

LDFLAGS += -lSDL_mixer -lSceAudio_stub -lSceLibKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceSysmodule_stub -lSceCtrl_stub -lSceTouch_stub -lm -lSceNet_stub -lvorbis \
	-lSceNetCtl_stub -lScePgf_stub -ljpeg -lfreetype -lc -lSDL -limgui -lvitaGL -lSceHid_stub \
	-lScePower_stub -lSceCommonDialog_stub -lpng16 -lz -lSceAppmgr_stub -logg -lvorbisfile

SRCS :=
SRCS += fmopl.cpp
SRCS += id_ca.cpp
SRCS += id_in.cpp
SRCS += id_pm.cpp
SRCS += id_sd.cpp
SRCS += id_us_1.cpp
SRCS += id_vh.cpp
SRCS += id_vl.cpp
SRCS += signon.cpp
SRCS += wl_act1.cpp
SRCS += wl_act2.cpp
SRCS += wl_agent.cpp
SRCS += wl_atmos.cpp
SRCS += wl_cloudsky.cpp
SRCS += wl_debug.cpp
SRCS += wl_draw.cpp
SRCS += wl_floorceiling.cpp
SRCS += wl_game.cpp
SRCS += wl_inter.cpp
SRCS += wl_main.cpp
SRCS += wl_menu.cpp
SRCS += wl_parallax.cpp
SRCS += wl_play.cpp
SRCS += wl_state.cpp
SRCS += wl_text.cpp
SRCS += sys_psp2.cpp

DEPS = $(filter %.d, $(SRCS:.c=.d) $(SRCS:.cpp=.d))
OBJS = $(filter %.o, $(SRCS:.c=.o) $(SRCS:.cpp=.o))

.SUFFIXES:
.SUFFIXES: .c .cpp .d .o

Q ?= @

all: $(BINARY)

ifndef NO_DEPS
depend: $(DEPS)

ifeq ($(findstring $(MAKECMDGOALS), clean depend Data),)
-include $(DEPS)
endif
endif

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	vita-make-fself -s $< .\release\eboot$(WMODE).bin
	
%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin
