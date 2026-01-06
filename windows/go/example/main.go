// Relay Leaf SDK Example - Windows
package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	relayleaf "relayleaf-sdk/lib"
)

func main() {
	fmt.Println("==================================================")
	fmt.Println("  Relay Leaf SDK Example - Windows (Go)")
	fmt.Println("==================================================")
	fmt.Println()

	// Ensure native library is available and verified
	if !relayleaf.EnsureLibrary("") {
		fmt.Println("Failed to ensure native library. Exiting.")
		os.Exit(1)
	}
	fmt.Println()

	// Show version
	fmt.Printf("Library version: %s\n", relayleaf.Version())
	fmt.Println()

	// Create client
	client, err := relayleaf.NewClient(true)
	if err != nil {
		fmt.Printf("Error creating client: %v\n", err)
		os.Exit(1)
	}
	defer client.Close()

	// Get device ID
	deviceID := client.GetDeviceID()
	fmt.Printf("Device ID: %s\n", deviceID)

	// Configure client (uncomment as needed)
	// client.SetDiscoveryURL("https://discovery.example.com")
	// client.SetPartnerID("your-partner-id")
	// client.AddProxy("socks5://127.0.0.1:1080")

	fmt.Println()
	fmt.Println("Starting relay client...")

	if err := client.Start(); err != nil {
		fmt.Printf("Error starting client: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("Client started! Press Ctrl+C to stop.")
	fmt.Println()

	// Setup signal handler
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)

	// Stats ticker
	ticker := time.NewTicker(5 * time.Second)
	defer ticker.Stop()

	running := true
	for running {
		select {
		case <-sigChan:
			fmt.Println("\nShutting down...")
			running = false

		case <-ticker.C:
			stats, err := client.GetStats()
			if err != nil {
				fmt.Printf("Error getting stats: %v\n", err)
				continue
			}

			fmt.Println("----------------------------------------")
			fmt.Printf("Connected: %v\n", stats.Connected)
			fmt.Printf("Uptime: %ds\n", stats.UptimeSeconds)
			fmt.Printf("Connected Nodes: %d\n", stats.ConnectedNodes)
			fmt.Printf("Active Streams: %d\n", stats.ActiveStreams)
			fmt.Printf("Total Streams: %d\n", stats.TotalStreams)
			fmt.Printf("Bytes Sent: %d\n", stats.BytesSent)
			fmt.Printf("Bytes Received: %d\n", stats.BytesReceived)
			fmt.Printf("Reconnects: %d\n", stats.ReconnectCount)
			if stats.LastError != "" {
				fmt.Printf("Last Error: %s\n", stats.LastError)
			}
		}
	}

	fmt.Println("Stopping client...")
	if err := client.Stop(); err != nil {
		fmt.Printf("Error stopping client: %v\n", err)
	}

	fmt.Println("Done!")
}
