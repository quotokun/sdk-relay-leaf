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

#ifdef __cplusplus
}
#endif

#endif /* RELAY_LEAF_H */
