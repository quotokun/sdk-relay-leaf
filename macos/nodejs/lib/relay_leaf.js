/**
 * Relay Leaf SDK for Node.js (Windows)
 * P2P Relay Network Client Library
 */

const ffi = require('ffi-napi');
const ref = require('ref-napi');
const path = require('path');
const fs = require('fs');
const https = require('https');
const http = require('http');
const crypto = require('crypto');
const os = require('os');

// Types
const voidPtr = ref.refType(ref.types.void);
const handlePtr = ref.refType(ref.types.uint64);
const charPtr = ref.types.CString;
const int = ref.types.int;
const int32 = ref.types.int32;
const int64 = ref.types.int64;
const bool = ref.types.bool;

// Stats structure
const StatsStruct = require('ref-struct-napi')({
    uptime_seconds: int64,
    total_streams: int64,
    bytes_sent: int64,
    bytes_received: int64,
    reconnect_count: int64,
    last_error: charPtr,
    exit_points_json: charPtr,
    node_addresses_json: charPtr,
    active_streams: int32,
    connected_nodes: int32,
    connected: bool
});
const StatsPtr = ref.refType(StatsStruct);

// Error codes
const RelayError = {
    OK: 0,
    NULL_PARAM: 1,
    INVALID_HANDLE: 2,
    CREATE_FAILED: 3,
    START_FAILED: 4,
    ALREADY_STARTED: 5,
    NOT_STARTED: 6,
    INVALID_PROXY: 7,
    INTERNAL: 99
};

// Load native library
const libPath = path.join(__dirname, 'librelay_leaf-darwin-arm64.dylib');
const lib = ffi.Library(libPath, {
    'relay_leaf_create': [int, [bool, handlePtr]],
    'relay_leaf_destroy': [int, [voidPtr]],
    'relay_leaf_set_discovery_url': [int, [voidPtr, charPtr]],
    'relay_leaf_set_partner_id': [int, [voidPtr, charPtr]],
    'relay_leaf_add_proxy': [int, [voidPtr, charPtr]],
    'relay_leaf_start': [int, [voidPtr]],
    'relay_leaf_stop': [int, [voidPtr]],
    'relay_leaf_get_device_id': [charPtr, [voidPtr]],
    'relay_leaf_get_stats': [int, [voidPtr, StatsPtr]],
    'relay_leaf_free_stats': ['void', [StatsPtr]],
    'relay_leaf_free_string': ['void', [charPtr]],
    'relay_leaf_version': [charPtr, []],
    'relay_leaf_error_message': [charPtr, [int]]
});

/**
 * RelayLeaf Error
 */
class RelayLeafError extends Error {
    constructor(code, message) {
        super(message);
        this.code = code;
        this.name = 'RelayLeafError';
    }
}

/**
 * RelayLeaf Client
 */
class RelayLeaf {
    /**
     * Create a new RelayLeaf client
     * @param {boolean} verbose - Enable verbose logging
     */
    constructor(verbose = false) {
        this._handle = null;
        this._verbose = verbose;
    }

    /**
     * Create the client instance
     */
    create() {
        if (this._handle !== null) return;

        const handleBuf = ref.alloc(ref.types.uint64);
        const code = lib.relay_leaf_create(this._verbose, handleBuf);
        this._checkError(code, 'create');
        this._handle = ref.deref(handleBuf);
    }

    /**
     * Destroy the client and free resources
     */
    destroy() {
        if (this._handle === null) return;

        lib.relay_leaf_destroy(this._handle);
        this._handle = null;
    }

    /**
     * Set discovery service URL
     * @param {string} url - Discovery URL
     */
    setDiscoveryUrl(url) {
        this._ensureCreated();
        const code = lib.relay_leaf_set_discovery_url(this._handle, url);
        this._checkError(code, 'set_discovery_url');
    }

    /**
     * Set partner ID for authentication
     * @param {string} partnerId - Partner ID
     */
    setPartnerId(partnerId) {
        this._ensureCreated();
        const code = lib.relay_leaf_set_partner_id(this._handle, partnerId);
        this._checkError(code, 'set_partner_id');
    }

    /**
     * Add a proxy server
     * @param {string} proxyUrl - Proxy URL (socks5:// or http://)
     */
    addProxy(proxyUrl) {
        this._ensureCreated();
        const code = lib.relay_leaf_add_proxy(this._handle, proxyUrl);
        this._checkError(code, 'add_proxy');
    }

