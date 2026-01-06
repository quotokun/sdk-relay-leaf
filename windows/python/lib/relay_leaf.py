"""
Relay Leaf SDK for Python (Windows)
P2P Relay Network Client Library
"""

import ctypes
import hashlib
import json
import os
import platform
import sys
import urllib.request
import urllib.error
from concurrent.futures import ThreadPoolExecutor, as_completed
from ctypes import c_bool, c_char_p, c_int, c_int32, c_int64, c_void_p, POINTER, Structure, byref
from typing import Optional, Dict, Any
from enum import IntEnum

# Error codes
class RelayError(IntEnum):
    OK = 0
    NULL_PARAM = 1
    INVALID_HANDLE = 2
    CREATE_FAILED = 3
    START_FAILED = 4
    ALREADY_STARTED = 5
    NOT_STARTED = 6
    INVALID_PROXY = 7
    INTERNAL = 99

class RelayLeafException(Exception):
    """Exception raised by RelayLeaf operations"""
    def __init__(self, code: int, message: str = ""):
        self.code = code
        self.message = message or f"Error code: {code}"
        super().__init__(self.message)

# Stats structure matching C struct
class RelayLeafStats(Structure):
    _fields_ = [
        ("uptime_seconds", c_int64),
        ("total_streams", c_int64),
        ("bytes_sent", c_int64),
        ("bytes_received", c_int64),
        ("reconnect_count", c_int64),
        ("last_error", c_char_p),
        ("exit_points_json", c_char_p),
        ("node_addresses_json", c_char_p),
        ("active_streams", c_int32),
        ("connected_nodes", c_int32),
        ("connected", c_bool),
    ]

