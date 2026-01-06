"""
Relay Leaf SDK for Python (Windows)
P2P Relay Network Client Library
"""

import ctypes
import os
import sys
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
        lib_path = os.path.join(lib_dir, "relay_leaf.dll")

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
        lib_path = os.path.join(lib_dir, "relay_leaf.dll")
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
