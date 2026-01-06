# Relay Leaf SDK for Node.js (Windows)

P2P Relay Network Client Library for Node.js on Windows.

## Requirements

- Node.js 14+
- Windows 10/11 (x64)
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
│   └── relay_leaf.dll
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
- Windows 10/11 (x64)
- Build tools for native modules (node-gyp)

### Step 1: Install Build Tools (if needed)
```bash
npm install -g windows-build-tools
```

### Step 2: Navigate to Example
```bash
cd sdk/windows/nodejs/example
```

### Step 3: Install Dependencies
```bash
npm install
```

### Step 4: Run the Example
```bash
npm start
# or
node main.js
```

### Run from Your Project
1. Copy `lib/` folder to your project
2. Install FFI dependencies:
```bash
npm install ffi-napi ref-napi ref-struct-napi
```
3. Use in your code:
```javascript
const { RelayLeaf, ensureLibrary } = require('./lib/relay_leaf');

// Ensure library is available
if (ensureLibrary()) {
    const client = new RelayLeaf(true);
    client.create();
    // ...
}
```

### Auto-Download Native Library
The SDK includes LibraryDownloader that automatically downloads and verifies the native DLL:
```javascript
const { ensureLibrary } = require('./lib/relay_leaf');

// Ensure relay_leaf.dll exists and is verified
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