class RelayLeaf:
    """
    RelayLeaf client for P2P relay network.

    Usage:
        with RelayLeaf() as client:
            client.set_partner_id("my-partner")
            client.add_proxy("socks5://proxy:1080")
            client.start()

            while client.is_connected():
                stats = client.get_stats()
                print(f"Uptime: {stats['uptime_seconds']}s")
                time.sleep(5)
    """

    def __init__(self, verbose: bool = False):
        self._handle = None
        self._lib = None
        self._verbose = verbose
        self._load_library()

    def _load_library(self):
        """Load the native library"""
        lib_dir = os.path.dirname(os.path.abspath(__file__))
        lib_path = os.path.join(lib_dir, "relay_leaf-windows-x64.dll")

        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"Library not found: {lib_path}")

        self._lib = ctypes.CDLL(lib_path)
        self._setup_functions()

    def _setup_functions(self):
        """Setup function signatures"""
        # relay_leaf_create
        self._lib.relay_leaf_create.argtypes = [c_bool, POINTER(c_void_p)]
        self._lib.relay_leaf_create.restype = c_int

        # relay_leaf_destroy
        self._lib.relay_leaf_destroy.argtypes = [c_void_p]
        self._lib.relay_leaf_destroy.restype = c_int

        # relay_leaf_set_discovery_url
        self._lib.relay_leaf_set_discovery_url.argtypes = [c_void_p, c_char_p]
        self._lib.relay_leaf_set_discovery_url.restype = c_int

        # relay_leaf_set_partner_id
        self._lib.relay_leaf_set_partner_id.argtypes = [c_void_p, c_char_p]
        self._lib.relay_leaf_set_partner_id.restype = c_int

        # relay_leaf_add_proxy
        self._lib.relay_leaf_add_proxy.argtypes = [c_void_p, c_char_p]
        self._lib.relay_leaf_add_proxy.restype = c_int

        # relay_leaf_start
        self._lib.relay_leaf_start.argtypes = [c_void_p]
        self._lib.relay_leaf_start.restype = c_int

        # relay_leaf_stop
        self._lib.relay_leaf_stop.argtypes = [c_void_p]
        self._lib.relay_leaf_stop.restype = c_int

        # relay_leaf_get_device_id
        self._lib.relay_leaf_get_device_id.argtypes = [c_void_p]
        self._lib.relay_leaf_get_device_id.restype = c_char_p

        # relay_leaf_get_stats
        self._lib.relay_leaf_get_stats.argtypes = [c_void_p, POINTER(RelayLeafStats)]
        self._lib.relay_leaf_get_stats.restype = c_int

        # relay_leaf_free_stats
        self._lib.relay_leaf_free_stats.argtypes = [POINTER(RelayLeafStats)]
        self._lib.relay_leaf_free_stats.restype = None

        # relay_leaf_free_string
        self._lib.relay_leaf_free_string.argtypes = [c_char_p]
        self._lib.relay_leaf_free_string.restype = None

        # relay_leaf_version
        self._lib.relay_leaf_version.argtypes = []
        self._lib.relay_leaf_version.restype = c_char_p

        # relay_leaf_error_message
        self._lib.relay_leaf_error_message.argtypes = [c_int]
        self._lib.relay_leaf_error_message.restype = c_char_p

    def _check_error(self, code: int, operation: str = ""):
        """Check error code and raise exception if needed"""
        if code != RelayError.OK:
            msg_ptr = self._lib.relay_leaf_error_message(code)
            msg = msg_ptr.decode('utf-8') if msg_ptr else f"Unknown error"
            raise RelayLeafException(code, f"{operation}: {msg}")

    def create(self) -> None:
        """Create the relay leaf client instance"""
        if self._handle is not None:
            return

        handle = c_void_p()
        code = self._lib.relay_leaf_create(self._verbose, byref(handle))
        self._check_error(code, "create")
        self._handle = handle

    def destroy(self) -> None:
        """Destroy the client instance and free resources"""
        if self._handle is None:
            return

        self._lib.relay_leaf_destroy(self._handle)
        self._handle = None

    def set_discovery_url(self, url: str) -> None:
        """Set the discovery service URL"""
        if self._handle is None:
            self.create()

        code = self._lib.relay_leaf_set_discovery_url(
            self._handle,
            url.encode('utf-8')
        )
        self._check_error(code, "set_discovery_url")

    def set_partner_id(self, partner_id: str) -> None:
        """Set the partner ID for authentication"""
        if self._handle is None:
            self.create()

        code = self._lib.relay_leaf_set_partner_id(
            self._handle,
            partner_id.encode('utf-8')
        )
        self._check_error(code, "set_partner_id")

    def add_proxy(self, proxy_url: str) -> None:
        """Add a proxy server (socks5://host:port or http://host:port)"""
        if self._handle is None:
            self.create()

        code = self._lib.relay_leaf_add_proxy(
            self._handle,
            proxy_url.encode('utf-8')
        )
        self._check_error(code, "add_proxy")

    def start(self) -> None:
        """Start the relay leaf client"""
        if self._handle is None:
            self.create()

        code = self._lib.relay_leaf_start(self._handle)
        self._check_error(code, "start")

    def stop(self) -> None:
        """Stop the relay leaf client"""
        if self._handle is None:
            return

        code = self._lib.relay_leaf_stop(self._handle)
        self._check_error(code, "stop")

    def get_device_id(self) -> str:
        """Get the unique device ID"""
        if self._handle is None:
            self.create()

        device_id_ptr = self._lib.relay_leaf_get_device_id(self._handle)
        if device_id_ptr:
            device_id = device_id_ptr.decode('utf-8')
            self._lib.relay_leaf_free_string(device_id_ptr)
            return device_id
        return ""

    def get_stats(self) -> Dict[str, Any]:
        """Get current statistics"""
        if self._handle is None:
            return {}

        stats = RelayLeafStats()
        code = self._lib.relay_leaf_get_stats(self._handle, byref(stats))

        if code != RelayError.OK:
            return {}

        result = {
            "uptime_seconds": stats.uptime_seconds,
            "total_streams": stats.total_streams,
            "bytes_sent": stats.bytes_sent,
            "bytes_received": stats.bytes_received,
            "reconnect_count": stats.reconnect_count,
            "active_streams": stats.active_streams,
            "connected_nodes": stats.connected_nodes,
            "connected": stats.connected,
            "last_error": stats.last_error.decode('utf-8') if stats.last_error else "",
            "exit_points_json": stats.exit_points_json.decode('utf-8') if stats.exit_points_json else "[]",
            "node_addresses_json": stats.node_addresses_json.decode('utf-8') if stats.node_addresses_json else "[]",
        }

        self._lib.relay_leaf_free_stats(byref(stats))
        return result

    def is_connected(self) -> bool:
        """Check if client is connected"""
        stats = self.get_stats()
        return stats.get("connected", False)

    @staticmethod
    def version() -> str:
        """Get library version"""
        lib_dir = os.path.dirname(os.path.abspath(__file__))
        lib_path = os.path.join(lib_dir, "relay_leaf-windows-x64.dll")
        lib = ctypes.CDLL(lib_path)
        lib.relay_leaf_version.restype = c_char_p
        ver_ptr = lib.relay_leaf_version()
        return ver_ptr.decode('utf-8') if ver_ptr else "unknown"

    def __enter__(self):
        self.create()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.stop()
        self.destroy()
        return False


# Convenience function
def create_client(verbose: bool = False) -> RelayLeaf:
    """Create a new RelayLeaf client"""
    client = RelayLeaf(verbose=verbose)
    client.create()
    return client


