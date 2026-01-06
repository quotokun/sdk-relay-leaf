# Relay Leaf SDK for macOS

P2P Relay Network Client Library for macOS.

## How to Run

### Prerequisites
- macOS 11 (Big Sur) or higher
- Intel Mac (x64) or Apple Silicon (ARM64)
- Go 1.18+ (for building native library)

### Step 1: Build Native Library (if not available)
```bash
cd relay-leaf-main/relay-leaf

# Build for Intel Mac
CGO_ENABLED=1 GOOS=darwin GOARCH=amd64 \
  go build -buildmode=c-shared -o dist/darwin_amd64/librelay_leaf.dylib ./clib/

# Build for Apple Silicon
CGO_ENABLED=1 GOOS=darwin GOARCH=arm64 \
  go build -buildmode=c-shared -o dist/darwin_arm64/librelay_leaf.dylib ./clib/
```

### Step 2: Copy Library to SDK
```bash
# For Intel Mac
cp dist/darwin_amd64/librelay_leaf.dylib sdk/macos/python/lib/

# For Apple Silicon
cp dist/darwin_arm64/librelay_leaf.dylib sdk/macos/python/lib/
```

### Step 3: Run Example (Python)
```bash
cd sdk/macos/python/example
python3 main.py
```

### Auto-Download Native Library
Each SDK includes LibraryDownloader that automatically downloads and verifies the native library:
```python
from lib.relay_leaf import ensure_library

if ensure_library():
    print("Library ready!")
```

## SDK Structure (After Build)

```
macos/
├── python/
│   ├── lib/
│   │   ├── relay_leaf.py
│   │   └── librelay_leaf.dylib
│   ├── example/
│   └── README.md
├── go/
├── csharp/
├── nodejs/
├── rust/
└── c/
```

## Languages Supported

- Python (ctypes)
- Go (cgo)
- C# (.NET Core with P/Invoke)
- Node.js (ffi-napi)
- Rust (FFI)
- C (native)

## License

MIT License
