#include <windows.h>

#define local_persist   static
#define global_variable static
#define internal        static

// [TODO] Global variable for now.
global_variable bool g_running;

global_variable BITMAPINFO g_bitmap_info;
global_variable void*      g_bitmap_memory;
global_variable HBITMAP    g_bitmap_handle;
global_variable HDC        g_bitmap_device_context;

internal void
win32_resize_dib_section(int width, int height)
{
    // [TODO] Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    if (g_bitmap_handle)
    {
        DeleteObject(g_bitmap_handle);
    }

    if (!g_bitmap_device_context)
    {
        // [TODO] Should we recreate these under certain special circumstances?
        g_bitmap_device_context = CreateCompatibleDC(0);
    }

    g_bitmap_info.bmiHeader.biSize        = sizeof(g_bitmap_info.bmiHeader);
    g_bitmap_info.bmiHeader.biWidth       = width;
    g_bitmap_info.bmiHeader.biHeight      = height;
    g_bitmap_info.bmiHeader.biPlanes      = 1;
    g_bitmap_info.bmiHeader.biBitCount    = 32;
    g_bitmap_info.bmiHeader.biCompression = BI_RGB;

    // [TODO] Maybe we can just allocate this ourselves?
    g_bitmap_handle = CreateDIBSection(g_bitmap_device_context, &g_bitmap_info,
                                       DIB_RGB_COLORS, &g_bitmap_memory, 0, 0);
}

internal void
win32_update_window(HDC device_context, int x, int y, int w, int h)
{
    StretchDIBits(device_context, x, y, w, h, x, y, w, h, g_bitmap_memory,
                  &g_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
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

            int w = client_rect.right - client_rect.left;
            int h = client_rect.bottom - client_rect.top;

            win32_resize_dib_section(w, h);
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC         device_context = BeginPaint(window, &paint);

            int x = paint.rcPaint.left;
            int y = paint.rcPaint.top;
            int w = paint.rcPaint.right - paint.rcPaint.left;
            int h = paint.rcPaint.bottom - paint.rcPaint.top;

            win32_update_window(device_context, x, y, w, h);
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

int CALLBACK
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line,
        int cmd_show)
{
    WNDCLASS window_class = {};

    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = win32_main_window_proc;
    window_class.hInstance     = instance;
    window_class.lpszClassName = "HandmadeHossWindowClass";

    if (RegisterClass(&window_class))
    {
        HWND window_handle
            = CreateWindowEx(0, window_class.lpszClassName, "Handmade Hoss",
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0,
                             instance, 0);

        if (window_handle)
        {
            MSG message;

            g_running = true;
            while (g_running)
            {
                BOOL message_result = GetMessage(&message, 0, 0, 0);

                if (message_result > 0)
                {
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
                else
                {
                    break;
                }
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
