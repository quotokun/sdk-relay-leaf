//! Relay Leaf SDK Example - Windows (Rust)

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;

// Import from lib
mod lib {
    include!("../lib/src/lib.rs");
}

use lib::{RelayLeaf, RelayError, ensure_library};

fn main() {
    println!("==================================================");
    println!("  Relay Leaf SDK Example - Windows (Rust)");
    println!("==================================================");
    println!();

    // Ensure native library is available and verified
    if !ensure_library(None) {
        eprintln!("Failed to ensure native library. Exiting.");
        std::process::exit(1);
    }
    println!();

    // Show version
    println!("Library version: {}", RelayLeaf::version());
    println!();

    // Setup Ctrl+C handler
    let running = Arc::new(AtomicBool::new(true));
    let r = running.clone();

    ctrlc::set_handler(move || {
        println!("\nShutting down...");
        r.store(false, Ordering::SeqCst);
    }).expect("Error setting Ctrl-C handler");

    // Create and run client
    match run_client(running) {
        Ok(_) => println!("Done!"),
        Err(e) => {
            eprintln!("Error: {:?}", e);
            std::process::exit(1);
        }
    }
}

fn run_client(running: Arc<AtomicBool>) -> Result<(), RelayError> {
    // Create client
    let client = RelayLeaf::new(true)?;

    // Get device ID
    println!("Device ID: {}", client.get_device_id());

    // Configure client (uncomment as needed)
    // client.set_discovery_url("https://discovery.example.com")?;
    // client.set_partner_id("your-partner-id")?;
    // client.add_proxy("socks5://127.0.0.1:1080")?;

    println!();
    println!("Starting relay client...");
    client.start()?;
    println!("Client started! Press Ctrl+C to stop.");
    println!();

    // Main loop - print stats every 5 seconds
    while running.load(Ordering::SeqCst) {
        if let Some(stats) = client.get_stats() {
            println!("----------------------------------------");
            println!("Connected: {}", stats.connected);
            println!("Uptime: {}s", stats.uptime_seconds);
            println!("Connected Nodes: {}", stats.connected_nodes);
            println!("Active Streams: {}", stats.active_streams);
            println!("Total Streams: {}", stats.total_streams);
            println!("Bytes Sent: {}", stats.bytes_sent);
            println!("Bytes Received: {}", stats.bytes_received);
            println!("Reconnects: {}", stats.reconnect_count);
            if !stats.last_error.is_empty() {
                println!("Last Error: {}", stats.last_error);
            }
        }

        // Wait 5 seconds
        for _ in 0..50 {
            if !running.load(Ordering::SeqCst) {
                break;
            }
            thread::sleep(Duration::from_millis(100));
        }
    }

    println!();
    println!("Stopping client...");
    client.stop()?;

    Ok(())
}
