all:
	make WMODE=0 -f Makefile.core
	make clean -f Makefile.core
	make WMODE=1 -f Makefile.core
	make clean -f Makefile.core
	make WMODE=2 -f Makefile.core
	make clean -f Makefile.core
	make WMODE=3 -f Makefile.core
	make clean -f Makefile.core
	make -C launcher
	cp launcher/vitaWolfen.vpk vitaWolfen.vpk
	
clean:
	make clean -f Makefile.core
	make -C launcher clean