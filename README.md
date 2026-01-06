# Relay Leaf SDK

Multi-platform, multi-language SDK for P2P Relay Network Client.

## Overview

Relay Leaf SDK provides native bindings for connecting to the P2P relay network. The SDK handles:
- Automatic connection to relay nodes via discovery service
- Stream multiplexing over relay connections
- Proxy support (SOCKS5, HTTP)
- Auto-reconnection with exponential backoff
- Real-time statistics and monitoring

## Supported Platforms

| Platform | Architecture | Native Library | Status |
|----------|--------------|----------------|--------|
| Windows | x64 | `relay_leaf-windows-x64.dll` | Ready |
| Windows | x86 | `relay_leaf-windows-x86.dll` | Ready |
| Linux | x64 | `librelay_leaf-linux-x64.so` | Ready |
| Linux | ARM64 | `librelay_leaf-linux-arm64.so` | Ready |
| macOS | ARM64 (Apple Silicon) | `librelay_leaf-darwin-arm64.dylib` | Ready |
| macOS | x64 (Intel) | `librelay_leaf-darwin-amd64.dylib` | Ready |
| Android | ARM64-v8a | `librelay_leaf.so` | Ready |
| Android | ARMv7a | `librelay_leaf.so` | Ready |
| Android | x86_64 | `librelay_leaf.so` | Ready |

## Supported Languages

| Language | Windows x64 | Windows x86 | Linux | macOS | Android |
|----------|-------------|-------------|-------|-------|---------|
| Python | ✅ | ✅ | ✅ | ✅ | - |
| Go | ✅ | ✅ | ✅ | ✅ | - |
| C# | ✅ | ✅ | ✅ | ✅ | - |
| Node.js | ✅ | - | ✅ | ✅ | - |
| Rust | ✅ | ✅ | ✅ | ✅ | - |
| C/C++ | ✅ | ✅ | ✅ | ✅ | - |
| Ruby | ✅ | - | - | - | - |
| Java | - | - | - | - | ✅ |
| Kotlin | - | - | - | - | ✅ |
| Flutter/Dart | - | - | - | - | ✅ |

## Directory Structure

```
sdk/
├── windows/                    # Windows x64 SDKs
│   ├── python/
│   │   ├── lib/
│   │   │   ├── relay_leaf.py           # Python wrapper with LibraryDownloader
│   │   │   └── relay_leaf-windows-x64.dll
│   │   └── example/
│   │       └── main.py
│   ├── go/
│   ├── csharp/
│   ├── nodejs/
│   ├── rust/
│   ├── c/
│   └── ruby/
│
├── windows-x86/                # Windows x86 (32-bit) SDKs
│   ├── python/
│   ├── go/
│   ├── csharp/
│   ├── rust/
│   └── c/
│
├── linux/                      # Linux x64 SDKs
│   ├── python/
│   ├── go/
│   ├── csharp/
│   ├── nodejs/
│   ├── rust/
│   └── c/
│
├── macos/                      # macOS ARM64 SDKs
│   ├── python/
│   ├── go/
│   ├── csharp/
│   ├── nodejs/
│   ├── rust/
│   └── c/
│
└── android/                    # Android SDKs
    ├── java/
    ├── kotlin/
    └── flutter/
```

## Quick Start

### Auto-Download Native Library

All SDKs include a **LibraryDownloader** that automatically downloads and verifies native libraries from release servers:

**Python:**
```python
from lib.relay_leaf import ensure_library, RelayLeaf

# Auto-download and verify native library
if ensure_library():
    print("Library ready!")

    with RelayLeaf(verbose=True) as client:
        client.set_partner_id("your-partner-id")
        client.start()
```

**Go:**
```go
import relayleaf "./lib"

// Auto-download and verify native library
if relayleaf.EnsureLibrary("") {
    fmt.Println("Library ready!")
}

client, _ := relayleaf.NewClient(true)
defer client.Close()
client.Start()
```

