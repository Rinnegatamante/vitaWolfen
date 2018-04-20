#include <vitasdk.h>
#include <vitaGL.h>
#include <imgui_vita.h>
#include <stdio.h>

#define VERSION "1.5"

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
	
	vglInit(0x1000);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplVitaGL_Init();
	ImGui_ImplVitaGL_TouchUsage(true);
	ImGui::StyleColorsDark();
	
	avail[0] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl1", SCE_O_RDONLY, 0777);
	avail[1] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.wl6", SCE_O_RDONLY, 0777);
	avail[2] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sdm", SCE_O_RDONLY, 0777);
	avail[3] = sceIoOpen("ux0:/data/Wolfenstein 3D/vswap.sod", SCE_O_RDONLY, 0777);

	while (exit_code == 0xDEAD){
		vglStartRendering();
		ImGui_ImplVitaGL_NewFrame();
		
		if (ImGui::BeginMainMenuBar()){
        if (ImGui::BeginMenu("Launcher")){
			if (ImGui::MenuItem("Launch Wolfenstein 3D Shareware", nullptr, false, avail[0] >= 0)){
				exit_code = 0;
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D Full", nullptr, false, avail[1] >= 0)){
				exit_code = 1;
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Shareware", nullptr, false, avail[2] >= 0)){
				exit_code = 2;
			}
			if (ImGui::MenuItem("Launch Wolfenstein 3D: Spear of Destiny Full", nullptr, false, avail[3] >= 0)){
				exit_code = 3;
			}
			if (ImGui::MenuItem("Exit vitaWolfen")){
				exit_code = 0xBEEF;
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
		ImGui::Begin("vitaWolfen Launcher", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
		ImGui::TextColored(ImVec4(255,255,0,255),"Welcome to vitaWolfen Launcher!");
		ImGui::Text("You're running vitaWolfen v.%s", VERSION);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Text("Author: Rinnegatamante");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(255,255,0,255),"Credits:");
		ImGui::Text("XandridFire for the awesome support on Patreon");
		ImGui::Text("Styde Pregny for the awesome support on Patreon");
		ImGui::Text("Billy McLaughlin II for the awesome support on Patreon");
		ImGui::Spacing();
		ImGui::Spacing();
		
		if (avail[0] >= 0){
			if (ImGui::Button("Launch Wolfenstein 3D Shareware")){
				exit_code = 0;
			}
		}
		if (avail[1] >= 0){
			if (ImGui::Button("Launch Wolfenstein 3D Full")){
				exit_code = 1;
			}
		}
		if (avail[2] >= 0){
			if (ImGui::Button("Launch Wolfenstein 3D: Spear of Destiny Shareware")){
				exit_code = 2;
			}
		}
		if (avail[3] >= 0){
			if (ImGui::Button("Launch Wolfenstein 3D: Spear of Destiny Full")){
				exit_code = 3;
			}
		}
		ImGui::End();
		
		glViewport(0, 0, static_cast<int>(ImGui::GetIO().DisplaySize.x), static_cast<int>(ImGui::GetIO().DisplaySize.y));
		ImGui::Render();
		ImGui_ImplVitaGL_RenderDrawData(ImGui::GetDrawData());
		vglStopRendering();
	}
	closeHandles();
	if (exit_code != 0xBEEF){
		char file[256];
		sprintf(file,"app0:/eboot%d.bin", exit_code);
		sceAppMgrLoadExec(file, NULL, NULL);
	}else{
		sceKernelExitProcess(0);
	}
	return 0;
}