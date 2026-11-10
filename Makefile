TARGET = pou_ps3
TITLE = POUPS3
APP_ID = POUPS3001
SFO_TITLE = Pou PS3 Edition

SRCDIRS = src
DATADIR = data
SOURCES = $(wildcard src/*.cpp) $(wildcard src/*.c)
OBJS = $(notdir $(SOURCES))
OBJS := $(OBJS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

XMB_ICON = ICON0.PNG
XMB_BG   = PIC1.PNG

CFLAGS = -O3 -Wall $(foreach dir,$(SRCDIRS),-I$(dir)) \
         -I$(PS3DEV)/portlibs/ppu/include \
         -I$(PS3DEV)/psl1ght/target/include \
         $(shell sdl2-config --cflags)
CXXFLAGS = $(CFLAGS) -std=gnu++17

LDFLAGS = -L$(PS3DEV)/portlibs/ppu/lib \
          -L$(PS3DEV)/psl1ght/target/lib \
          $(shell sdl2-config --libs) \
          -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGL -lrsx -lio -laudio -lsysio -lnet -lm

all: $(TARGET).pkg

PARAM.SFO:
	@echo "Criando PARAM.SFO..."
	@echo "TITLE = $(SFO_TITLE)" > .param.sfo.txt
	@echo "TITLE_ID = $(APP_ID)" >> .param.sfo.txt
	@echo "CATEGORY = GD" >> .param.sfo.txt
	@echo "APP_VER = 01.00" >> .param.sfo.txt
	@echo "VERSION = 01.00" >> .param.sfo.txt
	$(PS3DEV)/psl1ght/target/bin/sfo.py -f .param.sfo.txt PARAM.SFO
	@rm .param.sfo.txt

$(TARGET).self: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
	ppu-lv2-elf.strip $@

$(TARGET).pkg: $(TARGET).self PARAM.SFO $(XMB_ICON) $(XMB_BG)
	@echo "Gerando pacote PKG do Pou..."
	pkg_gen -t "$(TITLE)" -i $(XMB_ICON) -b $(XMB_BG) -d $(DATADIR) PARAM.SFO $(TARGET).self $@

%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET).self $(TARGET).pkg PARAM.SFO
