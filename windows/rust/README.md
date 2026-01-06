# Relay Leaf SDK for Rust (Windows)

P2P Relay Network Client Library for Rust on Windows.

## Requirements

- Rust 1.70+
- Windows 10/11 (x64)

## Installation

Add to your `Cargo.toml`:

```toml
[dependencies]
relay-leaf = { path = "lib" }
```

Or copy the `lib` folder to your project.

## Quick Start

```rust
use relay_leaf_sdk::RelayLeaf;

fn main() -> Result<(), relay_leaf_sdk::RelayError> {
    // Create client
    let client = RelayLeaf::new(true)?;

    // Configure
    client.set_partner_id("your-partner-id")?;

    // Start
    client.start()?;

    // Get stats
    if let Some(stats) = client.get_stats() {
        println!("Connected: {}", stats.connected);
    }

    // Stop (automatic on drop)
    client.stop()?;

    Ok(())
}
```

## API Reference

### RelayLeaf

```rust
impl RelayLeaf {
    fn new(verbose: bool) -> Result<Self>;
    fn set_discovery_url(&self, url: &str) -> Result<()>;
    fn set_partner_id(&self, partner_id: &str) -> Result<()>;
    fn add_proxy(&self, proxy_url: &str) -> Result<()>;
    fn start(&self) -> Result<()>;
    fn stop(&self) -> Result<()>;
    fn get_device_id(&self) -> String;
    fn get_stats(&self) -> Option<Stats>;
    fn is_connected(&self) -> bool;
    fn version() -> String;
}
```

### Stats

```rust
pub struct Stats {
    pub uptime_seconds: i64,
    pub total_streams: i64,
    pub bytes_sent: i64,
    pub bytes_received: i64,
    pub reconnect_count: i64,
    pub last_error: String,
    pub exit_points_json: String,
    pub node_addresses_json: String,
    pub active_streams: i32,
    pub connected_nodes: i32,
    pub connected: bool,
}
```

### Error Codes

```rust
pub enum RelayError {
    Ok = 0,
    NullParam = 1,
    InvalidHandle = 2,
    CreateFailed = 3,
    StartFailed = 4,
    AlreadyStarted = 5,
    NotStarted = 6,
    InvalidProxy = 7,
    Internal = 99,
}
```

## How to Run

### Prerequisites
- Rust 1.70 or higher
- Windows 10/11 (x64)

### Step 1: Navigate to Example
```bash
cd sdk/windows/rust/example
```

### Step 2: Ensure Native Library
```bash
# Copy relay_leaf.dll to example directory (or use LibraryDownloader)
copy ..\lib\relay_leaf.dll .
```

### Step 3: Build and Run
```bash
cargo run
```

### Or Build Release Version
```bash
cargo build --release
./target/release/example.exe
```

### Run from Your Project
1. Add to your `Cargo.toml`:
```toml
[dependencies]
relay-leaf = { path = "lib" }
sha2 = "0.10"
serde = { version = "1.0", features = ["derive"] }
serde_json = "1.0"
ureq = { version = "2.9", features = ["json"] }
```

2. Add `build.rs`:
```rust
fn main() {
    println!("cargo:rustc-link-search=native=.");
    println!("cargo:rustc-link-lib=dylib=relay_leaf");
}
```

### Auto-Download Native Library
The SDK includes LibraryDownloader that automatically downloads and verifies the native DLL:
```rust
use relay_leaf_sdk::ensure_library;

// Ensure relay_leaf.dll exists and is verified
if ensure_library(None) {
    println!("Library ready!");
}
```

## Build Configuration

Add to `build.rs`:

```rust
fn main() {
    println!("cargo:rustc-link-search=native=.");
    println!("cargo:rustc-link-lib=dylib=relay_leaf");
}
```

## License

MIT License
