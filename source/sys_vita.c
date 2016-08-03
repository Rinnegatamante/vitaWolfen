#include "include/wl_def.h"
#include <vita2d.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <stdio.h>

byte *gfxbuf = NULL;
static unsigned char pal[768];

extern void keyboard_handler(int code, int press);
extern boolean InternalKeyboard[NumCodes];

SceUInt16 d_8to16table[256];
int vwidth = 960;
int vheight = 544;
int vstride = 960;
int camera_x, move_x, move_y;
char path[256];
vita2d_texture* tex_buffer;

int main (int argc, signed char *argv[])
{

	// Init GPU
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x00, 0x00, 0x00, 0xFF));
	vita2d_set_vblank_wait(0);
	
	// Set main directory
	strcpy(path,"ux0:/data/Wolfenstein 3D/");
	
	// Enabling 444 MHZ mode and Analogs support
	scePowerSetArmClockFrequency(444);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);
	
	return WolfMain(argc, argv);
}

void DisplayTextSplash(byte *text);

/*
==========================
=
= Quit
=
==========================
*/

void exitGame(){
	vita2d_free_texture(tex_buffer);
	vita2d_fini();
	sceKernelExitProcess(0);
}

void Quit(signed char *error)
{
	memptr screen = NULL;

	if (!error || !*error) {
		CA_CacheGrChunk(ORDERSCREEN);
		screen = grsegs[ORDERSCREEN];
		WriteConfig();
	} else if (error) {
		CA_CacheGrChunk(ERRORSCREEN);
		screen = grsegs[ERRORSCREEN];
	}

	ShutdownId();

	if (screen) {
		//DisplayTextSplash(screen);
	}
	
	if (error && *error) {
		printf("Fatal Error: %s\nPress X to exit.", error);
		SceCtrlData pad;
		do{ sceCtrlPeekBufferPositive(0, &pad, 1); }while(!(pad.buttons & SCE_CTRL_CROSS));				
 	}
	
	exitGame();
}

void VL_WaitVBL(int vbls)
{
	long last = get_TimeCount() + vbls;
	while (last > get_TimeCount()) ;
}

void VW_UpdateScreen()
{

	vita2d_start_drawing();
	vita2d_draw_texture(tex_buffer, 0, 0);
	vita2d_end_drawing();
	vita2d_swap_buffers();
	sceDisplayWaitVblankStart();
}

/*
=======================
=
= VL_Startup
=
=======================
*/

void VL_Startup()
{	
	
	// Init GPU texture
	tex_buffer = vita2d_create_empty_texture_format(vwidth, vheight, SCE_GXM_TEXTURE_BASE_FORMAT_P8);
	gfxbuf = vita2d_texture_get_datap(tex_buffer);

}

/*
=======================
=
= VL_Shutdown
=
=======================
*/

void VL_Shutdown()
{
}

/* ======================================================================== */

/*
=================
=
= VL_SetPalette
=
=================
*/

void VL_SetPalette(const byte *palette)
{
	VL_WaitVBL(1);
	int i;
	SceUInt32* palette_tbl = vita2d_texture_get_palette(tex_buffer);
	const SceUInt8* pal = palette;
	unsigned r, g, b;
	
	for(i=0; i<256; i++){
		r = pal[0] << 2;
		g = pal[1] << 2;
		b = pal[2] << 2;
		palette_tbl[i] = r | (g << 8) | (b << 16) | (0xFF << 24);
		pal += 3;
	}
	
}

/*
=================
=
= VL_GetPalette
=
=================
*/

void VL_GetPalette(byte *palette)
{
	memcpy(palette, pal, 768);
}