**C#:**
```csharp
using RelayLeafSDK;

// Auto-download and verify native library
if (LibraryDownloader.EnsureLibrary()) {
    Console.WriteLine("Library ready!");
}

using var client = new RelayLeaf(verbose: true);
client.SetPartnerId("your-partner-id");
client.Start();
```

**Node.js:**
```javascript
const { ensureLibrary, RelayLeaf } = require('./lib/relay_leaf');

// Auto-download and verify native library
await ensureLibrary();

const client = new RelayLeaf(true);
client.create();
client.setPartnerId('your-partner-id');
client.start();
```

### Run Examples

| Platform | Language | Command |
|----------|----------|---------|
| Windows x64 | Python | `cd sdk/windows/python/example && python main.py` |
| Windows x64 | Go | `cd sdk/windows/go/example && go run main.go` |
| Windows x64 | C# | `cd sdk/windows/csharp/example && dotnet run` |
| Windows x64 | Node.js | `cd sdk/windows/nodejs/example && npm install && npm start` |
| Windows x64 | Rust | `cd sdk/windows/rust/example && cargo run` |
| Windows x64 | C | `cd sdk/windows/c/example && make run` |
| Windows x64 | Ruby | `cd sdk/windows/ruby/example && ruby main.rb` |
| Linux x64 | Python | `cd sdk/linux/python/example && python3 main.py` |
| Linux x64 | Go | `cd sdk/linux/go/example && go run main.go` |
| Linux x64 | C# | `cd sdk/linux/csharp/example && dotnet run` |
| Linux x64 | Node.js | `cd sdk/linux/nodejs/example && npm install && npm start` |
| Linux x64 | Rust | `cd sdk/linux/rust/example && cargo run` |
| Linux x64 | C | `cd sdk/linux/c/example && make run` |
| Android | Java/Kotlin | Open in Android Studio and run |

## API Reference

### Core Functions

