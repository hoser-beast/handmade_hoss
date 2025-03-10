#include <stdint.h>
#include <windows.h>
#include <xinput.h>

#define local_persist   static
#define global_variable static
#define internal        static

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

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

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD user_index, XINPUT_STATE* state)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(x_input_get_state_stub) { return 0; }
global_variable x_input_get_state* XInputGetState_ = x_input_get_state_stub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD user_index, XINPUT_VIBRATION* vibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(x_input_set_state_stub) { return 0; }
global_variable x_input_set_state* XInputSetState_ = x_input_set_state_stub;
#define XInputSetState XInputSetState_

internal void
win32_load_x_input(void)
{
    HMODULE x_input_library = LoadLibraryA("xinput1_4.dll");
    if (x_input_library)
    {
        XInputGetState = (x_input_get_state*)GetProcAddress(x_input_library, "XInputGetState");
        XInputSetState = (x_input_set_state*)GetProcAddress(x_input_library, "XInputSetState");
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
render_weird_gradient(win32_offscreen_buffer* buffer, i32 blue_offset, i32 green_offset)
{
    u8* row = (u8*)buffer->memory;

    for (i32 y = 0; y < buffer->height; y++)
    {
        u32* pixel = (u32*)row;

        for (i32 x = 0; x < buffer->width; x++)
        {
            // Pixel in memory: BB GG RR xx where xx is padding, BB is blue, GG
            // is green, and RR is red.
            u8 blue  = x + blue_offset;
            u8 green = y + green_offset;

            *pixel++ = (green << 8) | blue;
        }

        row += buffer->pitch;
    }
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

    buffer->memory = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch  = buffer->width * bytes_per_pixel;

    // [TODO] Probably clear this to black.
}

internal void
win32_display_buffer_in_window(win32_offscreen_buffer* buffer, HDC device_context, i32 window_width,
                               i32 window_height)
{
    // [TODO] Aspect ratio correction.
    // [TODO] Fix stretch modes.
    StretchDIBits(device_context, 0, 0, window_width, window_height, 0, 0, buffer->width,
                  buffer->height, buffer->memory, &buffer->info, DIB_RGB_COLORS, SRCCOPY);
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

            win32_display_buffer_in_window(&g_back_buffer, device_context, dimensions.width,
                                           dimensions.height);
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
    win32_load_x_input();

    WNDCLASSA window_class = {};

    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = win32_main_window_proc;
    window_class.hInstance     = instance;
    window_class.lpszClassName = "HandmadeHossWindowClass";

    win32_resize_dib_section(&g_back_buffer, 1280, 720);

    if (RegisterClass(&window_class))
    {
        HWND window = CreateWindowEx(0, window_class.lpszClassName, "Handmade Hoss",
                                     WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                     CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, 0);

        if (window)
        {
            // [NOTE] Since we specified CS_OWNDC, we only need one device
            // context since we are not sharing it with anyone else.
            HDC device_context = GetDC(window);

            i32 x_offset = 0;
            i32 y_offset = 0;

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

                        // Test code.
                        x_offset += stick_x >> 12;
                        y_offset += stick_y >> 12;
                    }
                    else
                    {
                        // [NOTE] The controller is not available.
                    }
                }
#endif

                render_weird_gradient(&g_back_buffer, x_offset, y_offset);

                win32_window_dimensions dimensions = win32_get_window_dimensions(window);

                win32_display_buffer_in_window(&g_back_buffer, device_context, dimensions.width,
                                               dimensions.height);
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
