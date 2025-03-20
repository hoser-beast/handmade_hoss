/*
[TODO]:
    - Saved game locations.
    - Getting a handle to our own executable file.
    - Asset loading.
    - Threading.
    - Raw input (support for multiple keyboards).
    - Sleep & time_begin_period().
    - clip_cursor() for multimonitor support.
    - Fullscreen support.
    - WM_SETCURSOR (control cursor visibility).
    - QueryCancelAutoplay().
    - WM_ACTIVATEAPP for when we are not the active application.
    - Blit speed improvements (BitBlt)
    - Hardware acceleration (OpenGL/Direct3D).
    - get_keybaord_layout (international layout, etc.).
    - ...
*/

#include <dsound.h>
#include <math.h>    // [TODO] Implement sine ourselves.
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <xinput.h>

#define PI32 3.14159265359f

#define local_persist   static
#define global_variable static
#define internal        static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t b32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#include "handmade.cpp"

struct win32_offscreen_buffer
{
    BITMAPINFO info;
    void*      memory;
    i32        width;
    i32        height;
    i32        pitch;
};

struct win32_window_dimensions
{
    i32 width;
    i32 height;
};

// [TODO] Global variable for now.
global_variable bool                   g_running;
global_variable win32_offscreen_buffer g_back_buffer;
global_variable LPDIRECTSOUNDBUFFER    g_secondary_buffer;

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD user_index, XINPUT_STATE* state)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(x_input_get_state_stub) { return ERROR_DEVICE_NOT_CONNECTED; }
global_variable x_input_get_state* XInputGetState_ = x_input_get_state_stub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD user_index, XINPUT_VIBRATION* vibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(x_input_set_state_stub) { return ERROR_DEVICE_NOT_CONNECTED; }
global_variable x_input_set_state* XInputSetState_ = x_input_set_state_stub;
#define XInputSetState XInputSetState_

internal void
win32_load_x_input(void)
{
    HMODULE x_input_library = LoadLibraryA("xinput1_4.dll");
    if (!x_input_library)
    {
        HMODULE x_input_library = LoadLibraryA("xinput9_1_0.dll");
    }
    if (!x_input_library)
    {
        HMODULE x_input_library = LoadLibraryA("xinput1_3.dll");
    }

    if (x_input_library)
    {
        // [TODO] Fix these variable names.
        XInputGetState = (x_input_get_state*)GetProcAddress(x_input_library, "XInputGetState");
        if (!XInputGetState)
        {
            XInputGetState = x_input_get_state_stub;
        }
        XInputSetState = (x_input_set_state*)GetProcAddress(x_input_library, "XInputSetState");
        if (!XInputSetState)
        {
            XInputSetState = x_input_set_state_stub;
        }
    }
    else
    {
        // [TODO] Diagnostics.
    }
}

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND* ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
win32_init_dsound(HWND window, i32 samples_per_sec, i32 buffer_size)
{
    // Load the library.
    HMODULE dsound_library = LoadLibraryA("dsound.dll");

    if (dsound_library)
    {
        direct_sound_create* DirectSoundCreate = (direct_sound_create*)GetProcAddress(dsound_library,
                                                                                      "DirectSoundCreate");

        LPDIRECTSOUND direct_sound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &direct_sound, 0)))
        {
            WAVEFORMATEX wave_format = {};

            wave_format.wFormatTag      = WAVE_FORMAT_PCM;
            wave_format.nChannels       = 2;
            wave_format.wBitsPerSample  = 16;
            wave_format.nSamplesPerSec  = samples_per_sec;
            wave_format.nBlockAlign     = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
            wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
            wave_format.cbSize          = 0;

            if (SUCCEEDED(direct_sound->SetCooperativeLevel(window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC buffer_description = {};
                buffer_description.dwSize       = sizeof(buffer_description);
                buffer_description.dwFlags      = DSBCAPS_PRIMARYBUFFER;

                // Create a primary buffer.
                LPDIRECTSOUNDBUFFER primary_buffer;
                if (SUCCEEDED(direct_sound->CreateSoundBuffer(&buffer_description, &primary_buffer, 0)))
                {

                    HRESULT error = primary_buffer->SetFormat(&wave_format);
                    if (SUCCEEDED(error))
                    {
                        OutputDebugStringA("Primary sound buffer created.\n");
                    }
                    else
                    {
                        // [TODO] Diagnostics.
                    }
                }
                else
                {
                    // [TODO] Diagnostics.
                }
            }
            else
            {
                // [TODO] Diagnostics.
            }

            DSBUFFERDESC buffer_description  = {};
            buffer_description.dwSize        = sizeof(buffer_description);
            buffer_description.dwFlags       = 0;
            buffer_description.dwBufferBytes = buffer_size;
            buffer_description.lpwfxFormat   = &wave_format;

            HRESULT error = direct_sound->CreateSoundBuffer(&buffer_description, &g_secondary_buffer, 0);
            if (SUCCEEDED(error))
            {
                OutputDebugStringA("Secondary sound buffer created.\n");
            }
        }
        else
        {
            // [TODO] Diagnostics.
        }
    }
    else
    {
        // [TODO] Diagnostics.
    }
}

