#include <windows.h>
#include <time.h>
#include <string>

#define ID_STARTPAUSE 1
#define ID_STOP 2
#define ID_TIMER 3

const wchar_t* App_Name = L"秒表";
const wchar_t* Text_Start = L"开始";
const wchar_t* Text_Pause = L"暂停";
const wchar_t* Text_Stop = L"停止";


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global variables
bool isRunning = false;
bool isStopped = false;
DWORD startTime = 0;
DWORD elapsedTime = 0;
HWND hWndStartPauseButton;
HWND hWndStopButton;
HFONT hFont;
RECT timeRect;  // Time display area

void UpdateTime(HWND hWnd)
{
    if (isRunning)
    {
        elapsedTime = GetTickCount() - startTime;
    }

    // Format elapsed time in milliseconds
    int milliseconds = elapsedTime % 1000;
    int seconds = (elapsedTime / 1000) % 60;
    int minutes = (elapsedTime / 60000) % 60;
    int hours = (elapsedTime / 3600000);

    wchar_t timeStr[64];
    swprintf(timeStr, sizeof(timeStr) / sizeof(wchar_t), L"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

    // Redraw the time area with the new time
    InvalidateRect(hWnd, &timeRect, TRUE); // Only invalidate time area
    UpdateWindow(hWnd); // Trigger WM_PAINT
}

void CreateUIElements(HWND hWnd)
{
    // Set the time display area
    timeRect.left = 10;
    timeRect.top = 10;
    timeRect.right = 230 - 20; // Window width minus some margin (e.g., 10px on each side)
    timeRect.bottom = 50; // Set the bottom of the time display area

    // Calculate left margin for buttons (equal spacing on both sides)
    int buttonMargin = (230 - 2 * 100) / 3; // (Window width - 2 * button width) / 3

    // Start/Pause Button
    hWndStartPauseButton = CreateWindowEx(
        0, L"BUTTON", Text_Start, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        buttonMargin, 60, 80, 30, hWnd, (HMENU)ID_STARTPAUSE, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    // Stop Button
    hWndStopButton = CreateWindowEx(
        0, L"BUTTON", Text_Stop, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        buttonMargin * 2 + 100, 60, 80, 30, hWnd, (HMENU)ID_STOP, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    // Set font (larger size for time display)
    hFont = CreateFontW(
        36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// Handle the custom button background color (red for the Stop button)
HBRUSH hRedBrush = CreateSolidBrush(RGB(255, 0, 0));  // Red background brush

void OnStartPause(HWND hWnd)
{
    if (isRunning)
    {
        // Pause the stopwatch
        elapsedTime += GetTickCount() - startTime;
        isRunning = false;
        SetWindowTextW(hWndStartPauseButton, Text_Start);
        KillTimer(hWnd, ID_TIMER); // Stop the timer
    }
    else
    {
        // Start or resume the stopwatch
        startTime = GetTickCount();
        isRunning = true;
        SetWindowTextW(hWndStartPauseButton, Text_Pause);
        SetTimer(hWnd, ID_TIMER, 50, NULL); // Start the timer with 50ms interval
    }
}

void OnStop(HWND hWnd)
{
    // Stop the stopwatch and reset
    isRunning = false;
    isStopped = true;
    elapsedTime = 0; // Reset time to 0
    SetWindowTextW(hWndStartPauseButton, Text_Start);
    SetWindowTextW(hWndStopButton, Text_Stop);
    InvalidateRect(hWnd, &timeRect, TRUE); // Invalidate only time area
    KillTimer(hWnd, ID_TIMER); // Stop the timer
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"StopwatchClass";
    wc.hbrBackground = CreateSolidBrush(RGB(173, 216, 230)); // Light blue background color

    if (!RegisterClass(&wc))
    {
        return -1;
    }

    // Use WS_OVERLAPPEDWINDOW instead of WS_POPUP to show the title bar and close button
    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wc.lpszClassName, App_Name, WS_OVERLAPPEDWINDOW, // Change to WS_OVERLAPPEDWINDOW
        GetSystemMetrics(SM_CXSCREEN) - 250, 0, 230, 150, NULL, NULL, hInstance, NULL); // Increase window height

    if (!hWnd)
    {
        return -1;
    }

    SetLayeredWindowAttributes(hWnd, RGB(173, 216, 230), 0, LWA_COLORKEY); // Make window transparent
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    CreateUIElements(hWnd);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        break;
    case WM_COMMAND:
        if ((HWND)lParam == hWndStartPauseButton)
        {
            OnStartPause(hWnd);
        }
        else if ((HWND)lParam == hWndStopButton)
        {
            OnStop(hWnd);
        }
        break;
    case WM_TIMER:
        UpdateTime(hWnd); // Update the time display on each timer tick
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Set text color to red
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        // Format elapsed time in milliseconds
        int milliseconds = elapsedTime % 1000;
        int seconds = (elapsedTime / 1000) % 60;
        int minutes = (elapsedTime / 60000) % 60;
        int hours = (elapsedTime / 3600000);

        wchar_t timeStr[64];
        swprintf(timeStr, sizeof(timeStr) / sizeof(wchar_t), L"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

        // Draw the time in the timeRect area
        DrawText(hdc, timeStr, -1, &timeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hWnd); // Destroy the window when the close button is clicked
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}