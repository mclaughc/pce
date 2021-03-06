#include "host_interface.h"
#include "YBaseLib/ByteStream.h"
#include "YBaseLib/Error.h"
#include "common/display_renderer_d3d.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "nfd.h"
#include "pce-sdl/scancodes_sdl.h"
#include "pce/system.h"
#include <SDL.h>
#include <cinttypes>
#include <glad.h>
#ifdef Y_COMPILER_MSVC
#include "imgui_impl_dx11.h"
#include <SDL_syswm.h>
#endif

SDLHostInterface::SDLHostInterface(SDL_Window* window, std::unique_ptr<DisplayRenderer> display_renderer,
                                   std::unique_ptr<MixerType> mixer)
  : m_window(window), m_display_renderer(std::move(display_renderer)), m_mixer(std::move(mixer))
{
  m_simulation_thread = std::thread([this]() { SimulationThreadRoutine(); });
}

SDLHostInterface::~SDLHostInterface()
{
  StopSimulationThread();
  m_simulation_thread.join();

  m_mixer.reset();

  switch (m_display_renderer->GetBackendType())
  {
#ifdef Y_COMPILER_MSVC
    case DisplayRenderer::BackendType::Direct3D:
    {
      ImGui_ImplDX11_Shutdown();
      ImGui::DestroyContext();
      m_display_renderer.reset();
    }
    break;
#endif

    case DisplayRenderer::BackendType::OpenGL:
    {
      SDL_GLContext context = SDL_GL_GetCurrentContext();
      ImGui_ImplOpenGL3_Shutdown();
      ImGui_ImplSDL2_Shutdown();
      ImGui::DestroyContext();
      m_display_renderer.reset();
      SDL_GL_MakeCurrent(nullptr, nullptr);
      SDL_GL_DeleteContext(context);
    }
    break;

    default:
    {

      ImGui::DestroyContext();
      m_display_renderer.reset();
    }
    break;
  }
}

std::unique_ptr<SDLHostInterface> SDLHostInterface::Create(
  DisplayRenderer::BackendType display_renderer_backend /* = DisplayRenderer::GetDefaultBackendType() */)
{
  constexpr u32 DEFAULT_WINDOW_WIDTH = 900;
  constexpr u32 DEFAULT_WINDOW_HEIGHT = 700;
  constexpr u32 MAIN_MENU_BAR_HEIGHT = 20;

  // Create window.
  u32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
  if (display_renderer_backend == DisplayRenderer::BackendType::OpenGL)
    window_flags |= SDL_WINDOW_OPENGL;

  auto window = std::unique_ptr<SDL_Window, void (*)(SDL_Window*)>(
    SDL_CreateWindow("PCE - Initializing...", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DEFAULT_WINDOW_WIDTH,
                     DEFAULT_WINDOW_HEIGHT, window_flags),
    [](SDL_Window* win) { SDL_DestroyWindow(win); });
  if (!window)
  {
    Panic("Failed to create window");
    return nullptr;
  }

  DisplayRenderer::WindowHandleType window_handle = nullptr;
  if (display_renderer_backend == DisplayRenderer::BackendType::OpenGL)
  {
    // We need a GL context. TODO: Move this to common.
    SDL_GLContext gl_context = SDL_GL_CreateContext(window.get());
    if (!gl_context || SDL_GL_MakeCurrent(window.get(), gl_context) != 0 || !gladLoadGL())
    {
      Panic("Failed to create GL context");
      return nullptr;
    }
  }
#ifdef Y_COMPILER_MSVC
  if (display_renderer_backend == DisplayRenderer::BackendType::Direct3D)
  {
    // Get window handle from SDL window
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window.get(), &info))
    {
      Panic("SDL_GetWindowWMInfo failed");
      return nullptr;
    }

    window_handle = info.info.win.window;
  }
