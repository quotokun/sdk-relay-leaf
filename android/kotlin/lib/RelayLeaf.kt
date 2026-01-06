package com.relayleaf.sdk

import java.io.Closeable

/**
 * RelayLeaf SDK for Android (Kotlin)
 * P2P Relay Network Client Library
 */
class RelayLeaf(verbose: Boolean = false) : Closeable {
    private var handle: Long = 0
    private var isStarted = false

    init {
        if (!libraryLoaded) {
            throw RelayLeafException(RelayLeafError.INTERNAL, "Library not loaded")
        }

        val outHandle = LongArray(1)
        val code = nativeCreate(verbose, outHandle)
        checkError(code, "create")
        handle = outHandle[0]
    }

    companion object {
        private var libraryLoaded = false

        init {
            try {
                System.loadLibrary("relay_leaf")
                libraryLoaded = true
            } catch (e: UnsatisfiedLinkError) {
                System.err.println("Failed to load relay_leaf library: ${e.message}")
            }
        }

        /** Get library version */
        @JvmStatic
        fun version(): String = if (libraryLoaded) nativeVersion() else "unknown"

        // Native methods
        @JvmStatic private external fun nativeCreate(verbose: Boolean, outHandle: LongArray): Int
        @JvmStatic private external fun nativeDestroy(handle: Long): Int
        @JvmStatic private external fun nativeSetDiscoveryUrl(handle: Long, url: String): Int
        @JvmStatic private external fun nativeSetPartnerId(handle: Long, partnerId: String): Int
        @JvmStatic private external fun nativeAddProxy(handle: Long, proxyUrl: String): Int
        @JvmStatic private external fun nativeStart(handle: Long): Int
        @JvmStatic private external fun nativeStop(handle: Long): Int
        @JvmStatic private external fun nativeGetDeviceId(handle: Long): String?
        @JvmStatic private external fun nativeGetStats(handle: Long): RelayLeafStats?
        @JvmStatic private external fun nativeVersion(): String
        @JvmStatic private external fun nativeErrorMessage(code: Int): String
    }

    /** Set the discovery service URL */
    fun setDiscoveryUrl(url: String) {
        checkHandle()
        val code = nativeSetDiscoveryUrl(handle, url)
        checkError(code, "setDiscoveryUrl")
    }

    /** Set the partner ID for authentication */
    fun setPartnerId(partnerId: String) {
        checkHandle()
        val code = nativeSetPartnerId(handle, partnerId)
        checkError(code, "setPartnerId")
    }

    /** Add a proxy server (socks5:// or http://) */
    fun addProxy(proxyUrl: String) {
        checkHandle()
        val code = nativeAddProxy(handle, proxyUrl)
        checkError(code, "addProxy")
    }

    /** Start the relay client */
    fun start() {
        checkHandle()
        val code = nativeStart(handle)
        checkError(code, "start")
        isStarted = true
    }

    /** Stop the relay client */
    fun stop() {
        if (handle == 0L) return
        val code = nativeStop(handle)
        checkError(code, "stop")
        isStarted = false
    }

    /** Get the unique device ID */
    val deviceId: String
        get() = if (handle == 0L) "" else nativeGetDeviceId(handle) ?: ""

    /** Get current statistics */
    val stats: RelayLeafStats?
        get() = if (handle == 0L) null else nativeGetStats(handle)

    /** Check if client is connected */
    val isConnected: Boolean
        get() = stats?.connected ?: false

    /** Check if client has been started */
    val started: Boolean
        get() = isStarted

    /** Destroy the client and free resources */
    override fun close() {
        val h = handle
        handle = 0
        if (h != 0L) {
            nativeDestroy(h)
        }
    }

    private fun checkHandle() {
        if (handle == 0L) {
            throw RelayLeafException(RelayLeafError.INVALID_HANDLE, "Client not initialized")
        }
    }

    private fun checkError(code: Int, operation: String) {
        if (code != 0) {
            val msg = nativeErrorMessage(code)
            throw RelayLeafException(RelayLeafError.fromCode(code), "$operation: $msg")
        }
    }
}

/** Statistics from the RelayLeaf client */
data class RelayLeafStats(
    val uptimeSeconds: Long,
    val totalStreams: Long,
    val bytesSent: Long,
    val bytesReceived: Long,
    val reconnectCount: Long,
    val lastError: String,
    val exitPointsJson: String,
    val nodeAddressesJson: String,
    val activeStreams: Int,
    val connectedNodes: Int,
    val connected: Boolean
)

/** Error codes for RelayLeaf operations */
enum class RelayLeafError(val code: Int) {
    OK(0),
    NULL_PARAM(1),
    INVALID_HANDLE(2),
    CREATE_FAILED(3),
    START_FAILED(4),
    ALREADY_STARTED(5),
    NOT_STARTED(6),
    INVALID_PROXY(7),
    INTERNAL(99);

    companion object {
        fun fromCode(code: Int): RelayLeafError =
            values().find { it.code == code } ?: INTERNAL
    }
}

/** Exception thrown by RelayLeaf operations */
class RelayLeafException(
    val error: RelayLeafError,
    message: String
) : Exception(message) {
    val errorCode: Int get() = error.code
}
