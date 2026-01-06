// Package relayleaf provides Go bindings for the Relay Leaf P2P network client.
// Windows version - uses relay_leaf.dll via syscall
package relayleaf

import (
	"errors"
	"fmt"
	"path/filepath"
	"runtime"
	"syscall"
	"unsafe"
)

// Error codes
const (
	ErrOK             = 0
	ErrNullParam      = 1
	ErrInvalidHandle  = 2
	ErrCreateFailed   = 3
	ErrStartFailed    = 4
	ErrAlreadyStarted = 5
	ErrNotStarted     = 6
	ErrInvalidProxy   = 7
	ErrInternal       = 99
)

// Stats represents relay client statistics
type Stats struct {
	UptimeSeconds     int64
	TotalStreams      int64
	BytesSent         int64
	BytesReceived     int64
	ReconnectCount    int64
	LastError         string
	ExitPointsJSON    string
	NodeAddressesJSON string
	ActiveStreams     int32
	ConnectedNodes    int32
	Connected         bool
}

// cStats matches the C struct layout
type cStats struct {
	UptimeSeconds     int64
	TotalStreams      int64
	BytesSent         int64
	BytesReceived     int64
	ReconnectCount    int64
	LastError         uintptr
	ExitPointsJSON    uintptr
	NodeAddressesJSON uintptr
	ActiveStreams     int32
	ConnectedNodes    int32
	Connected         int32 // bool as int32 for alignment
}

var (
	dll                      *syscall.DLL
	procCreate               *syscall.Proc
	procDestroy              *syscall.Proc
	procSetDiscoveryURL      *syscall.Proc
	procSetPartnerID         *syscall.Proc
	procAddProxy             *syscall.Proc
	procStart                *syscall.Proc
	procStop                 *syscall.Proc
	procGetDeviceID          *syscall.Proc
	procGetStats             *syscall.Proc
	procFreeStats            *syscall.Proc
	procFreeString           *syscall.Proc
	procVersion              *syscall.Proc
	procErrorMessage         *syscall.Proc
)

func init() {
	// Get the directory of the current executable
	_, filename, _, _ := runtime.Caller(0)
	dir := filepath.Dir(filename)
	dllPath := filepath.Join(dir, "relay_leaf-windows-x86.dll")

	var err error
	dll, err = syscall.LoadDLL(dllPath)
	if err != nil {
		// Try current directory
		dll, err = syscall.LoadDLL("relay_leaf-windows-x86.dll")
		if err != nil {
			panic(fmt.Sprintf("failed to load relay_leaf-windows-x86.dll: %v", err))
		}
	}

	procCreate, _ = dll.FindProc("relay_leaf_create")
	procDestroy, _ = dll.FindProc("relay_leaf_destroy")
	procSetDiscoveryURL, _ = dll.FindProc("relay_leaf_set_discovery_url")
	procSetPartnerID, _ = dll.FindProc("relay_leaf_set_partner_id")
	procAddProxy, _ = dll.FindProc("relay_leaf_add_proxy")
	procStart, _ = dll.FindProc("relay_leaf_start")
	procStop, _ = dll.FindProc("relay_leaf_stop")
	procGetDeviceID, _ = dll.FindProc("relay_leaf_get_device_id")
	procGetStats, _ = dll.FindProc("relay_leaf_get_stats")
	procFreeStats, _ = dll.FindProc("relay_leaf_free_stats")
	procFreeString, _ = dll.FindProc("relay_leaf_free_string")
	procVersion, _ = dll.FindProc("relay_leaf_version")
	procErrorMessage, _ = dll.FindProc("relay_leaf_error_message")
}

// Client represents a Relay Leaf client
type Client struct {
	handle  uintptr
	created bool
}

// NewClient creates a new Relay Leaf client
func NewClient(verbose bool) (*Client, error) {
	client := &Client{}

	var verboseInt uintptr = 0
	if verbose {
		verboseInt = 1
	}

	var handle uintptr
	ret, _, _ := procCreate.Call(verboseInt, uintptr(unsafe.Pointer(&handle)))

	if ret != ErrOK {
		return nil, getError(int(ret), "create")
	}

	client.handle = handle
	client.created = true
	return client, nil
}

// Close destroys the client and frees resources
func (c *Client) Close() error {
	if !c.created {
		return nil
	}

	ret, _, _ := procDestroy.Call(c.handle)
	c.created = false

	if ret != ErrOK {
		return getError(int(ret), "destroy")
	}
	return nil
}

