@echo off
rem ......................
echo Configurando Entorno
rem ......................
call setenv.bat
rem Compila map.json
rem echo Compilando Mapa (utils\mappack)
rem utils\mappack.exe assets/map.json 16 11  > nul
rem move map.index assets/map.index > nul
rem move map.pack assets/map.pack > nul
rem move map.txt assets/map.txt > nul
rem ......................
echo Compilando Cargador Basic (utils\bas2tap)
rem ......................
utils\bas2tap -a10 -sPARSER loader.bas loader.tap > nul
rem .....-................
echo Compilando Proyecto C  (zcc)
rem ......................
zcc +zx -vn -SO3 -startup=1 -clib=sdcc_iy --max-allocs-per-node10000 game.c include/prisma.c include/steeve.c assets.asm -o game
rem .....-................
echo Creando Bloque Tap de Carga (z88dk-appmake)
rem ......................
z88dk-appmake +zx -b assets\screen.scr -o screen.tap --blockname SCREEN --org 16384 --noloader	
rem .....-................
echo Creando Bloque Tap Nirvana (z88dk-appmake)
rem ......................
z88dk-appmake +zx -b game_NIRVANAP.bin -o nirvanap.tap --noloader --org 56323 --blockname NIRVANAP
rem .....-................
echo Creando Bloque Tap Codigo (z88dk-appmake)
rem ......................
z88dk-appmake +zx -b game_CODE.bin -o code.tap --noloader --org 32768 --blockname CODE
rem .....-................
echo Generando Tap Final (copy)
rem ......................
copy /b loader.tap + screen.tap + nirvanap.tap + code.tap tap\game.tap >> foo
rem ......................
rem # BORRANDO TEMPORALES
rem ......................
del foo
del *.tap
del *.bin
rem .....-................
echo Lanzando Juego en Emulador (tap\game.tap)
rem ......................
tap\game.tap

echo Hecho!