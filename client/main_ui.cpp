

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include "menubar.h"
#include "nlohmann/json.hpp"
#include "queryworker.h"

#define PORT 9010
#define ADDR "127.0.0.1"


#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)

// Portable helpers
static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }


xistor::BlockingQueue<std::string> g_query_queue;


// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window* window = SDL_CreateWindow("translator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // const ImGuiViewport* viewport = ImGui::GetMainViewport();


    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    io.Fonts->AddFontDefault();
    ImFont* Roboto_Medium_font = io.Fonts->AddFontFromFileTTF("SourceHanSans-VF.ttf.ttc", 16.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());


    io.FontDefault = Roboto_Medium_font;
    io.FontGlobalScale = 2;


    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    io.DisplayFramebufferScale = ImVec2(2,2);

    xistor::BlockingQueue<xistor::ResponseMsg> res_queue;

    xistor::QueryWorker query_worker(ADDR, PORT, &res_queue);

    query_worker.start();

    // Main loop
    bool done = false;

    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        // full window
        ImGui::SetNextWindowPos(main_viewport->WorkPos);
        ImGui::SetNextWindowSize(main_viewport->WorkSize);




        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {

            ImGui::Begin("translate", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar);                          // Create a window called "Hello, world!" and append into it.

            ShowMainMenuBar();

            std::string from, to, pre, words;
            // Child 1: no border
            {
                const char* source_languages[] = { "auto", "zh", "en" };
                static int src_item = 0;


                static char text[1024 * 8] = "";
                static ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoNav;

                ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 300), false, window_flags);

                ImGui::Combo(" ", &src_item, source_languages, IM_ARRAYSIZE(source_languages));
                if(ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), flags))
                {
                    std::cout << "InputTextMultiline enter" << std::endl;
                    from = source_languages[src_item];
                    char* s = text;
                    Strtrim(s);
                    if (s[0])
                        words = s;
                }

                ImGui::EndChild();
            }
            ImGui::SameLine();
            // Child 2: no border
            {
                const char* target_languages[] = { "zh", "en"};
                static int tgr_item = 0;

                static char text[1024 * 8] = "";
                static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;

                ImGui::BeginChild("ChildR", ImVec2(0, 300), false, window_flags);
                ImGui::Combo(" ", &tgr_item, target_languages, IM_ARRAYSIZE(target_languages));

                to = target_languages[tgr_item];
                if( pre != words) {
                    query_worker.query({from, to, words});
                    pre = words;
                }

                if(res_queue.size()) {
                    auto resp = res_queue.take();
                    memcpy(text, resp.dst.c_str(), resp.dst.size());
                    memset(text + resp.dst.size() , '\0', resp.dst.size());
                }
                ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 8), flags);
                ImGui::EndChild();
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    query_worker.stop();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