#endif

  // Create renderer.
  auto display_renderer =
    DisplayRenderer::Create(display_renderer_backend, window_handle, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
  if (!display_renderer)
  {
    Panic("Failed to create display");
    return nullptr;
  }
  display_renderer->SetTopPadding(MAIN_MENU_BAR_HEIGHT);

  // Create audio renderer.
  auto mixer = MixerType::Create();
  if (!mixer)
  {
    Panic("Failed to create audio mixer");
    return nullptr;
  }

  // Initialize imgui.
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = nullptr;

  switch (display_renderer->GetBackendType())
  {
#ifdef Y_COMPILER_MSVC
    case DisplayRenderer::BackendType::Direct3D:
    {
      if (!ImGui_ImplSDL2_InitForD3D(window.get()) ||
          !ImGui_ImplDX11_Init(static_cast<DisplayRendererD3D*>(display_renderer.get())->GetD3DDevice(),
                               static_cast<DisplayRendererD3D*>(display_renderer.get())->GetD3DContext()))
      {
        return nullptr;
      }

      ImGui_ImplDX11_NewFrame();
      ImGui_ImplSDL2_NewFrame(window.get());
      ImGui::NewFrame();
    }
    break;
#endif

    case DisplayRenderer::BackendType::OpenGL:
    {
      if (!ImGui_ImplSDL2_InitForOpenGL(window.get(), SDL_GL_GetCurrentContext()) || !ImGui_ImplOpenGL3_Init())
        return nullptr;

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL2_NewFrame(window.get());
      ImGui::NewFrame();
    }
    break;

    default:
      break;
  }

  return std::make_unique<SDLHostInterface>(window.release(), std::move(display_renderer), std::move(mixer));
}

TinyString SDLHostInterface::GetSaveStateFilename(u32 index)
{
  return TinyString::FromFormat("savestate_%u.bin", index);
}

bool SDLHostInterface::CreateSystem(const char* filename, s32 save_state_index /* = -1 */)
{
  Error error;
  if (!HostInterface::CreateSystem(filename, &error))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Creating system failed", error.GetErrorCodeAndDescription(),
                             m_window);
    return false;
  }

  if (save_state_index >= 0)
  {
    // Load the save state.
    if (!HostInterface::LoadSystemState(GetSaveStateFilename(static_cast<u32>(save_state_index)), &error))
    {
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Loading save state failed", error.GetErrorCodeAndDescription(),
                               m_window);
    }
  }

  // Resume execution.
  ResumeSimulation();
  m_running = true;
  return true;
}

void SDLHostInterface::ReportMessage(const char* message)
{
  AddOSDMessage(message, 3.0f);
}

void SDLHostInterface::OnSimulationStatsUpdate(const SimulationStats& stats)
{
  LargeString window_title;
#if 0
  window_title.Format("PCE | System: %s | CPU: %s (%.2f MHz, %s) | Speed: %.1f%% | VPS: %.1f",
                      m_system->GetTypeInfo()->GetTypeName(), m_system->GetCPU()->GetModelString(),
                      m_system->GetCPU()->GetFrequency() / 1000000.0f,
                      CPU::BackendTypeToString(m_system->GetCPU()->GetBackend()), stats.simulation_speed * 100.0f,
                      m_display_renderer->GetPrimaryDisplayFramesPerSecond());
#else
  window_title.Format("PCE | CPU: %s (%.2f MHz, %s) | Speed: %.1f%% | VPS: %.1f", m_system->GetCPU()->GetModelString(),
                      m_system->GetCPU()->GetFrequency() / 1000000.0f,
                      CPU::BackendTypeToString(m_system->GetCPU()->GetBackend()), stats.simulation_speed * 100.0f,
                      m_display_renderer->GetPrimaryDisplayFramesPerSecond());
#endif

  SDL_SetWindowTitle(m_window, window_title);

  {
    std::unique_lock<std::mutex> lock(m_stats_mutex);
    m_stats.last_stats = stats;
    m_stats.simulation_speed_history[m_stats.history_position] = stats.simulation_speed * 100.0f;
    m_stats.host_cpu_usage_history[m_stats.history_position] = stats.host_cpu_usage * 100.0f;
    m_stats.instructions_executed_history[m_stats.history_position] =
      static_cast<float>(stats.cpu_delta_instructions_interpreted + stats.cpu_delta_code_cache_instructions_executed);
    m_stats.interrupts_serviced_history[m_stats.history_position] =
      static_cast<float>(stats.cpu_delata_interrupts_serviced);
    m_stats.exceptions_raised_history[m_stats.history_position] = static_cast<float>(stats.cpu_delta_exceptions_raised);
    m_stats.history_position = (m_stats.history_position + 1) % NUM_STATS_HISTORY_VALUES;
  }
}

