/**
 * relay_leaf.h - NetProxy Relay Leaf C API
 *
 * Cross-platform shared library for relay client functionality.
 *
 * Memory Management:
 * - Strings returned by relay_leaf_get_stats (last_error, exit_points_json, node_addresses_json)
 *   must be freed using relay_leaf_free_string() or relay_leaf_free_stats()
 * - Strings returned by relay_leaf_get_device_id(), relay_leaf_version() and relay_leaf_error_message()
 *   must be freed using relay_leaf_free_string()
 * - Handles created with relay_leaf_create() must be destroyed with relay_leaf_destroy()
 */

#ifndef RELAY_LEAF_H
#define RELAY_LEAF_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Platform-specific export macros */
#if defined(_WIN32) || defined(_WIN64)
    #ifdef RELAY_LEAF_EXPORTS
        #define RELAY_LEAF_API __declspec(dllexport)
    #else
        #define RELAY_LEAF_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define RELAY_LEAF_API __attribute__((visibility("default")))
#else
    #define RELAY_LEAF_API
#endif

/* Opaque handle type */
typedef uintptr_t RelayLeafHandle;

/* Error codes */
typedef enum {
    RELAY_LEAF_OK                  = 0,
    RELAY_LEAF_ERR_NULL_PARAM      = 1,
    RELAY_LEAF_ERR_INVALID_HANDLE  = 2,
    RELAY_LEAF_ERR_CREATE_FAILED   = 3,
    RELAY_LEAF_ERR_START_FAILED    = 4,
    RELAY_LEAF_ERR_ALREADY_STARTED = 5,
    RELAY_LEAF_ERR_NOT_STARTED     = 6,
    RELAY_LEAF_ERR_INVALID_PROXY   = 7,   /* Invalid proxy URL */
    RELAY_LEAF_ERR_INTERNAL        = 99
} RelayLeafError;

/* Statistics structure - fields ordered to minimize padding */
typedef struct {
    int64_t uptime_seconds;      /* Connection uptime in seconds */
    int64_t total_streams;       /* Total streams processed */
    int64_t bytes_sent;          /* Total bytes sent */
    int64_t bytes_received;      /* Total bytes received */
    int64_t reconnect_count;     /* Number of reconnection attempts */
    char* last_error;            /* Last error message (caller must free) */
    char* exit_points_json;      /* JSON array of exit points (caller must free) */
    char* node_addresses_json;   /* JSON array of connected node IPs (caller must free) */
    int32_t active_streams;      /* Number of active streams */
    int32_t connected_nodes;     /* Number of connected relay nodes */
    bool connected;              /* Whether connected to any relay node */
} RelayLeafStats;

/**
 * Create a new relay leaf client instance.
 * Configuration is done via setter functions called before relay_leaf_start.
 *
 * @param verbose      Enable verbose (debug) logging
 * @param out_handle   Output. Pointer to receive the client handle
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_create(
    bool verbose,
    RelayLeafHandle* out_handle
);

/**
 * Set the discovery API URL.
 * Must be called before relay_leaf_start.
 * If not called, uses default: https://api.prx.network/public/relay/nodes
 *
 * @param handle  Client handle from relay_leaf_create()
 * @param url     Discovery API URL
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_set_discovery_url(
    RelayLeafHandle handle,
    const char* url
);

/**
 * Set the partner ID.
 * Must be called before relay_leaf_start.
 * This is optional and will be included in ClientRegister if provided.
 *
 * @param handle      Client handle from relay_leaf_create()
 * @param partner_id  Partner identifier string
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_set_partner_id(
    RelayLeafHandle handle,
    const char* partner_id
);

/**
 * Add a proxy URL to the configuration.
 * Must be called before relay_leaf_start.
 * Can be called multiple times to add multiple proxies.
 * Proxy URL format: scheme://[user:pass@]host:port
 * Supported schemes: http, socks5
 *
 * @param handle     Client handle from relay_leaf_create()
 * @param proxy_url  Proxy URL string
 *
 * @return RELAY_LEAF_OK on success, RELAY_LEAF_ERR_INVALID_PROXY for malformed URLs
 */
