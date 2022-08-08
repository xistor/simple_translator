#include "imgui.h"

void ShowMenuTheme() {
    if (ImGui::BeginMenu("Theme")) 
    { 
        if (ImGui::MenuItem("Dark"))
        {
            ImGui::StyleColorsDark();
        }
        if (ImGui::MenuItem("Light") )
        {
            ImGui::StyleColorsLight();
        }
        if (ImGui::MenuItem("Classic")) 
        {
            ImGui::StyleColorsClassic();
        }
        ImGui::EndMenu();
    }
}

void ShowMainMenuBar() {
     // show MenuBar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Setting"))
            {
                ShowMenuTheme();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
}