bool SDLHostInterface::IsWindowFullscreen() const
{
  return ((SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN) != 0);
}

static inline u32 SDLButtonToHostButton(u32 button)
{
  // SDL left = 1, middle = 2, right = 3 :/
  switch (button)
  {
    case 1:
      return 0;
    case 2:
      return 2;
    case 3:
      return 1;
    default:
      return 0xFFFFFFFF;
  }
}

bool SDLHostInterface::HandleSDLEvent(const SDL_Event* event)
{
  if (!IsMouseGrabbed() && PassEventToImGui(event))
    return true;

  switch (event->type)
  {
    case SDL_MOUSEBUTTONDOWN:
    {
      u32 button = SDLButtonToHostButton(event->button.button);
      if (IsMouseGrabbed())
      {
        ExecuteMouseButtonChangeCallbacks(button, true);
        return true;
      }
    }
    break;

    case SDL_MOUSEBUTTONUP:
    {
      u32 button = SDLButtonToHostButton(event->button.button);
      if (IsMouseGrabbed())
      {
        ExecuteMouseButtonChangeCallbacks(button, false);
        return true;
      }
      else
      {
        // Are we capturing the mouse?
        if (button == 0)
          GrabMouse();
      }
    }
    break;

    case SDL_MOUSEMOTION:
    {
      if (!IsMouseGrabbed())
        return false;

      s32 dx = event->motion.xrel;
      s32 dy = event->motion.yrel;
      ExecuteMousePositionChangeCallbacks(dx, dy);
      return true;
    }
    break;

    case SDL_KEYDOWN:
    {
      // Release mouse key combination
      if (((event->key.keysym.sym == SDLK_LCTRL || event->key.keysym.sym == SDLK_RCTRL) &&
           (SDL_GetModState() & KMOD_ALT) != 0) ||
          ((event->key.keysym.sym == SDLK_LALT || event->key.keysym.sym == SDLK_RALT) &&
           (SDL_GetModState() & KMOD_CTRL) != 0))
      {
        // But don't consume the key event.
        ReleaseMouse();
      }

      // Create keyboard event.
      // TODO: Since we have crap in the input polling, we can't return true here.
      GenScanCode scancode;
      if (MapSDLScanCode(&scancode, event->key.keysym.scancode))
      {
        ExecuteKeyboardCallbacks(scancode, true);
        return false;
      }
    }
    break;

    case SDL_KEYUP:
    {
      // Create keyboard event.
      // TODO: Since we have crap in the input polling, we can't return true here.
      GenScanCode scancode;
      if (MapSDLScanCode(&scancode, event->key.keysym.scancode))
      {
        ExecuteKeyboardCallbacks(scancode, false);
        return false;
      }
    }
    break;

    case SDL_WINDOWEVENT:
    {
      if (event->window.event == SDL_WINDOWEVENT_RESIZED)
        m_display_renderer->WindowResized(u32(event->window.data1), u32(event->window.data2));
    }
    break;

    case SDL_QUIT:
      m_running = false;
      break;
  }

  return false;
}

bool SDLHostInterface::PassEventToImGui(const SDL_Event* event)
{
  ImGuiIO& io = ImGui::GetIO();
  switch (event->type)
  {
    case SDL_MOUSEWHEEL:
    {
      if (event->wheel.x > 0)
        io.MouseWheelH += 1;
      if (event->wheel.x < 0)
        io.MouseWheelH -= 1;
      if (event->wheel.y > 0)
        io.MouseWheel += 1;
      if (event->wheel.y < 0)
        io.MouseWheel -= 1;
      return io.WantCaptureMouse;
    }

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      bool down = event->type == SDL_MOUSEBUTTONDOWN;
      if (event->button.button == SDL_BUTTON_LEFT)
        io.MouseDown[0] = down;
      if (event->button.button == SDL_BUTTON_RIGHT)
        io.MouseDown[1] = down;
      if (event->button.button == SDL_BUTTON_MIDDLE)
        io.MouseDown[2] = down;
      return io.WantCaptureMouse;
    }

    case SDL_MOUSEMOTION:
    {
      io.MousePos.x = float(event->motion.x);
      io.MousePos.y = float(event->motion.y);
      return io.WantCaptureMouse;
    }

    case SDL_TEXTINPUT:
    {
      io.AddInputCharactersUTF8(event->text.text);
      return io.WantCaptureKeyboard;
    }

    case SDL_KEYDOWN:
    case SDL_KEYUP:
    {
      int key = event->key.keysym.scancode;
      IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
      io.KeysDown[key] = (event->type == SDL_KEYDOWN);
      io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
      io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
      io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
      io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
      return io.WantCaptureKeyboard;
    }
  }
  return false;
}

