#include <lizard/lizard.h>

#include <windows.h>
#include <winhttp.h>
#include <strsafe.h>
#include <assert.h>

#define LZ_HTTP_READ_BUFFER_SIZE 8192

struct lz_http
{
        HINTERNET hSession;
        HINTERNET hConnect;
        HINTERNET hRequest;
        uint8_t* readBuffer;
        uint8_t* responseBuffer;
        DWORD responseCapacity;
        DWORD responseLength;
        wchar_t* userAgentW;
        int recvTimeout;
};

void LzHttp_Free(LzHttp* ctx)
{
        if (!ctx)
                return;

        if (ctx->hRequest)
        {
                WinHttpCloseHandle(ctx->hRequest);
                ctx->hRequest = NULL;
        }

        if (ctx->hConnect)
        {
                WinHttpCloseHandle(ctx->hConnect);
                ctx->hConnect = NULL;
        }

        if (ctx->hSession)
        {
                WinHttpCloseHandle(ctx->hSession);
                ctx->hSession = NULL;
        }

        if (ctx->readBuffer)
        {
                free(ctx->readBuffer);
                ctx->readBuffer = NULL;
        }

        if (ctx->responseBuffer)
        {
                free(ctx->responseBuffer);
                ctx->responseBuffer = NULL;
        }

        if (ctx->userAgentW)
        {
                free(ctx->userAgentW);
                ctx->userAgentW = NULL;
        }

        free(ctx);
}

LzHttp* LzHttp_New(const char* userAgent)
{
        LzHttp* ctx = (LzHttp*) calloc(1, sizeof(LzHttp));

        if (!ctx)
                return NULL;

        if (!userAgent)
                goto error;

        ctx->recvTimeout = 30 * 1000;
        ctx->userAgentW = LzUnicode_UTF8toUTF16_dup(userAgent);

        if (!ctx->userAgentW)
                goto error;

        ctx->readBuffer = (uint8_t*) malloc(LZ_HTTP_READ_BUFFER_SIZE);

        if (!ctx->readBuffer)
                goto error;

        return ctx;

error:
        LzHttp_Free(ctx);

        return NULL;
}

int LzHttp_SetRecvTimeout(LzHttp* ctx, int timeout)
{
        if (timeout < -1)
                return LZ_ERROR_PARAM;

        ctx->recvTimeout = timeout;

        return LZ_OK;
}

char* LzHttp_Response(LzHttp* ctx)
{
        if (!ctx)
                return NULL;

        if (ctx->responseBuffer)
                ctx->responseBuffer[ctx->responseLength] = '\0';

        return ctx->responseBuffer;
}

uint32_t LzHttp_ResponseLength(LzHttp* ctx)
{
        if (!ctx)
                return 0;

        return ctx->responseLength;
}

static int LzHttp_OnWriteResponse(void* param, LzHttp* ctx, uint8_t* data, DWORD len)
{
        if ((ctx->responseLength + len) > ctx->responseCapacity)
        {
                DWORD newCapacity = ctx->responseCapacity * 2;
                uint8_t* newFileData = (uint8_t*) realloc(ctx->responseBuffer, newCapacity);

                if (!newFileData)
                        return LZ_ERROR_MEM;

                ctx->responseBuffer = newFileData;
                ctx->responseCapacity = newCapacity;
        }

        memcpy(ctx->responseBuffer + ctx->responseLength, data, len);

        ctx->responseLength += len;

        return 0;
}

