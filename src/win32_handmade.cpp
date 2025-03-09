#include <stdint.h>
#include <windows.h>

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

// [TODO] Global variable for now.
global_variable bool g_running;

global_variable BITMAPINFO g_bitmap_info;
global_variable void*      g_bitmap_memory;
global_variable i32        g_bitmap_width;
global_variable i32        g_bitmap_height;

internal void
render_weird_gradient(i32 blue_offset, i32 green_offset)
{
    i32 pitch = g_bitmap_width * 4;
    u8* row   = (u8*)g_bitmap_memory;

    for (i32 y = 0; y < g_bitmap_height; y++)
    {
        u32* pixel = (u32*)row;

        for (i32 x = 0; x < g_bitmap_width; x++)
        {
            // Pixel in memory: BB GG RR xx where xx is padding, BB is blue, GG
            // is green, and RR is red.
            u8 blue  = x + blue_offset;
            u8 green = y + green_offset;

            *pixel++ = (green << 8) | blue;
        }

        row += pitch;
    }
}

internal void
win32_resize_dib_section(i32 width, i32 height)
{
    // [TODO] Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.

    if (g_bitmap_memory)
    {
        VirtualFree(g_bitmap_memory, 0, MEM_RELEASE);
    }

    g_bitmap_width  = width;
    g_bitmap_height = height;

    g_bitmap_info.bmiHeader.biSize        = sizeof(g_bitmap_info.bmiHeader);
    g_bitmap_info.bmiHeader.biWidth       = g_bitmap_width;
    g_bitmap_info.bmiHeader.biHeight      = -g_bitmap_height;
    g_bitmap_info.bmiHeader.biPlanes      = 1;
    g_bitmap_info.bmiHeader.biBitCount    = 32;
    g_bitmap_info.bmiHeader.biCompression = BI_RGB;

    // [TODO] Maybe we can just allocate this ourselves?
    i32 bitmap_memory_size = g_bitmap_width * g_bitmap_height * 4;
    g_bitmap_memory
        = VirtualAlloc(0, bitmap_memory_size, MEM_COMMIT, PAGE_READWRITE);

    // [TODO] Probably clear this to black.
}

internal void
win32_update_window(HDC device_context, RECT* client_rect, i32 x, i32 y, i32 w,
                    i32 h)
{
    i32 window_width  = client_rect->right - client_rect->left;
    i32 window_height = client_rect->bottom - client_rect->top;

    StretchDIBits(device_context, 0, 0, g_bitmap_width, g_bitmap_height, 0, 0,
                  window_width, window_height, g_bitmap_memory, &g_bitmap_info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
win32_main_window_proc(HWND window, UINT message, WPARAM w_param,
                       LPARAM l_param)
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
        case WM_SIZE:
        {
            RECT client_rect;
            GetClientRect(window, &client_rect);

            i32 w = client_rect.right - client_rect.left;
            i32 h = client_rect.bottom - client_rect.top;

            win32_resize_dib_section(w, h);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC         device_context = BeginPaint(window, &paint);

            i32 x = paint.rcPaint.left;
            i32 y = paint.rcPaint.top;
            i32 w = paint.rcPaint.right - paint.rcPaint.left;
            i32 h = paint.rcPaint.bottom - paint.rcPaint.top;

            RECT client_rect;
            GetClientRect(window, &client_rect);

            win32_update_window(device_context, &client_rect, x, y, w, h);
            EndPaint(window, &paint);
            break;
        }
        default:
        {
            // OutputDebugStringA("Default\n");
            result = DefWindowProc(window, message, w_param, l_param);
        };
    }

    return result;
}

i32 CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
        i32 cmd_show)
{
    WNDCLASS window_class = {};

    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = win32_main_window_proc;
    window_class.hInstance     = instance;
    window_class.lpszClassName = "HandmadeHossWindowClass";

    if (RegisterClass(&window_class))
    {
        HWND window
            = CreateWindowEx(0, window_class.lpszClassName, "Handmade Hoss",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                             instance, 0);

        if (window)
        {
            i32 blue_offset  = 0;
            i32 green_offset = 0;

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

                render_weird_gradient(blue_offset, green_offset);

                RECT client_rect;
                GetClientRect(window, &client_rect);

                i32 w = client_rect.right - client_rect.left;
                i32 h = client_rect.bottom - client_rect.top;

                HDC device_context = GetDC(window);
                win32_update_window(device_context, &client_rect, 0, 0, w, h);
                ReleaseDC(window, device_context);

                blue_offset++;
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
