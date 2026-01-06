# RelayLeaf SDK for Ruby (Windows)
# P2P Relay Network Client Library
#
# Requires 'ffi' gem: gem install ffi

require 'ffi'
require 'net/http'
require 'json'
require 'digest'
require 'fileutils'

module RelayLeaf
  extend FFI::Library

  # ============================================================================
  # Library Downloader - Downloads and verifies native libraries from servers
  # ============================================================================

  module LibraryDownloader
    SERVERS = [
      'https://release.prx.network',
      'https://github.com/lebachhiep/relay-leaf-library/releases/latest/download'
    ].freeze

    TIMEOUT = 30

    class << self
      def get_library_name
        'relay_leaf-windows-x64.dll'
      end

      def compute_file_hash(path)
        Digest::SHA256.file(path).hexdigest.upcase
      end

      def fetch_url(url)
        puts "[LibraryDownloader] Fetching #{url}"
        uri = URI(url)
        http = Net::HTTP.new(uri.host, uri.port)
        http.use_ssl = uri.scheme == 'https'
        http.open_timeout = TIMEOUT
        http.read_timeout = TIMEOUT

        response = http.get(uri.request_uri)
        case response
        when Net::HTTPSuccess
          response.body
        when Net::HTTPRedirection
          fetch_url(response['location'])
        else
          nil
        end
      rescue StandardError => e
        puts "[LibraryDownloader] Failed: #{e.message}"
        nil
      end

      def fetch_checksums
        SERVERS.each do |server|
          data = fetch_url("#{server}/checksums.json")
          return JSON.parse(data) if data
        rescue JSON::ParserError
          next
        end
        nil
      end

      def get_expected_hash(checksums, library_name)
        files = checksums['files'] || []
        file = files.find { |f| f['name'] == library_name }
        file&.dig('sha256')
      end

      def download_library(library_name, destination, expected_hash)
        SERVERS.each do |server|
          url = "#{server}/#{library_name}"
          content = fetch_url(url)
          next unless content

          FileUtils.mkdir_p(File.dirname(destination))
          File.binwrite(destination, content)
          puts "[LibraryDownloader] Downloaded #{content.bytesize.to_s.reverse.gsub(/(\d{3})(?=\d)/, '\\1,').reverse} bytes"

          downloaded_hash = compute_file_hash(destination)
          if downloaded_hash.casecmp(expected_hash).zero?
            return true
          end

          puts '[LibraryDownloader] Hash mismatch, trying next server...'
          File.delete(destination)
        rescue StandardError => e
          puts "[LibraryDownloader] Download failed: #{e.message}, trying next server..."
        end
        false
      end

      def ensure_library(library_path = nil)
        library_name = get_library_name
        library_path ||= File.join(File.dirname(__FILE__), library_name)

        puts "[LibraryDownloader] Checking #{library_name}..."

        checksums = fetch_checksums
        unless checksums
          puts '[LibraryDownloader] Failed to fetch checksums from all servers'
          return File.exist?(library_path)
        end

        expected_hash = get_expected_hash(checksums, library_name)
        unless expected_hash
          puts "[LibraryDownloader] No checksum found for #{library_name}"
          return File.exist?(library_path)
        end

        if File.exist?(library_path)
          local_hash = compute_file_hash(library_path)
          if local_hash.casecmp(expected_hash).zero?
            puts "[LibraryDownloader] #{library_name} hash verified OK"
            return true
          end
          puts "[LibraryDownloader] Hash mismatch! Local: #{local_hash}, Expected: #{expected_hash}"
        else
          puts "[LibraryDownloader] #{library_name} not found, downloading..."
        end

        unless download_library(library_name, library_path, expected_hash)
          puts '[LibraryDownloader] Failed to download library from all servers'
          if File.exist?(library_path)
            puts "[LibraryDownloader] Using existing #{library_name} (download failed)"
            return true
          end
          return false
        end

        puts "[LibraryDownloader] #{library_name} downloaded and verified OK"
        true
      end
    end
  end

  # Error codes
  module Error
    OK = 0
    NULL_PARAM = 1
    INVALID_HANDLE = 2
    CREATE_FAILED = 3
    START_FAILED = 4
    ALREADY_STARTED = 5
    NOT_STARTED = 6
    INVALID_PROXY = 7
    INTERNAL = 99
  end

  # Exception class
  class RelayLeafError < StandardError
    attr_reader :code

    def initialize(code, message)
      @code = code
      super(message)
    end
  end

  # Statistics structure
  class Stats < FFI::Struct
    layout :uptime_seconds, :int64,
           :total_streams, :int64,
           :bytes_sent, :int64,
           :bytes_received, :int64,
           :reconnect_count, :int64,
           :last_error, :pointer,
           :exit_points_json, :pointer,
           :node_addresses_json, :pointer,
           :active_streams, :int32,
           :connected_nodes, :int32,
           :connected, :bool
  end

  # Load the library
  lib_path = File.join(File.dirname(__FILE__), 'relay_leaf-windows-x64.dll')
  ffi_lib lib_path

  # Native function bindings
  attach_function :relay_leaf_create, [:bool, :pointer], :int32
  attach_function :relay_leaf_destroy, [:uintptr_t], :int32
  attach_function :relay_leaf_set_discovery_url, [:uintptr_t, :string], :int32
  attach_function :relay_leaf_set_partner_id, [:uintptr_t, :string], :int32
  attach_function :relay_leaf_add_proxy, [:uintptr_t, :string], :int32
  attach_function :relay_leaf_start, [:uintptr_t], :int32
  attach_function :relay_leaf_stop, [:uintptr_t], :int32
  attach_function :relay_leaf_get_device_id, [:uintptr_t], :pointer
  attach_function :relay_leaf_get_stats, [:uintptr_t, :pointer], :int32
  attach_function :relay_leaf_free_stats, [:pointer], :void
  attach_function :relay_leaf_free_string, [:pointer], :void
  attach_function :relay_leaf_version, [], :pointer
  attach_function :relay_leaf_error_message, [:int32], :pointer

  # Client class
  class Client
    def initialize(verbose: false)
      @handle = nil
      @started = false

      handle_ptr = FFI::MemoryPointer.new(:uintptr_t)
      code = RelayLeaf.relay_leaf_create(verbose, handle_ptr)
      check_error(code, 'create')
      @handle = handle_ptr.read(:uintptr_t)
    end

    # Get library version
    def self.version
      ptr = RelayLeaf.relay_leaf_version
      return 'unknown' if ptr.null?
      version = ptr.read_string
      RelayLeaf.relay_leaf_free_string(ptr)
      version
    end

    # Set discovery URL
    def set_discovery_url(url)
      check_handle
      code = RelayLeaf.relay_leaf_set_discovery_url(@handle, url)
      check_error(code, 'set_discovery_url')
    end

    # Set partner ID
    def set_partner_id(partner_id)
      check_handle
      code = RelayLeaf.relay_leaf_set_partner_id(@handle, partner_id)
      check_error(code, 'set_partner_id')
    end

    # Add proxy
    def add_proxy(proxy_url)
      check_handle
      code = RelayLeaf.relay_leaf_add_proxy(@handle, proxy_url)
      check_error(code, 'add_proxy')
    end

    # Start client
    def start
      check_handle
      code = RelayLeaf.relay_leaf_start(@handle)
      check_error(code, 'start')
      @started = true
    end

    # Stop client
    def stop
      return if @handle.nil?
      code = RelayLeaf.relay_leaf_stop(@handle)
      check_error(code, 'stop')
      @started = false
    end

    # Get device ID
    def device_id
      return '' if @handle.nil?
      ptr = RelayLeaf.relay_leaf_get_device_id(@handle)
      return '' if ptr.null?
      id = ptr.read_string
      RelayLeaf.relay_leaf_free_string(ptr)
      id
    end

    # Get statistics
    def stats
      return nil if @handle.nil?

      stats_struct = Stats.new
      code = RelayLeaf.relay_leaf_get_stats(@handle, stats_struct.pointer)
      return nil if code != 0

      result = {
        uptime_seconds: stats_struct[:uptime_seconds],
        total_streams: stats_struct[:total_streams],
        bytes_sent: stats_struct[:bytes_sent],
        bytes_received: stats_struct[:bytes_received],
        reconnect_count: stats_struct[:reconnect_count],
        active_streams: stats_struct[:active_streams],
        connected_nodes: stats_struct[:connected_nodes],
        connected: stats_struct[:connected],
        last_error: '',
        exit_points_json: '[]',
        node_addresses_json: '[]'
      }

      unless stats_struct[:last_error].null?
        result[:last_error] = stats_struct[:last_error].read_string
      end
      unless stats_struct[:exit_points_json].null?
        result[:exit_points_json] = stats_struct[:exit_points_json].read_string
      end
      unless stats_struct[:node_addresses_json].null?
        result[:node_addresses_json] = stats_struct[:node_addresses_json].read_string
      end

      RelayLeaf.relay_leaf_free_stats(stats_struct.pointer)
      result
    end

    # Check if connected
    def connected?
      s = stats
      s && s[:connected]
    end

    # Check if started
    def started?
      @started
    end

    # Destroy client
    def destroy
      return if @handle.nil?
      RelayLeaf.relay_leaf_destroy(@handle)
      @handle = nil
    end

    # Alias for destroy
    def close
      destroy
    end

    private

    def check_handle
      raise RelayLeafError.new(Error::INVALID_HANDLE, 'Client not initialized') if @handle.nil?
    end

    def check_error(code, operation)
      return if code == 0
      ptr = RelayLeaf.relay_leaf_error_message(code)
      msg = ptr.null? ? 'Unknown error' : ptr.read_string
      RelayLeaf.relay_leaf_free_string(ptr) unless ptr.null?
      raise RelayLeafError.new(code, "#{operation}: #{msg}")
    end
  end
end
