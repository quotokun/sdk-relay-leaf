#!/usr/bin/env python3
"""
Relay Leaf SDK Example - Windows
Demonstrates basic usage of the P2P Relay client
"""

import sys
import os
import time
import signal

# Add lib directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'lib'))

from relay_leaf import RelayLeaf, RelayLeafException, ensure_library

# Global flag for graceful shutdown
running = True

def signal_handler(sig, frame):
    global running
    print("\nShutting down...")
    running = False

def main():
    global running

    # Setup signal handler for Ctrl+C
    signal.signal(signal.SIGINT, signal_handler)

    print("=" * 50)
    print("  Relay Leaf SDK Example - Windows")
    print("=" * 50)
    print()

    # Ensure native library is available and verified
    if not ensure_library():
        print("Failed to ensure native library. Exiting.")
        return 1
    print()

    # Show version
    print(f"Library version: {RelayLeaf.version()}")
    print()

    try:
        # Create client with context manager for automatic cleanup
        with RelayLeaf(verbose=True) as client:
            # Get device ID
            device_id = client.get_device_id()
            print(f"Device ID: {device_id}")

            # Configure client
            # Uncomment and modify these as needed:
            # client.set_discovery_url("https://discovery.example.com")
            # client.set_partner_id("your-partner-id")
            # client.add_proxy("socks5://127.0.0.1:1080")

            print()
            print("Starting relay client...")
            client.start()
            print("Client started! Press Ctrl+C to stop.")
            print()

            # Main loop - print stats every 5 seconds
            while running:
                stats = client.get_stats()

                if stats:
                    print("-" * 40)
                    print(f"Connected: {stats['connected']}")
                    print(f"Uptime: {stats['uptime_seconds']}s")
                    print(f"Connected Nodes: {stats['connected_nodes']}")
                    print(f"Active Streams: {stats['active_streams']}")
                    print(f"Total Streams: {stats['total_streams']}")
                    print(f"Bytes Sent: {stats['bytes_sent']:,}")
                    print(f"Bytes Received: {stats['bytes_received']:,}")
                    print(f"Reconnects: {stats['reconnect_count']}")
                    if stats['last_error']:
                        print(f"Last Error: {stats['last_error']}")

                # Wait 5 seconds
                for _ in range(50):
                    if not running:
                        break
                    time.sleep(0.1)

            print()
            print("Stopping client...")
            # Client will be automatically stopped and destroyed by context manager

    except RelayLeafException as e:
        print(f"Error: {e.message} (code: {e.code})")
        return 1
    except Exception as e:
        print(f"Unexpected error: {e}")
        return 1

    print("Done!")
    return 0


if __name__ == "__main__":
    sys.exit(main())