bool SDLHostInterface::IsMouseGrabbed() const
{
  // Giant hack, TODO
  return (SDL_GetRelativeMouseMode() == SDL_TRUE);
  // return (SDL_GetWindowGrab(static_cast<DisplayGL*>(m_display.get())->GetSDLWindow()) == SDL_TRUE);
}

void SDLHostInterface::GrabMouse()
{
  SDL_SetWindowGrab(m_window, SDL_TRUE);
  SDL_SetRelativeMouseMode(SDL_TRUE);
}

void SDLHostInterface::ReleaseMouse()
{
  SDL_SetWindowGrab(m_window, SDL_FALSE);
  SDL_SetRelativeMouseMode(SDL_FALSE);
}

void SDLHostInterface::Render()
{
  if (!m_display_renderer->BeginFrame())
    return;

  m_display_renderer->RenderDisplays();

  RenderImGui();

  const DisplayRenderer::BackendType backend_type = m_display_renderer->GetBackendType();
  switch (backend_type)
  {
#ifdef Y_COMPILER_MSVC
    case DisplayRenderer::BackendType::Direct3D:
    {
      ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
      m_display_renderer->EndFrame();
      ImGui_ImplSDL2_NewFrame(m_window);
      ImGui_ImplDX11_NewFrame();
    }
    break;
#endif

    case DisplayRenderer::BackendType::OpenGL:
    {
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      m_display_renderer->EndFrame();
      SDL_GL_SwapWindow(m_window);
      ImGui_ImplSDL2_NewFrame(m_window);
      ImGui_ImplOpenGL3_NewFrame();
    }
    break;

    default:
      break;
  }

  ImGui::NewFrame();
}

void SDLHostInterface::RenderImGui()
{
  RenderMainMenuBar();
  RenderStatsWindow();
  RenderOSDMessages();
  RenderActivityWindow();

  ImGui::Render();
}

