# Relay Leaf SDK for Go (Windows)

P2P Relay Network Client Library for Go on Windows.

## Requirements

- Go 1.18+
- Windows 10/11 (x64)

## Installation

Copy the `lib` folder to your project:

```
your-project/
├── lib/
│   ├── relay_leaf.go
│   └── relay_leaf.dll
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
- Windows 10/11 (x64)

### Step 1: Initialize Go Module (first time)
```bash
cd sdk/windows/go/example
go mod init example
go mod tidy
```

### Step 2: Build and Run
```bash
go build -o example.exe
./example.exe
```

### Or Run Directly
```bash
go run main.go
```

### Auto-Download Native Library
The SDK includes LibraryDownloader that automatically downloads and verifies the native DLL:
```go
import relayleaf "your-module/lib"

// Ensure relay_leaf.dll exists and is verified
if relayleaf.EnsureLibrary("") {
    fmt.Println("Library ready!")
}
```

## License

MIT License
