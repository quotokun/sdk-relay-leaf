package com.relayleaf.sdk;

import java.util.concurrent.atomic.AtomicLong;

/**
 * RelayLeaf SDK for Android (Java)
 * P2P Relay Network Client Library
 *
 * All classes consolidated into single file for easier SDK distribution.
 */
public class RelayLeaf implements AutoCloseable {
    private static boolean libraryLoaded = false;
    private final AtomicLong handle = new AtomicLong(0);
    private volatile boolean started = false;

    static {
        try {
            System.loadLibrary("relay_leaf");
            libraryLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Failed to load relay_leaf library: " + e.getMessage());
        }
    }

    // Native methods
    private static native int nativeCreate(boolean verbose, long[] outHandle);
    private static native int nativeDestroy(long handle);
    private static native int nativeSetDiscoveryUrl(long handle, String url);
    private static native int nativeSetPartnerId(long handle, String partnerId);
    private static native int nativeAddProxy(long handle, String proxyUrl);
    private static native int nativeStart(long handle);
    private static native int nativeStop(long handle);
    private static native String nativeGetDeviceId(long handle);
    private static native Stats nativeGetStats(long handle);
    private static native String nativeVersion();
    private static native String nativeErrorMessage(int code);

    /**
     * Create a new RelayLeaf client
     * @param verbose Enable verbose logging
     * @throws Exception if creation fails
     */
    public RelayLeaf(boolean verbose) throws Exception {
        if (!libraryLoaded) {
            throw new Exception(Error.INTERNAL, "Library not loaded");
        }

        long[] outHandle = new long[1];
        int code = nativeCreate(verbose, outHandle);
        checkError(code, "create");
        handle.set(outHandle[0]);
    }

    /**
     * Create a new RelayLeaf client with default settings
     */
    public RelayLeaf() throws Exception {
        this(false);
    }

    /**
     * Set the discovery service URL
     */
    public void setDiscoveryUrl(String url) throws Exception {
        checkHandle();
        int code = nativeSetDiscoveryUrl(handle.get(), url);
        checkError(code, "setDiscoveryUrl");
    }

    /**
     * Set the partner ID for authentication
     */
    public void setPartnerId(String partnerId) throws Exception {
        checkHandle();
        int code = nativeSetPartnerId(handle.get(), partnerId);
        checkError(code, "setPartnerId");
    }

    /**
     * Add a proxy server (socks5:// or http://)
     */
    public void addProxy(String proxyUrl) throws Exception {
        checkHandle();
        int code = nativeAddProxy(handle.get(), proxyUrl);
        checkError(code, "addProxy");
    }

    /**
     * Start the relay client
     */
    public void start() throws Exception {
        checkHandle();
        int code = nativeStart(handle.get());
        checkError(code, "start");
        started = true;
    }

    /**
     * Stop the relay client
     */
    public void stop() throws Exception {
        if (handle.get() == 0) return;
        int code = nativeStop(handle.get());
        checkError(code, "stop");
        started = false;
    }

    /**
     * Get the unique device ID
     */
    public String getDeviceId() {
        if (handle.get() == 0) return "";
        return nativeGetDeviceId(handle.get());
    }

    /**
     * Get current statistics
     */
    public Stats getStats() {
        if (handle.get() == 0) return null;
        return nativeGetStats(handle.get());
    }

    /**
     * Check if client is connected
     */
    public boolean isConnected() {
        Stats stats = getStats();
        return stats != null && stats.connected;
    }

    /**
     * Check if client is started
     */
    public boolean isStarted() {
        return started;
    }

    /**
     * Get library version
     */
    public static String version() {
        if (!libraryLoaded) return "unknown";
        return nativeVersion();
    }

    /**
     * Destroy the client and free resources
     */
    @Override
    public void close() {
        long h = handle.getAndSet(0);
        if (h != 0) {
            nativeDestroy(h);
        }
    }

    private void checkHandle() throws Exception {
        if (handle.get() == 0) {
            throw new Exception(Error.INVALID_HANDLE, "Client not initialized");
        }
    }

    private void checkError(int code, String operation) throws Exception {
        if (code != 0) {
            String msg = nativeErrorMessage(code);
            throw new Exception(Error.fromCode(code), operation + ": " + msg);
        }
    }

    // ============================================================================
    // Inner Classes - Error, Exception, Stats
    // ============================================================================

    /**
     * Error codes for RelayLeaf operations
     */
    public enum Error {
        OK(0),
        NULL_PARAM(1),
        INVALID_HANDLE(2),
        CREATE_FAILED(3),
        START_FAILED(4),
        ALREADY_STARTED(5),
        NOT_STARTED(6),
        INVALID_PROXY(7),
        INTERNAL(99);

        private final int code;

        Error(int code) {
            this.code = code;
        }

        public int getCode() {
            return code;
        }

        public static Error fromCode(int code) {
            for (Error e : values()) {
                if (e.code == code) return e;
            }
            return INTERNAL;
        }
    }

    /**
     * Exception thrown by RelayLeaf operations
     */
    public static class Exception extends java.lang.Exception {
        private final Error error;

        public Exception(Error error, String message) {
            super(message);
            this.error = error;
        }

        public Error getError() {
            return error;
        }

        public int getErrorCode() {
            return error.getCode();
        }
    }

    /**
     * Statistics from the RelayLeaf client
     */
    public static class Stats {
        public final long uptimeSeconds;
        public final long totalStreams;
        public final long bytesSent;
        public final long bytesReceived;
        public final long reconnectCount;
        public final String lastError;
        public final String exitPointsJson;
        public final String nodeAddressesJson;
        public final int activeStreams;
        public final int connectedNodes;
        public final boolean connected;

        public Stats(
                long uptimeSeconds,
                long totalStreams,
                long bytesSent,
                long bytesReceived,
                long reconnectCount,
                String lastError,
                String exitPointsJson,
                String nodeAddressesJson,
                int activeStreams,
                int connectedNodes,
                boolean connected) {
            this.uptimeSeconds = uptimeSeconds;
            this.totalStreams = totalStreams;
            this.bytesSent = bytesSent;
            this.bytesReceived = bytesReceived;
            this.reconnectCount = reconnectCount;
            this.lastError = lastError != null ? lastError : "";
            this.exitPointsJson = exitPointsJson != null ? exitPointsJson : "[]";
            this.nodeAddressesJson = nodeAddressesJson != null ? nodeAddressesJson : "[]";
            this.activeStreams = activeStreams;
            this.connectedNodes = connectedNodes;
            this.connected = connected;
        }

        @Override
        public String toString() {
            return String.format(
                "Stats{connected=%b, uptime=%ds, nodes=%d, streams=%d/%d, sent=%d, received=%d}",
                connected, uptimeSeconds, connectedNodes, activeStreams, totalStreams, bytesSent, bytesReceived
            );
        }
    }
}
