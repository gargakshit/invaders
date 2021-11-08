#include <SDL_render.h>
#include <iostream>
#include <stdint.h>

// SDL2
#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

// Dear Imgui
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "bus.hpp"
#include "font.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "shcore")
#include <ShellScalingAPI.h>
void initializePlatform() {
  SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
}
#else
void initializePlatform() {}
#endif

int main(int argc, char **args) {
  if (argc < 5) {
    std::cout << "No invaders ROM files specified. Aborting..." << std::endl;
    return 1;
  }

  invaders::Bus bus;
  bus.Reset();

  if (!(bus.LoadFileAt(args[1], 0x0000) && bus.LoadFileAt(args[2], 0x0800) &&
        bus.LoadFileAt(args[3], 0x1000) && bus.LoadFileAt(args[4], 0x0800))) {
    std::cerr << "Unable to start the emulator";
    return -1;
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
    std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
    return -1;
  }

  // Initialize platform specific stuff like DPI awareness
  initializePlatform();

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // Create a window
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  auto window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                        SDL_WINDOW_ALLOW_HIGHDPI);
  auto *window =
      SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
  auto gl_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  style.FrameRounding = 2;
  // style.FramePadding = ImVec2(2, 1);
  style.WindowRounding = 4;
  style.WindowPadding = ImVec2(16, 12);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(0.094f, 0.094f, 0.101f, 1); // ~#18181A
  style.WindowRounding = 0.0f;

  // Fonts
  ImFont *font = io.Fonts->AddFontFromMemoryCompressedTTF(
      jetbrains_mono_compressed_data, jetbrains_mono_compressed_size, 18);
  io.Fonts->Build();
  // ImGui::SetCurrentFont(font);

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  bool done = false;
  bool paused = false;

  auto lastPartialFrame = SDL_GetTicks();
  bool vblank = false;

  const uint16_t vramStart = 0x2400;

  auto displayRenderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  auto displayTexture =
      SDL_CreateTexture(displayRenderer, SDL_PIXELFORMAT_RGB888,
                        SDL_TEXTUREACCESS_TARGET, 256, 224);
  auto textureId = SDL_GL_BindTexture(displayTexture, nullptr, nullptr);
  auto displayDimens = ImVec2(256, 224);

  auto displayScaledRect = SDL_Rect{};
  displayScaledRect.w = 1;
  displayScaledRect.h = 1;

  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_QUIT) {
        done = true;
      }

      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window)) {
        done = true;
      }
    }

    if (!paused) {
      // Timers
      auto now = SDL_GetTicks();

      // Fire approximately every 8 milliseconds
      auto delta = now - lastPartialFrame;

      if (delta >= 16) {
        for (int i = 0; i < 16'500; i++) {
          bus.TickCPU();
        }
        bus.cpu.Interrupt(1);
        for (int i = 0; i < 16'500; i++) {
          bus.TickCPU();
        }
        bus.cpu.Interrupt(2);
        // bus.cpu.Interrupt(vblank ? 2 : 1);

        // vblank = !vblank;
        lastPartialFrame = now;
      }

      SDL_SetRenderTarget(displayRenderer, displayTexture);
      SDL_RenderClear(displayRenderer);

      for (unsigned int x = 0; x < 224; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
          auto byte = bus.mem[vramStart + ((x * 32) + y)];
          // std::cerr << std::hex << vramStart + ((x * 32) + y) << ' ' << +byte
          //           << std::endl;

          for (unsigned int bit = 0; bit < 8; ++bit) {
            if (byte & (1 << bit)) {
              SDL_SetRenderDrawColor(displayRenderer, 0x00, 0x00, 0x00, 0xff);
            } else if (x > 200 && (y * 8) < 220) {
              SDL_SetRenderDrawColor(displayRenderer, 0xff, 0x00, 0x00, 0xff);
            } else if (y < 10) {
              SDL_SetRenderDrawColor(displayRenderer, 0x00, 0xff, 0x00, 0xff);
            } else {
              SDL_SetRenderDrawColor(displayRenderer, 0xff, 0xff, 0xff, 0xff);
            }

            displayScaledRect.x = x;
            displayScaledRect.y = y * bit;
            SDL_RenderFillRect(displayRenderer, &displayScaledRect);
          }
        }
      }

      SDL_SetRenderTarget(displayRenderer, NULL);
      SDL_RenderCopy(displayRenderer, displayTexture, NULL, NULL);
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
      ImGui::Begin("Stats");

      ImGui::Text("Framerate: %f", io.Framerate);
      ImGui::Text("PC:     %04x", bus.cpu.pc);
      ImGui::Text("Opcode: %02x", bus.cpu.opcode);

      if (ImGui::Button(paused ? "Resume" : "Pause")) {
        paused = !paused;
      }

      ImGui::Image((void *)(intptr_t)textureId, displayDimens);

      ImGui::End();
    }

    ImGui::Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    auto *backup_current_window = SDL_GL_GetCurrentWindow();
    auto backup_current_context = SDL_GL_GetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    SDL_GL_MakeCurrent(backup_current_window, backup_current_context);

    SDL_GL_SwapWindow(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyTexture(displayTexture);
  SDL_DestroyRenderer(displayRenderer);

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
