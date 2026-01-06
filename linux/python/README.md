# Relay Leaf SDK for Python (Linux)

P2P Relay Network Client Library for Python on Linux.

## Requirements

- Python 3.8+
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)

## Installation

Copy the `lib` folder to your project, or add it to your Python path.

```
your-project/
├── lib/
│   ├── relay_leaf.py
│   └── librelay_leaf.so
└── main.py
```

Make sure the shared library is executable:
```bash
chmod +x lib/librelay_leaf.so
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

#### Methods

| Method | Description |
|--------|-------------|
| `create()` | Initialize the client |
| `destroy()` | Free resources |
| `set_discovery_url(url)` | Set discovery service URL |
| `set_partner_id(partner_id)` | Set partner ID |
| `add_proxy(proxy_url)` | Add proxy server |
| `start()` | Start the client |
| `stop()` | Stop the client |
| `get_device_id()` | Get device ID |
| `get_stats()` | Get statistics |
| `is_connected()` | Check connection |
| `version()` | Get library version |

## How to Run

### Prerequisites
- Python 3.8 or higher
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)

### Step 1: Set Library Permissions
```bash
chmod +x lib/librelay_leaf.so
```

### Step 2: Set Library Path (if needed)
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

### Step 3: Run the Example
```bash
cd sdk/linux/python/example
python3 main.py
```

### Run from Your Project
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
```python
from lib.relay_leaf import ensure_library

# Ensure librelay_leaf.so exists and is verified
if ensure_library():
    print("Library ready!")
```

## Troubleshooting

### Library not found
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

### Permission denied
```bash
chmod +x lib/librelay_leaf.so
```

## License

MIT License
