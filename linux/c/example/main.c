/**
 * Relay Leaf SDK Example - Linux (C)
 * P2P Relay Network Client
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#define RELAY_LEAF_IMPLEMENTATION
#include "../lib/relay_leaf.h"

static volatile bool running = true;

void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down...\n");
    running = false;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    printf("==================================================\n");
    printf("  Relay Leaf SDK Example - Linux (C)\n");
    printf("==================================================\n\n");

    // Ensure native library is available and verified
    if (!ensure_library(NULL)) {
        fprintf(stderr, "Failed to ensure native library. Exiting.\n");
        return 1;
    }
    printf("\n");

    // Setup signal handler
    signal(SIGINT, signal_handler);

    // Show version
    char* version = relay_leaf_version();
    printf("Library version: %s\n\n", version ? version : "unknown");

    // Create client
    RelayLeafHandle handle;
    int code = relay_leaf_create(true, &handle);
    if (code != RELAY_LEAF_OK) {
        char* msg = relay_leaf_error_message(code);
        fprintf(stderr, "Error creating client: %s\n", msg ? msg : "unknown");
        return 1;
    }

    // Get device ID
    char* device_id = relay_leaf_get_device_id(handle);
    printf("Device ID: %s\n", device_id ? device_id : "unknown");
    if (device_id) relay_leaf_free_string(device_id);

    // Configure client (uncomment as needed)
    // relay_leaf_set_discovery_url(handle, "https://discovery.example.com");
    // relay_leaf_set_partner_id(handle, "your-partner-id");
    // relay_leaf_add_proxy(handle, "socks5://127.0.0.1:1080");

    printf("\nStarting relay client...\n");
    code = relay_leaf_start(handle);
    if (code != RELAY_LEAF_OK) {
        char* msg = relay_leaf_error_message(code);
        fprintf(stderr, "Error starting client: %s\n", msg ? msg : "unknown");
        relay_leaf_destroy(handle);
        return 1;
    }

    printf("Client started! Press Ctrl+C to stop.\n\n");

    // Main loop - print stats every 5 seconds
    while (running) {
        RelayLeafStats stats;
        code = relay_leaf_get_stats(handle, &stats);

        if (code == RELAY_LEAF_OK) {
            printf("----------------------------------------\n");
            printf("Connected: %s\n", stats.connected ? "true" : "false");
            printf("Uptime: %lld s\n", (long long)stats.uptime_seconds);
            printf("Connected Nodes: %d\n", stats.connected_nodes);
            printf("Active Streams: %d\n", stats.active_streams);
            printf("Total Streams: %lld\n", (long long)stats.total_streams);
            printf("Bytes Sent: %lld\n", (long long)stats.bytes_sent);
            printf("Bytes Received: %lld\n", (long long)stats.bytes_received);
            printf("Reconnects: %lld\n", (long long)stats.reconnect_count);
            if (stats.last_error && stats.last_error[0]) {
                printf("Last Error: %s\n", stats.last_error);
            }

            relay_leaf_free_stats(&stats);
        }

        // Wait 5 seconds
        for (int i = 0; i < 50 && running; i++) {
            sleep_ms(100);
        }
    }

    printf("\nStopping client...\n");
    relay_leaf_stop(handle);
    relay_leaf_destroy(handle);

    printf("Done!\n");
    return 0;
}
