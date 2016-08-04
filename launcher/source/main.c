#include <psp2/appmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/types.h>
#include <psp2/ctrl.h>
#include <vita2d.h>
#include <stdio.h>
#include "font.h"

SceUID avail[4];

void closeHandles(){
	int i;
	for (i=0;i<4;i++){
		if (avail[i] >= 0) sceIoClose(avail[i]);
	}
}

int main(){
	SceCtrlData kDown;
	int exit_code = 0xDEAD;
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	avail[0] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl1", SCE_O_RDONLY, 0777);
	avail[1] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl6", SCE_O_RDONLY, 0777);
	avail[2] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sdm", SCE_O_RDONLY, 0777);
	avail[3] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sod", SCE_O_RDONLY, 0777);
	gpu_printf("Welcome on vitaWolfen launcher!");
	gpu_printf("-------------------------------");
	if (avail[0] >= 0) gpu_printf("Press Cross to open WOLF3D Shareware");
	if (avail[1] >= 0) gpu_printf("Press Circle to open WOLF3D Full");
	if (avail[2] >= 0) gpu_printf("Press Square to open SoD Demo");
	if (avail[3] >= 0) gpu_printf("Press Triangle to open SoD Full");
	gpu_printf("Press Start to exit");
	while (exit_code == 0xDEAD){
		sceCtrlPeekBufferPositive(0, &kDown, 1);
		if ((avail[0] >= 0) && (kDown.buttons & SCE_CTRL_CROSS)) exit_code = 0;
		else if ((avail[1] >= 0) && (kDown.buttons & SCE_CTRL_CIRCLE)) exit_code = 1;
		else if ((avail[2] >= 0) && (kDown.buttons & SCE_CTRL_SQUARE)) exit_code = 2;
		else if ((avail[3] >= 0) && (kDown.buttons & SCE_CTRL_TRIANGLE)) exit_code = 3;
		else if (kDown.buttons & SCE_CTRL_START) exit_code = 0xBEEF;
	}
	closeHandles();
	if (exit_code != 0xBEEF){
		char file[256];
		sprintf(file,"app0:/eboot%d.bin", exit_code);
		gpu_printf("-------------------------------");
		gpu_printf("Now launching %s...", file);
		sceAppMgrLoadExec(file, NULL, NULL);
	}else{
		gpu_printf("-------------------------------");
		gpu_printf("Exiting...");
		sceKernelExitProcess(0);
	}
	return 0;
}