#include "Console.h"
#include "Camera.h"
#include "../Renderer/RenderContext.h"
#include "../Physics/PhysicsEngine.h"
#include "../Physics/HitboxGraphics.h"
#include "StateManager.h"
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdarg>
#include <cstring>
#include <algorithm>
#include <sstream>

Console::Console() {}
Console::~Console() {}

void Console::Init() {
    AddLog("Calcium3D Runtime Console");
    AddLog("Type /help for available commands");
    AddLog("Press ~ to toggle console");
    AddLog("──────────────────────────────────");
}

void Console::Toggle() {
    m_Open = !m_Open;
    if (m_Open) {
        m_ReclaimFocus = true;
    }
}

void Console::AddLog(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    m_Log.push_back(std::string(buf));
    m_ScrollToBottom = true;
}

void Console::AddEngineLog(const std::string& msg) {
    if (m_EngineLoggingEnabled) {
        AddLog("[ENGINE] %s", msg.c_str());
    }
}

void Console::ExecuteCommand(const std::string& cmd) {
    AddLog("> %s", cmd.c_str());

    
    std::string lower = cmd;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    
    std::string parsed = lower;
    if (!parsed.empty() && parsed[0] == '/') {
        parsed = parsed.substr(1);
    }

    if (parsed == "help") {
        AddLog("  Available Commands:");
        AddLog("  /settingsCamera     — Camera controls (speed, FOV, sensitivity)");
        AddLog("  /settingsRendering  — Rendering settings (wireframe, MSAA, VSync)");
        AddLog("  /settingsPhysics    — Physics engine configuration");
        AddLog("  /settingsEnv        — Environment settings (sun, moon, time, sky)");
        AddLog("  /clear              — Clear console log");
        AddLog("  /close              — Close console");
        AddLog("  /fps                — Show current FPS");
        AddLog("  /wireframe [on|off] — Toggle wireframe mode");
        AddLog("  /vsync [on|off]     — Toggle VSync");
        AddLog("  /timescale <value>  — Set time speed multiplier");
        AddLog("  /pause              — Pause/unpause time");
        AddLog("  /clouds [on|off]    — Toggle clouds");
        AddLog("  /fov <value>        — Set camera FOV");
        AddLog("  /dynamicsky [on|off]— Toggle dynamic sky mode");
        AddLog("  /ms [on|off]        — Toggle Master Control (Free Camera)");
        AddLog("  /hitbox [on|off]    — Toggle Hitbox/AABB Rendering");
        AddLog("  /enable logging     — Toggle internal Engine event logs");
    }
    else if (parsed == "enable logging") {
        m_EngineLoggingEnabled = !m_EngineLoggingEnabled;
        AddLog("  Engine Logging: %s", m_EngineLoggingEnabled ? "ON" : "OFF");
        if (m_EngineLoggingEnabled) {
            AddLog("  Current Target App State: %s", StateManager::GetCurrentStateName().c_str());
            AddLog("  State Registrations: ");
            for (auto& s : StateManager::GetRegisteredStateNames()) {
                AddLog("    - %s", s.c_str());
            }
        }
    }
    else if (parsed.rfind("log ", 0) == 0) {
        std::string arg = parsed.substr(4);
        if (arg == "true" || arg == "on") {
            m_ActivePanel = SettingsPanel::Logging;
            m_EngineLoggingEnabled = true;
            AddLog("  Opened Logging Screen & Enabled Diagnostics");
        } else if (arg == "false" || arg == "off") {
            if (m_ActivePanel == SettingsPanel::Logging) m_ActivePanel = SettingsPanel::None;
            m_EngineLoggingEnabled = false;
            AddLog("  Closed Logging Screen & Disabled Diagnostics");
        }
    }
    else if (parsed == "settingscamera") {
        m_ActivePanel = SettingsPanel::Camera;
        AddLog("  Opened Camera Settings panel");
    }
    else if (parsed == "settingsrendering") {
        m_ActivePanel = SettingsPanel::Rendering;
        AddLog("  Opened Rendering Settings panel");
    }
    else if (parsed == "settingsphysics") {
        m_ActivePanel = SettingsPanel::Physics;
        AddLog("  Opened Physics Settings panel");
    }
    else if (parsed == "settingsenv") {
        m_ActivePanel = SettingsPanel::Environment;
        AddLog("  Opened Environment Settings panel");
    }
    else if (parsed == "clear") {
        m_Log.clear();
    }
    else if (parsed == "close") {
        m_Open = false;
        m_ActivePanel = SettingsPanel::None;
    }
    else if (parsed == "fps") {
        AddLog("  FPS: %.1f (%.3f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    }
    else if (parsed == "pause") {
        m_TimePaused = !m_TimePaused;
        AddLog("  Time %s", m_TimePaused ? "PAUSED" : "RESUMED");
    }
    else if (parsed.rfind("wireframe", 0) == 0) {
        std::string arg = (parsed.size() > 10) ? parsed.substr(10) : "";
        if (arg == "on") m_Wireframe = true;
        else if (arg == "off") m_Wireframe = false;
        else m_Wireframe = !m_Wireframe;
        
        glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
        AddLog("  Wireframe: %s", m_Wireframe ? "ON" : "OFF");
    }
    else if (parsed.rfind("vsync", 0) == 0) {
        std::string arg = (parsed.size() > 6) ? parsed.substr(6) : "";
        if (arg == "on") m_VSync = true;
        else if (arg == "off") m_VSync = false;
        else m_VSync = !m_VSync;
        
        glfwSwapInterval(m_VSync ? 1 : 0);
        AddLog("  VSync: %s", m_VSync ? "ON" : "OFF");
    }
    else if (parsed.rfind("timescale", 0) == 0) {
        std::string arg = (parsed.size() > 10) ? parsed.substr(10) : "";
        try {
            float val = std::stof(arg);
            m_TimeSpeed = val;
            AddLog("  Time scale set to: %.2f", m_TimeSpeed);
        } catch (...) {
            AddLog("  [ERROR] Usage: /timescale <value>");
        }
    }
    else if (parsed.rfind("skybox", 0) == 0) {
        std::string arg = (parsed.size() > 7) ? parsed.substr(7) : "";
        if (arg == "on") m_ShowSkybox = true;
        else if (arg == "off") m_ShowSkybox = false;
        else m_ShowSkybox = !m_ShowSkybox;
        AddLog("  Skybox: %s", m_ShowSkybox ? "ON" : "OFF");
    }
    else if (parsed.rfind("clouds", 0) == 0) {
        std::string arg = (parsed.size() > 7) ? parsed.substr(7) : "";
        if (arg == "on") m_ShowClouds = true;
        else if (arg == "off") m_ShowClouds = false;
        else m_ShowClouds = !m_ShowClouds;
        AddLog("  Clouds: %s", m_ShowClouds ? "ON" : "OFF");
    }
    else if (parsed.rfind("fov", 0) == 0) {
        
        std::string arg = (parsed.size() > 4) ? parsed.substr(4) : "";
        AddLog("  Use /settingsCamera panel to adjust FOV interactively");
    }
    else if (parsed.rfind("ms", 0) == 0) {
        std::string arg = (parsed.size() > 2) ? parsed.substr(3) : ""; 
        if (arg == "on") m_MasterControl = true;
        else if (arg == "off") m_MasterControl = false;
        else m_MasterControl = !m_MasterControl;
        AddLog("  Master Control: %s", m_MasterControl ? "ON" : "OFF");
    }
    else if (parsed.rfind("hitbox", 0) == 0) {
        std::string arg = (parsed.size() > 7) ? parsed.substr(7) : "";
        if (arg == "on") m_ShowHitboxes = true;
        else if (arg == "off") m_ShowHitboxes = false;
        else m_ShowHitboxes = !m_ShowHitboxes;
        AddLog("  Hitboxes: %s", m_ShowHitboxes ? "ON" : "OFF");
    }
    else if (parsed.rfind("dynamicsky", 0) == 0) {
        std::string arg = (parsed.size() > 11) ? parsed.substr(11) : "";
        if (arg == "on") {
            m_ShowGradientSky = true;
            m_ShowSkybox = false;
        } else if (arg == "off") {
            m_ShowGradientSky = false;
            m_ShowSkybox = true;
        } else {
            m_ShowGradientSky = !m_ShowGradientSky;
            m_ShowSkybox = !m_ShowGradientSky;
        }
        AddLog("  Dynamic Sky: %s", m_ShowGradientSky ? "ON" : "OFF");
    }
    else {
        AddLog("  [ERROR] Unknown command: '%s'. Type /help for available commands.", cmd.c_str());
    }
}




void Console::Render(Camera* camera, RenderContext& ctx) {
    if (!m_Open) return;

    ImGuiIO& io = ImGui::GetIO();
    float consoleHeight = io.DisplaySize.y * 0.45f;

    
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, consoleHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.08f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.80f, 0.40f, 0.60f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.08f, 0.12f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.10f, 0.20f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.92f, 0.85f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.10f, 0.10f, 0.14f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.05f, 0.05f, 0.08f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.25f, 0.60f, 0.30f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("Console", nullptr, flags);

    
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.40f, 0.85f, 0.45f, 1.0f));
    ImGui::Text("Calcium3D Console");
    ImGui::PopStyleColor();
    ImGui::SameLine(ImGui::GetWindowWidth() - 220);
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%.1f FPS | ~ to close", io.Framerate);
    ImGui::Separator();

    
    float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ConsoleLog", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& line : m_Log) {
        
        ImVec4 color = ImVec4(0.85f, 0.92f, 0.85f, 1.0f); 

        if (line.find("[ERROR]") != std::string::npos) {
            color = ImVec4(1.0f, 0.35f, 0.35f, 1.0f); 
        } else if (line.find(">") == 0) {
            color = ImVec4(0.45f, 0.90f, 0.50f, 1.0f); 
        } else if (line.find("  ") == 0) {
            color = ImVec4(0.70f, 0.80f, 0.95f, 1.0f); 
        } else if (line.find("──") != std::string::npos) {
            color = ImVec4(0.35f, 0.55f, 0.35f, 0.6f); 
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(line.c_str());
        ImGui::PopStyleColor();
    }

    if (m_ScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        m_ScrollToBottom = false;
    }

    ImGui::EndChild();

    
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.12f, 1.0f));
    ImGui::PushItemWidth(-1);

    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.90f, 0.50f, 1.0f));
    if (ImGui::InputText("##ConsoleInput", m_InputBuf, sizeof(m_InputBuf), inputFlags)) {
        if (m_InputBuf[0] != '\0') {
            ExecuteCommand(std::string(m_InputBuf));
            m_InputBuf[0] = '\0';
        }
        m_ReclaimFocus = true;
    }
    ImGui::PopStyleColor();

    ImGui::PopItemWidth();
    ImGui::PopStyleColor();

    
    if (m_ReclaimFocus) {
        ImGui::SetKeyboardFocusHere(-1);
        m_ReclaimFocus = false;
    }

    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(8);

    
    if (m_ActivePanel != SettingsPanel::None) {
        float panelWidth = 350.0f;
        float panelX = io.DisplaySize.x - panelWidth - 10.0f;
        float panelY = consoleHeight + 10.0f;

        ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(panelWidth, 0), ImGuiCond_Appearing);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.10f, 0.95f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.80f, 0.40f, 0.50f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.08f, 0.15f, 0.08f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.10f, 0.25f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.30f, 0.75f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.35f, 0.85f, 0.40f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.12f, 0.12f, 0.16f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);

        bool panelOpen = true;

        switch (m_ActivePanel) {
            case SettingsPanel::Camera:
                RenderCameraSettings(camera);
                break;
            case SettingsPanel::Rendering:
                RenderRenderingSettings(ctx);
                break;
            case SettingsPanel::Physics:
                RenderPhysicsSettings();
                break;
            case SettingsPanel::Environment:
                RenderEnvironmentSettings(ctx);
                break;
            case SettingsPanel::Logging:
                RenderLoggingScreen();
                break;
            default:
                break;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(7);
    }
}




