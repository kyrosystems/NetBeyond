#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/x509_crt.h>
#include <psa/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tls.h"
#include "http.h"

#define RAW_LIMIT (NB_NETWORK_MAX_BODY + 16384)

static int split_https(const char *url, char host[256], char port[6], char path[2048]) {
    const char *start, *end, *separator;
    size_t count;
    if (strncmp(url, "https://", 8)) return 0;
    start = url + 8;
    end = start;
    while (*end && *end != '/' && *end != '?' && *end != '#') ++end;
    if (*end == '#') return 0;
    separator = memchr(start, ':', (size_t)(end - start));
    count = (size_t)((separator ? separator : end) - start);
    if (!count || count >= 256 || memchr(start, '@', (size_t)(end - start))) return 0;
    memcpy(host, start, count); host[count] = 0;
    strcpy(port, "443");
    if (separator) {
        size_t digits = (size_t)(end - separator - 1), i;
        if (!digits || digits > 5) return 0;
        for (i = 0; i < digits; ++i)
            if (separator[i + 1] < '0' || separator[i + 1] > '9') return 0;
        memcpy(port, separator + 1, digits); port[digits] = 0;
    }
    if (strlen(end) + (*end == '?') >= 2048) return 0;
    if (!*end) strcpy(path, "/");
    else if (*end == '?') { path[0] = '/'; strcpy(path + 1, end); }
    else strcpy(path, end);
    return !strchr(path, '\r') && !strchr(path, '\n');
}

static int load_roots(mbedtls_x509_crt *roots) {
    char file[MAX_PATH], *slash;
    HCERTSTORE store;
    PCCERT_CONTEXT cert = NULL;
    int count = 0;
    if (GetModuleFileNameA(NULL, file, MAX_PATH)) {
        slash = strrchr(file, '\\');
        if (slash && (size_t)(slash - file) + 11 < MAX_PATH) {
            strcpy(slash + 1, "cacert.pem");
            if (mbedtls_x509_crt_parse_file(roots, file) >= 0) return 1;
        }
    }
    store = CertOpenSystemStoreA(0, "ROOT");
    if (!store) return 0;
    while ((cert = CertEnumCertificatesInStore(store, cert)) != NULL) {
        if (!mbedtls_x509_crt_parse_der(roots, cert->pbCertEncoded,
                                        cert->cbCertEncoded)) ++count;
    }
    CertCloseStore(store, 0);
    return count > 0;
}

int nb_tls_get(const char *url, NB_NetworkResponse *response) {
    mbedtls_net_context net;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    mbedtls_x509_crt roots;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context rng;
    WSADATA wsa;
    NB_HttpResponse parsed;
    char host[256], port[6], path[2048], request[2600];
    char *raw = NULL;
    size_t size = 0, sent = 0, request_size;
    int result, success = 0;
    if (!url || !response || !split_https(url, host, port, path)) return 0;
    response->size = 0; response->status = 0; response->body[0] = 0;
    if (WSAStartup(MAKEWORD(2, 2), &wsa)) return 0;
    mbedtls_net_init(&net); mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&config); mbedtls_x509_crt_init(&roots);
    mbedtls_entropy_init(&entropy); mbedtls_ctr_drbg_init(&rng);
    raw = malloc(RAW_LIMIT);
    if (!raw || psa_crypto_init() != PSA_SUCCESS || !load_roots(&roots)) goto done;
    if (mbedtls_ctr_drbg_seed(&rng, mbedtls_entropy_func, &entropy,
                              (const unsigned char *)"NetBeyond", 9)) goto done;
    if (mbedtls_net_connect(&net, host, port, MBEDTLS_NET_PROTO_TCP)) goto done;
    {
        DWORD timeout = 15000;
        if (setsockopt(net.fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
                       sizeof timeout) ||
            setsockopt(net.fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout,
                       sizeof timeout)) goto done;
    }
    if (mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_CLIENT,
            MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) goto done;
    mbedtls_ssl_conf_authmode(&config, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&config, &roots, NULL);
    mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, &rng);
    if (mbedtls_ssl_setup(&ssl, &config) ||
        mbedtls_ssl_set_hostname(&ssl, host)) goto done;
    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, NULL);
    do { result = mbedtls_ssl_handshake(&ssl); }
    while (result == MBEDTLS_ERR_SSL_WANT_READ ||
           result == MBEDTLS_ERR_SSL_WANT_WRITE);
    if (result || mbedtls_ssl_get_verify_result(&ssl)) goto done;
    result = _snprintf(request, sizeof request,
        "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n"
        "Accept-Encoding: identity\r\nUser-Agent: NetBeyond/0.2\r\n\r\n",
        path, host);
    if (result < 0 || (size_t)result >= sizeof request) goto done;
    request_size = (size_t)result;
    while (sent < request_size) {
        result = mbedtls_ssl_write(&ssl, (const unsigned char *)request + sent,
                                   request_size - sent);
        if (result == MBEDTLS_ERR_SSL_WANT_READ ||
            result == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (result <= 0) goto done;
        sent += (size_t)result;
    }
    while (size < RAW_LIMIT) {
        result = mbedtls_ssl_read(&ssl, (unsigned char *)raw + size, RAW_LIMIT - size);
        if (result == MBEDTLS_ERR_SSL_WANT_READ ||
            result == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (result == 0 || result == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) break;
        if (result < 0) goto done;
        size += (size_t)result;
    }
    if (size == RAW_LIMIT || !nb_http_parse_response(raw, size, &parsed) ||
        parsed.chunked || parsed.body_bytes > NB_NETWORK_MAX_BODY) goto done;
    memcpy(response->body, raw + parsed.header_bytes, parsed.body_bytes);
    response->body[parsed.body_bytes] = 0;
    response->size = parsed.body_bytes; response->status = parsed.status;
    success = 1;
done:
    free(raw);
    mbedtls_ssl_free(&ssl); mbedtls_ssl_config_free(&config);
    mbedtls_x509_crt_free(&roots); mbedtls_ctr_drbg_free(&rng);
    mbedtls_entropy_free(&entropy); mbedtls_net_free(&net);
    WSACleanup();
    return success;
}
