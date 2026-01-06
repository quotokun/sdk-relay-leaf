/// RelayLeaf SDK for Flutter/Dart
/// P2P Relay Network Client Library
library relay_leaf;

import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

/// Error codes for RelayLeaf operations
enum RelayLeafError {
  ok,
  nullParam,
  invalidHandle,
  createFailed,
  startFailed,
  alreadyStarted,
  notStarted,
  invalidProxy,
  internal,
}

RelayLeafError _errorFromCode(int code) {
  switch (code) {
    case 0: return RelayLeafError.ok;
    case 1: return RelayLeafError.nullParam;
    case 2: return RelayLeafError.invalidHandle;
    case 3: return RelayLeafError.createFailed;
    case 4: return RelayLeafError.startFailed;
    case 5: return RelayLeafError.alreadyStarted;
    case 6: return RelayLeafError.notStarted;
    case 7: return RelayLeafError.invalidProxy;
    default: return RelayLeafError.internal;
  }
}

/// Exception thrown by RelayLeaf operations
class RelayLeafException implements Exception {
  final RelayLeafError error;
  final String message;

  RelayLeafException(this.error, this.message);

  @override
  String toString() => 'RelayLeafException: $message (${error.name})';
}

/// Statistics from the RelayLeaf client
class RelayLeafStats {
  final int uptimeSeconds;
  final int totalStreams;
  final int bytesSent;
  final int bytesReceived;
  final int reconnectCount;
  final String lastError;
  final String exitPointsJson;
  final String nodeAddressesJson;
  final int activeStreams;
  final int connectedNodes;
  final bool connected;

  RelayLeafStats({
    required this.uptimeSeconds,
    required this.totalStreams,
    required this.bytesSent,
    required this.bytesReceived,
    required this.reconnectCount,
    required this.lastError,
    required this.exitPointsJson,
    required this.nodeAddressesJson,
    required this.activeStreams,
    required this.connectedNodes,
    required this.connected,
  });

  @override
  String toString() =>
      'RelayLeafStats(connected: $connected, nodes: $connectedNodes, streams: $activeStreams/$totalStreams)';
}

// Native struct matching C layout
final class NativeStats extends Struct {
  @Int64() external int uptimeSeconds;
  @Int64() external int totalStreams;
  @Int64() external int bytesSent;
  @Int64() external int bytesReceived;
  @Int64() external int reconnectCount;
  external Pointer<Utf8> lastError;
  external Pointer<Utf8> exitPointsJson;
  external Pointer<Utf8> nodeAddressesJson;
  @Int32() external int activeStreams;
  @Int32() external int connectedNodes;
  @Bool() external bool connected;
}

// Native function typedefs
typedef NativeCreate = Int32 Function(Bool verbose, Pointer<IntPtr> outHandle);
typedef NativeDestroy = Int32 Function(IntPtr handle);
typedef NativeSetString = Int32 Function(IntPtr handle, Pointer<Utf8> str);
typedef NativeStart = Int32 Function(IntPtr handle);
typedef NativeStop = Int32 Function(IntPtr handle);
typedef NativeGetDeviceId = Pointer<Utf8> Function(IntPtr handle);
typedef NativeGetStats = Int32 Function(IntPtr handle, Pointer<NativeStats> stats);
typedef NativeFreeStats = Void Function(Pointer<NativeStats> stats);
typedef NativeFreeString = Void Function(Pointer<Utf8> str);
typedef NativeVersion = Pointer<Utf8> Function();
typedef NativeErrorMessage = Pointer<Utf8> Function(Int32 code);

/// RelayLeaf client for P2P relay network
class RelayLeaf {
  static DynamicLibrary? _lib;
  int _handle = 0;
  bool _started = false;

  // Native function pointers
  static late final int Function(bool, Pointer<IntPtr>) _create;
  static late final int Function(int) _destroy;
  static late final int Function(int, Pointer<Utf8>) _setDiscoveryUrl;
  static late final int Function(int, Pointer<Utf8>) _setPartnerId;
  static late final int Function(int, Pointer<Utf8>) _addProxy;
  static late final int Function(int) _start;
  static late final int Function(int) _stop;
  static late final Pointer<Utf8> Function(int) _getDeviceId;
  static late final int Function(int, Pointer<NativeStats>) _getStats;
  static late final void Function(Pointer<NativeStats>) _freeStats;
  static late final void Function(Pointer<Utf8>) _freeString;
  static late final Pointer<Utf8> Function() _version;
  static late final Pointer<Utf8> Function(int) _errorMessage;

