#!/usr/bin/env ruby
# RelayLeaf SDK Example for Ruby (Windows)

require_relative '../lib/relay_leaf'

puts "=== RelayLeaf Ruby SDK Example ==="
puts

# Ensure native library is available and verified
unless RelayLeaf::LibraryDownloader.ensure_library
  puts "Failed to ensure native library. Exiting."
  exit 1
end
puts

puts "Version: #{RelayLeaf::Client.version}"
puts

begin
  # Create client with verbose logging
  client = RelayLeaf::Client.new(verbose: true)
  puts "Client created successfully"
  puts "Device ID: #{client.device_id}"
  puts

  # Configure client
  client.set_partner_id("example-partner-id")
  puts "Partner ID set"

  # Optional: Add proxy
  # client.add_proxy("socks5://127.0.0.1:1080")
  # puts "Proxy added"

  # Start client
  puts "Starting client..."
  client.start
  puts "Client started"
  puts

  # Monitor connection
  puts "Monitoring connection (press Ctrl+C to stop)..."
  loop do
    stats = client.stats
    if stats
      puts "Connected: #{stats[:connected]} | " \
           "Uptime: #{stats[:uptime_seconds]}s | " \
           "Nodes: #{stats[:connected_nodes]} | " \
           "Streams: #{stats[:active_streams]}/#{stats[:total_streams]} | " \
           "Sent: #{stats[:bytes_sent]} | " \
           "Received: #{stats[:bytes_received]}"
    else
      puts "Failed to get stats"
    end
    sleep 5
  end

rescue RelayLeaf::RelayLeafError => e
  puts "RelayLeaf Error: #{e.message} (code: #{e.code})"
rescue Interrupt
  puts "\nStopping..."
ensure
  if defined?(client) && client
    client.stop rescue nil
    client.destroy
    puts "Client destroyed"
  end
end
