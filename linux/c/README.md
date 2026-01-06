# Relay Leaf SDK for C (Linux)

P2P Relay Network Client Library for C on Linux.

## Requirements

- GCC (MinGW-w64) or MSVC
- Linux 10/11 (x64)

## Files

```
lib/
├── relay_leaf.h    # Header file
└── librelay_leaf.so  # Dynamic library
```

## Quick Start

```c
#include "relay_leaf.h"

int main() {
    // Create client
    RelayLeafHandle handle;
    int code = relay_leaf_create(true, &handle);
    if (code != RELAY_LEAF_OK) {
        return 1;
    }

    // Configure
    relay_leaf_set_partner_id(handle, "your-partner-id");

    // Start
    relay_leaf_start(handle);

    // Get stats
    RelayLeafStats stats;
    relay_leaf_get_stats(handle, &stats);
    printf("Connected: %d\n", stats.connected);
    relay_leaf_free_stats(&stats);

    // Cleanup
    relay_leaf_stop(handle);
    relay_leaf_destroy(handle);

    return 0;
}
```

## API Reference

### Lifecycle Functions

```c
int relay_leaf_create(bool verbose, RelayLeafHandle* out_handle);
int relay_leaf_destroy(RelayLeafHandle handle);
```

### Configuration Functions

```c
int relay_leaf_set_discovery_url(RelayLeafHandle handle, const char* url);
int relay_leaf_set_partner_id(RelayLeafHandle handle, const char* partner_id);
int relay_leaf_add_proxy(RelayLeafHandle handle, const char* proxy_url);
```

### Control Functions

```c
int relay_leaf_start(RelayLeafHandle handle);
int relay_leaf_stop(RelayLeafHandle handle);
```

### Query Functions

```c
char* relay_leaf_get_device_id(RelayLeafHandle handle);
int relay_leaf_get_stats(RelayLeafHandle handle, RelayLeafStats* out_stats);
char* relay_leaf_version(void);
char* relay_leaf_error_message(int code);
```

### Memory Management

```c
void relay_leaf_free_string(char* str);
void relay_leaf_free_stats(RelayLeafStats* stats);
```

### Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | RELAY_LEAF_OK | Success |
| 1 | RELAY_LEAF_ERR_NULL_PARAM | Null parameter |
| 2 | RELAY_LEAF_ERR_INVALID_HANDLE | Invalid handle |
| 3 | RELAY_LEAF_ERR_CREATE_FAILED | Create failed |
| 4 | RELAY_LEAF_ERR_START_FAILED | Start failed |
| 5 | RELAY_LEAF_ERR_ALREADY_STARTED | Already running |
| 6 | RELAY_LEAF_ERR_NOT_STARTED | Not started |
| 7 | RELAY_LEAF_ERR_INVALID_PROXY | Invalid proxy |
| 99 | RELAY_LEAF_ERR_INTERNAL | Internal error |

### Stats Structure

```c
typedef struct {
    int64_t uptime_seconds;
    int64_t total_streams;
    int64_t bytes_sent;
    int64_t bytes_received;
    int64_t reconnect_count;
    char* last_error;           // Must free
    char* exit_points_json;     // Must free
    char* node_addresses_json;  // Must free
    int32_t active_streams;
    int32_t connected_nodes;
    bool connected;
} RelayLeafStats;
```

## How to Run

### Prerequisites
- GCC or Clang
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)

### Option 1: Build with GCC

```bash
cd sdk/linux/c/example

# Compile
gcc -Wall -O2 -o example main.c -L../lib -lrelay_leaf -Wl,-rpath,'$ORIGIN/../lib'

# Or set library path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../lib

# Run
./example
```

### Option 2: Build with Make

```bash
cd sdk/linux/c/example
make run
```

### Run from Your Project
1. Copy `lib/relay_leaf.h` to your project
2. Copy `lib/librelay_leaf.so` to /usr/local/lib or set LD_LIBRARY_PATH
3. Link with -lrelay_leaf

### Auto-Download Native Library
Define RELAY_LEAF_IMPLEMENTATION in ONE .c file:
```c
#define RELAY_LEAF_IMPLEMENTATION
#include "relay_leaf.h"

int main() {
    // Ensure librelay_leaf.so exists and is verified
    if (relay_leaf_ensure_library(NULL)) {
        printf("Library ready!\n");
    }
    return 0;
}
```

## Memory Management

**Important:** Always free strings and stats returned by the library:

```c
// Strings
char* device_id = relay_leaf_get_device_id(handle);
// use device_id...
relay_leaf_free_string(device_id);

// Stats
RelayLeafStats stats;
relay_leaf_get_stats(handle, &stats);
// use stats...
relay_leaf_free_stats(&stats);
```

## License

MIT License