RELAY_LEAF_API int32_t relay_leaf_add_proxy(
    RelayLeafHandle handle,
    const char* proxy_url
);

/**
 * Start the relay client (non-blocking).
 * The client will run in the background until stopped.
 *
 * @param handle  Client handle from relay_leaf_create()
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_start(RelayLeafHandle handle);

/**
 * Stop the relay client gracefully.
 * Blocks until the client has stopped (with 30s timeout).
 *
 * @param handle  Client handle from relay_leaf_create()
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_stop(RelayLeafHandle handle);

/**
 * Destroy the client and release all resources.
 * Automatically stops the client if still running.
 * The handle becomes invalid after this call.
 *
 * @param handle  Client handle from relay_leaf_create()
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 */
RELAY_LEAF_API int32_t relay_leaf_destroy(RelayLeafHandle handle);

/**
 * Get the auto-generated device identifier.
 * Available immediately after relay_leaf_create().
 *
 * @param handle  Client handle from relay_leaf_create()
 *
 * @return Device ID string (caller must free with relay_leaf_free_string),
 *         or NULL if handle is invalid
 */
RELAY_LEAF_API char* relay_leaf_get_device_id(RelayLeafHandle handle);

/**
 * Get current client statistics.
 *
 * @param handle     Client handle from relay_leaf_create()
 * @param out_stats  Output. Pointer to receive statistics
 *
 * @return RELAY_LEAF_OK on success, error code on failure
 *
 * @note The caller MUST call relay_leaf_free_stats() when done with the stats
 *       to free allocated strings (last_error, exit_points_json, node_addresses_json).
 */
RELAY_LEAF_API int32_t relay_leaf_get_stats(
    RelayLeafHandle handle,
    RelayLeafStats* out_stats
);

/**
 * Free strings allocated in RelayLeafStats.
 *
 * @param stats  Pointer to stats structure to free strings from
 */
RELAY_LEAF_API void relay_leaf_free_stats(RelayLeafStats* stats);

/**
 * Free a string allocated by the library.
 *
 * @param str  String to free (may be NULL)
 */
RELAY_LEAF_API void relay_leaf_free_string(char* str);

/**
 * Get the library version string.
 *
 * @return Version string (caller must free with relay_leaf_free_string)
 */
RELAY_LEAF_API char* relay_leaf_version(void);

/**
 * Get error message for an error code.
 *
 * @param code  Error code from a relay_leaf_* function
 *
 * @return Error message string (caller must free with relay_leaf_free_string)
 */
RELAY_LEAF_API char* relay_leaf_error_message(int32_t code);

/* ============================================================================
 * Library Downloader - Downloads and verifies native libraries from servers
 * ============================================================================ */

/**
 * Get the appropriate library name for current platform
 * @return Static string with library name
 */
const char* get_library_name(void);

/**
 * Compute SHA256 hash of a file
 * @param filepath Path to the file
 * @param hash_out Buffer to store hash (must be at least 65 bytes)
 * @return 0 on success, -1 on error
 */
int compute_file_hash(const char* filepath, char* hash_out);

/**
 * Ensure native library is available and verified
 * @param library_path Optional path to library (NULL for default)
 * @return 1 if library is ready, 0 otherwise
 */
int ensure_library(const char* library_path);

#ifdef __cplusplus
}
#endif

#endif /* RELAY_LEAF_H */

/* ============================================================================
 * Library Downloader Implementation (included inline for single-file SDK)
 * ============================================================================ */

#ifdef RELAY_LEAF_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#include <wincrypt.h>
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "crypt32.lib")
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

static const char* DOWNLOAD_SERVERS[] = {
    "https://release.prx.network",
    "https://github.com/lebachhiep/relay-leaf-library/releases/latest/download"
};
static const int NUM_DOWNLOAD_SERVERS = 2;

const char* get_library_name(void) {
    return "librelay_leaf-linux-x64.so";
}