// SetDiscoveryURL sets the discovery service URL
func (c *Client) SetDiscoveryURL(url string) error {
	urlPtr, _ := syscall.BytePtrFromString(url)

	ret, _, _ := procSetDiscoveryURL.Call(c.handle, uintptr(unsafe.Pointer(urlPtr)))
	if ret != ErrOK {
		return getError(int(ret), "set_discovery_url")
	}
	return nil
}

// SetPartnerID sets the partner ID for authentication
func (c *Client) SetPartnerID(partnerID string) error {
	partnerIDPtr, _ := syscall.BytePtrFromString(partnerID)

	ret, _, _ := procSetPartnerID.Call(c.handle, uintptr(unsafe.Pointer(partnerIDPtr)))
	if ret != ErrOK {
		return getError(int(ret), "set_partner_id")
	}
	return nil
}

// AddProxy adds a proxy server
func (c *Client) AddProxy(proxyURL string) error {
	proxyURLPtr, _ := syscall.BytePtrFromString(proxyURL)

	ret, _, _ := procAddProxy.Call(c.handle, uintptr(unsafe.Pointer(proxyURLPtr)))
	if ret != ErrOK {
		return getError(int(ret), "add_proxy")
	}
	return nil
}

// Start starts the relay client
func (c *Client) Start() error {
	ret, _, _ := procStart.Call(c.handle)
	if ret != ErrOK {
		return getError(int(ret), "start")
	}
	return nil
}

// Stop stops the relay client
func (c *Client) Stop() error {
	ret, _, _ := procStop.Call(c.handle)
	if ret != ErrOK {
		return getError(int(ret), "stop")
	}
	return nil
}

// GetDeviceID returns the unique device ID
func (c *Client) GetDeviceID() string {
	ret, _, _ := procGetDeviceID.Call(c.handle)
	if ret == 0 {
		return ""
	}

	deviceID := ptrToString(ret)
	procFreeString.Call(ret)
	return deviceID
}

// GetStats returns current statistics
func (c *Client) GetStats() (*Stats, error) {
	var cs cStats

	ret, _, _ := procGetStats.Call(c.handle, uintptr(unsafe.Pointer(&cs)))
	if ret != ErrOK {
		return nil, getError(int(ret), "get_stats")
	}

	stats := &Stats{
		UptimeSeconds:  cs.UptimeSeconds,
		TotalStreams:   cs.TotalStreams,
		BytesSent:      cs.BytesSent,
		BytesReceived:  cs.BytesReceived,
		ReconnectCount: cs.ReconnectCount,
		ActiveStreams:  cs.ActiveStreams,
		ConnectedNodes: cs.ConnectedNodes,
		Connected:      cs.Connected != 0,
	}

	if cs.LastError != 0 {
		stats.LastError = ptrToString(cs.LastError)
	}
	if cs.ExitPointsJSON != 0 {
		stats.ExitPointsJSON = ptrToString(cs.ExitPointsJSON)
	}
	if cs.NodeAddressesJSON != 0 {
		stats.NodeAddressesJSON = ptrToString(cs.NodeAddressesJSON)
	}

	procFreeStats.Call(uintptr(unsafe.Pointer(&cs)))
	return stats, nil
}

// IsConnected returns true if the client is connected
func (c *Client) IsConnected() bool {
	stats, err := c.GetStats()
	if err != nil {
		return false
	}
	return stats.Connected
}

// Version returns the library version
func Version() string {
	ret, _, _ := procVersion.Call()
	if ret == 0 {
		return "unknown"
	}
	return ptrToString(ret)
}

func getError(code int, operation string) error {
	ret, _, _ := procErrorMessage.Call(uintptr(code))
	msg := "unknown error"
	if ret != 0 {
		msg = ptrToString(ret)
	}
	return errors.New(fmt.Sprintf("%s: %s (code: %d)", operation, msg, code))
}

func ptrToString(ptr uintptr) string {
	if ptr == 0 {
		return ""
	}

	// Find the null terminator
	var length int
	for {
		b := *(*byte)(unsafe.Pointer(ptr + uintptr(length)))
		if b == 0 {
			break
		}
		length++
		if length > 10000 { // Safety limit
			break
		}
	}

	if length == 0 {
		return ""
	}

	bytes := make([]byte, length)
	for i := 0; i < length; i++ {
		bytes[i] = *(*byte)(unsafe.Pointer(ptr + uintptr(i)))
	}
	return string(bytes)
}