void Console::RenderCameraSettings(Camera* camera) {
    bool open = true;
    ImGui::Begin("Camera Settings", &open);
    if (!open) { m_ActivePanel = SettingsPanel::None; ImGui::End(); return; }

    if (camera) {
        ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Camera Controls");
        ImGui::Separator();

        ImGui::DragFloat("Speed", &camera->speed, 0.05f, 0.01f, 20.0f);
        ImGui::DragFloat("Sensitivity", &camera->sensitivity, 1.0f, 10.0f, 300.0f);
        ImGui::DragFloat("FOV", &camera->FOV, 0.5f, 20.0f, 150.0f);
        ImGui::DragFloat("Near Plane", &camera->nearPlane, 0.01f, 0.001f, 10.0f);
        ImGui::DragFloat("Far Plane", &camera->farPlane, 1.0f, 10.0f, 2000.0f);

        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Position: (%.2f, %.2f, %.2f)",
            camera->Position.x, camera->Position.y, camera->Position.z);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Yaw: %.1f  Pitch: %.1f",
            camera->yaw, camera->pitch);
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No camera available");
    }

    ImGui::End();
}




void Console::RenderRenderingSettings(RenderContext& ctx) {
    bool open = true;
    ImGui::Begin("Rendering Settings", &open);
    if (!open) { m_ActivePanel = SettingsPanel::None; ImGui::End(); return; }

    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Rendering");
    ImGui::Separator();

    if (ImGui::Checkbox("Wireframe", &m_Wireframe)) {
        glPolygonMode(GL_FRONT_AND_BACK, m_Wireframe ? GL_LINE : GL_FILL);
    }

    if (ImGui::Checkbox("VSync", &m_VSync)) {
        glfwSwapInterval(m_VSync ? 1 : 0);
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "MSAA");

    const char* msaaOptions[] = { "Off", "2x", "4x", "8x" };
    int currentMsaaIndex = 0;
    if (ctx.msaaSamples == 2) currentMsaaIndex = 1;
    else if (ctx.msaaSamples == 4) currentMsaaIndex = 2;
    else if (ctx.msaaSamples == 8) currentMsaaIndex = 3;

    if (ImGui::Combo("MSAA##combo", &currentMsaaIndex, msaaOptions, IM_ARRAYSIZE(msaaOptions))) {
        if (currentMsaaIndex == 0) ctx.msaaSamples = 0;
        else if (currentMsaaIndex == 1) ctx.msaaSamples = 2;
        else if (currentMsaaIndex == 2) ctx.msaaSamples = 4;
        else if (currentMsaaIndex == 3) ctx.msaaSamples = 8;
    }

    if (ctx.msaaSamples > 0) {
        ImGui::Indent();
        ImGui::Text("Apply MSAA To:");
        ImGui::Checkbox("Sky Pass", &ctx.msaaSkyPass);
        ImGui::Checkbox("Geometry Pass", &ctx.msaaGeometryPass);
        ImGui::Checkbox("Transparency Pass", &ctx.msaaTransparencyPass);
        ImGui::Unindent();
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Environment Toggles");

    const char* skyModes[] = {"Cubemap", "Dynamic Sky"};
    int skyMode = m_ShowGradientSky ? 1 : 0;
    if (ImGui::Combo("Sky Mode", &skyMode, skyModes, IM_ARRAYSIZE(skyModes))) {
        m_ShowSkybox = (skyMode == 0);
        m_ShowGradientSky = (skyMode == 1);
    }

    ImGui::Checkbox("Clouds", &m_ShowClouds);

    ImGui::Separator();
    ImGui::SliderFloat("Tiling Factor", &ctx.globalTilingFactor, 0.1f, 10.0f);

    ImGui::End();
}




void Console::RenderPhysicsSettings() {
    bool open = true;
    ImGui::Begin("Physics Settings", &open);
    if (!open) { m_ActivePanel = SettingsPanel::None; ImGui::End(); return; }

    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Physics Engine");
    ImGui::Separator();

    ImGui::Checkbox("Global Physics Enabled", &PhysicsEngine::GlobalPhysicsEnabled);
    ImGui::Checkbox("Global Gravity Enabled", &PhysicsEngine::GlobalGravityEnabled);
    ImGui::Checkbox("Use Impulses", &PhysicsEngine::ImpulseEnabled);
    ImGui::Checkbox("Use Center of Mass", &PhysicsEngine::GlobalCOMEnabled);

    ImGui::Separator();
    ImGui::DragFloat("Air Resistance", &PhysicsEngine::GlobalAirResistance, 0.01f, 0.0f, 10.0f);
    ImGui::SliderInt("Sub-Steps", &PhysicsEngine::SubSteps, 1, 10);
    ImGui::DragFloat("Linear Damping", &PhysicsEngine::LinearDamping, 0.001f, 0.0f, 1.0f);
    ImGui::DragFloat("Angular Damping", &PhysicsEngine::AngularDamping, 0.001f, 0.0f, 1.0f);

    ImGui::Separator();
    ImGui::Checkbox("Show Hitboxes", &HitboxGraphics::ShowHitboxes);

    float grav[3] = { PhysicsEngine::Gravity.x, PhysicsEngine::Gravity.y, PhysicsEngine::Gravity.z };
    if (ImGui::DragFloat3("Gravity", grav, 0.1f)) {
        PhysicsEngine::Gravity = glm::vec3(grav[0], grav[1], grav[2]);
    }

    float gAcc[3] = { PhysicsEngine::GlobalAcceleration.x, PhysicsEngine::GlobalAcceleration.y, PhysicsEngine::GlobalAcceleration.z };
    if (ImGui::DragFloat3("Global Accel", gAcc, 0.1f)) {
        PhysicsEngine::GlobalAcceleration = glm::vec3(gAcc[0], gAcc[1], gAcc[2]);
    }

    ImGui::End();
}




void Console::RenderEnvironmentSettings(RenderContext& ctx) {
    bool open = true;
    ImGui::Begin("Environment Settings", &open);
    if (!open) { m_ActivePanel = SettingsPanel::None; ImGui::End(); return; }

    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Time of Day");
    ImGui::Separator();

    int h = (int)ctx.timeOfDay;
    float m_f = (ctx.timeOfDay - h) * 60.0f;
    int m = (int)m_f;
    int s = (int)((m_f - m) * 60.0f);
    ImGui::Text("Time: %02d:%02d:%02d", h, m, s);

    bool timeChanged = false;
    if (ImGui::SliderInt("Hour", &h, 0, 23)) timeChanged = true;
    if (ImGui::SliderInt("Minute", &m, 0, 59)) timeChanged = true;
    if (timeChanged) {
        ctx.timeOfDay = h + m / 60.0f + s / 3600.0f;
    }

    ImGui::DragFloat("Time Speed", &m_TimeSpeed, 0.1f, 0.0f, 20.0f);
    ImGui::Checkbox("Pause Time", &m_TimePaused);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Sun");

    ImGui::Checkbox("Sun Enabled", &ctx.sunEnabled);
    float sunCol[4] = { ctx.sunColor.r, ctx.sunColor.g, ctx.sunColor.b, ctx.sunColor.a };
    if (ImGui::ColorEdit4("Sun Color", sunCol)) {
        ctx.sunColor = glm::vec4(sunCol[0], sunCol[1], sunCol[2], sunCol[3]);
    }
    ImGui::SliderFloat("Sun Intensity", &ctx.sunIntensity, 0.0f, 5.0f);
    ImGui::SliderFloat("Sun Bloom", &ctx.sunBloom, 0.0f, 0.1f);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Moon");

    ImGui::Checkbox("Moon Enabled", &ctx.moonEnabled);
    float moonCol[4] = { ctx.moonColor.r, ctx.moonColor.g, ctx.moonColor.b, ctx.moonColor.a };
    if (ImGui::ColorEdit4("Moon Color", moonCol)) {
        ctx.moonColor = glm::vec4(moonCol[0], moonCol[1], moonCol[2], moonCol[3]);
    }
    ImGui::SliderFloat("Moon Intensity", &ctx.moonIntensity, 0.0f, 5.0f);
    ImGui::SliderFloat("Moon Bloom", &ctx.moonBloom, 0.0f, 0.1f);

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Clouds");

    if (m_ShowClouds) {
        const char* cloudModes[] = {"2D Clouds", "Volumetric Clouds"};
        ImGui::Combo("Cloud Mode", &ctx.cloudMode, cloudModes, IM_ARRAYSIZE(cloudModes));
        ImGui::SliderFloat("Cloud Height", &ctx.cloudHeight, 5.0f, 90.0f);
        ImGui::SliderFloat("Cloud Density", &ctx.cloudDensity, 0.0f, 1.0f);
        ImGui::SliderFloat("Cloud Cover", &ctx.cloudCover, 0.0f, 2.0f);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Clouds disabled. Enable via /clouds on");
    }

    ImGui::End();
}

void Console::RenderLoggingScreen() {
    bool open = true;
    ImGui::Begin("Logging & Diagnostics Overlay", &open);
    if (!open) { m_ActivePanel = SettingsPanel::None; ImGui::End(); return; }

    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Live Game State Telemetry");
    ImGui::Separator();
    
    ImGui::Text("Active Target State: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.2f, 1.0f), "%s", StateManager::GetCurrentStateName().c_str());

    ImGui::Text("Target ID Pointer: ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.9f, 0.6f, 0.2f, 1.0f), "%d", StateManager::GetState());
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Sub-State Registrations");
    
    for (const auto& mapping : StateManager::GetAllStates()) {
        ImGui::Text("ID [%d] - ", mapping.first);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.9f, 1.0f), "%s", mapping.second.c_str());
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.45f, 0.90f, 0.50f, 1.0f), "Available Native Interfaces");
    for (const auto& instance : StateManager::GetRegisteredStateNames()) {
        ImGui::BulletText("%s", instance.c_str());
    }
    
    ImGui::End();
}