| Function | Description |
|----------|-------------|
| `create(verbose)` | Create a new client instance with optional verbose logging |
| `destroy()` | Destroy the client and free all resources |
| `set_discovery_url(url)` | Set custom discovery service URL (optional) |
| `set_partner_id(id)` | Set partner ID for authentication |
| `add_proxy(url)` | Add proxy server (socks5:// or http://) |
| `start()` | Start the relay client (non-blocking) |
| `stop()` | Stop the relay client gracefully |
| `get_device_id()` | Get unique device identifier |
| `get_stats()` | Get current connection statistics |
| `is_connected()` | Check if connected to relay network |
| `version()` | Get library version string |

### Statistics Structure

```
Stats {
    uptime_seconds      # Connection uptime in seconds
    total_streams       # Total streams processed
    bytes_sent          # Total bytes sent
    bytes_received      # Total bytes received
    reconnect_count     # Number of reconnection attempts
    active_streams      # Currently active streams
    connected_nodes     # Number of connected relay nodes
    connected           # Connection status (bool)
    last_error          # Last error message
    exit_points_json    # JSON array of exit points
    node_addresses_json # JSON array of connected node IPs
}
```

### Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | OK | Success |
| 1 | NULL_PARAM | Null parameter provided |
| 2 | INVALID_HANDLE | Invalid client handle |
| 3 | CREATE_FAILED | Failed to create client |
| 4 | START_FAILED | Failed to start client |
| 5 | ALREADY_STARTED | Client already started |
| 6 | NOT_STARTED | Client not started |
| 7 | INVALID_PROXY | Invalid proxy URL format |
| 99 | INTERNAL | Internal error |

## Code Examples

### Python

```python
from lib.relay_leaf import RelayLeaf, ensure_library

# Ensure library is downloaded and verified
ensure_library()

# Create client with context manager (auto cleanup)
with RelayLeaf(verbose=True) as client:
    # Configure
    client.set_partner_id("your-partner-id")
    # client.set_discovery_url("https://custom-discovery.example.com")
    # client.add_proxy("socks5://127.0.0.1:1080")

    # Start
    client.start()
    print(f"Device ID: {client.get_device_id()}")

    # Monitor
    while client.is_connected():
        stats = client.get_stats()
        print(f"Uptime: {stats['uptime_seconds']}s, Nodes: {stats['connected_nodes']}")
        time.sleep(5)
```

### Go

```go
package main

import (
    "fmt"
    "time"
    relayleaf "./lib"
)

func main() {
    // Ensure library is downloaded
    relayleaf.EnsureLibrary("")

    // Create client
    client, err := relayleaf.NewClient(true)
    if err != nil {
        panic(err)
    }
    defer client.Close()

    // Configure
    client.SetPartnerID("your-partner-id")

    // Start
    client.Start()
    fmt.Printf("Device ID: %s\n", client.GetDeviceID())

    // Monitor
    for client.IsConnected() {
        stats, _ := client.GetStats()
        fmt.Printf("Uptime: %ds, Nodes: %d\n", stats.UptimeSeconds, stats.ConnectedNodes)
        time.Sleep(5 * time.Second)
    }
}
```

### C#

```csharp
using RelayLeafSDK;

// Ensure library is downloaded
LibraryDownloader.EnsureLibrary();

// Create client with using statement (auto dispose)
using var client = new RelayLeaf(verbose: true);

// Configure
client.SetPartnerId("your-partner-id");

// Start
client.Start();
Console.WriteLine($"Device ID: {client.GetDeviceId()}");

// Monitor
while (client.IsConnected)
{
    var stats = client.GetStats();
    Console.WriteLine($"Uptime: {stats.UptimeSeconds}s, Nodes: {stats.ConnectedNodes}");
    Thread.Sleep(5000);
}
```

### Node.js

```javascript
const { RelayLeaf, ensureLibrary } = require('./lib/relay_leaf');

async function main() {
    // Ensure library is downloaded
    await ensureLibrary();

    // Create client
    const client = new RelayLeaf(true);
    client.create();

    try {
        // Configure
        client.setPartnerId('your-partner-id');

        // Start
        client.start();
        console.log(`Device ID: ${client.getDeviceId()}`);

        // Monitor
        const interval = setInterval(() => {
            if (client.isConnected()) {
                const stats = client.getStats();
                console.log(`Uptime: ${stats.uptimeSeconds}s, Nodes: ${stats.connectedNodes}`);
            }
        }, 5000);

    } finally {
        client.stop();
        client.destroy();
    }
}

main();
```

### Rust

```rust
use relay_leaf_sdk::{RelayLeaf, ensure_library};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Ensure library is downloaded
    ensure_library(None);

    // Create client (auto cleanup via Drop)
    let client = RelayLeaf::new(true)?;

    // Configure
    client.set_partner_id("your-partner-id")?;

    // Start
    client.start()?;
    println!("Device ID: {}", client.get_device_id());

    // Monitor
    while client.is_connected() {
        if let Some(stats) = client.get_stats() {
            println!("Uptime: {}s, Nodes: {}", stats.uptime_seconds, stats.connected_nodes);
        }
        std::thread::sleep(std::time::Duration::from_secs(5));
    }

    Ok(())
}
```

### C/C++

```c
#include "relay_leaf.h"
#include <stdio.h>

int main() {
    // Ensure library is downloaded
    ensure_library(NULL);

    // Create client
    RelayLeafHandle handle;
    if (relay_leaf_create(true, &handle) != RELAY_LEAF_OK) {
        printf("Failed to create client\n");
        return 1;
    }

    // Configure
    relay_leaf_set_partner_id(handle, "your-partner-id");

    // Start
    relay_leaf_start(handle);

    char* device_id = relay_leaf_get_device_id(handle);
    printf("Device ID: %s\n", device_id);
    relay_leaf_free_string(device_id);

    // Monitor
    RelayLeafStats stats;
    while (1) {
        if (relay_leaf_get_stats(handle, &stats) == RELAY_LEAF_OK) {
            printf("Uptime: %llds, Nodes: %d, Connected: %d\n",
                   stats.uptime_seconds, stats.connected_nodes, stats.connected);
            relay_leaf_free_stats(&stats);

            if (!stats.connected) break;
        }
        sleep(5);
    }

    // Cleanup
    relay_leaf_stop(handle);
    relay_leaf_destroy(handle);

    return 0;
}
```

## Library Download Servers

SDKs automatically download native libraries from these servers:

1. `https://release.prx.network` (Primary)
2. `https://github.com/lebachhiep/relay-leaf-library/releases/latest/download` (Fallback)

Libraries are verified using SHA256 checksums from `checksums.json`.

## Building Native Libraries

### Windows (x64)

```bash
# Using MinGW-w64
export PATH=/c/ProgramData/mingw64/mingw64/bin:$PATH
CGO_ENABLED=1 GOOS=windows GOARCH=amd64 \
  go build -buildmode=c-shared -o relay_leaf-windows-x64.dll ./clib/
```

### Windows (x86)

```bash
export PATH=/c/ProgramData/mingw32/mingw32/bin:$PATH
CGO_ENABLED=1 GOOS=windows GOARCH=386 \
  go build -buildmode=c-shared -o relay_leaf-windows-x86.dll ./clib/
```

### Linux (x64)

```bash
CGO_ENABLED=1 GOOS=linux GOARCH=amd64 \
  go build -buildmode=c-shared -o librelay_leaf-linux-x64.so ./clib/
```

### Linux (ARM64)

```bash
CGO_ENABLED=1 GOOS=linux GOARCH=arm64 CC=aarch64-linux-gnu-gcc \
  go build -buildmode=c-shared -o librelay_leaf-linux-arm64.so ./clib/
```

### macOS (Apple Silicon)

```bash
CGO_ENABLED=1 GOOS=darwin GOARCH=arm64 \
  go build -buildmode=c-shared -o librelay_leaf-darwin-arm64.dylib ./clib/
```

### macOS (Intel)

```bash
CGO_ENABLED=1 GOOS=darwin GOARCH=amd64 \
  go build -buildmode=c-shared -o librelay_leaf-darwin-amd64.dylib ./clib/
```

### Android (ARM64)

```bash
export ANDROID_NDK_HOME=/path/to/android-ndk
export CC=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang

CGO_ENABLED=1 GOOS=android GOARCH=arm64 \
  go build -buildmode=c-shared -o librelay_leaf.so ./clib/
```

## Memory Management

### Important Notes

1. **Strings returned by native library must be freed:**
   - `relay_leaf_get_device_id()` - free with `relay_leaf_free_string()`
   - `relay_leaf_version()` - free with `relay_leaf_free_string()`
   - `relay_leaf_error_message()` - free with `relay_leaf_free_string()`

2. **Stats structure contains allocated strings:**
   - Always call `relay_leaf_free_stats()` after using stats
   - This frees: `last_error`, `exit_points_json`, `node_addresses_json`

3. **Client handle lifecycle:**
   - Create with `relay_leaf_create()`
   - Destroy with `relay_leaf_destroy()`
   - `relay_leaf_destroy()` automatically calls `relay_leaf_stop()`

## Troubleshooting

### Library not found

```
Error: Library not found: relay_leaf-windows-x64.dll
```

**Solution:** Run `ensure_library()` first, or manually download from release servers.

### Hash mismatch

```
[LibraryDownloader] Hash mismatch! Local: ABC..., Expected: XYZ...
```

**Solution:** Delete local library file and re-run `ensure_library()` to download fresh copy.

### Connection failed

```
Error: start: connection failed
```

**Solutions:**
1. Check internet connectivity
2. Verify partner ID is valid
3. Check if proxy settings are correct (if using proxy)
4. Enable verbose mode for detailed logs

## License

MIT License

## Support

- GitHub Issues: [Report bugs and feature requests](https://github.com/lebachhiep/relay-leaf-library/issues)
- Documentation: See individual SDK README files for language-specific details