internal win32_window_dimensions
win32_get_window_dimensions(HWND window)
{
    win32_window_dimensions result;

    RECT client_rect;
    GetClientRect(window, &client_rect);

    result.width  = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;

    return result;
}

internal void
win32_resize_dib_section(win32_offscreen_buffer* buffer, i32 width, i32 height)
{
    i32 bytes_per_pixel = 4;

    // [TODO] Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width  = width;
    buffer->height = height;

    // [NOTE] biHeight is negative to make Windows treat the bitmap top-down.
    buffer->info.bmiHeader.biSize        = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth       = buffer->width;
    buffer->info.bmiHeader.biHeight      = -buffer->height;
    buffer->info.bmiHeader.biPlanes      = 1;
    buffer->info.bmiHeader.biBitCount    = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    i32 bitmap_memory_size = buffer->width * buffer->height * bytes_per_pixel;

    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch  = buffer->width * bytes_per_pixel;

    // [TODO] Probably clear this to black.
}

internal void
win32_display_buffer_in_window(win32_offscreen_buffer* buffer, HDC device_context, i32 window_width, i32 window_height)
{
    // [TODO] Aspect ratio correction.
    // [TODO] Fix stretch modes.
    StretchDIBits(device_context, 0, 0, window_width, window_height, 0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
}

struct win32_sound_output
{
    i32 samples_per_second;
    i32 tone_hz;
    i16 tone_volume;
    u32 running_sample_index;
    i32 wave_period;
    i32 bytes_per_sample;
    i32 secondary_buffer_size;
    f32 t_sine;
    i32 latency_sample_count;
};

internal void
win32_clear_buffer(win32_sound_output* buffer)
{
    VOID* region1;
    DWORD region1_size;
    VOID* region2;
    DWORD region2_size;

    if (SUCCEEDED(g_secondary_buffer->Lock(0, buffer->secondary_buffer_size, &region1, &region1_size, &region2,
                                           &region2_size, 0)))
    {
        u8* dest_sample = (u8*)region1;
        for (DWORD byte_index = 0; byte_index < region1_size; byte_index++)
        {
            *dest_sample++ = 0;
        }

        dest_sample = (u8*)region2;
        for (DWORD byte_index = 0; byte_index < region2_size; byte_index++)
        {
            *dest_sample++ = 0;
        }

        g_secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}
internal void
win32_fill_sound_buffer(win32_sound_output* sound_output, DWORD byte_to_lock, DWORD bytes_to_write,
                        game_sound_output_buffer* source_buffer)
{
    VOID* region1;
    DWORD region1_size;
    VOID* region2;
    DWORD region2_size;

    if (SUCCEEDED(g_secondary_buffer->Lock(byte_to_lock, bytes_to_write, &region1, &region1_size, &region2,
                                           &region2_size, 0)))
    {
        // [TODO] Assert that region1_size and region2_size are valid.
        // [TODO] Collapse these loops into one.
        DWORD region1_sample_count = region1_size / sound_output->bytes_per_sample;
        i16*  dest_sample          = (i16*)region1;
        i16*  source_sample        = source_buffer->samples;
        for (DWORD sample_index = 0; sample_index < region1_sample_count; sample_index++)
        {
            *dest_sample++ = *source_sample++;
            *dest_sample++ = *source_sample++;
            sound_output->running_sample_index++;
        }
        DWORD region2_sample_count = region2_size / sound_output->bytes_per_sample;
        dest_sample                = (i16*)region2;
        for (DWORD sample_index = 0; sample_index < region2_sample_count; sample_index++)
        {
            *dest_sample++ = *source_sample++;
            *dest_sample++ = *source_sample++;
            sound_output->running_sample_index++;
        }

        g_secondary_buffer->Unlock(region1, region1_size, region2, region2_size);
    }
}

LRESULT CALLBACK
win32_main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATE_APP\n");
            break;
        }
        case WM_CLOSE:
        {
            // [TODO] Handle this with a message to the user?
            g_running = false;
            break;
        }
        case WM_DESTROY:
        {
            // [TODO] Handle this as an error. Recreate window?
            g_running = false;
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT             paint;
            HDC                     device_context = BeginPaint(window, &paint);
            win32_window_dimensions dimensions     = win32_get_window_dimensions(window);

            win32_display_buffer_in_window(&g_back_buffer, device_context, dimensions.width, dimensions.height);
            EndPaint(window, &paint);
            break;
        }
        // [TODO] This method of key input is not optimal. Implement virtual keycodes later.
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            u32  vk_code  = w_param;
            bool was_down = (l_param & (1 << 30)) != 0;
            bool is_down  = (l_param & (1 << 31)) == 0;

            if (is_down != was_down)    // Only worry about new inputs.
            {
                if (vk_code == 'R')
                {
                }
                if (vk_code == 'S')
                {
                }
                if (vk_code == 'H')
                {
                }
                if (vk_code == 'T')
                {
                }
                if (vk_code == 'D')
                {
                }
                if (vk_code == 'W')
                {
                }
                if (vk_code == VK_LEFT)
                {
                }
                if (vk_code == VK_RIGHT)
                {
                }
                if (vk_code == VK_UP)
                {
                }
                if (vk_code == VK_DOWN)
                {
                }
                if (vk_code == VK_SPACE)
                {
                }
                if (vk_code == VK_ESCAPE)
                {
                    g_running = false;
                }

                b32 alt_key_was_down = l_param & (1 << 29);
                if ((vk_code == VK_F4) && alt_key_was_down)
                {
                    g_running = false;
                }
            }
            break;
        }
        default:
        {
            result = DefWindowProc(window, message, w_param, l_param);
        };
    }

    return result;
}

