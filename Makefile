
INCL = $(ZCCCFG)/../../include/_DEVELOPMENT/sdcc
ifeq ($(OS),Windows_NT)
	EP = '-e'
	ZCCCFG_CYG = $(shell cygpath -u $(ZCCCFG))
	Z88DK_PATH = $(shell echo $(ZCCCFG_CYG) | sed 's,/lib/config,,')
	Z88DK_PATH_WIN = $(shell echo $(ZCCCFG) | sed 's,/lib/config,,')
else
	EP = ''
	Z88DK_PATH = $(subst $(LIB),$(REP), $(ZCCCFG))
endif



# Parser Build Script
.PHONY: all test clean bas2tap

all:
#Exporta paths para evitar errores de commando se supone que el z88dk estan en $HOME/z88dk
	@export Z88DK=$HOME/z88dk
	@export PATH=$PATH:$Z88DK/bin
	@export Z80_OZFILES=$Z88DK/lib/
	@export ZCCCFG=$Z88DK/lib/config/
#Compila map.json se deben proporcionar el ancho y alto de habitación
#	@echo 'Compilando Mapa (utils\mappack)'
#	@./utils/mappack assets/map.json 16 11  > /dev/null
#	@mv map.index assets/map.index
#	@mv map.pack assets/map.pack
#	@mv map.txt assets/map.txt
#Crea Tap loader del loader.bas
	@echo 'Compilando Cargador Basic (utils\bas2tap)'
	@./utils/bas2tap -a10 -sPARSER loader.bas loader.tap > /dev/null
#Empaqueta Pantalla de Carga
	@z88dk-appmake +zx -b assets/screen.scr -o screen.tap --blockname SCREEN --org 16384 --noloader		
#Compila Juego
	@echo 'Compilando Proyecto C  (zcc)'
	@zcc +zx -vn -SO3 -startup=1 -clib=sdcc_iy --max-allocs-per-node1000 game.c include/prisma.c include/steeve.c assets.asm -o game
	@z88dk-appmake +zx -b game_NIRVANAP.bin -o nirvanap.tap --noloader --org 56323 --blockname NIRVANAP
	@z88dk-appmake +zx -b game_CODE.bin -o code.tap --noloader --org 32768 --blockname CODE
#Concatena el tap final
	@cat loader.tap screen.tap nirvanap.tap code.tap   > tap/game.tap
	@rm *.tap
	@rm *.bin
#Launch on wsl
	@wslview tap/game.tap	
clean:
	@rm *.tap 
	@rm *.bin



bas2tap:
	@echo Building bas2tap
	make -C ./bas2tap_src/
	cp ./bas2tap_src/bas2tap ./utils/bas2tap