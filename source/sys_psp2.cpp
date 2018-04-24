#include "wl_def.h"
#include <vitaGL.h>
#include <imgui_vita.h>

int _newlib_heap_size_user = 192 * 1024 * 1024;

bool avail[6];
uint64_t tmr1;
uint64_t tick = 0;
int frames = 0;
int _fps = 0;
bool inf_ammo = false;
bool bilinear = true;
bool vflux_window = false;
bool vflux_enabled = false;
float vcolors[3];
uint16_t *vindices;
float *colors;
float *vertices;

void ImGui_callback() {

	uint64_t t_tick = sceKernelGetProcessTimeWide();
	if ((t_tick - tick) > 1000000){
		_fps = frames;
		frames = 0;
		tick = t_tick;
	}
	frames++;
	
	ImGui_ImplVitaGL_NewFrame();
	
	if (ImGui::BeginMainMenuBar()){
		
		if (ImGui::BeginMenu("Launcher")){
			if (ImGui::MenuItem("Launch Wolfenstein 3D Shareware", nullptr, false, avail[0])){
				sceAppMgrLoadExec("app0:/eboot00.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D Full", nullptr, false, avail[1])){
				sceAppMgrLoadExec("app0:/eboot10.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Shareware", nullptr, false, avail[2])){
				sceAppMgrLoadExec("app0:/eboot20.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Full", nullptr, false, avail[3])){
				sceAppMgrLoadExec("app0:/eboot30.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Mission 2: Return to Danger", nullptr, false, avail[4])){
				sceAppMgrLoadExec("app0:/eboot22.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Mission 3: Ultimate Challenge", nullptr, false, avail[5])){
				sceAppMgrLoadExec("app0:/eboot23.bin", NULL, NULL);
			}
			if (ImGui::MenuItem("Exit vitaWolfen")){
				sceKernelExitProcess(0);
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Graphics")){
			if (ImGui::MenuItem("Bilinear Filter", nullptr, bilinear)){
				bilinear = !bilinear;
				SDL_SetVideoModeBilinear(bilinear);
			}
			if (ImGui::MenuItem("vFlux Config", nullptr, vflux_window)){
				vflux_window = !vflux_window;
			}
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Cheats")){
			if (ImGui::MenuItem("God Mode", nullptr, godmode)){
				godmode = !godmode;
			}
			if (ImGui::MenuItem("Unlock All Weapons")){
				GiveWeapon(gamestate.bestweapon+1);
				GiveWeapon(gamestate.bestweapon+1);
			}
			if (ImGui::MenuItem("Unlimited Ammo", nullptr, inf_ammo)){
				inf_ammo = !inf_ammo;
			}
			if (ImGui::MenuItem("Max Health")){
				HealSelf(99);
			}
			if (ImGui::MenuItem("Give 100.000 Points")){
				GivePoints(100000);
			}
			if (ImGui::MenuItem("Complete current stage")){
				playstate = ex_completed;
			}
			ImGui::EndMenu();
		}
		
		if (vflux_window){
			ImGui::Begin("vFlux Configuration", &vflux_window);
			ImGui::ColorPicker3("Filter Color", vcolors);
			ImGui::Checkbox("Enable vFlux", &vflux_enabled);
			ImGui::End();
		}
		
		ImGui::SameLine();
		ImGui::SetCursorPosX(870);
		
		ImGui::Text("FPS: %d", _fps); 
		ImGui::EndMainMenuBar();
	}
	
	if (vflux_enabled) {
		memcpy(&colors[0], vcolors, sizeof(float) * 3);
		memcpy(&colors[4], vcolors, sizeof(float) * 3);
		memcpy(&colors[8], vcolors, sizeof(float) * 3);
		memcpy(&colors[12], vcolors, sizeof(float) * 3);
		
		float a;
		SceDateTime time;
		sceRtcGetCurrentClockLocalTime(&time);
		if (time.hour < 6)		// Night/Early Morning
			a = 0.25f;
		else if (time.hour < 10) // Morning/Early Day
			a = 0.1f;
		else if (time.hour < 15) // Mid day
			a = 0.05f;
		else if (time.hour < 19) // Late day
			a = 0.15f;
		else // Evening/Night
			a = 0.2f;
		colors[3] = colors[7] = colors[11] = colors[15] = a;
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_VERTEX_ARRAY);
		vglIndexPointerMapped(vindices);
		vglVertexPointerMapped(vertices);
		vglColorPointerMapped(GL_FLOAT, colors);
		vglDrawObjects(GL_TRIANGLE_FAN, 4, true);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
	
	if (inf_ammo) {
		gamestate.ammo = 99;
	}
	
}

void ImGui_SetCallback() {
	
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
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sd2", SCE_O_RDONLY, 0777);
	avail[4] = (fd >= 0);
	sceIoClose(fd);
	fd = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sd3", SCE_O_RDONLY, 0777);
	avail[5] = (fd >= 0);
	sceIoClose(fd);
	
	scePowerSetArmClockFrequency(444);
	scePowerSetBusClockFrequency(222);
	scePowerSetGpuClockFrequency(222);
	scePowerSetGpuXbarClockFrequency(166);
	
	vindices = (uint16_t*)malloc(sizeof(uint16_t)*4);
	colors = (float*)malloc(sizeof(float)*4*4);
	vertices = (float*)malloc(sizeof(float)*3*4);
	vertices[0] =   0.0f;
	vertices[1] =   0.0f;
	vertices[2] =   0.0f;
	vertices[3] = 960.0f;
	vertices[4] =   0.0f;
	vertices[5] =   0.0f;
	vertices[6] = 960.0f;
	vertices[7] = 544.0f;
	vertices[8] =   0.0f;
	vertices[9] =   0.0f;
	vertices[10]= 544.0f;
	vertices[11]=   0.0f;
	vindices[0] = 0;
	vindices[1] = 1;
	vindices[2] = 2;
	vindices[3] = 3;
	
	ImGui::CreateContext();
	ImGui_ImplVitaGL_Init();
	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui_ImplVitaGL_KeysUsage(false);
	ImGui_ImplVitaGL_UseIndirectFrontTouch(true);
	ImGui::StyleColorsDark();
	ImGui::GetIO().MouseDrawCursor = false;
	
	SDL_SetVideoCallback(ImGui_callback);
	
}
