//! Relay Leaf SDK for Rust (Windows)
//! P2P Relay Network Client Library

use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int};
use std::ptr;

/// Error codes
#[repr(i32)]
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum RelayError {
    Ok = 0,
    NullParam = 1,
    InvalidHandle = 2,
    CreateFailed = 3,
    StartFailed = 4,
    AlreadyStarted = 5,
    NotStarted = 6,
    InvalidProxy = 7,
    Internal = 99,
}

impl From<c_int> for RelayError {
    fn from(code: c_int) -> Self {
        match code {
            0 => RelayError::Ok,
            1 => RelayError::NullParam,
            2 => RelayError::InvalidHandle,
            3 => RelayError::CreateFailed,
            4 => RelayError::StartFailed,
            5 => RelayError::AlreadyStarted,
            6 => RelayError::NotStarted,
            7 => RelayError::InvalidProxy,
            _ => RelayError::Internal,
        }
    }
}

/// Statistics from the relay client
#[derive(Debug, Clone, Default)]
pub struct Stats {
    pub uptime_seconds: i64,
    pub total_streams: i64,
    pub bytes_sent: i64,
    pub bytes_received: i64,
    pub reconnect_count: i64,
    pub last_error: String,
    pub exit_points_json: String,
    pub node_addresses_json: String,
    pub active_streams: i32,
    pub connected_nodes: i32,
    pub connected: bool,
}

/// Native stats structure matching C layout
#[repr(C)]
struct NativeStats {
    uptime_seconds: i64,
    total_streams: i64,
    bytes_sent: i64,
    bytes_received: i64,
    reconnect_count: i64,
    last_error: *mut c_char,
    exit_points_json: *mut c_char,
    node_addresses_json: *mut c_char,
    active_streams: i32,
    connected_nodes: i32,
    connected: bool,
}

impl Default for NativeStats {
    fn default() -> Self {
        NativeStats {
            uptime_seconds: 0,
            total_streams: 0,
            bytes_sent: 0,
            bytes_received: 0,
            reconnect_count: 0,
            last_error: ptr::null_mut(),
            exit_points_json: ptr::null_mut(),
            node_addresses_json: ptr::null_mut(),
            active_streams: 0,
            connected_nodes: 0,
            connected: false,
        }
    }
}

#[link(name = "relay_leaf")]
extern "C" {
    fn relay_leaf_create(verbose: bool, out_handle: *mut usize) -> c_int;
    fn relay_leaf_destroy(handle: usize) -> c_int;
    fn relay_leaf_set_discovery_url(handle: usize, url: *const c_char) -> c_int;
    fn relay_leaf_set_partner_id(handle: usize, partner_id: *const c_char) -> c_int;
    fn relay_leaf_add_proxy(handle: usize, proxy_url: *const c_char) -> c_int;
    fn relay_leaf_start(handle: usize) -> c_int;
    fn relay_leaf_stop(handle: usize) -> c_int;
    fn relay_leaf_get_device_id(handle: usize) -> *mut c_char;
    fn relay_leaf_get_stats(handle: usize, out_stats: *mut NativeStats) -> c_int;
    fn relay_leaf_free_stats(stats: *mut NativeStats);
    fn relay_leaf_free_string(s: *mut c_char);
    fn relay_leaf_version() -> *mut c_char;
    fn relay_leaf_error_message(code: c_int) -> *mut c_char;
}

/// Result type for RelayLeaf operations
pub type Result<T> = std::result::Result<T, RelayError>;

/// Relay Leaf client for P2P relay network
pub struct RelayLeaf {
    handle: usize,
}

impl RelayLeaf {
    /// Create a new RelayLeaf client
    pub fn new(verbose: bool) -> Result<Self> {
        let mut handle: usize = 0;
        let code = unsafe { relay_leaf_create(verbose, &mut handle) };

        if code != 0 {
            return Err(RelayError::from(code));
        }

        Ok(RelayLeaf { handle })
    }

    /// Set the discovery service URL
    pub fn set_discovery_url(&self, url: &str) -> Result<()> {
        let c_url = CString::new(url).map_err(|_| RelayError::NullParam)?;
        let code = unsafe { relay_leaf_set_discovery_url(self.handle, c_url.as_ptr()) };

        if code != 0 {
            return Err(RelayError::from(code));
        }
        Ok(())
    }

    /// Set the partner ID for authentication
    pub fn set_partner_id(&self, partner_id: &str) -> Result<()> {
        let c_id = CString::new(partner_id).map_err(|_| RelayError::NullParam)?;
        let code = unsafe { relay_leaf_set_partner_id(self.handle, c_id.as_ptr()) };

        if code != 0 {
            return Err(RelayError::from(code));
        }
        Ok(())
    }

    /// Add a proxy server (socks5:// or http://)
    pub fn add_proxy(&self, proxy_url: &str) -> Result<()> {
        let c_url = CString::new(proxy_url).map_err(|_| RelayError::NullParam)?;
        let code = unsafe { relay_leaf_add_proxy(self.handle, c_url.as_ptr()) };

        if code != 0 {
            return Err(RelayError::from(code));
        }
        Ok(())
    }

    /// Start the relay client
    pub fn start(&self) -> Result<()> {
        let code = unsafe { relay_leaf_start(self.handle) };

        if code != 0 {
            return Err(RelayError::from(code));
        }
        Ok(())
    }

    /// Stop the relay client
    pub fn stop(&self) -> Result<()> {
        let code = unsafe { relay_leaf_stop(self.handle) };

        if code != 0 {
            return Err(RelayError::from(code));
        }
        Ok(())
    }

    /// Get the unique device ID
    pub fn get_device_id(&self) -> String {
        unsafe {
            let ptr = relay_leaf_get_device_id(self.handle);
            if ptr.is_null() {
                return String::new();
            }
            let device_id = CStr::from_ptr(ptr).to_string_lossy().into_owned();
            relay_leaf_free_string(ptr);
            device_id
        }
    }

    /// Get current statistics
    pub fn get_stats(&self) -> Option<Stats> {
        let mut native = NativeStats::default();
        let code = unsafe { relay_leaf_get_stats(self.handle, &mut native) };

        if code != 0 {
            return None;
        }

        let stats = Stats {
            uptime_seconds: native.uptime_seconds,
            total_streams: native.total_streams,
            bytes_sent: native.bytes_sent,
            bytes_received: native.bytes_received,
            reconnect_count: native.reconnect_count,
            active_streams: native.active_streams,
            connected_nodes: native.connected_nodes,
            connected: native.connected,
            last_error: ptr_to_string(native.last_error),
            exit_points_json: ptr_to_string(native.exit_points_json),
            node_addresses_json: ptr_to_string(native.node_addresses_json),
        };

        unsafe { relay_leaf_free_stats(&mut native) };
        Some(stats)
    }

    /// Check if client is connected
    pub fn is_connected(&self) -> bool {
        self.get_stats().map(|s| s.connected).unwrap_or(false)
    }

    /// Get library version
    pub fn version() -> String {
        unsafe {
            let ptr = relay_leaf_version();
            if ptr.is_null() {
                return "unknown".to_string();
            }
            CStr::from_ptr(ptr).to_string_lossy().into_owned()
        }
    }
}

impl Drop for RelayLeaf {
    fn drop(&mut self) {
        unsafe {
            relay_leaf_stop(self.handle);
            relay_leaf_destroy(self.handle);
        }
    }
}

fn ptr_to_string(ptr: *mut c_char) -> String {
    if ptr.is_null() {
        return String::new();
    }
    unsafe { CStr::from_ptr(ptr).to_string_lossy().into_owned() }
}
