#include <SDL_events.h>
#include <SDL_keycode.h>
#include <iostream>
#include <stdint.h>

// Dear Imgui
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

// SDL2
#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#include "config.h"

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
  if (argc < 1) {
    std::cout << "No invaders ROM files specified. Aborting..." << std::endl;
    return 1;
  }

  invaders::Bus bus;
  bus.Reset();

  if (!bus.LoadFileAt(args[1], 0x0000)) {
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

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  style.FrameRounding = 2;
  style.WindowRounding = 4;
  style.WindowPadding = ImVec2(16, 12);
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.f);
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
  auto displayScale = 3;

  GLubyte displayFramebuffer[224 * 256 * 3];

  GLuint displayTexture;
  glGenTextures(1, &displayTexture);
  glBindTexture(GL_TEXTURE_2D, displayTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 224, 256, 0, GL_RGB, GL_UNSIGNED_BYTE,
               displayFramebuffer);

  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);

      if (event.type == SDL_QUIT) {
        done = true;
      }

      if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
        auto p = event.type == SDL_KEYDOWN;

        switch (event.key.keysym.sym) {
        case SDLK_LEFT: bus.SetKeyboardState(invaders::P1_LEFT, p); break;
        case SDLK_RIGHT: bus.SetKeyboardState(invaders::P1_RIGHT, p); break;
        case SDLK_c: bus.SetKeyboardState(invaders::COIN, p); break;
        case SDLK_SPACE: bus.SetKeyboardState(invaders::P1_FIRE, p); break;
        case SDLK_1: bus.SetKeyboardState(invaders::P1_START, p); break;
        }
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

      // Fire approximately every 16 milliseconds (will try to squeeze 60 FPS,
      // or otherwise will run slower)
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
        lastPartialFrame = now;
      }

      for (unsigned int x = 0; x < 224; ++x) {
        for (unsigned int y = 0; y < 32; ++y) {
          auto byte = bus.mem[vramStart + ((x * 32) + y)];
          for (unsigned int bit = 0; bit < 8; ++bit) {
            auto i = (x + ((255 - ((y * 8) + bit)) * 224)) * 3;

            if ((byte & (1 << bit)) == 0) {
              // Black
              displayFramebuffer[i] = 0x00;
              displayFramebuffer[i + 1] = 0x00;
              displayFramebuffer[i + 2] = 0x00;
            } else if (y > 25 && y < 28) {
              // Red
              displayFramebuffer[i] = 0xff;
              displayFramebuffer[i + 1] = 0x00;
              displayFramebuffer[i + 2] = 0x00;
            } else if (y < 10) {
              // Green
              displayFramebuffer[i] = 0x00;
              displayFramebuffer[i + 1] = 0xff;
              displayFramebuffer[i + 2] = 0x00;
            } else {
              // White
              displayFramebuffer[i] = 0xff;
              displayFramebuffer[i + 1] = 0xff;
              displayFramebuffer[i + 2] = 0xff;
            }
          }
        }
      }

      glBindTexture(GL_TEXTURE_2D, displayTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 224, 256, 0, GL_RGB,
                   GL_UNSIGNED_BYTE, displayFramebuffer);
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    {
      ImGui::Begin("General");
      ImGui::Text("Framerate: %f", io.Framerate);
      if (ImGui::Button(paused ? "Resume" : "Pause")) {
        paused = !paused;
      }
      ImGui::InputInt("Display Scale", &displayScale, 1, 1);

      if (displayScale < 1) {
        displayScale = 1;
      } else if (displayScale > 8) {
        displayScale = 8;
      }

      ImGui::End();
    }

    {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
      ImGui::Begin("Display", NULL,
                   ImGuiWindowFlags_AlwaysAutoResize |
                       ImGuiWindowFlags_NoDecoration);
      ImGui::Image((void *)(intptr_t)displayTexture,
                   ImVec2(224 * displayScale, 256 * displayScale));
      ImGui::End();
      ImGui::PopStyleVar();
    }

    ImGui::Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
