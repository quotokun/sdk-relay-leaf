# Relay Leaf SDK for C# (Windows)

P2P Relay Network Client Library for C# on Windows.

## Requirements

- .NET 6.0+ or .NET Framework 4.7.2+
- Windows 10/11 (x64)

## Installation

Copy the `lib` folder to your project:

```
your-project/
├── lib/
│   ├── RelayLeaf.cs
│   └── relay_leaf.dll
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
- .NET 6.0 SDK or higher (or .NET Framework 4.7.2+)
- Windows 10/11 (x64)

### Step 1: Navigate to Example
```bash
cd sdk/windows/csharp/example
```

### Step 2: Run the Example
```bash
dotnet run
```

### Or Build and Run Separately
```bash
dotnet build -c Release
./bin/Release/net6.0/Example.exe
```

### Run from Your Project
1. Copy `lib/RelayLeaf.cs` to your project
2. Copy `lib/relay_leaf.dll` to your output directory
3. Add reference to your .csproj:
```xml
<ItemGroup>
  <Compile Include="lib/RelayLeaf.cs" />
</ItemGroup>
<ItemGroup>
  <None Update="relay_leaf.dll">
    <CopyToOutputDirectory>Always</CopyToOutputDirectory>
  </None>
</ItemGroup>
```

### Auto-Download Native Library
The SDK includes LibraryDownloader that automatically downloads and verifies the native DLL:
```csharp
using RelayLeafSDK;

// Ensure relay_leaf.dll exists and is verified
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
