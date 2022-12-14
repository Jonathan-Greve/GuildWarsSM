#include "pch.h"
#include "GuildWarsSM.h"

#include "KeyboardProc.h"
#include "SafeWndProc.h"

#include "SharedMemoryDataManager.h"

using namespace boost::interprocess;

void GuildWarsSM::Init()
{
    ChatWriter::WriteIngameDebugChat("Init: Called.", ChatColor::Green);
    // Set up our own NewWndHandle and store a copy of the default WndProc in DefaultWndProc
    // We can then later restore current_GW_window_handle to be the default when we terminate.
    current_GW_window_handle = GW::MemoryMgr::GetGWWindowHandle();
    DefaultWndProc =
      SetWindowLongPtrW(current_GW_window_handle, GWL_WNDPROC, reinterpret_cast<long>(SafeWndProc));

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is the thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);

    keyboard_hook_handle = SetWindowsHookExA(WH_KEYBOARD, &KeyboardProc, nullptr, GetCurrentThreadId());

    if (keyboard_hook_handle == NULL)
    {
        ChatWriter::WriteIngameDebugChat("Init: Failed setting keyboard hook.");
        Terminate();
        return;
    }

    // Get client accounts email
    std::wstring email_wchar = GW::GetCharContext()->player_email;
    std::string email{email_wchar.begin(), email_wchar.end()};

    if (! m_client_shared_memory.init(email))
    {
        Terminate();
        return;
    }

    if (! m_connections_shared_memory.init())
    {
        Terminate();
        return;
    }

    if (! m_connections_shared_memory.get_connected_clients()->connect(
          m_client_shared_memory, m_connections_shared_memory.get_mutex_handle(),
          m_connections_shared_memory.get_event_handle()))
    {
        ChatWriter::WriteIngameDebugChat("Init: Could not connect!", ChatColor::LightRed);
        Terminate();
        return;
    }

    m_shared_memory_data_manager.init(m_client_shared_memory);

    ChatWriter::WriteIngameDebugChat("Init: Finished.", ChatColor::Green);
}

// Remove all hooks. Free all resources. Disconnect any connections to external processes.
void GuildWarsSM::Terminate()
{
    ChatWriter::WriteIngameDebugChat("Terminate: Called.", ChatColor::Blue);
    if (! has_freed_resources)
    {
        if (! m_connections_shared_memory.get_connected_clients()->disconnect(
              m_client_shared_memory, m_connections_shared_memory.get_mutex_handle(),
              m_connections_shared_memory.get_event_handle()))
        {
            ChatWriter::WriteIngameDebugChat("Terminate: Could not disconnect!", ChatColor::LightRed);
        }

        GW::GameThread::RemoveGameThreadCallback(&Update_Entry);

        UnhookWindowsHookEx(keyboard_hook_handle);

        // Restore the window handle to be the default one that GW launched with.
        SetWindowLongPtr(current_GW_window_handle, GWL_WNDPROC, DefaultWndProc);

        // Let ThreadEntry know that it can finish terminating our dll thread.
        has_freed_resources = true;
        ChatWriter::WriteIngameDebugChat("Terminate: Freed resources.", ChatColor::Blue);

        // If terminate was called because the window is closing (i.e. Alt-f4 or pressed close)
        // Then resend the WM_CLOSE signal that we intercepted earlier in NewWndProc.
        if (GW_is_closing)
        {
            SendMessageW(current_GW_window_handle, WM_CLOSE, NULL, NULL);
        }
    }
    ChatWriter::WriteIngameDebugChat("Terminate: Finished.", ChatColor::Blue);
}

void GuildWarsSM::Update(GW::HookStatus*)
{
    if (! GuildWarsSM::Instance().has_freed_resources && ! GuildWarsSM::Instance().GW_is_closing)
    {
        static DWORD last_tick_count;
        if (last_tick_count == 0)
            last_tick_count = GetTickCount();

        const DWORD tick = GetTickCount();
        const DWORD delta = tick - last_tick_count;
        const float dt = static_cast<float>(delta) / 1000.f;

        last_tick_count = tick;

        const auto instance_type = GW::Map::GetInstanceType();
        const auto pregame_context = GW::GetPreGameContext();
        const auto cam = GW::CameraMgr::GetCamera();
        auto& sm_instance = GuildWarsSM::Instance();
        if (instance_type == GW::Constants::InstanceType::Loading)
        {
            // In load screen.
            return;
        }
        else if (pregame_context != nullptr)
        {
            // We are on the character select screen.
        }
        else if (cam && ! std::isinf(cam->position.x))
        {
            // Our character is loaded in game and can move around.
            const auto char_context = GW::GetCharContext();
            if (char_context)
            {
                InstanceId instance_id = char_context->token1;
                //GuildWarsSM::Instance().m_shared_memory_data_manager.update_client_data();
            }
        }
    }
}