    /**
     * Start the relay client
     */
    start() {
        this._ensureCreated();
        const code = lib.relay_leaf_start(this._handle);
        this._checkError(code, 'start');
    }

    /**
     * Stop the relay client
     */
    stop() {
        if (this._handle === null) return;
        const code = lib.relay_leaf_stop(this._handle);
        this._checkError(code, 'stop');
    }

    /**
     * Get the unique device ID
     * @returns {string}
     */
    getDeviceId() {
        this._ensureCreated();
        const ptr = lib.relay_leaf_get_device_id(this._handle);
        if (ptr === null) return '';
        const deviceId = ptr;
        lib.relay_leaf_free_string(ptr);
        return deviceId;
    }

    /**
     * Get current statistics
     * @returns {Object}
     */
    getStats() {
        if (this._handle === null) return null;

        const stats = new StatsStruct();
        const code = lib.relay_leaf_get_stats(this._handle, stats.ref());

        if (code !== RelayError.OK) return null;

        const result = {
            uptimeSeconds: stats.uptime_seconds,
            totalStreams: stats.total_streams,
            bytesSent: stats.bytes_sent,
            bytesReceived: stats.bytes_received,
            reconnectCount: stats.reconnect_count,
            activeStreams: stats.active_streams,
            connectedNodes: stats.connected_nodes,
            connected: stats.connected,
            lastError: stats.last_error || '',
            exitPointsJson: stats.exit_points_json || '[]',
            nodeAddressesJson: stats.node_addresses_json || '[]'
        };

        lib.relay_leaf_free_stats(stats.ref());
        return result;
    }

    /**
     * Check if client is connected
     * @returns {boolean}
     */
    isConnected() {
        const stats = this.getStats();
        return stats?.connected || false;
    }

    /**
     * Get library version
     * @returns {string}
     */
    static version() {
        const ptr = lib.relay_leaf_version();
        return ptr || 'unknown';
    }

    _ensureCreated() {
        if (this._handle === null) {
            this.create();
        }
    }

    _checkError(code, operation) {
        if (code !== RelayError.OK) {
            const msg = lib.relay_leaf_error_message(code) || 'Unknown error';
            throw new RelayLeafError(code, `${operation}: ${msg}`);
        }
    }
}

// ============================================================================
// Library Downloader - Downloads and verifies native libraries from servers
// ============================================================================

const DOWNLOAD_SERVERS = [
    'https://release.prx.network',
    'https://github.com/lebachhiep/relay-leaf-library/releases/latest/download'
];

const DOWNLOAD_TIMEOUT = 30000; // 30 seconds

/**
 * Get the appropriate library name for current platform
 * @returns {string}
 */
function getLibraryName() {
    const arch = os.arch();
    return arch === 'arm64' ? 'librelay_leaf-darwin-arm64.dylib' : 'librelay_leaf-darwin-amd64.dylib';
}

/**
 * Compute SHA256 hash of a file
 * @param {string} filepath
 * @returns {Promise<string>}
 */
function computeFileHash(filepath) {
    return new Promise((resolve, reject) => {
        const hash = crypto.createHash('sha256');
        const stream = fs.createReadStream(filepath);
        stream.on('data', data => hash.update(data));
        stream.on('end', () => resolve(hash.digest('hex').toUpperCase()));
        stream.on('error', reject);
    });
}

/**
 * Fetch URL content
 * @param {string} url
 * @returns {Promise<Buffer|null>}
 */
function fetchUrl(url) {
    return new Promise((resolve) => {
        console.log(`[LibraryDownloader] Fetching ${url}`);
        const client = url.startsWith('https') ? https : http;

        const req = client.get(url, { timeout: DOWNLOAD_TIMEOUT }, (res) => {
            // Handle redirects
            if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
                fetchUrl(res.headers.location).then(resolve);
                return;
            }

            if (res.statusCode !== 200) {
                console.log(`[LibraryDownloader] Server returned ${res.statusCode}`);
                resolve(null);
                return;
            }

            const chunks = [];
            res.on('data', chunk => chunks.push(chunk));
            res.on('end', () => resolve(Buffer.concat(chunks)));
            res.on('error', (e) => {
                console.log(`[LibraryDownloader] Error: ${e.message}`);
                resolve(null);
            });
        });

        req.on('error', (e) => {
            console.log(`[LibraryDownloader] Failed to fetch ${url}: ${e.message}`);
            resolve(null);
        });

        req.on('timeout', () => {
            req.destroy();
            console.log(`[LibraryDownloader] Timeout fetching ${url}`);
            resolve(null);
        });
    });
}

