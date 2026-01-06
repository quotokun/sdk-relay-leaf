# Relay Leaf SDK for Python (Windows)

P2P Relay Network Client Library for Python on Windows.

## Requirements

- Python 3.8+
- Windows 10/11 (x64)

## Installation

Copy the `lib` folder to your project, or add it to your Python path.

```
your-project/
├── lib/
│   ├── relay_leaf.py
│   └── relay_leaf.dll
└── main.py
```

## Quick Start

```python
from lib.relay_leaf import RelayLeaf

# Using context manager (recommended)
with RelayLeaf(verbose=True) as client:
    client.set_partner_id("your-partner-id")
    client.start()

    while client.is_connected():
        stats = client.get_stats()
        print(f"Uptime: {stats['uptime_seconds']}s")
```

## API Reference

### RelayLeaf Class

#### Constructor
```python
client = RelayLeaf(verbose: bool = False)
```
- `verbose`: Enable verbose logging

#### Methods

| Method | Description |
|--------|-------------|
| `create()` | Initialize the client (called automatically) |
| `destroy()` | Free resources (called automatically with context manager) |
| `set_discovery_url(url)` | Set discovery service URL |
| `set_partner_id(partner_id)` | Set partner ID for authentication |
| `add_proxy(proxy_url)` | Add proxy server (socks5:// or http://) |
| `start()` | Start the relay client |
| `stop()` | Stop the relay client |
| `get_device_id()` | Get unique device ID |
| `get_stats()` | Get current statistics |
| `is_connected()` | Check connection status |
| `version()` | Get library version (static) |

### Statistics Dictionary

```python
stats = client.get_stats()
# Returns:
{
    "uptime_seconds": int,
    "total_streams": int,
    "bytes_sent": int,
    "bytes_received": int,
    "reconnect_count": int,
    "active_streams": int,
    "connected_nodes": int,
    "connected": bool,
    "last_error": str,
    "exit_points_json": str,
    "node_addresses_json": str,
}
```

### Error Handling

```python
from lib.relay_leaf import RelayLeaf, RelayLeafException

try:
    client = RelayLeaf()
    client.create()
    client.start()
except RelayLeafException as e:
    print(f"Error: {e.message} (code: {e.code})")
finally:
    client.destroy()
```

### Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | OK | Success |
| 1 | NULL_PARAM | Null parameter passed |
| 2 | INVALID_HANDLE | Invalid client handle |
| 3 | CREATE_FAILED | Failed to create client |
| 4 | START_FAILED | Failed to start client |
| 5 | ALREADY_STARTED | Client already running |
| 6 | NOT_STARTED | Client not started |
| 7 | INVALID_PROXY | Invalid proxy URL |
| 99 | INTERNAL | Internal error |

## How to Run

### Prerequisites
- Python 3.8 or higher
- Windows 10/11 (x64)

### Step 1: Install Dependencies (if needed)
```bash
pip install requests  # Only if using LibraryDownloader features
```

### Step 2: Run the Example
```bash
cd sdk/windows/python/example
python main.py
```

### Step 3: Run from Your Project
```python
# Copy lib/ folder to your project, then:
from lib.relay_leaf import RelayLeaf

with RelayLeaf(verbose=True) as client:
    client.set_partner_id("your-partner-id")
    client.start()

    # Your code here
    stats = client.get_stats()
    print(f"Connected: {stats['connected']}")
```

### Auto-Download Native Library
The SDK includes LibraryDownloader that automatically downloads and verifies the native DLL:
```python
from lib.relay_leaf import ensure_library

# Ensure relay_leaf.dll exists and is verified
if ensure_library():
    print("Library ready!")
```

## Proxy Support

```python
# SOCKS5 proxy
client.add_proxy("socks5://127.0.0.1:1080")

# SOCKS5 with authentication
client.add_proxy("socks5://user:pass@127.0.0.1:1080")

# HTTP proxy
client.add_proxy("http://proxy.example.com:8080")
```

## License

MIT License
