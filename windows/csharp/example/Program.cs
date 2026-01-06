// Relay Leaf SDK Example - Windows (C#)
using System;
using System.Threading;
using RelayLeafSDK;

class Program
{
    static bool running = true;

    static void Main(string[] args)
    {
        Console.WriteLine("==================================================");
        Console.WriteLine("  Relay Leaf SDK Example - Windows (C#)");
        Console.WriteLine("==================================================");
        Console.WriteLine();

        // Ensure native library is available and verified
        if (!LibraryDownloader.EnsureLibrary())
        {
            Console.WriteLine("Failed to ensure native library. Exiting.");
            Environment.Exit(1);
        }
        Console.WriteLine();

        // Show version
        Console.WriteLine($"Library version: {RelayLeaf.Version}");
        Console.WriteLine();

        // Handle Ctrl+C
        Console.CancelKeyPress += (s, e) =>
        {
            e.Cancel = true;
            Console.WriteLine("\nShutting down...");
            running = false;
        };

        try
        {
            // Create client with using statement for automatic cleanup
            using (var client = new RelayLeaf(verbose: true))
            {
                // Get device ID
                Console.WriteLine($"Device ID: {client.GetDeviceId()}");

                // Configure client (uncomment as needed)
                // client.SetDiscoveryUrl("https://discovery.example.com");
                // client.SetPartnerId("your-partner-id");
                // client.AddProxy("socks5://127.0.0.1:1080");

                Console.WriteLine();
                Console.WriteLine("Starting relay client...");
                client.Start();
                Console.WriteLine("Client started! Press Ctrl+C to stop.");
                Console.WriteLine();

                // Main loop - print stats every 5 seconds
                while (running)
                {
                    var stats = client.GetStats();
                    if (stats != null)
                    {
                        Console.WriteLine("----------------------------------------");
                        Console.WriteLine($"Connected: {stats.Connected}");
                        Console.WriteLine($"Uptime: {stats.UptimeSeconds}s");
                        Console.WriteLine($"Connected Nodes: {stats.ConnectedNodes}");
                        Console.WriteLine($"Active Streams: {stats.ActiveStreams}");
                        Console.WriteLine($"Total Streams: {stats.TotalStreams}");
                        Console.WriteLine($"Bytes Sent: {stats.BytesSent:N0}");
                        Console.WriteLine($"Bytes Received: {stats.BytesReceived:N0}");
                        Console.WriteLine($"Reconnects: {stats.ReconnectCount}");
                        if (!string.IsNullOrEmpty(stats.LastError))
                        {
                            Console.WriteLine($"Last Error: {stats.LastError}");
                        }
                    }

                    // Wait 5 seconds
                    for (int i = 0; i < 50 && running; i++)
                    {
                        Thread.Sleep(100);
                    }
                }

                Console.WriteLine();
                Console.WriteLine("Stopping client...");
                client.Stop();
            }
        }
        catch (RelayLeafException ex)
        {
            Console.WriteLine($"Error: {ex.Message} (code: {ex.ErrorCode})");
            Environment.Exit(1);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Unexpected error: {ex.Message}");
            Environment.Exit(1);
        }

        Console.WriteLine("Done!");
    }
}
