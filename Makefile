.PHONY = all clean

PBSDK ?= ../../pocketbook-sdk5
CC = $(PBSDK)/bin/arm-obreey-linux-gnueabi-gcc
PBRES = $(PBSDK)/bin/pbres

GIT_VERSION := "$(shell git describe --always --tags)"

CFLAGS = -DCOMBINED -std=c99 -DNDEBUG -fsigned-char -fomit-frame-pointer -fPIC -O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp -linkview -lfreetype -lm -D_XOPEN_SOURCE=632 -DVERSION=\"$(GIT_VERSION)\"

UTILSRCS := $(wildcard utils/*.c)
UTILOBJS := $(UTILSRCS:%.c=%.o)

GAMESRCS := $(wildcard games/*.c)
GAMEOBJS := $(GAMESRCS:%.c=%.o)

FRONTENDSRCS := $(wildcard frontend/*.c)
FRONTENDOBJS := $(FRONTENDSRCS:%.c=%.o)
FRONTENDHDRS := $(wildcard include/frontend/*.h)

HEADERS := $(wildcard include /*.h)

all: build/SGTPuzzles.app

build/SGTPuzzles.app: icons/icons.c $(GAMEOBJS) $(FRONTENDOBJS) $(UTILOBJS)
	mkdir -p ./build
	LC_ALL=C $(CC) -s icons/icons.c $(GAMEOBJS) $(FRONTENDOBJS) $(UTILOBJS) -I./include -o ./build/SGTPuzzles.app $(CFLAGS)

icons/icons.c: ./icons/*.bmp
	LC_ALL=C $(PBRES) -c ./icons/icons.c -4 ./icons/*.bmp

frontend/%.o: frontend/%.c $(HEADERS) $(FRONTENDHDRS)
	LC_ALL=C $(CC) -c $< -o $@ -I./include $(CFLAGS)

games/%.o: games/%.c $(HEADERS)
	LC_ALL=C $(CC) -c $< -o $@ -I./include $(CFLAGS)

utils/%.o: utils/%.c $(HEADERS)
	LC_ALL=C $(CC) -c $< -o $@ -I./include $(CFLAGS)

clean:
	rm -f build/puzzles.app icons/icons.c games/*.o utils/*.o frontend/*.o