void SDLHostInterface::RenderMainMenuBar()
{
  if (!ImGui::BeginMainMenuBar())
    return;

  if (ImGui::BeginMenu("System"))
  {
    if (ImGui::MenuItem("Reset"))
      ResetSystem();

    ImGui::Separator();

    if (ImGui::BeginMenu("CPU Backend"))
    {
      CPU::BackendType current_backend = GetCPUBackend();
      if (ImGui::MenuItem("Interpreter", nullptr, current_backend == CPU::BackendType::Interpreter))
        SetCPUBackend(CPU::BackendType::Interpreter);
      if (ImGui::MenuItem("Cached Interpreter", nullptr, current_backend == CPU::BackendType::CachedInterpreter))
        SetCPUBackend(CPU::BackendType::CachedInterpreter);
      if (ImGui::MenuItem("Recompiler", nullptr, current_backend == CPU::BackendType::Recompiler))
        SetCPUBackend(CPU::BackendType::Recompiler);

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("CPU Speed"))
    {
      float frequency = GetCPUFrequency();
      if (ImGui::InputFloat("Frequency", &frequency, 1000000.0f))
      {
        frequency = std::max(frequency, 1000000.0f);
        SetCPUFrequency(frequency);
      }
      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Enable Speed Limiter", nullptr, IsSpeedLimiterEnabled()))
      SetSpeedLimiterEnabled(!IsSpeedLimiterEnabled());

    if (ImGui::MenuItem("Flush Code Cache"))
      FlushCPUCodeCache();

    ImGui::Separator();

    if (ImGui::BeginMenu("Load State"))
    {
      for (u32 i = 1; i <= 8; i++)
      {
        if (ImGui::MenuItem(TinyString::FromFormat("State %u", i).GetCharArray()))
          DoLoadState(i);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Save State"))
    {
      for (u32 i = 1; i <= 8; i++)
      {
        if (ImGui::MenuItem(TinyString::FromFormat("State %u", i).GetCharArray()))
          DoSaveState(i);
      }
      ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Exit"))
      m_running = false;

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View"))
  {
    ImGui::MenuItem("Simulation Statistics", nullptr, &m_show_stats);

    if (ImGui::MenuItem("Fullscreen", nullptr, IsWindowFullscreen()))
      SDL_SetWindowFullscreen(m_window, IsWindowFullscreen() ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Devices"))
  {
    if (ImGui::MenuItem("Capture Mouse") && !IsMouseGrabbed())
      GrabMouse();

    ImGui::Separator();

    if (ImGui::MenuItem("Send CTRL+ALT+DEL"))
      SendCtrlAltDel();

    ImGui::Separator();

    for (const ComponentUIElement& ui : m_component_ui_elements)
    {
      const size_t total_callbacks = ui.callbacks.size() + ui.file_callbacks.size();
      if (total_callbacks == 0)
        continue;

      if (ImGui::BeginMenu(ui.component->GetIdentifier()))
      {
        for (const auto& it : ui.file_callbacks)
        {
          if (ImGui::MenuItem(it.first))
          {
            nfdchar_t* path;
            if (NFD_OpenDialog("", "", &path) == NFD_OKAY)
            {
              const auto& callback = it.second;
              String str(path);
              QueueExternalEvent([&callback, str]() { callback(str); }, false);
            }
          }
        }

        for (const auto& it : ui.callbacks)
        {
          if (ImGui::MenuItem(it.first))
          {
            const auto& callback = it.second;
            QueueExternalEvent([&callback]() { callback(); }, false);
          }
        }

        ImGui::EndMenu();
      }
    }

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

void SDLHostInterface::RenderActivityWindow()
{
  bool has_activity = false;
  for (const auto& elem : m_component_ui_elements)
  {
    if (elem.indicator_state != IndicatorState::Off)
    {
      has_activity = true;
      break;
    }
  }
  if (has_activity)
  {
    ImGui::SetNextWindowPos(ImVec2(10.0f, static_cast<float>(m_display_renderer->GetWindowHeight() - 40)));
    if (ImGui::Begin("Activity", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
      for (const auto& elem : m_component_ui_elements)
      {
        if (elem.indicator_state == IndicatorState::Off)
          continue;

        const char* text = elem.indicator_state == IndicatorState::Reading ? "Reading" : "Writing";
        const ImVec4 color = elem.indicator_state == IndicatorState::Reading ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) :
                                                                               ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        ImGui::TextColored(color, "%s (%s): %s", elem.component->GetIdentifier().GetCharArray(),
                           elem.component->GetTypeInfo()->GetTypeName(), text);
      }
      ImGui::End();
    }
  }
}

void SDLHostInterface::RenderStatsWindow()
{
  static const ImVec2 STATS_WINDOW_SIZE(300.0f, 420.0f);
  static const ImVec2 HISTORY_GRAPH_SIZE(280.0f, 32.0f);
  if (!m_show_stats)
    return;

  ImGui::SetNextWindowSize(STATS_WINDOW_SIZE, ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Simulation Statistics", &m_show_stats))
  {
    ImGui::End();
    return;
  }

  {
    std::unique_lock<std::mutex> lock(m_stats_mutex);
    const SimulationStats& stats = m_stats.last_stats;

    ImGui::Text("Simulation Speed: %.2f%%", static_cast<float>(stats.simulation_speed) * 100.0f);
    ImGui::PlotLines("##stats_simulation_speed_history", m_stats.simulation_speed_history.data(),
                     NUM_STATS_HISTORY_VALUES, m_stats.history_position, nullptr, 0.0f, 100.0f, HISTORY_GRAPH_SIZE);
    ImGui::NewLine();

    ImGui::Text("Simulation Thread CPU Usage: %.2f%%", static_cast<float>(stats.host_cpu_usage) * 100.0f);
    ImGui::PlotLines("##stats_simulation_speed_history", m_stats.host_cpu_usage_history.data(),
                     NUM_STATS_HISTORY_VALUES, m_stats.history_position, nullptr, 0.0f, FLT_MAX, HISTORY_GRAPH_SIZE);
    ImGui::NewLine();

    ImGui::Text("Code Block Count: %" PRIu64, stats.cpu_stats.num_code_cache_blocks);
    ImGui::Text("Blocks Executed: %" PRIu64, stats.cpu_delta_code_cache_blocks_executed);
    ImGui::Text("Cached Instructions Executed: %" PRIu64, stats.cpu_delta_code_cache_instructions_executed);
    ImGui::Text("Instructions Interpreted: %" PRIu64, stats.cpu_delta_instructions_interpreted);
    ImGui::PlotLines("##stats_instructions_executed", m_stats.instructions_executed_history.data(),
                     NUM_STATS_HISTORY_VALUES, m_stats.history_position, nullptr, FLT_MIN, FLT_MAX, HISTORY_GRAPH_SIZE);
    ImGui::NewLine();

    ImGui::Text("Interrupts Serviced: %" PRIu64 " (delta %" PRIu64 ")", stats.cpu_stats.interrupts_serviced,
                stats.cpu_delata_interrupts_serviced);
    ImGui::PlotLines("##interrupts_serviced_history", m_stats.interrupts_serviced_history.data(),
                     NUM_STATS_HISTORY_VALUES, m_stats.history_position, nullptr, FLT_MIN, FLT_MAX, HISTORY_GRAPH_SIZE);
    ImGui::NewLine();

    ImGui::Text("Exceptions Raised: %" PRIu64 " (delta %" PRIu64 ")", stats.cpu_stats.exceptions_raised,
                stats.cpu_delta_exceptions_raised);
    ImGui::PlotLines("##stats_exceptions_raised", m_stats.exceptions_raised_history.data(), NUM_STATS_HISTORY_VALUES,
                     m_stats.history_position, nullptr, FLT_MIN, FLT_MAX, HISTORY_GRAPH_SIZE);
  }

  ImGui::End();
}

void SDLHostInterface::RenderOSDMessages()
{
  constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs |
                                            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                                            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav |
                                            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;

  std::unique_lock<std::mutex> lock(m_osd_messages_lock);

  auto iter = m_osd_messages.begin();
  float position_x = 10.0f;
  float position_y = 10.0f + 20.0f;
  u32 index = 0;
  while (iter != m_osd_messages.end())
  {
    const OSDMessage& msg = *iter;
    const double time = msg.time.GetTimeSeconds();
    const float time_remaining = static_cast<float>(msg.duration - time);
    if (time_remaining <= 0.0f)
    {
      iter = m_osd_messages.erase(iter);
      continue;
    }

    const float opacity = std::min(time_remaining, 1.0f);
    ImGui::SetNextWindowPos(ImVec2(position_x, position_y));
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, opacity);

    if (ImGui::Begin(SmallString::FromFormat("osd_%u", index++), nullptr, window_flags))
    {
      ImGui::TextUnformatted(msg.text);
      position_y += ImGui::GetWindowSize().y + (4.0f * ImGui::GetIO().DisplayFramebufferScale.x);
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ++iter;
  }
}

void SDLHostInterface::DoLoadState(u32 index)
{
  Error error;
  if (!LoadSystemState(TinyString::FromFormat("savestate_%u.bin", index), &error))
  {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Loading save state failed", error.GetErrorCodeAndDescription(),
                             m_window);
  }
}

void SDLHostInterface::DoSaveState(u32 index)
{
  SaveSystemState(TinyString::FromFormat("savestate_%u.bin", index));
}

void SDLHostInterface::Run()
{
  while (m_running)
  {
    for (;;)
    {
      SDL_Event ev;
      if (SDL_PollEvent(&ev))
        HandleSDLEvent(&ev);
      else
        break;
    }

    Render();
  }

  StopSimulation();
}