#ifdef _WIN32
int compute_file_hash(const char* filepath, char* hash_out) {
    HANDLE hFile = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }

    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    int result = -1;

    if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        if (CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
            BYTE buffer[8192];
            DWORD bytesRead;

            while (ReadFile(hFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
                CryptHashData(hHash, buffer, bytesRead, 0);
            }

            BYTE hash[32];
            DWORD hashLen = 32;
            if (CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
                for (DWORD i = 0; i < hashLen; i++) {
                    sprintf(hash_out + (i * 2), "%02X", hash[i]);
                }
                hash_out[64] = '\0';
                result = 0;
            }

            CryptDestroyHash(hHash);
        }
        CryptReleaseContext(hProv, 0);
    }

    CloseHandle(hFile);
    return result;
}

static int download_file_impl(const char* url, const char* destination) {
    printf("[LibraryDownloader] Downloading from %s\n", url);

    HINTERNET hInternet = InternetOpenA("RelayLeafSDK/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        printf("[LibraryDownloader] Failed to open internet\n");
        return -1;
    }

    HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0,
                                       INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (!hUrl) {
        printf("[LibraryDownloader] Failed to open URL\n");
        InternetCloseHandle(hInternet);
        return -1;
    }

    FILE* fp = fopen(destination, "wb");
    if (!fp) {
        printf("[LibraryDownloader] Failed to create file\n");
        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);
        return -1;
    }

    BYTE buffer[8192];
    DWORD bytesRead;
    DWORD totalBytes = 0;

    while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        fwrite(buffer, 1, bytesRead, fp);
        totalBytes += bytesRead;
    }

    fclose(fp);
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);

    printf("[LibraryDownloader] Downloaded %lu bytes\n", totalBytes);
    return totalBytes > 0 ? 0 : -1;
}

static int fetch_checksums_impl(char* expected_hash, const char* library_name) {
    for (int i = 0; i < NUM_DOWNLOAD_SERVERS; i++) {
        char url[512];
        snprintf(url, sizeof(url), "%s/checksums.json", DOWNLOAD_SERVERS[i]);
        printf("[LibraryDownloader] Fetching %s\n", url);

        HINTERNET hInternet = InternetOpenA("RelayLeafSDK/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) continue;

        HINTERNET hUrl = InternetOpenUrlA(hInternet, url, NULL, 0,
                                           INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
        if (!hUrl) {
            InternetCloseHandle(hInternet);
            continue;
        }

        char buffer[16384] = {0};
        DWORD bytesRead;
        DWORD totalRead = 0;

        while (InternetReadFile(hUrl, buffer + totalRead, sizeof(buffer) - totalRead - 1, &bytesRead) && bytesRead > 0) {
            totalRead += bytesRead;
        }

        InternetCloseHandle(hUrl);
        InternetCloseHandle(hInternet);

        char* pos = strstr(buffer, library_name);
        if (pos) {
            char* sha_pos = strstr(pos, "\"sha256\"");
            if (sha_pos) {
                char* hash_start = strchr(sha_pos + 8, '"');
                if (hash_start) {
                    hash_start++;
                    char* hash_end = strchr(hash_start, '"');
                    if (hash_end && (hash_end - hash_start) == 64) {
                        strncpy(expected_hash, hash_start, 64);
                        expected_hash[64] = '\0';
                        return 0;
                    }
                }
            }
        }
    }
    return -1;
}

#else
int compute_file_hash(const char* filepath, char* hash_out) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "sha256sum '%s' 2>/dev/null | cut -d' ' -f1", filepath);

    FILE* fp = popen(cmd, "r");
    if (!fp) return -1;

    if (fgets(hash_out, 65, fp) != NULL) {
        hash_out[64] = '\0';
        for (int i = 0; hash_out[i]; i++) {
            if (hash_out[i] >= 'a' && hash_out[i] <= 'f') {
                hash_out[i] -= 32;
            }
        }
        pclose(fp);
        return 0;
    }

    pclose(fp);
    return -1;
}

