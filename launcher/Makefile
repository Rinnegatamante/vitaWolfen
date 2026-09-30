TARGET		:= vitaWolfen
WMODE		:= 1
TITLE		:= WOLF00001

LIBS = -Wl,--start-group -limgui -lvitaGL -lSceAppMgr_stub -lSceDisplay_stub -lSceGxm_stub \
	-lSceSysmodule_stub -lSceCtrl_stub -lm -lc -lSceCommonDialog_stub -lz \
	-lSceTouch_stub -lmathneon -lvitashark -lSceShaccCgExt -ltaihen_stub \
	-lSceShaccCg_stub -lSceKernelDmacMgr_stub -lSceAppUtil_stub -Wl,--end-group

COMMON_OBJS = source/main.o

CFILES		:=	$(COMMON_OBJS)
CPPFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) 
			
PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CXX      = $(PREFIX)-g++
CFLAGS  = -Wl,-q -O3
CXXFLAGS  = $(CFLAGS) -fno-exceptions
ASFLAGS = $(CFLAGS)

all: $(TARGET).vpk

$(TARGET).vpk: $(TARGET).velf
	mkdir -p ../release/sce_sys
	vita-make-fself -s $< ../release/eboot.bin
	vita-mksfoex -s TITLE_ID=$(TITLE) "$(TARGET)" param.sfo
	cp -f param.sfo ../release/sce_sys/param.sfo
	vita-pack-vpk -s param.sfo -b ../release/eboot.bin \
		--add ../release/sce_sys/icon0.png=sce_sys/icon0.png \
		--add ../release/sce_sys/livearea=sce_sys/livearea \
		--add ../release/shaders=shaders \
		--add ../release/eboot00.bin=eboot00.bin \
		--add ../release/eboot10.bin=eboot10.bin \
		--add ../release/eboot20.bin=eboot20.bin \
		--add ../release/eboot30.bin=eboot30.bin \
		--add ../release/eboot32.bin=eboot32.bin \
		--add ../release/eboot33.bin=eboot33.bin $@

%.velf: %.elf
	cp $< $<.unstripped.elf
	$(PREFIX)-strip -g $<
	vita-elf-create $< $@

$(TARGET).elf: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@

clean:
	@rm -rf $(TARGET).velf $(TARGET).elf $(OBJS) param.sfo eboot.bin