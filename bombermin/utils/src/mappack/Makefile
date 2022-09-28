default:
	gcc funcs.c parson/parson.c zx7/compress.c zx7/optimize.c map_pack.c  -lm
ifeq ($(OS),Windows_NT)
	del mappack.exe
	ren a.exe mappack.exe
else
	mv a.out mappack
	chmod 777 mappack   
endif

clean:
ifeq ($(OS),Windows_NT)
	del mappack
	del a.out
	del *.o
	del *.exe
else
	rm -f mappack a.out *.o *.exe
endif


