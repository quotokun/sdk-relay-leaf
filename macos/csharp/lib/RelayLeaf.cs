// Relay Leaf SDK for C# (macOS)
// P2P Relay Network Client Library

using System;
using System.IO;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace RelayLeafSDK
{
    // ============================================================================
    // Library Downloader - Downloads and verifies native libraries from servers
    // ============================================================================

    public static class LibraryDownloader
    {
        private static readonly string[] Servers = new[]
        {
            "https://release.prx.network",
            "https://github.com/lebachhiep/relay-leaf-library/releases/latest/download"
        };

        private static readonly HttpClient _httpClient = new HttpClient { Timeout = TimeSpan.FromSeconds(30) };

        public static string GetLibraryName()
        {
            return RuntimeInformation.ProcessArchitecture == Architecture.Arm64
                ? "librelay_leaf-darwin-arm64.dylib" : "librelay_leaf-darwin-amd64.dylib";
        }

        public static string ComputeFileHash(string filePath)
        {
            using var sha256 = SHA256.Create();
            using var stream = File.OpenRead(filePath);
            var hash = sha256.ComputeHash(stream);
            return BitConverter.ToString(hash).Replace("-", "");
        }

        public static bool EnsureLibrary(string? libraryPath = null)
        {
            return EnsureLibraryAsync(libraryPath).GetAwaiter().GetResult();
        }

        public static async Task<bool> EnsureLibraryAsync(string? libraryPath = null)
        {
            var libraryName = GetLibraryName();
            var path = libraryPath ?? Path.Combine(AppContext.BaseDirectory, libraryName);

            Console.WriteLine($"[LibraryDownloader] Checking {libraryName}...");

            var checksums = await FetchChecksumsAsync();
            if (checksums == null)
            {
                Console.WriteLine("[LibraryDownloader] Failed to fetch checksums from all servers");
                return File.Exists(path);
            }

            var expectedHash = GetExpectedHash(checksums, libraryName);
            if (string.IsNullOrEmpty(expectedHash))
            {
                Console.WriteLine($"[LibraryDownloader] No checksum found for {libraryName}");
                return File.Exists(path);
            }

            if (File.Exists(path))
            {
                var localHash = ComputeFileHash(path);
                if (string.Equals(localHash, expectedHash, StringComparison.OrdinalIgnoreCase))
                {
                    Console.WriteLine($"[LibraryDownloader] {libraryName} hash verified OK");
                    return true;
                }
                Console.WriteLine($"[LibraryDownloader] Hash mismatch! Local: {localHash}, Expected: {expectedHash}");
            }
            else
            {
                Console.WriteLine($"[LibraryDownloader] {libraryName} not found, downloading...");
            }

            if (!await DownloadLibraryAsync(libraryName, path, expectedHash))
            {
                Console.WriteLine("[LibraryDownloader] Failed to download library from all servers");
                if (File.Exists(path))
                {
                    Console.WriteLine($"[LibraryDownloader] Using existing {libraryName} (download failed)");
                    return true;
                }
                return false;
            }

            Console.WriteLine($"[LibraryDownloader] {libraryName} downloaded and verified OK");
            return true;
        }

        private static async Task<JsonDocument?> FetchChecksumsAsync()
        {
            foreach (var server in Servers)
            {
                try
                {
                    var url = $"{server}/checksums.json";
                    Console.WriteLine($"[LibraryDownloader] Fetching {url}");
                    var response = await _httpClient.GetStringAsync(url);
                    return JsonDocument.Parse(response);
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[LibraryDownloader] Failed: {ex.Message}");
                }
            }
            return null;
        }

        private static string? GetExpectedHash(JsonDocument checksums, string libraryName)
        {
            if (checksums.RootElement.TryGetProperty("files", out var files))
            {
                foreach (var file in files.EnumerateArray())
                {
                    if (file.TryGetProperty("name", out var name) &&
                        file.TryGetProperty("sha256", out var sha256) &&
                        string.Equals(name.GetString(), libraryName, StringComparison.OrdinalIgnoreCase))
                    {
                        return sha256.GetString();
                    }
                }
            }
            return null;
        }

        private static async Task<bool> DownloadLibraryAsync(string remoteName, string destinationPath, string expectedHash)
        {
            foreach (var server in Servers)
            {
                try
                {
                    var url = $"{server}/{remoteName}";
                    Console.WriteLine($"[LibraryDownloader] Downloading from {url}");

                    var response = await _httpClient.GetAsync(url);
                    if (response.IsSuccessStatusCode)
                    {
                        var bytes = await response.Content.ReadAsByteArrayAsync();
                        var dir = Path.GetDirectoryName(destinationPath);
                        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                            Directory.CreateDirectory(dir);

                        await File.WriteAllBytesAsync(destinationPath, bytes);
                        Console.WriteLine($"[LibraryDownloader] Downloaded {bytes.Length:N0} bytes");

                        var downloadedHash = ComputeFileHash(destinationPath);
                        if (string.Equals(downloadedHash, expectedHash, StringComparison.OrdinalIgnoreCase))
                            return true;

                        Console.WriteLine("[LibraryDownloader] Hash mismatch, trying next server...");
                        File.Delete(destinationPath);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[LibraryDownloader] Download failed: {ex.Message}");
                }
            }
            return false;
        }
    }

    // ============================================================================
    // Relay Leaf SDK - Main Client
    // ============================================================================

    /// <summary>
    /// Error codes returned by the native library
    /// </summary>
    public enum RelayError
    {
        OK = 0,
        NullParam = 1,
        InvalidHandle = 2,
        CreateFailed = 3,
        StartFailed = 4,
        AlreadyStarted = 5,
        NotStarted = 6,
        InvalidProxy = 7,
        Internal = 99
    }

    /// <summary>
    /// Statistics from the relay client
    /// </summary>
    public class RelayStats
    {
        public long UptimeSeconds { get; set; }
        public long TotalStreams { get; set; }
        public long BytesSent { get; set; }
        public long BytesReceived { get; set; }
        public long ReconnectCount { get; set; }
        public string LastError { get; set; }
        public string ExitPointsJson { get; set; }
        public string NodeAddressesJson { get; set; }
        public int ActiveStreams { get; set; }
        public int ConnectedNodes { get; set; }
        public bool Connected { get; set; }
    }

    /// <summary>
    /// Native stats structure matching C layout
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeStats
    {
        public long uptime_seconds;
        public long total_streams;
        public long bytes_sent;
        public long bytes_received;
        public long reconnect_count;
        public IntPtr last_error;
        public IntPtr exit_points_json;
        public IntPtr node_addresses_json;
        public int active_streams;
        public int connected_nodes;
        [MarshalAs(UnmanagedType.I1)]
        public bool connected;
    }

    /// <summary>
    /// Exception thrown by RelayLeaf operations
    /// </summary>
    public class RelayLeafException : Exception
    {
        public RelayError ErrorCode { get; }

        public RelayLeafException(RelayError code, string message)
            : base(message)
        {
            ErrorCode = code;
        }
    }

    /// <summary>
    /// Relay Leaf client for P2P relay network
    /// </summary>
    public class RelayLeaf : IDisposable
    {
        private const string DllName = "librelay_leaf-darwin-arm64.dylib";

        #region Native Methods

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_create(bool verbose, out IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_destroy(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_set_discovery_url(IntPtr handle, byte[] url);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_set_partner_id(IntPtr handle, byte[] partnerId);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_add_proxy(IntPtr handle, byte[] proxyUrl);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_start(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_stop(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr relay_leaf_get_device_id(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern int relay_leaf_get_stats(IntPtr handle, ref NativeStats stats);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void relay_leaf_free_stats(ref NativeStats stats);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern void relay_leaf_free_string(IntPtr str);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr relay_leaf_version();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr relay_leaf_error_message(int code);

        #endregion

        private IntPtr _handle = IntPtr.Zero;
        private bool _disposed = false;

        /// <summary>
        /// Create a new RelayLeaf client
        /// </summary>
        /// <param name="verbose">Enable verbose logging</param>
        public RelayLeaf(bool verbose = false)
        {
            int code = relay_leaf_create(verbose, out _handle);
            CheckError(code, "create");
        }

        /// <summary>
        /// Set the discovery service URL
        /// </summary>
        public void SetDiscoveryUrl(string url)
        {
            byte[] urlBytes = Encoding.UTF8.GetBytes(url + "\0");
            int code = relay_leaf_set_discovery_url(_handle, urlBytes);
            CheckError(code, "set_discovery_url");
        }

        /// <summary>
        /// Set the partner ID for authentication
        /// </summary>
        public void SetPartnerId(string partnerId)
        {
            byte[] idBytes = Encoding.UTF8.GetBytes(partnerId + "\0");
            int code = relay_leaf_set_partner_id(_handle, idBytes);
            CheckError(code, "set_partner_id");
        }

        /// <summary>
        /// Add a proxy server (socks5:// or http://)
        /// </summary>
        public void AddProxy(string proxyUrl)
        {
            byte[] urlBytes = Encoding.UTF8.GetBytes(proxyUrl + "\0");
            int code = relay_leaf_add_proxy(_handle, urlBytes);
            CheckError(code, "add_proxy");
        }

        /// <summary>
        /// Start the relay client
        /// </summary>
        public void Start()
        {
            int code = relay_leaf_start(_handle);
            CheckError(code, "start");
        }

        /// <summary>
        /// Stop the relay client
        /// </summary>
        public void Stop()
        {
            int code = relay_leaf_stop(_handle);
            CheckError(code, "stop");
        }

        /// <summary>
        /// Get the unique device ID
        /// </summary>
        public string GetDeviceId()
        {
            IntPtr ptr = relay_leaf_get_device_id(_handle);
            if (ptr == IntPtr.Zero)
                return "";

            string deviceId = PtrToString(ptr);
            relay_leaf_free_string(ptr);
            return deviceId;
        }

        /// <summary>
        /// Get current statistics
        /// </summary>
        public RelayStats GetStats()
        {
            NativeStats native = new NativeStats();
            int code = relay_leaf_get_stats(_handle, ref native);

            if (code != (int)RelayError.OK)
                return null;

            var stats = new RelayStats
            {
                UptimeSeconds = native.uptime_seconds,
                TotalStreams = native.total_streams,
                BytesSent = native.bytes_sent,
                BytesReceived = native.bytes_received,
                ReconnectCount = native.reconnect_count,
                ActiveStreams = native.active_streams,
                ConnectedNodes = native.connected_nodes,
                Connected = native.connected,
                LastError = PtrToString(native.last_error),
                ExitPointsJson = PtrToString(native.exit_points_json),
                NodeAddressesJson = PtrToString(native.node_addresses_json)
            };

            relay_leaf_free_stats(ref native);
            return stats;
        }

        /// <summary>
        /// Check if client is connected
        /// </summary>
        public bool IsConnected
        {
            get
            {
                var stats = GetStats();
                return stats?.Connected ?? false;
            }
        }

        /// <summary>
        /// Get library version
        /// </summary>
        public static string Version
        {
            get
            {
                IntPtr ptr = relay_leaf_version();
                return ptr != IntPtr.Zero ? PtrToString(ptr) : "unknown";
            }
        }

        private void CheckError(int code, string operation)
        {
            if (code != (int)RelayError.OK)
            {
                IntPtr msgPtr = relay_leaf_error_message(code);
                string msg = PtrToString(msgPtr);
                throw new RelayLeafException((RelayError)code, $"{operation}: {msg}");
            }
        }

        private static string PtrToString(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero)
                return "";

            int length = 0;
            while (Marshal.ReadByte(ptr, length) != 0)
                length++;

            if (length == 0)
                return "";

            byte[] buffer = new byte[length];
            Marshal.Copy(ptr, buffer, 0, length);
            return Encoding.UTF8.GetString(buffer);
        }

        #region IDisposable

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    relay_leaf_stop(_handle);
                    relay_leaf_destroy(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
        }

        ~RelayLeaf()
        {
            Dispose(false);
        }

        #endregion
    }
}