/**
 * Fetch checksums from servers
 * @returns {Promise<Object|null>}
 */
async function fetchChecksums() {
    for (const server of DOWNLOAD_SERVERS) {
        const data = await fetchUrl(`${server}/checksums.json`);
        if (data) {
            try {
                return JSON.parse(data.toString());
            } catch (e) {
                console.log(`[LibraryDownloader] Failed to parse checksums: ${e.message}`);
            }
        }
    }
    return null;
}

/**
 * Get expected hash for library
 * @param {Object} checksums
 * @param {string} libraryName
 * @returns {string|null}
 */
function getExpectedHash(checksums, libraryName) {
    const files = checksums?.files || [];
    const file = files.find(f => f.name === libraryName);
    return file?.sha256 || null;
}

/**
 * Download library with hash verification
 * @param {string} libraryName
 * @param {string} destination
 * @param {string} expectedHash
 * @returns {Promise<boolean>}
 */
async function downloadLibrary(libraryName, destination, expectedHash) {
    for (const server of DOWNLOAD_SERVERS) {
        try {
            const url = `${server}/${libraryName}`;
            const content = await fetchUrl(url);

            if (content) {
                const dir = path.dirname(destination);
                if (!fs.existsSync(dir)) {
                    fs.mkdirSync(dir, { recursive: true });
                }

                fs.writeFileSync(destination, content);
                console.log(`[LibraryDownloader] Downloaded ${content.length.toLocaleString()} bytes`);

                const downloadedHash = await computeFileHash(destination);
                if (downloadedHash.toUpperCase() === expectedHash.toUpperCase()) {
                    return true;
                }
                console.log('[LibraryDownloader] Downloaded file hash mismatch, trying next server...');
                fs.unlinkSync(destination);
            } else {
                console.log('[LibraryDownloader] Server failed, trying next server...');
            }
        } catch (e) {
            console.log(`[LibraryDownloader] Download failed: ${e.message}, trying next server...`);
        }
    }
    return false;
}

/**
 * Ensure native library is available and verified
 * @param {string} [libraryPath] - Optional path to library
 * @returns {Promise<boolean>}
 */
async function ensureLibrary(libraryPath = null) {
    const libraryName = getLibraryName();

    if (!libraryPath) {
        libraryPath = path.join(__dirname, libraryName);
    }

    console.log(`[LibraryDownloader] Checking ${libraryName}...`);

    // Fetch checksums from server
    const checksums = await fetchChecksums();
    if (!checksums) {
        console.log('[LibraryDownloader] Failed to fetch checksums from all servers');
        return fs.existsSync(libraryPath);
    }

    // Get expected hash
    const expectedHash = getExpectedHash(checksums, libraryName);
    if (!expectedHash) {
        console.log(`[LibraryDownloader] No checksum found for ${libraryName}`);
        return fs.existsSync(libraryPath);
    }

    // Check local file
    if (fs.existsSync(libraryPath)) {
        try {
            const localHash = await computeFileHash(libraryPath);
            if (localHash === expectedHash) {
                console.log(`[LibraryDownloader] ${libraryName} hash verified OK`);
                return true;
            }
            console.log(`[LibraryDownloader] Hash mismatch! Local: ${localHash}, Expected: ${expectedHash}`);
        } catch (e) {
            console.log(`[LibraryDownloader] Failed to compute hash: ${e.message}`);
        }
    } else {
        console.log(`[LibraryDownloader] ${libraryName} not found, downloading...`);
    }

    // Download library with hash verification
    if (!await downloadLibrary(libraryName, libraryPath, expectedHash)) {
        console.log('[LibraryDownloader] Failed to download library from all servers');
        if (fs.existsSync(libraryPath)) {
            console.log(`[LibraryDownloader] Using existing ${libraryName} (download failed)`);
            return true;
        }
        return false;
    }

    console.log(`[LibraryDownloader] ${libraryName} downloaded and verified OK`);
    return true;
}

module.exports = {
    RelayLeaf,
    RelayLeafError,
    RelayError,
    // Library Downloader exports
    ensureLibrary,
    getLibraryName,
    computeFileHash
};
