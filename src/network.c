#define _WIN32_WINNT 0x0501
#include "network.h"
#include <windows.h>
#include <winhttp.h>
#include <string.h>

static int utf8_to_wide(const char *input, WCHAR *output, int capacity) {
    return MultiByteToWideChar(CP_UTF8, 0, input, -1, output, capacity) != 0;
}

int nb_network_get(const char *url, NB_NetworkResponse *response) {
    HINTERNET session = NULL, connection = NULL, request = NULL;
    URL_COMPONENTS parts;
    WCHAR wide_url[2048], host[256], path[2048];
    DWORD status = 0, status_size = sizeof status, available, received;
    int secure, ok = 0;
    if (!url || !response || !utf8_to_wide(url, wide_url, 2048)) return 0;
    ZeroMemory(response, sizeof *response);
    ZeroMemory(&parts, sizeof parts);
    parts.dwStructSize = sizeof parts;
    parts.lpszHostName = host; parts.dwHostNameLength = 256;
    parts.lpszUrlPath = path; parts.dwUrlPathLength = 2048;
    if (!WinHttpCrackUrl(wide_url, 0, 0, &parts)) return 0;
    secure = parts.nScheme == INTERNET_SCHEME_HTTPS;
    session = WinHttpOpen(L"NetBeyond/0.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) goto done;
    WinHttpSetTimeouts(session, 15000, 15000, 15000, 15000);
    connection = WinHttpConnect(session, host, parts.nPort, 0);
    if (!connection) goto done;
    request = WinHttpOpenRequest(connection, L"GET", path[0] ? path : L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, secure ? WINHTTP_FLAG_SECURE : 0);
    if (!request || !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(request, NULL)) goto done;
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size, WINHTTP_NO_HEADER_INDEX)) goto done;
    response->status = (int)status;
    while (response->size < NB_NETWORK_MAX_BODY && WinHttpQueryDataAvailable(request, &available) && available) {
        if (available > NB_NETWORK_MAX_BODY - response->size) available = NB_NETWORK_MAX_BODY - (DWORD)response->size;
        if (!WinHttpReadData(request, response->body + response->size, available, &received)) goto done;
        response->size += received;
    }
    response->body[response->size] = 0;
    ok = 1;
done:
    if (request) WinHttpCloseHandle(request);
    if (connection) WinHttpCloseHandle(connection);
    if (session) WinHttpCloseHandle(session);
    return ok;
}
