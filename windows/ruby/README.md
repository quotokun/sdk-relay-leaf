# Relay Leaf SDK for Ruby (Windows)

P2P Relay Network Client Library for Ruby on Windows.

## Requirements

- Ruby 2.7 or higher
- Windows 10/11 (x64)
- FFI gem

## Installation

```bash
gem install ffi
```

## How to Run

### Step 1: Install FFI gem
```bash
gem install ffi
```

### Step 2: Run the Example
```bash
cd sdk/windows/ruby/example
ruby main.rb
```

## Quick Start

```ruby
require_relative 'lib/relay_leaf'

# Create client
client = RelayLeaf::Client.new(verbose: true)

# Configure
client.set_partner_id("your-partner-id")

# Start
client.start

# Get stats
stats = client.stats
puts "Connected: #{stats[:connected]}"

# Cleanup
client.stop
client.destroy
```

## API Reference

### RelayLeaf::Client

```ruby
# Create client
client = RelayLeaf::Client.new(verbose: false)

# Configuration
client.set_discovery_url(url)
client.set_partner_id(partner_id)
client.add_proxy(proxy_url)  # socks5:// or http://

# Control
client.start
client.stop
client.destroy  # or client.close

# Query
client.device_id      # => String
client.stats          # => Hash
client.connected?     # => Boolean
client.started?       # => Boolean

# Class methods
RelayLeaf::Client.version  # => String
```

### Stats Hash

```ruby
{
  uptime_seconds: Integer,
  total_streams: Integer,
  bytes_sent: Integer,
  bytes_received: Integer,
  reconnect_count: Integer,
  active_streams: Integer,
  connected_nodes: Integer,
  connected: Boolean,
  last_error: String,
  exit_points_json: String,
  node_addresses_json: String
}
```

### Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | OK | Success |
| 1 | NULL_PARAM | Null parameter |
| 2 | INVALID_HANDLE | Invalid handle |
| 3 | CREATE_FAILED | Create failed |
| 4 | START_FAILED | Start failed |
| 5 | ALREADY_STARTED | Already running |
| 6 | NOT_STARTED | Not started |
| 7 | INVALID_PROXY | Invalid proxy |
| 99 | INTERNAL | Internal error |

## Error Handling

```ruby
begin
  client = RelayLeaf::Client.new
  client.start
rescue RelayLeaf::RelayLeafError => e
  puts "Error: #{e.message}"
  puts "Code: #{e.code}"
ensure
  client&.destroy
end
```

## License

MIT License