i32 CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, i32 cmd_show)
{
    LARGE_INTEGER perf_counter_frequency_result;
    QueryPerformanceFrequency(&perf_counter_frequency_result);
    i64 perf_counter_frequency = perf_counter_frequency_result.QuadPart;

    win32_load_x_input();

    WNDCLASSA window_class = {};

    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = win32_main_window_proc;
    window_class.hInstance     = instance;
    window_class.lpszClassName = "HandmadeHossWindowClass";

    win32_resize_dib_section(&g_back_buffer, 1280, 720);

    if (RegisterClass(&window_class))
    {
        HWND window = CreateWindowEx(0, window_class.lpszClassName, "Handmade Hoss", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

        if (window)
        {
            // [NOTE] Since we specified CS_OWNDC above, we only need one device
            // context since we are not sharing it with anyone else.
            HDC device_context = GetDC(window);

            i32 x_offset = 0;
            i32 y_offset = 0;

            win32_sound_output sound_output = {};

            sound_output.samples_per_second    = 48000;
            sound_output.tone_hz               = 256;
            sound_output.tone_volume           = 1024;
            sound_output.wave_period           = sound_output.samples_per_second / sound_output.tone_hz;
            sound_output.bytes_per_sample      = sizeof(i16) * 2;
            sound_output.secondary_buffer_size = sound_output.samples_per_second * sound_output.bytes_per_sample;
            sound_output.latency_sample_count  = sound_output.samples_per_second / 15;

            // [TODO] Pool with bitmap VirtualAlloc().
            i16* samples = (i16*)VirtualAlloc(0, sound_output.secondary_buffer_size, MEM_RESERVE | MEM_COMMIT,
                                              PAGE_READWRITE);

            win32_init_dsound(window, sound_output.samples_per_second, sound_output.secondary_buffer_size);
            win32_clear_buffer(&sound_output);
            g_secondary_buffer->Play(0, 0, DSBPLAY_LOOPING);

            LARGE_INTEGER last_counter;
            QueryPerformanceCounter(&last_counter);
            u64 last_cycle_count = __rdtsc();

            g_running = true;
            while (g_running)
            {
                MSG message;

                while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        g_running = false;
                    }
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }

#if 0
                //  [TODO] Should we poll this more frequently?
                for (DWORD controller_index = 0; controller_index < XUSER_MAX_COUNT;
                     controller_index++)
                {
                    XINPUT_STATE controller_state;
                    if (XInputGetState(controller_index, &controller_state))
                    {
                        // [NOTE] The controller is plugged in.
                        // [TODO] See if controller_state.dwPacketNumber increments too rapidly.
                        XINPUT_GAMEPAD* pad = &controller_state.Gamepad;

                        bool button_up         = pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
                        bool button_down       = pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                        bool button_left       = pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                        bool button_right      = pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                        bool button_start      = pad->wButtons & XINPUT_GAMEPAD_START;
                        bool button_back       = pad->wButtons & XINPUT_GAMEPAD_BACK;
                        bool button_l_shoulder = pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                        bool button_r_shoulder = pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                        bool button_a          = pad->wButtons & XINPUT_GAMEPAD_A;
                        bool button_b          = pad->wButtons & XINPUT_GAMEPAD_B;
                        bool button_x          = pad->wButtons & XINPUT_GAMEPAD_X;
                        bool button_y          = pad->wButtons & XINPUT_GAMEPAD_Y;

                        i16 stick_x = pad->sThumbLX;
                        i16 stick_y = pad->sThumbRX;

                        // [TODO] Handle deadzone.
                        x_offset += stick_x / 4096;
                        y_offset += stick_y / 4096;

                        sound_output.tone_hz = 512 + (int)(256.0f * ((f32)stick_y / 30000.0f));
                        sound_output.wave_period = sound_output.samples_per_second / sound_output.tone_hz;
                    }
                    else
                    {
                        // [NOTE] The controller is not available.
                    }
                }
#endif

                b32   sound_is_valid = false;
                DWORD byte_to_lock   = 0;
                DWORD target_cursor  = 0;
                DWORD bytes_to_write = 0;
                DWORD play_cursor    = 0;
                DWORD write_cursor   = 0;
                // [TODO] Tighten up sound logic so that we know where we should be
                // writing to and can anticpate the time spent in the game update.
                if (SUCCEEDED(g_secondary_buffer->GetCurrentPosition(&play_cursor, &write_cursor)))
                {
                    byte_to_lock = (sound_output.running_sample_index * sound_output.bytes_per_sample) %
                                   sound_output.secondary_buffer_size;
                    target_cursor = (play_cursor +
                                     (sound_output.latency_sample_count * sound_output.bytes_per_sample)) %
                                    sound_output.secondary_buffer_size;
                    bytes_to_write;

                    if (byte_to_lock > target_cursor)
                    {
                        bytes_to_write  = sound_output.secondary_buffer_size - byte_to_lock;
                        bytes_to_write += target_cursor;
                    }
                    else
                    {
                        bytes_to_write = target_cursor - byte_to_lock;
                    }

                    sound_is_valid = true;
                }
                game_offscreen_buffer buffer = {};
                buffer.memory                = g_back_buffer.memory;
                buffer.width                 = g_back_buffer.width;
                buffer.height                = g_back_buffer.height;
                buffer.pitch                 = g_back_buffer.pitch;

                game_sound_output_buffer sound_buffer = {};
                sound_buffer.samples_per_second       = sound_output.samples_per_second;
                sound_buffer.sample_count             = bytes_to_write / sound_output.bytes_per_sample;
                sound_buffer.samples                  = samples;

                game_update_and_render(&buffer, x_offset, y_offset, &sound_buffer, sound_output.tone_hz);

                // [NOTE] DirectSound output test.
                if (sound_is_valid)
                {
                    win32_fill_sound_buffer(&sound_output, byte_to_lock, bytes_to_write, &sound_buffer);
                }

                win32_window_dimensions dimensions = win32_get_window_dimensions(window);

                win32_display_buffer_in_window(&g_back_buffer, device_context, dimensions.width, dimensions.height);

                u64           end_cycle_count = __rdtsc();
                LARGE_INTEGER end_counter;
                QueryPerformanceCounter(&end_counter);

                u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
                i64 counter_elapsed = end_counter.QuadPart - last_counter.QuadPart;
                f32 ms_per_frame    = (f32)((1000.0f * (f32)counter_elapsed) / (f32)perf_counter_frequency);
                f32 fps             = (f32)perf_counter_frequency / (f32)counter_elapsed;
                f32 mcpf            = (f32)(cycles_elapsed / (1000.0f * 1000.0f));

#if 0
                char buffer[256];
                sprintf(buffer, "%.02f ms/frame, %.02f frame/s, %.02f M.cycles/frame\n",
                        ms_per_frame, fps, mcpf);
                OutputDebugStringA(buffer);
#endif

                last_counter     = end_counter;
                last_cycle_count = end_cycle_count;
            }
        }
        else
        {
            // [TODO] Logging.
        }
    }
    else
    {
        // [TODO] Logging.
    }

    return 0;
}