static int download_file_impl(const char* url, const char* destination) {
    printf("[LibraryDownloader] Downloading from %s\n", url);

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "curl -sL -o '%s' '%s' 2>/dev/null", destination, url);

    int result = system(cmd);
    if (result == 0) {
        struct stat st;
        if (stat(destination, &st) == 0) {
            printf("[LibraryDownloader] Downloaded %ld bytes\n", (long)st.st_size);
            return 0;
        }
    }
    return -1;
}

static int fetch_checksums_impl(char* expected_hash, const char* library_name) {
    for (int i = 0; i < NUM_DOWNLOAD_SERVERS; i++) {
        char cmd[1024];
        printf("[LibraryDownloader] Fetching %s/checksums.json\n", DOWNLOAD_SERVERS[i]);

        snprintf(cmd, sizeof(cmd),
                 "curl -sL '%s/checksums.json' 2>/dev/null | grep -A1 '\"%s\"' | grep sha256 | cut -d'\"' -f4",
                 DOWNLOAD_SERVERS[i], library_name);

        FILE* fp = popen(cmd, "r");
        if (fp) {
            if (fgets(expected_hash, 65, fp) != NULL) {
                expected_hash[64] = '\0';
                pclose(fp);
                return 0;
            }
            pclose(fp);
        }
    }
    return -1;
}
#endif

static int file_exists_impl(const char* path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
#endif
}

int ensure_library(const char* library_path) {
    const char* library_name = get_library_name();

    char path[512];
    if (library_path && library_path[0]) {
        strncpy(path, library_path, sizeof(path) - 1);
    } else {
        snprintf(path, sizeof(path), "./%s", library_name);
    }

    printf("[LibraryDownloader] Checking %s...\n", library_name);

    char expected_hash[65] = {0};
    if (fetch_checksums_impl(expected_hash, library_name) != 0) {
        printf("[LibraryDownloader] Failed to fetch checksums from all servers\n");
        return file_exists_impl(path);
    }

    if (file_exists_impl(path)) {
        char local_hash[65] = {0};
        if (compute_file_hash(path, local_hash) == 0) {
#ifdef _WIN32
            if (_stricmp(local_hash, expected_hash) == 0) {
#else
            if (strcasecmp(local_hash, expected_hash) == 0) {
#endif
                printf("[LibraryDownloader] %s hash verified OK\n", library_name);
                return 1;
            }
            printf("[LibraryDownloader] Hash mismatch! Local: %s, Expected: %s\n",
                   local_hash, expected_hash);
        }
    } else {
        printf("[LibraryDownloader] %s not found, downloading...\n", library_name);
    }

    for (int i = 0; i < NUM_DOWNLOAD_SERVERS; i++) {
        char url[512];
        snprintf(url, sizeof(url), "%s/%s", DOWNLOAD_SERVERS[i], library_name);

        if (download_file_impl(url, path) == 0) {
            char downloaded_hash[65] = {0};
            if (compute_file_hash(path, downloaded_hash) == 0) {
#ifdef _WIN32
                if (_stricmp(downloaded_hash, expected_hash) == 0) {
#else
                if (strcasecmp(downloaded_hash, expected_hash) == 0) {
#endif
                    printf("[LibraryDownloader] %s downloaded and verified OK\n", library_name);
                    return 1;
                }
                printf("[LibraryDownloader] Downloaded file hash mismatch, trying next server...\n");
                remove(path);
            } else {
                printf("[LibraryDownloader] Failed to verify hash, trying next server...\n");
                remove(path);
            }
        } else {
            printf("[LibraryDownloader] Download failed, trying next server...\n");
        }
    }

    printf("[LibraryDownloader] Failed to download library from all servers\n");
    if (file_exists_impl(path)) {
        printf("[LibraryDownloader] Using existing %s (download failed)\n", library_name);
        return 1;
    }
    return 0;
}

#endif /* RELAY_LEAF_IMPLEMENTATION */
