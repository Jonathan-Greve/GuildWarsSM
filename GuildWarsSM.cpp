#include "pch.h"
#include "GuildWarsSM.h"

#include "KeyboardProc.h"
#include "SafeWndProc.h"

using namespace boost::interprocess;

void GuildWarsSM::Init()
{
    // Set up our own NewWndHandle and store a copy of the default WndProc in DefaultWndProc
    // We can then later restore current_GW_window_handle to be the default when we terminate.
    current_GW_window_handle = GW::MemoryMgr::GetGWWindowHandle();
    DefaultWndProc =
      SetWindowLongPtrW(current_GW_window_handle, GWL_WNDPROC, reinterpret_cast<long>(SafeWndProc));

    // Get client email
    std::wstring email_wchar = GW::GetCharContext()->player_email;
    std::string email{email_wchar.begin(), email_wchar.end()};

    // Register our Update method to be called on each frame from within the game thread.
    // Note that the game thread is separate from the current thread. It is the thread
    // controlled by the GW client.
    GW::GameThread::RegisterGameThreadCallback(&Update_Entry, Update);

    //Remove shared memory on construction and destruction

    managed_shared_memory segment(open_or_create, "GuildWarsSM", 65536);
    std::string* ptr = segment.construct<std::string>("Email")(email);

    keyboard_hook_handle = SetWindowsHookExA(WH_KEYBOARD, &KeyboardProc, nullptr, GetCurrentThreadId());

    if (keyboard_hook_handle == NULL)
    {
        std::wstring init_message =
          std::format(L"<c=#FFFFFF>: {}</c>", L"Init: Failed setting keyboard hook.");
        GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
        Terminate();
    }

    std::wstring init_message = std::format(L"<c=#FFFFFF>: {}</c>", L"Init: Finished.");
    GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
}

// Remove all hooks. Free all resources. Disconnect any connections to external processes.
void GuildWarsSM::Terminate()
{
    std::wstring init_message = std::format(L"<c=#FFFFFF>: {}</c>", L"Terminate called.");
    GW::Chat::WriteChat(GW::Chat::CHANNEL_GWCA2, init_message.c_str(), nullptr);
    if (! has_freed_resources)
    {
        GW::GameThread::RemoveGameThreadCallback(&Update_Entry);

        UnhookWindowsHookEx(keyboard_hook_handle);

        // Restore the window handle to be the default one that GW launched with.
        SetWindowLongPtr(current_GW_window_handle, GWL_WNDPROC, DefaultWndProc);

        // Let ThreadEntry know that it can finish terminating our dll thread.
        has_freed_resources = true;

        // If terminate was called because the window is closing (i.e. Alt-f4 or pressed close)
        // Then resend the WM_CLOSE signal that we intercepted earlier in NewWndProc.
        if (GW_is_closing)
        {
            SendMessageW(current_GW_window_handle, WM_CLOSE, NULL, NULL);
        }
    }
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
    }
}
