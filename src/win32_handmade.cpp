#include <windows.h>

LRESULT CALLBACK
main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    LRESULT result = 0;

    switch (message)
    {
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            break;
        }
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            break;
        }
        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
            break;
        }
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATE_APP\n");
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

            static DWORD operation = WHITENESS;

            PatBlt(device_context, x, y, w, h, operation);

            if (operation == WHITENESS)
                operation = BLACKNESS;
            else
                operation = WHITENESS;

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
WinMain(HINSTANCE instance,
        HINSTANCE prev_instance,
        LPSTR     cmd_line,
        int       cmd_show)
{
    WNDCLASS window_class = {};

    window_class.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = main_window_proc;
    window_class.hInstance     = instance;
    window_class.lpszClassName = "HandmadeHossWindowClass";

    if (RegisterClass(&window_class))
    {
        HWND window_handle = CreateWindowEx(0,
                                            window_class.lpszClassName,
                                            "Handmade Hoss",
                                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            CW_USEDEFAULT,
                                            0,
                                            0,
                                            instance,
                                            0);

        if (window_handle)
        {
            MSG message;

            for (;;)
            {
                BOOL message_result = GetMessage(&message, 0, 0, 0);

                if (message_result)
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
            // @TODO: Logging.
        }
    }
    else
    {
        // @TODO: Logging.
    }

    return 0;
}
