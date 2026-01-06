# Relay Leaf SDK for Node.js (Linux)

P2P Relay Network Client Library for Node.js on Linux.

## Requirements

- Node.js 14+
- Linux 10/11 (x64)
- Build tools for native modules (node-gyp)

## Installation

```bash
# Install dependencies
npm install ffi-napi ref-napi ref-struct-napi
```

Copy the `lib` folder to your project:

```
your-project/
├── lib/
│   ├── relay_leaf.js
│   └── librelay_leaf.so
└── main.js
```

## Quick Start

```javascript
const { RelayLeaf } = require('./lib/relay_leaf');

const client = new RelayLeaf(true);
client.create();

client.setPartnerId('your-partner-id');
client.start();

// Get stats
const stats = client.getStats();
console.log(`Connected: ${stats.connected}`);

// Cleanup
client.stop();
client.destroy();
```

## API Reference

### RelayLeaf Class

#### Constructor
```javascript
const client = new RelayLeaf(verbose = false);
```

#### Methods

| Method | Description |
|--------|-------------|
| `create()` | Initialize the client |
| `destroy()` | Free resources |
| `setDiscoveryUrl(url)` | Set discovery URL |
| `setPartnerId(id)` | Set partner ID |
| `addProxy(url)` | Add proxy server |
| `start()` | Start the client |
| `stop()` | Stop the client |
| `getDeviceId()` | Get device ID |
| `getStats()` | Get statistics |
| `isConnected()` | Check connection |

#### Static Methods

| Method | Description |
|--------|-------------|
| `RelayLeaf.version()` | Get library version |

### Stats Object

```javascript
{
    uptimeSeconds: number,
    totalStreams: number,
    bytesSent: number,
    bytesReceived: number,
    reconnectCount: number,
    activeStreams: number,
    connectedNodes: number,
    connected: boolean,
    lastError: string,
    exitPointsJson: string,
    nodeAddressesJson: string
}
```

## How to Run

### Prerequisites
- Node.js 14 or higher
- Linux x64 (Ubuntu 20.04+, Debian 11+, etc.)
- Build essentials (gcc, g++, make)

### Step 1: Install Build Tools (if needed)
```bash
sudo apt-get install build-essential
```

### Step 2: Set Library Path
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
```

### Step 3: Navigate to Example
```bash
cd sdk/linux/nodejs/example
```

### Step 4: Install Dependencies
```bash
npm install
```

### Step 5: Run the Example
```bash
npm start
# or
node main.js
```

### Run from Your Project
```bash
npm install ffi-napi ref-napi ref-struct-napi
```
```javascript
const { RelayLeaf, ensureLibrary } = require('./lib/relay_leaf');

if (ensureLibrary()) {
    const client = new RelayLeaf(true);
    client.create();
    // ...
}
```

### Auto-Download Native Library
```javascript
const { ensureLibrary } = require('./lib/relay_leaf');

ensureLibrary().then(ready => {
    if (ready) console.log("Library ready!");
});
```

## Error Handling

```javascript
const { RelayLeaf, RelayLeafError } = require('./lib/relay_leaf');

try {
    const client = new RelayLeaf();
    client.create();
    client.start();
} catch (err) {
    if (err instanceof RelayLeafError) {
        console.error(`Error: ${err.message}`);
        console.error(`Code: ${err.code}`);
    }
}
```

## License

MIT License
