//! Relay Leaf SDK for Rust (Windows)
//! P2P Relay Network Client Library

use std::ffi::{CStr, CString};
use std::fs::{self, File};
use std::io::{Read, Write};
use std::os::raw::{c_char, c_int};
use std::path::{Path, PathBuf};
use std::ptr;
use sha2::{Sha256, Digest};

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

#[link(name = "relay_leaf-linux-x64")]
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

// ============================================================================
// Library Downloader - Downloads and verifies native libraries from servers
// ============================================================================

const DOWNLOAD_SERVERS: &[&str] = &[
    "https://release.prx.network",
    "https://github.com/lebachhiep/relay-leaf-library/releases/latest/download",
];

/// Get the appropriate library name for current platform
pub fn get_library_name() -> &'static str {
    "librelay_leaf-linux-x64.so"
}

/// Compute SHA256 hash of a file
pub fn compute_file_hash(path: &Path) -> std::io::Result<String> {
    let mut file = File::open(path)?;
    let mut hasher = Sha256::new();
    let mut buffer = [0u8; 8192];

    loop {
        let bytes_read = file.read(&mut buffer)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }

    Ok(format!("{:X}", hasher.finalize()))
}

/// Checksums JSON structure
#[derive(Debug, serde::Deserialize)]
struct Checksums {
    files: Vec<FileInfo>,
}

#[derive(Debug, serde::Deserialize)]
struct FileInfo {
    name: String,
    sha256: String,
}

/// Fetch checksums from servers
fn fetch_checksums() -> Option<Checksums> {
    for server in DOWNLOAD_SERVERS {
        let url = format!("{}/checksums.json", server);
        println!("[LibraryDownloader] Fetching {}", url);

        match ureq::get(&url).timeout(std::time::Duration::from_secs(30)).call() {
            Ok(response) => {
                if let Ok(checksums) = response.into_json::<Checksums>() {
                    return Some(checksums);
                }
            }
            Err(e) => {
                println!("[LibraryDownloader] Failed to fetch {}: {}", url, e);
            }
        }
    }
    None
}

/// Get expected hash for library
fn get_expected_hash(checksums: &Checksums, library_name: &str) -> Option<String> {
    checksums.files.iter()
        .find(|f| f.name == library_name)
        .map(|f| f.sha256.clone())
}

/// Download library from servers with hash verification
fn download_library(library_name: &str, destination: &Path, expected_hash: &str) -> bool {
    for server in DOWNLOAD_SERVERS {
        let url = format!("{}/{}", server, library_name);
        println!("[LibraryDownloader] Downloading from {}", url);

        match ureq::get(&url).timeout(std::time::Duration::from_secs(60)).call() {
            Ok(response) => {
                let mut bytes = Vec::new();
                if response.into_reader().read_to_end(&mut bytes).is_ok() {
                    if let Some(parent) = destination.parent() {
                        let _ = fs::create_dir_all(parent);
                    }

                    if let Ok(mut file) = File::create(destination) {
                        if file.write_all(&bytes).is_ok() {
                            println!("[LibraryDownloader] Downloaded {} bytes", bytes.len());

                            // Verify hash immediately after download
                            match compute_file_hash(destination) {
                                Ok(downloaded_hash) => {
                                    if downloaded_hash.eq_ignore_ascii_case(expected_hash) {
                                        return true;
                                    }
                                    println!("[LibraryDownloader] Downloaded file hash mismatch, trying next server...");
                                    let _ = fs::remove_file(destination);
                                }
                                Err(e) => {
                                    println!("[LibraryDownloader] Failed to verify hash: {}, trying next server...", e);
                                    let _ = fs::remove_file(destination);
                                }
                            }
                        }
                    }
                }
            }
            Err(e) => {
                println!("[LibraryDownloader] Download failed: {}, trying next server...", e);
            }
        }
    }
    false
}

/// Ensure native library is available and verified
pub fn ensure_library(library_path: Option<&Path>) -> bool {
    let library_name = get_library_name();

    let path = library_path
        .map(|p| p.to_path_buf())
        .unwrap_or_else(|| {
            std::env::current_exe()
                .ok()
                .and_then(|p| p.parent().map(|p| p.join(library_name)))
                .unwrap_or_else(|| PathBuf::from(library_name))
        });

    println!("[LibraryDownloader] Checking {}...", library_name);

    // Fetch checksums
    let checksums = match fetch_checksums() {
        Some(c) => c,
        None => {
            println!("[LibraryDownloader] Failed to fetch checksums from all servers");
            return path.exists();
        }
    };

    // Get expected hash
    let expected_hash = match get_expected_hash(&checksums, library_name) {
        Some(h) => h,
        None => {
            println!("[LibraryDownloader] No checksum found for {}", library_name);
            return path.exists();
        }
    };

    // Check local file
    if path.exists() {
        match compute_file_hash(&path) {
            Ok(local_hash) => {
                if local_hash.eq_ignore_ascii_case(&expected_hash) {
                    println!("[LibraryDownloader] {} hash verified OK", library_name);
                    return true;
                }
                println!("[LibraryDownloader] Hash mismatch! Local: {}, Expected: {}",
                    local_hash, expected_hash);
            }
            Err(e) => {
                println!("[LibraryDownloader] Failed to compute hash: {}", e);
            }
        }
    } else {
        println!("[LibraryDownloader] {} not found, downloading...", library_name);
    }

    // Download library with hash verification
    if !download_library(library_name, &path, &expected_hash) {
        println!("[LibraryDownloader] Failed to download library from all servers");
        // Fallback to existing file if download failed
        if path.exists() {
            println!("[LibraryDownloader] Using existing {} (download failed)", library_name);
            return true;
        }
        return false;
    }

    println!("[LibraryDownloader] {} downloaded and verified OK", library_name);
    true
}