# ============================================================================
# Library Downloader - Downloads and verifies native libraries from servers
# ============================================================================

_DOWNLOAD_SERVERS = [
    "https://release.prx.network",
    "https://github.com/lebachhiep/relay-leaf-library/releases/latest/download"
]

_DOWNLOAD_TIMEOUT = 30  # seconds


def get_library_name() -> str:
    """Get the appropriate library name for current platform."""
    return "relay_leaf-windows-x64.dll"


def compute_file_hash(filepath: str) -> str:
    """Compute SHA256 hash of a file."""
    sha256 = hashlib.sha256()
    with open(filepath, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            sha256.update(chunk)
    return sha256.hexdigest().upper()


def _fetch_url(url: str) -> Optional[bytes]:
    """Fetch content from URL."""
    try:
        print(f"[LibraryDownloader] Fetching {url}")
        req = urllib.request.Request(url, headers={"User-Agent": "RelayLeafSDK/1.0"})
        with urllib.request.urlopen(req, timeout=_DOWNLOAD_TIMEOUT) as response:
            return response.read()
    except Exception as e:
        print(f"[LibraryDownloader] Failed to fetch {url}: {e}")
        return None


def _fetch_checksums() -> Optional[Dict]:
    """Fetch checksums.json from servers (race condition - first wins)."""
    with ThreadPoolExecutor(max_workers=len(_DOWNLOAD_SERVERS)) as executor:
        futures = {
            executor.submit(_fetch_url, f"{server}/checksums.json"): server
            for server in _DOWNLOAD_SERVERS
        }

        for future in as_completed(futures):
            result = future.result()
            if result:
                try:
                    return json.loads(result)
                except json.JSONDecodeError:
                    continue
    return None


def _get_expected_hash(checksums: Dict, library_name: str) -> Optional[str]:
    """Get expected hash for library from checksums."""
    for file_info in checksums.get("files", []):
        if file_info.get("name") == library_name:
            return file_info.get("sha256")
    return None


def _download_library(library_name: str, destination: str, expected_hash: str) -> bool:
    """Download library from servers with hash verification."""
    for server in _DOWNLOAD_SERVERS:
        try:
            url = f"{server}/{library_name}"
            content = _fetch_url(url)
            if content:
                os.makedirs(os.path.dirname(destination) or ".", exist_ok=True)
                with open(destination, "wb") as f:
                    f.write(content)
                print(f"[LibraryDownloader] Downloaded {len(content):,} bytes")

                # Verify hash immediately after download
                downloaded_hash = compute_file_hash(destination)
                if downloaded_hash.upper() == expected_hash.upper():
                    return True
                print(f"[LibraryDownloader] Downloaded file hash mismatch, trying next server...")
                os.remove(destination)
            else:
                print(f"[LibraryDownloader] Server failed, trying next server...")
        except Exception as e:
            print(f"[LibraryDownloader] Download failed: {e}, trying next server...")
    return False


def ensure_library(library_path: Optional[str] = None) -> bool:
    """
    Ensure native library is available and verified.

    Args:
        library_path: Optional path to library. If None, uses default location.

    Returns:
        True if library is ready, False otherwise.
    """
    library_name = get_library_name()

    if library_path is None:
        # Default to lib directory
        script_dir = os.path.dirname(os.path.abspath(__file__))
        library_path = os.path.join(script_dir, library_name)

    print(f"[LibraryDownloader] Checking {library_name}...")

    # Fetch checksums from server
    checksums = _fetch_checksums()
    if not checksums:
        print("[LibraryDownloader] Failed to fetch checksums from all servers")
        return os.path.exists(library_path)

    # Get expected hash
    expected_hash = _get_expected_hash(checksums, library_name)
    if not expected_hash:
        print(f"[LibraryDownloader] No checksum found for {library_name}")
        return os.path.exists(library_path)

    # Check local file
    if os.path.exists(library_path):
        local_hash = compute_file_hash(library_path)
        if local_hash == expected_hash:
            print(f"[LibraryDownloader] {library_name} hash verified OK")
            return True
        print(f"[LibraryDownloader] Hash mismatch! Local: {local_hash}, Expected: {expected_hash}")
    else:
        print(f"[LibraryDownloader] {library_name} not found, downloading...")

    # Download library with hash verification
    if not _download_library(library_name, library_path, expected_hash):
        print("[LibraryDownloader] Failed to download library from all servers")
        # Fallback to existing file if download failed
        if os.path.exists(library_path):
            print(f"[LibraryDownloader] Using existing {library_name} (download failed)")
            return True
        return False

    print(f"[LibraryDownloader] {library_name} downloaded and verified OK")
    return True