int LzHttp_Get(LzHttp* ctx, const char* url, fnHttpWriteFunction writeCallback, void* param, DWORD* error)
{
        int result = LZ_ERROR_FAIL;
        DWORD lastError = ERROR_SUCCESS;
        bool done = false;
        DWORD dwAvailableSize = 0;
        DWORD dwDownloaded = 0;
        wchar_t* urlW = NULL;
        URL_COMPONENTSW urlParts;
        wchar_t* hostnameW = NULL;
        wchar_t* pathW = NULL;
        LPCWSTR accepts[2] = { L"*/*", NULL };
        DWORD requestFlags = 0;
        DWORD protocols = WINHTTP_FLAG_SECURE_PROTOCOL_SSL3 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1 |
                WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_1 | WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;

        if (!ctx || !url)
        {
                result = LZ_ERROR_PARAM;
                goto cleanup;
        }

        if (!writeCallback)
        {
                ctx->responseCapacity = 20 * 1000;
                ctx->responseBuffer = (LPSTR) malloc(ctx->responseCapacity);

                if (!ctx->responseBuffer)
                {
                        result = LZ_ERROR_MEM;
                        goto cleanup;
                }

                writeCallback = LzHttp_OnWriteResponse;
                param = NULL;
        }

        urlW = LzUnicode_UTF8toUTF16_dup(url);

        if (!urlW)
        {
                result = LZ_ERROR_PARAM;
                goto cleanup;
        }

        ZeroMemory(&urlParts, sizeof(urlParts));
        urlParts.dwStructSize = sizeof(urlParts);
        urlParts.dwHostNameLength = -1;
        urlParts.dwUrlPathLength = -1;

        if (!WinHttpCrackUrl(urlW, 0, 0, &urlParts))
        {
                result = LZ_ERROR_DATA;
                goto cleanup;
        }

        hostnameW = malloc((urlParts.dwHostNameLength + 1) * sizeof(wchar_t));
        pathW = malloc((urlParts.dwUrlPathLength + 1) * sizeof(wchar_t));

        if (!hostnameW || !pathW)
        {
                result = LZ_ERROR_MEM;
                goto cleanup;
        }

        wcsncpy_s(hostnameW, urlParts.dwHostNameLength + 1, urlParts.lpszHostName, urlParts.dwHostNameLength);
        wcsncpy_s(pathW, urlParts.dwUrlPathLength + 1, urlParts.lpszUrlPath, urlParts.dwUrlPathLength);

        ctx->hSession = WinHttpOpen(ctx->userAgentW, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

        if (!ctx->hSession)
        {
                lastError = GetLastError();
                result = LZ_ERROR_FAIL;
                goto cleanup;
        }

        WinHttpSetTimeouts(ctx->hSession, 0, 60 * 1000, 30 * 1000, ctx->recvTimeout);
        WinHttpSetOption(ctx->hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &protocols, sizeof(protocols));

        ctx->hConnect = WinHttpConnect(ctx->hSession, hostnameW, urlParts.nPort, 0);

        if (!ctx->hConnect)
        {
                lastError = GetLastError();
                result = LZ_ERROR_FAIL;
                goto cleanup;
        }
        
        if (urlParts.nScheme == INTERNET_SCHEME_HTTPS)
                requestFlags |= WINHTTP_FLAG_SECURE;

        ctx->hRequest = WinHttpOpenRequest(ctx->hConnect, L"GET", pathW, NULL, WINHTTP_NO_REFERER, accepts, requestFlags);

        if (!ctx->hRequest)
        {
                lastError = GetLastError();
                result = LZ_ERROR_FAIL;
                goto cleanup;
        }

        if (!WinHttpSendRequest(ctx->hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        {
                lastError = GetLastError();
                result = LZ_ERROR_FAIL;
                goto cleanup;
        }

        if (!WinHttpReceiveResponse(ctx->hRequest, NULL))
        {
                lastError = GetLastError();
                result = LZ_ERROR_FAIL;
                goto cleanup;
        }

        do
        {
                dwAvailableSize = 0;

                if (!WinHttpQueryDataAvailable(ctx->hRequest, &dwAvailableSize))
                {
                        lastError = GetLastError();
                        result = LZ_ERROR_FAIL;
                        goto cleanup;
                }

                do
                {
                        if (!WinHttpReadData(ctx->hRequest, ctx->readBuffer, min(dwAvailableSize, LZ_HTTP_READ_BUFFER_SIZE), &dwDownloaded))
                        {
                                lastError = GetLastError();
                                result = LZ_ERROR_FAIL;
                                goto cleanup;
                        }

                        if (dwDownloaded == 0)
                        {
                                done = true;
                                break;
                        }

                        result = writeCallback(param, ctx, ctx->readBuffer, dwDownloaded);
                        
                        dwAvailableSize -= dwDownloaded;

                        if (result != LZ_OK)
                        {
                                goto cleanup;
                        }
                } while (dwAvailableSize > 0);
        } while (!done);

cleanup:
        if (urlW)
        {
                free(urlW);
                urlW = NULL;
        }

        if (hostnameW)
        {
                free(hostnameW);
                hostnameW = NULL;
        }

        if (pathW)
        {
                free(pathW);
                pathW = NULL;
        }

        if (lastError > 0 && error)
                *error = lastError;

        return result;
}

int LzHttp_CrackUrl(const char* url, int component, char* pszResult, size_t cchResult)
{
        int result = LZ_ERROR_FAIL;
        DWORD flags = 0;
        wchar_t* urlW = NULL;
        URL_COMPONENTSW urlParts;
        wchar_t* compW = NULL;
        DWORD compLen = 0;
        char* comp = NULL;

        if (!url)
        {
                result = LZ_ERROR_PARAM;
                goto cleanup;
        }

        urlW = LzUnicode_UTF8toUTF16_dup(url);

        if (!urlW)
        {
                result = LZ_ERROR_PARAM;
                goto cleanup;
        }

        ZeroMemory(&urlParts, sizeof(urlParts));
        urlParts.dwStructSize = sizeof(urlParts);
        urlParts.dwSchemeLength = -1;
        urlParts.dwHostNameLength = -1;
        urlParts.dwUrlPathLength = -1;
        urlParts.dwExtraInfoLength = -1;

        if (!WinHttpCrackUrl(urlW, 0, flags, &urlParts))
                goto cleanup;

        switch (component)
        {
                case LZ_URL_COMPONENT_SCHEME:
                        compW = urlParts.lpszScheme;
                        compLen = urlParts.dwSchemeLength;
                        break;
                case LZ_URL_COMPONENT_HOST:
                        compW = urlParts.lpszHostName;
                         compLen = urlParts.dwHostNameLength;
                        break;
                case LZ_URL_COMPONENT_PATH:
                        compW = urlParts.lpszUrlPath;
                         compLen = urlParts.dwUrlPathLength;
                        break;
                case LZ_URL_COMPONENT_EXTRA:
                        compW = urlParts.lpszExtraInfo;
                         compLen = urlParts.dwExtraInfoLength;
                        break;
                default:
                        result = LZ_ERROR_PARAM;
                        goto cleanup;
        }

        comp = LzUnicode_UTF16toUTF8_dup(compW);

        if (!comp)
                goto cleanup;

        if (compLen <= cchResult - 1)
                result = LZ_OK;

        strncpy(pszResult, comp, min(cchResult, compLen));
        pszResult[cchResult - 1] = '\0';

cleanup:
        if (urlW)
        {
                free(urlW);
                urlW = NULL;
        }

        if (comp)
        {
                free(comp);
                comp = NULL;
        }

        return result;
}