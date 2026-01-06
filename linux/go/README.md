# Relay Leaf SDK for Go (Linux)

P2P Relay Network Client Library for Go on Linux.

## Requirements

- Go 1.18+
- Linux 10/11 (x64)

## Installation

Copy the `lib` folder to your project:

```
your-project/
├── lib/
│   ├── relay_leaf.go
│   └── librelay_leaf.so
└── main.go
```

## Quick Start

```go
package main

import (
    "fmt"
    relayleaf "./lib"
)

func main() {
    // Create client
    client, err := relayleaf.NewClient(true)
    if err != nil {
        panic(err)
    }
    defer client.Close()

    // Configure
    client.SetPartnerID("your-partner-id")

    // Start
    if err := client.Start(); err != nil {
        panic(err)
    }

    // Get stats
    stats, _ := client.GetStats()
    fmt.Printf("Connected: %v\n", stats.Connected)

    // Stop
    client.Stop()
}
```

## API Reference

### Client Methods

| Method | Description |
|--------|-------------|
| `NewClient(verbose bool)` | Create new client |
| `Close()` | Destroy client |
| `SetDiscoveryURL(url string)` | Set discovery URL |
| `SetPartnerID(id string)` | Set partner ID |
| `AddProxy(url string)` | Add proxy server |
| `Start()` | Start client |
| `Stop()` | Stop client |
| `GetDeviceID()` | Get device ID |
| `GetStats()` | Get statistics |
| `IsConnected()` | Check connection |

### Stats Struct

```go
type Stats struct {
    UptimeSeconds     int64
    TotalStreams      int64
    BytesSent         int64
    BytesReceived     int64
    ReconnectCount    int64
    LastError         string
    ExitPointsJSON    string
    NodeAddressesJSON string
    ActiveStreams     int32
    ConnectedNodes    int32
    Connected         bool
}
```

## How to Run

### Prerequisites
- Go 1.18 or higher
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)
- GCC or Clang (for CGO)

### Step 1: Set Library Path
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

### Step 2: Initialize Go Module (first time)
```bash
cd sdk/linux/go/example
go mod init example
go mod tidy
```

### Step 3: Build and Run
```bash
go build -o example
./example
```

### Or Run Directly
```bash
go run main.go
```

### Auto-Download Native Library
```go
import relayleaf "your-module/lib"

// Ensure librelay_leaf.so exists and is verified
if relayleaf.EnsureLibrary("") {
    fmt.Println("Library ready!")
}
```

## License

MIT License
