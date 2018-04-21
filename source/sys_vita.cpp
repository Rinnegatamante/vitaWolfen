extern "C"{
	#include "include/wl_def.h"
}
#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <stdio.h>

byte *gfxbuf = NULL;
static unsigned char pal[768];

SceUInt16 d_8to16table[256];
int vwidth = 480;
int vheight = 272;
int vstride = 480;
int camera_x, move_x, move_y;
char path[256];
bool avail[4];
GLuint fs, vs, program;

int main (int argc, char ** argv)
{
	// Checking for available games
	SceUID fd;
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl1", SCE_O_RDONLY, 0777);
	avail[0] = (fd >= 0);
	sceIoClose(fd);
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl6", SCE_O_RDONLY, 0777);
	avail[1] = (fd >= 0);
	sceIoClose(fd);
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sdm", SCE_O_RDONLY, 0777);
	avail[2] = (fd >= 0);
	sceIoClose(fd);
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sod", SCE_O_RDONLY, 0777);
	avail[3] = (fd >= 0);
	sceIoClose(fd);
	
	// Init vitaGL
	vglInit(0x100000);
	
	// Init ImGui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplVitaGL_Init();
	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui_ImplVitaGL_KeysUsage(false);
    ImGui_ImplVitaGL_UseIndirectFrontTouch(true);
	ImGui::StyleColorsDark();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	io.MouseDrawCursor = false;
	
	// Set main directory
	strcpy(path,"ux0:/data/Wolfenstein 3D/");
	
	// Enabling 444 MHZ mode and Analogs support
	scePowerSetArmClockFrequency(444);
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
	
	return WolfMain(argc, (signed char **)argv);
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

GLuint texture;
uint32_t *tex_buffer;
SceUInt32 palette_tbl[256];
uint64_t tmr1;

void VW_UpdateScreen()
{
	for (int y = 0; y < vheight; y++){
		for (int x = 0; x < vwidth; x++){
			int i = x + y * vwidth;
			tex_buffer[i] = palette_tbl[gfxbuf[i]];
		}
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, vwidth, vheight, GL_RGBA, GL_UNSIGNED_BYTE, tex_buffer);
	vglStartRendering();
	ImGui_ImplVitaGL_NewFrame();
	if (ImGui::BeginMainMenuBar()){
        if (ImGui::BeginMenu("Launcher")){
            if (ImGui::MenuItem("Launch Wolfenstein 3D Shareware", nullptr, false, avail[0])){
				sceAppMgrLoadExec("app0:/eboot0.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D Full", nullptr, false, avail[1])){
				sceAppMgrLoadExec("app0:/eboot1.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Shareware", nullptr, false, avail[2])){
				sceAppMgrLoadExec("app0:/eboot2.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Full", nullptr, false, avail[3])){
				sceAppMgrLoadExec("app0:/eboot3.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Exit vitaWolfen")){
				sceKernelExitProcess(0);
			}
            ImGui::EndMenu();
        }
		ImGui::SameLine();
		ImGui::SetCursorPosX(870);
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate); 
        ImGui::EndMainMenuBar();
	}
	ImGui::SetNextWindowPos(ImVec2(0, 19), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(960, 525), ImGuiSetCond_Always);
	ImGui::Begin("", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::Image(reinterpret_cast<void *>(texture), ImVec2(960, 525));
	ImGui::End();
	glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
	vglStopRendering();
    
    SceTouchData touch;
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
    uint64_t delta_touch = sceKernelGetProcessTimeWide() - tmr1;
    if (touch.reportNum > 0){
        ImGui::GetIO().MouseDrawCursor = true;
        tmr1 = sceKernelGetProcessTimeWide();
    }else if (delta_touch < 1000000) ImGui::GetIO().MouseDrawCursor = false;
	
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
	glGenTextures(1, &texture);
	tex_buffer = (uint32_t*)malloc(vwidth * vheight * 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vwidth, vheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_buffer);
	gfxbuf = (byte*)malloc(vwidth * vheight);

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
	vglEnd();
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
uint32_t oldpad;
void INL_SetKeys(SceUInt32 keys, SceUInt32 state){
	if(keys & SCE_CTRL_SELECT){ // Swap Weapons / Confirm Savegames
        gamestate.weapon =
		gamestate.chosenweapon =
        gamestate.bestweapon = wp_knife;
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
			keyboard_handler(sc_1, 0); 
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
}

void INL_Update()
{

	static int multiplex = 0;
	static int mu = 0;
	static int md = 0;
	
    SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
    uint32_t kDown = pad.buttons;
	uint32_t kUp = oldpad;
    if(kDown)
		INL_SetKeys(kDown, 1);
	if(kUp != kDown)
		INL_SetKeys(kUp, 0);
	
    oldpad = pad.buttons;
    
	// Analogs Support
	int left_x = pad.lx - 127;
	int left_y = pad.ly - 127;
	int right_x = pad.rx - 127;
	camera_x = (abs(right_x) < 50 ? 0 : right_x) * 8 /(13-mouseadjustment);
	move_x = (abs(left_x) < 50 ? 0 : left_x >> 1) * 8 /(13-mouseadjustment);
	move_y = (abs(left_y) < 50 ? 0 : left_y >> 1) * 8 /(13-mouseadjustment);
	
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