  static void _loadLibrary() {
    if (_lib != null) return;

    String libName;
    if (Platform.isAndroid) {
      libName = 'librelay_leaf.so';
    } else if (Platform.isWindows) {
      libName = 'relay_leaf.dll';
    } else if (Platform.isLinux) {
      libName = 'librelay_leaf.so';
    } else if (Platform.isMacOS || Platform.isIOS) {
      libName = 'librelay_leaf.dylib';
    } else {
      throw UnsupportedError('Unsupported platform');
    }

    _lib = DynamicLibrary.open(libName);

    _create = _lib!.lookupFunction<NativeCreate, int Function(bool, Pointer<IntPtr>)>('relay_leaf_create');
    _destroy = _lib!.lookupFunction<NativeDestroy, int Function(int)>('relay_leaf_destroy');
    _setDiscoveryUrl = _lib!.lookupFunction<NativeSetString, int Function(int, Pointer<Utf8>)>('relay_leaf_set_discovery_url');
    _setPartnerId = _lib!.lookupFunction<NativeSetString, int Function(int, Pointer<Utf8>)>('relay_leaf_set_partner_id');
    _addProxy = _lib!.lookupFunction<NativeSetString, int Function(int, Pointer<Utf8>)>('relay_leaf_add_proxy');
    _start = _lib!.lookupFunction<NativeStart, int Function(int)>('relay_leaf_start');
    _stop = _lib!.lookupFunction<NativeStop, int Function(int)>('relay_leaf_stop');
    _getDeviceId = _lib!.lookupFunction<NativeGetDeviceId, Pointer<Utf8> Function(int)>('relay_leaf_get_device_id');
    _getStats = _lib!.lookupFunction<NativeGetStats, int Function(int, Pointer<NativeStats>)>('relay_leaf_get_stats');
    _freeStats = _lib!.lookupFunction<NativeFreeStats, void Function(Pointer<NativeStats>)>('relay_leaf_free_stats');
    _freeString = _lib!.lookupFunction<NativeFreeString, void Function(Pointer<Utf8>)>('relay_leaf_free_string');
    _version = _lib!.lookupFunction<NativeVersion, Pointer<Utf8> Function()>('relay_leaf_version');
    _errorMessage = _lib!.lookupFunction<NativeErrorMessage, Pointer<Utf8> Function(int)>('relay_leaf_error_message');
  }

  /// Create a new RelayLeaf client
  RelayLeaf({bool verbose = false}) {
    _loadLibrary();

    final outHandle = calloc<IntPtr>();
    try {
      final code = _create(verbose, outHandle);
      _checkError(code, 'create');
      _handle = outHandle.value;
    } finally {
      calloc.free(outHandle);
    }
  }

  /// Get library version
  static String version() {
    _loadLibrary();
    final ptr = _version();
    if (ptr == nullptr) return 'unknown';
    final version = ptr.toDartString();
    _freeString(ptr);
    return version;
  }

  /// Set the discovery service URL
  void setDiscoveryUrl(String url) {
    _checkHandle();
    final urlPtr = url.toNativeUtf8();
    try {
      final code = _setDiscoveryUrl(_handle, urlPtr);
      _checkError(code, 'setDiscoveryUrl');
    } finally {
      calloc.free(urlPtr);
    }
  }

  /// Set the partner ID for authentication
  void setPartnerId(String partnerId) {
    _checkHandle();
    final idPtr = partnerId.toNativeUtf8();
    try {
      final code = _setPartnerId(_handle, idPtr);
      _checkError(code, 'setPartnerId');
    } finally {
      calloc.free(idPtr);
    }
  }

  /// Add a proxy server (socks5:// or http://)
  void addProxy(String proxyUrl) {
    _checkHandle();
    final urlPtr = proxyUrl.toNativeUtf8();
    try {
      final code = _addProxy(_handle, urlPtr);
      _checkError(code, 'addProxy');
    } finally {
      calloc.free(urlPtr);
    }
  }

  /// Start the relay client
  void start() {
    _checkHandle();
    final code = _start(_handle);
    _checkError(code, 'start');
    _started = true;
  }

  /// Stop the relay client
  void stop() {
    if (_handle == 0) return;
    final code = _stop(_handle);
    _checkError(code, 'stop');
    _started = false;
  }

  /// Get the unique device ID
  String get deviceId {
    if (_handle == 0) return '';
    final ptr = _getDeviceId(_handle);
    if (ptr == nullptr) return '';
    final id = ptr.toDartString();
    _freeString(ptr);
    return id;
  }

  /// Get current statistics
  RelayLeafStats? get stats {
    if (_handle == 0) return null;

    final statsPtr = calloc<NativeStats>();
    try {
      final code = _getStats(_handle, statsPtr);
      if (code != 0) return null;

      final native = statsPtr.ref;
      final stats = RelayLeafStats(
        uptimeSeconds: native.uptimeSeconds,
        totalStreams: native.totalStreams,
        bytesSent: native.bytesSent,
        bytesReceived: native.bytesReceived,
        reconnectCount: native.reconnectCount,
        lastError: native.lastError != nullptr ? native.lastError.toDartString() : '',
        exitPointsJson: native.exitPointsJson != nullptr ? native.exitPointsJson.toDartString() : '[]',
        nodeAddressesJson: native.nodeAddressesJson != nullptr ? native.nodeAddressesJson.toDartString() : '[]',
        activeStreams: native.activeStreams,
        connectedNodes: native.connectedNodes,
        connected: native.connected,
      );

      _freeStats(statsPtr);
      return stats;
    } finally {
      calloc.free(statsPtr);
    }
  }

  /// Check if client is connected
  bool get isConnected => stats?.connected ?? false;

  /// Check if client has been started
  bool get isStarted => _started;

  /// Destroy the client and free resources
  void dispose() {
    final h = _handle;
    _handle = 0;
    if (h != 0) {
      _destroy(h);
    }
  }

  void _checkHandle() {
    if (_handle == 0) {
      throw RelayLeafException(RelayLeafError.invalidHandle, 'Client not initialized');
    }
  }

  void _checkError(int code, String operation) {
    if (code != 0) {
      final msgPtr = _errorMessage(code);
      final msg = msgPtr != nullptr ? msgPtr.toDartString() : 'Unknown error';
      if (msgPtr != nullptr) _freeString(msgPtr);
      throw RelayLeafException(_errorFromCode(code), '$operation: $msg');
    }
  }
}
