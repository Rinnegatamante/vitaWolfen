#include "wl_def.h"
#include <vitaGL.h>
#include <imgui_vita.h>

int _newlib_heap_size_user = 192 * 1024 * 1024;

bool avail[6];
uint64_t tmr1;
uint64_t tick = 0;
int frames = 0;
int _fps = 0;

void ImGui_callback() {

	uint64_t t_tick = sceKernelGetProcessTimeWide();
	if ((t_tick - tick) > 1000000){
		_fps = frames;
		frames = 0;
		tick = t_tick;
	}
	frames++;
	
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	uint64_t delta_touch = sceKernelGetProcessTimeWide() - tmr1;
	if (touch.reportNum > 0){
		ImGui::GetIO().MouseDrawCursor = true;
		tmr1 = sceKernelGetProcessTimeWide();
	}else if (delta_touch > 1000000) ImGui::GetIO().MouseDrawCursor = false;
	
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
		ImGui::SameLine();
		ImGui::SetCursorPosX(870);
		
		ImGui::Text("FPS: %d", _fps); 
		ImGui::EndMainMenuBar();
	}
	
	ImGui::Render();
	ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
	
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
	
	ImGui::CreateContext();
	ImGui_ImplVitaGL_Init();
	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui_ImplVitaGL_KeysUsage(false);
	ImGui_ImplVitaGL_UseIndirectFrontTouch(true);
	ImGui::StyleColorsDark();
	ImGui::GetIO().MouseDrawCursor = false;
	
	SDL_SetVideoCallback(ImGui_callback);
	
}
