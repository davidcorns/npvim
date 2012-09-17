#include <callback.h>

extern HWND shwnd;


/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)                  /* handle the messages */
    {
        case WM_DESTROY: {
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
        } break;
        
        case WM_CREATE: {
            
        } break;

        case WM_SIZE: {
            //SendMessage(shwnd, msg, wParam, lParam);
            const int w = LOWORD(lParam);
            const int h = HIWORD(lParam);
            SetWindowPos(shwnd, 0, 0, 0, w, h, SWP_NOZORDER);
        } break;
        
        case WM_CHAR: {
            char str[8];
            wsprintf(str, "%c", wParam);
            MessageBox(hwnd, str, str, MB_OK);
        } break;

        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, msg, wParam, lParam);
    }

    return 0;
}
