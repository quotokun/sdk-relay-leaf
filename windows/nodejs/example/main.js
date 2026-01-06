/**
 * Relay Leaf SDK Example - Windows (Node.js)
 */

const { RelayLeaf, RelayLeafError, ensureLibrary } = require('../lib/relay_leaf');

let running = true;
let client = null;

// Handle Ctrl+C
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    running = false;
});

async function main() {
    console.log('==================================================');
    console.log('  Relay Leaf SDK Example - Windows (Node.js)');
    console.log('==================================================');
    console.log();

    // Ensure native library is available and verified
    if (!await ensureLibrary()) {
        console.log('Failed to ensure native library. Exiting.');
        process.exit(1);
    }
    console.log();

    // Show version
    console.log(`Library version: ${RelayLeaf.version()}`);
    console.log();
    try {
        // Create client
        client = new RelayLeaf(true);
        client.create();

        // Get device ID
        console.log(`Device ID: ${client.getDeviceId()}`);

        // Configure client (uncomment as needed)
        // client.setDiscoveryUrl('https://discovery.example.com');
        // client.setPartnerId('your-partner-id');
        // client.addProxy('socks5://127.0.0.1:1080');

        console.log();
        console.log('Starting relay client...');
        client.start();
        console.log('Client started! Press Ctrl+C to stop.');
        console.log();

        // Main loop - print stats every 5 seconds
        while (running) {
            const stats = client.getStats();
            if (stats) {
                console.log('----------------------------------------');
                console.log(`Connected: ${stats.connected}`);
                console.log(`Uptime: ${stats.uptimeSeconds}s`);
                console.log(`Connected Nodes: ${stats.connectedNodes}`);
                console.log(`Active Streams: ${stats.activeStreams}`);
                console.log(`Total Streams: ${stats.totalStreams}`);
                console.log(`Bytes Sent: ${stats.bytesSent.toLocaleString()}`);
                console.log(`Bytes Received: ${stats.bytesReceived.toLocaleString()}`);
                console.log(`Reconnects: ${stats.reconnectCount}`);
                if (stats.lastError) {
                    console.log(`Last Error: ${stats.lastError}`);
                }
            }

            // Wait 5 seconds
            await new Promise(resolve => {
                const checkInterval = setInterval(() => {
                    if (!running) {
                        clearInterval(checkInterval);
                        resolve();
                    }
                }, 100);
                setTimeout(() => {
                    clearInterval(checkInterval);
                    resolve();
                }, 5000);
            });
        }

        console.log();
        console.log('Stopping client...');
        client.stop();
        client.destroy();

    } catch (err) {
        if (err instanceof RelayLeafError) {
            console.error(`Error: ${err.message} (code: ${err.code})`);
        } else {
            console.error(`Unexpected error: ${err.message}`);
        }
        process.exit(1);
    }

    console.log('Done!');
}

main();
