#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winhttp.h>
#include <string.h>
#include "network.h"
#include "tls.h"

int nb_network_get(const char *url, NB_NetworkResponse *response) {
    HINTERNET session = NULL, connection = NULL, request = NULL;
    URL_COMPONENTS parts;
    WCHAR wide_url[2048], host[256], path[2048], extra[2048];
    WCHAR encoding[64];
    DWORD status = 0, status_size = sizeof status;
    DWORD encoding_size = sizeof encoding, count = 0, block;
    DWORD no_redirect = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    char overflow;
    int done_reading = 0;
    if (!url || !response) return 0;
    if (!strncmp(url, "https://", 8)) return nb_tls_get(url, response);
    if (strncmp(url, "http://", 7)) return 0;
    response->size = 0; response->status = 0; response->body[0] = 0;
    if (!MultiByteToWideChar(CP_UTF8, 0, url, -1, wide_url, 2048)) return 0;
    ZeroMemory(&parts, sizeof parts);
    parts.dwStructSize = sizeof parts;
    parts.lpszHostName = host; parts.dwHostNameLength = 256;
    parts.lpszUrlPath = path; parts.dwUrlPathLength = 2048;
    parts.lpszExtraInfo = extra; parts.dwExtraInfoLength = 2048;
    if (!WinHttpCrackUrl(wide_url, 0, 0, &parts)) return 0;
    if (!path[0]) lstrcpyW(path, L"/");
    if (lstrlenW(path) + lstrlenW(extra) >= 2048) return 0;
    lstrcatW(path, extra);
    session = WinHttpOpen(L"NetBeyond/0.2", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session || !WinHttpSetTimeouts(session, 15000, 15000, 15000, 15000)) goto cleanup;
    connection = WinHttpConnect(session, host, parts.nPort, 0);
    if (!connection) goto cleanup;
    request = WinHttpOpenRequest(connection, L"GET", path, NULL,
                                 WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
    if (!request || !WinHttpSetOption(request, WINHTTP_OPTION_REDIRECT_POLICY,
                                      &no_redirect, sizeof no_redirect)) goto cleanup;
    if (!WinHttpSendRequest(request, L"Accept-Encoding: identity\r\n", (DWORD)-1,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(request, NULL)) goto cleanup;
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
                             WINHTTP_NO_HEADER_INDEX)) goto cleanup;
    response->status = (int)status;
    if (WinHttpQueryHeaders(request, WINHTTP_QUERY_CUSTOM, L"Content-Encoding",
                            encoding, &encoding_size, WINHTTP_NO_HEADER_INDEX) &&
        lstrcmpiW(encoding, L"identity")) goto cleanup;
    for (;;) {
        if (response->size == NB_NETWORK_MAX_BODY) {
            block = 1;
        } else {
            size_t left = NB_NETWORK_MAX_BODY - response->size;
            block = (DWORD)(left < 8192 ? left : 8192);
        }
        if (!WinHttpReadData(request, response->size == NB_NETWORK_MAX_BODY ?
                             (void *)&overflow : (void *)(response->body + response->size),
                             block, &count)) goto cleanup;
        if (!count) { done_reading = 1; break; }
        if (response->size == NB_NETWORK_MAX_BODY) goto cleanup;
        response->size += count;
    }
    response->body[response->size] = 0;
cleanup:
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
    return done_reading;
}
