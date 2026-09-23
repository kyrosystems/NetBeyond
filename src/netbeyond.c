#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "core.h"
#include "html.h"
#include "layout.h"
#include "network.h"

#define ID_ADDRESS 100
#define ID_GO 101
#define ID_RELOAD 102
#define NB_ADDRESS_CAPACITY 2048

static HWND g_address;
static NB_Document g_document;
static NB_Layout g_layout;
static NB_NetworkResponse g_response;

static const char g_welcome[] =
    "<title>NetBeyond</title><body>"
    "<p>Native C renderer is active.</p>"
    "<p>This browser does not use Internet Explorer or ActiveX.</p>"
    "<p>Enter an HTTP or HTTPS URL to load it with WinHTTP.</p>"
    "</body>";

static void utf8_to_wide(const char *input, WCHAR *output, int capacity) {
    MultiByteToWideChar(CP_UTF8, 0, input, -1, output, capacity);
}

static void wide_to_utf8(const WCHAR *input, char *output, int capacity) {
    WideCharToMultiByte(CP_UTF8, 0, input, -1, output, capacity, NULL, NULL);
}

static void set_document(const char *html) {
    if (!nb_html_parse(html, strlen(html), &g_document)) {
        memset(&g_document, 0, sizeof g_document);
        strcpy(g_document.title, "Parse error");
        strcpy(g_document.text, "The native HTML parser rejected this document.");
    }
}

static void set_error_document(const char *title, const char *message) {
    static char html[NB_DOCUMENT_TEXT_MAX];
    _snprintf(html, sizeof html - 1,
              "<title>%s</title><body><p>%s</p></body>", title, message);
    html[sizeof html - 1] = 0;
    set_document(html);
}

static void redraw_document(HWND window) {
    nb_layout_text(g_document.text, 88, &g_layout);
    InvalidateRect(window, NULL, TRUE);
}

static void navigate(HWND window) {
    WCHAR wide_address[NB_ADDRESS_CAPACITY];
    char raw_address[NB_ADDRESS_CAPACITY];
    char normalized_address[NB_ADDRESS_CAPACITY];
    GetWindowTextW(g_address, wide_address, NB_ADDRESS_CAPACITY);
    wide_to_utf8(wide_address, raw_address, sizeof raw_address);
    if (!strcmp(raw_address, "about:welcome") || !strcmp(raw_address, "about:blank")) {
        set_document(g_welcome);
        redraw_document(window);
        return;
    }
    if (nb_address(raw_address, normalized_address, sizeof normalized_address) != 1) {
        set_error_document("Address error", "The address is empty or too long.");
        redraw_document(window);
        return;
    }
    utf8_to_wide(normalized_address, wide_address, NB_ADDRESS_CAPACITY);
    SetWindowTextW(g_address, wide_address);
    if (!nb_network_get(normalized_address, &g_response)) {
        set_error_document("Network error", "Connection, TLS or response read failed.");
        redraw_document(window);
        return;
    }
    if (g_response.status < 200 || g_response.status >= 400) {
        char message[128];
        _snprintf(message, sizeof message - 1, "Server returned HTTP status %d.", g_response.status);
        message[sizeof message - 1] = 0;
        set_error_document("HTTP status", message);
        redraw_document(window);
        return;
    }
    set_document(g_response.body);
    redraw_document(window);
}

static void paint_document(HWND window) {
    PAINTSTRUCT paint;
    HDC dc;
    RECT client;
    RECT tab_strip;
    RECT toolbar;
    HBRUSH brush;
    HFONT body_font;
    HFONT title_font;
    WCHAR line[NB_LAYOUT_LINE_MAX];
    int index;
    int y;
    dc = BeginPaint(window, &paint);
    GetClientRect(window, &client);
    tab_strip.left = 0; tab_strip.top = 0; tab_strip.right = client.right; tab_strip.bottom = 36;
    toolbar.left = 0; toolbar.top = 36; toolbar.right = client.right; toolbar.bottom = 80;
    brush = CreateSolidBrush(RGB(217, 222, 229));
    FillRect(dc, &tab_strip, brush);
    DeleteObject(brush);
    brush = CreateSolidBrush(RGB(245, 247, 250));
    FillRect(dc, &toolbar, brush);
    DeleteObject(brush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(43, 54, 67));
    TextOutW(dc, 20, 12, L"NetBeyond", 10);
    title_font = CreateFontW(-24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    body_font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    SelectObject(dc, title_font);
    utf8_to_wide(g_document.title[0] ? g_document.title : "NetBeyond", line, NB_LAYOUT_LINE_MAX);
    TextOutW(dc, 28, 104, line, lstrlenW(line));
    SelectObject(dc, body_font);
    SetTextColor(dc, RGB(25, 25, 25));
    y = 148;
    for (index = 0; index < (int)g_layout.count && y < client.bottom - 18; ++index) {
        utf8_to_wide(g_layout.text[index], line, NB_LAYOUT_LINE_MAX);
        TextOutW(dc, 30, y, line, lstrlenW(line));
        y += 21;
    }
    DeleteObject(body_font);
    DeleteObject(title_font);
    EndPaint(window, &paint);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE:
        CreateWindowW(L"BUTTON", L"Reload", WS_CHILD | WS_VISIBLE,
                      8, 45, 64, 25, window, (HMENU)ID_RELOAD, NULL, NULL);
        g_address = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"about:welcome",
                                    WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
                                    78, 45, 620, 25, window, (HMENU)ID_ADDRESS, NULL, NULL);
        CreateWindowW(L"BUTTON", L"Go", WS_CHILD | WS_VISIBLE,
                      704, 45, 48, 25, window, (HMENU)ID_GO, NULL, NULL);
        navigate(window);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wparam) == ID_GO || LOWORD(wparam) == ID_RELOAD) navigate(window);
        return 0;
    case WM_KEYDOWN:
        if (wparam == VK_RETURN && GetFocus() == g_address) {
            navigate(window);
            return 0;
        }
        return 0;
    case WM_PAINT:
        paint_document(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPSTR command, int show) {
    WNDCLASSW window_class;
    HWND window;
    MSG message;
    (void)previous;
    (void)command;
    (void)show;
    ZeroMemory(&window_class, sizeof window_class);
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = L"NetBeyondWindow";
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&window_class);
    window = CreateWindowW(window_class.lpszClassName, L"NetBeyond",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                           CW_USEDEFAULT, CW_USEDEFAULT, 920, 680,
                           NULL, NULL, instance, NULL);
    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return 0;
}
