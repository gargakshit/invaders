#include <chrono>
#include <iostream>
#include <stdint.h>

// Dear Imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLFW
#include <GLFW/glfw3.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include "config.h"

#include "bus.hpp"
#include "font.h"

static void glfw_error_callback(int error, const char *description) {
  std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

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

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(1280, 720, "Invaders", NULL, NULL);
  if (window == NULL) {
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable VSync

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
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.f);
  style.WindowRounding = 0.0f;

  // Fonts
  ImFont *font = io.Fonts->AddFontFromMemoryCompressedTTF(
      jetbrains_mono_compressed_data, jetbrains_mono_compressed_size, 18);
  io.Fonts->Build();
  // ImGui::SetCurrentFont(font);

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  bool paused = false;

  using namespace std::chrono;

  auto lastPartialFrame = system_clock::now();
  bool vblank = false;

  const uint16_t vramStart = 0x2400;
  auto displayScale = 3;

  GLubyte displayFramebuffer[224 * 256 * 3];

  GLuint displayTexture;
  glGenTextures(1, &displayTexture);
  glBindTexture(GL_TEXTURE_2D, displayTexture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 224, 256, 0, GL_RGB, GL_UNSIGNED_BYTE,
               displayFramebuffer);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (!paused) {
      // Timers
      auto now = system_clock::now();

      // Fire approximately every 16 milliseconds (will try to squeeze 60 FPS,
      // or otherwise will run slower)
      auto delta = now - lastPartialFrame;

      // It gives back the nanoseconds
      if (delta.count() >= 160'000) {
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
    ImGui_ImplGlfw_NewFrame();
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

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