/*
=================
=
= INL_Update
=
=================
*/
static int mx = 0;
static int my = 0;
static int weapon;
void INL_SetKeys(SceUInt32 keys, SceUInt32 state){
	if( keys & SCE_CTRL_SELECT){ // Swap Weapons / Confirm Savegames
		if (state == 1){
			weapon = gamestate.weapon;
			if (gamestate.weapon == 0){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 1);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 0);
			}else if (gamestate.weapon == 1){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 1); 
				keyboard_handler(sc_4, 0);
			}else if (gamestate.weapon == 2){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 1);
			}else{
				keyboard_handler(sc_1, 1);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 0);
			}
		}else{
			if (gamestate.weapon == weapon) keyboard_handler(sc_1, 1); 
			else keyboard_handler(sc_1, 0); 
			keyboard_handler(sc_2, 0);
			keyboard_handler(sc_3, 0); 
			keyboard_handler(sc_4, 0);
		}
		keyboard_handler(sc_Enter, state);
	}
	if( keys & SCE_CTRL_CROSS){ // Yes button/Fire
		keyboard_handler(sc_Y, state); 
		keyboard_handler(sc_Control, state);
	}
	if( keys & SCE_CTRL_TRIANGLE){ // Run
		keyboard_handler(sc_RShift, state);
	}
	if( keys & SCE_CTRL_SQUARE){ // Open/Operate
		keyboard_handler(sc_Space, state);
	}
	if( keys & SCE_CTRL_CIRCLE){ // Back
		keyboard_handler(sc_BackSpace, state);
		keyboard_handler(sc_N, state);
	}
	if( keys & SCE_CTRL_START){ // Open menu
		keyboard_handler(sc_Escape, state);
	}
	if( keys & SCE_CTRL_UP){
		keyboard_handler(sc_UpArrow, state);
	}
	if( keys & SCE_CTRL_DOWN){
		keyboard_handler(sc_DownArrow, state);
	}
	if( keys & SCE_CTRL_LEFT){
		keyboard_handler(sc_LeftArrow, state);
	}
	if( keys & SCE_CTRL_RIGHT){
		keyboard_handler(sc_RightArrow, state);
	}
	if( keys & SCE_CTRL_LTRIGGER){ // Change weapon
		if (state == 1){
			weapon = gamestate.weapon;
			if (gamestate.weapon == 0){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 1);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 0);
			}else if (gamestate.weapon == 1){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 1); 
				keyboard_handler(sc_4, 0);
			}else if (gamestate.weapon == 2){
				keyboard_handler(sc_1, 0);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 1);
			}else{
				keyboard_handler(sc_1, 1);
				keyboard_handler(sc_2, 0);
				keyboard_handler(sc_3, 0); 
				keyboard_handler(sc_4, 0);
			}
		}else{
			if (gamestate.weapon == weapon) keyboard_handler(sc_1, 1); 
			else keyboard_handler(sc_1, 0); 
			keyboard_handler(sc_2, 0);
			keyboard_handler(sc_3, 0); 
			keyboard_handler(sc_4, 0);
		}
		keyboard_handler(sc_Enter, state);
	}
	if( keys & SCE_CTRL_RTRIGGER){ // Yes button/Fire
		keyboard_handler(sc_Y, state); 
		keyboard_handler(sc_Control, state);
	}
	/*if (keys & KEY_TOUCH){ // Touchscreen support
		if (state == 1) INL_ReadTouch();
		else{
			keyboard_handler(sc_LeftArrow, 0);
			keyboard_handler(sc_RightArrow, 0);
		}
	}*/
}

/*void INL_ReadTouch(){
	touchPosition cpos;
	hidTouchRead(&cpos);
	if (cpos.px < 160){
		keyboard_handler(sc_LeftArrow, 1);
		keyboard_handler(sc_RightArrow, 0);
	}else{
		keyboard_handler(sc_RightArrow, 1);
		keyboard_handler(sc_LeftArrow, 0);
	}
}*/

void INL_Update()
{

	static int multiplex = 0;
	static int mu = 0;
	static int md = 0;
	
	SceCtrlData kDown, kUp;
	sceCtrlPeekBufferPositive(0, &kDown, 1);
	sceCtrlPeekBufferNegative(0, &kUp, 1);
	if(kUp.buttons)
		INL_SetKeys(kUp.buttons, 0);
	if(kDown.buttons)
		INL_SetKeys(kDown.buttons, 1);
	
	// Analogs Support
	int left_x = kDown.lx - 127;
	int left_y = kDown.ly - 127;
	int right_x = kDown.rx - 127;
	camera_x = (abs(right_x) < 10 ? 0 : right_x) * 10 /(13-mouseadjustment);
	move_x = (abs(left_x) < 10 ? 0 : left_x >> 1) * 10 /(13-mouseadjustment);
	move_y = (abs(left_y) < 15 ? 0 : left_y >> 1) * 10 /(13-mouseadjustment);
	
}

void IN_GetMouseDelta(int *dx, int *dy)
{
	if (dx)
		*dx = mx;
	if (dy)
		*dy = my;
}

byte IN_MouseButtons()
{
	return 0;
}
