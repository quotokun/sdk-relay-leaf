# Relay Leaf SDK for C# (Linux)

P2P Relay Network Client Library for C# on Linux.

## Requirements

- .NET 6.0+ or .NET Framework 4.7.2+
- Linux 10/11 (x64)

## Installation

Copy the `lib` folder to your project:

```
your-project/
├── lib/
│   ├── RelayLeaf.cs
│   └── librelay_leaf.so
└── Program.cs
```

## Quick Start

```csharp
using RelayLeafSDK;

// Using statement for automatic cleanup
using (var client = new RelayLeaf(verbose: true))
{
    client.SetPartnerId("your-partner-id");
    client.Start();

    while (client.IsConnected)
    {
        var stats = client.GetStats();
        Console.WriteLine($"Uptime: {stats.UptimeSeconds}s");
        Thread.Sleep(5000);
    }

    client.Stop();
}
```

## API Reference

### RelayLeaf Class

#### Constructor
```csharp
var client = new RelayLeaf(verbose: false);
```

#### Methods

| Method | Description |
|--------|-------------|
| `SetDiscoveryUrl(url)` | Set discovery service URL |
| `SetPartnerId(id)` | Set partner ID |
| `AddProxy(url)` | Add proxy server |
| `Start()` | Start the client |
| `Stop()` | Stop the client |
| `GetDeviceId()` | Get device ID |
| `GetStats()` | Get statistics |
| `Dispose()` | Clean up resources |

#### Properties

| Property | Description |
|----------|-------------|
| `IsConnected` | Connection status |
| `Version` | Library version (static) |

### RelayStats Class

```csharp
public class RelayStats
{
    public long UptimeSeconds { get; }
    public long TotalStreams { get; }
    public long BytesSent { get; }
    public long BytesReceived { get; }
    public long ReconnectCount { get; }
    public string LastError { get; }
    public int ActiveStreams { get; }
    public int ConnectedNodes { get; }
    public bool Connected { get; }
}
```

## How to Run

### Prerequisites
- .NET 6.0 SDK or higher
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)

### Step 1: Set Library Path
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

### Step 2: Navigate to Example
```bash
cd sdk/linux/csharp/example
```

### Step 3: Run the Example
```bash
dotnet run
```

### Or Build and Run Separately
```bash
dotnet build -c Release
./bin/Release/net6.0/Example
```

### Run from Your Project
1. Copy `lib/RelayLeaf.cs` to your project
2. Copy `lib/librelay_leaf.so` to your output directory
3. Set LD_LIBRARY_PATH or copy to /usr/local/lib

### Auto-Download Native Library
```csharp
using RelayLeafSDK;

// Ensure librelay_leaf.so exists and is verified
if (LibraryDownloader.EnsureLibrary())
{
    Console.WriteLine("Library ready!");
}
```

## Error Handling

```csharp
try
{
    using (var client = new RelayLeaf())
    {
        client.Start();
    }
}
catch (RelayLeafException ex)
{
    Console.WriteLine($"Error: {ex.Message}");
    Console.WriteLine($"Code: {ex.ErrorCode}");
}
```

## License

MIT License
