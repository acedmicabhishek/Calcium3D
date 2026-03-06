#include "Behavior.h"
#include "BehaviorRegistry.h"
#include <imgui.h>
#include <string>

class FlightHUD : public Behavior {
public:
    void OnUI() override {
        
        float speed = 0.0f;
        float altitude = 0.0f;
        bool afterburner = false;

        auto propSpeed = GetProperty("aircraft.speed");
        auto propAlt = GetProperty("aircraft.altitude");
        auto propAB = GetProperty("aircraft.afterburner");

        if (propSpeed.has_value()) speed = std::any_cast<float>(propSpeed);
        if (propAlt.has_value())   altitude = std::any_cast<float>(propAlt);
        if (propAB.has_value())    afterburner = std::any_cast<bool>(propAB);

        
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | 
                                      ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
                                      ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
        
        const float PAD = 10.0f;
        ImVec2 base_pos = ImGui::GetCursorScreenPos();
        float v_height = ImGui::GetWindowHeight();
        
        
        ImVec2 window_pos = ImVec2(base_pos.x + PAD, base_pos.y + v_height - PAD);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.35f);

        if (ImGui::Begin("FlightHUD", nullptr, window_flags)) {
            ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), " SYSTEM STATUS: J-20 INTERCEPTOR ");
            ImGui::Separator();
            
            float speedKph = speed * 3.6f;
            ImGui::Text(" AIRSPEED  >"); ImGui::SameLine();
            if (afterburner) 
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), " %.1f km/h [AFTERBURNER]", speedKph);
            else 
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), " %.1f km/h", speedKph);

            ImGui::Text(" ALTITUDE  >"); ImGui::SameLine();
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), " %.1f m", altitude);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.00f), " CONTROL MAPPING ");
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), " W/S: Core Thrust | Shift: Burner | A/D: Roll | Q/E: Pitch");
        }
        ImGui::End();
    }
};

REGISTER_BEHAVIOR(FlightHUD)
