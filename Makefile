all:
	make WMODE=0 -C source
	make clean -C source
	make WMODE=1 -C source
	make clean -C source
	make WMODE=2 -C source
	make clean -C source
	make WMODE=3 -C source
	make clean -C source
	make -C launcher
	cp launcher/vitaWolfen.vpk vitaWolfen.vpk
	
clean:
	make clean -C source
	make -C launcher clean