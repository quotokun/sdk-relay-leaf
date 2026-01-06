# Relay Leaf SDK for Android

P2P Relay Network Client Library for Android.

## How to Run

### Prerequisites
- Android Studio Arctic Fox or higher
- Android NDK r26b or later
- Min SDK: Android 5.0 (API 21)
- Target SDK: Android 14 (API 34)

### Step 1: Build Native Library (if not available)
```bash
cd relay-leaf-main/relay-leaf

export ANDROID_NDK_HOME=/path/to/ndk
TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64

# Build for ARM64 (most modern devices)
CGO_ENABLED=1 GOOS=android GOARCH=arm64 \
  CC=$TOOLCHAIN/bin/aarch64-linux-android21-clang \
  go build -buildmode=c-shared -o dist/android_arm64/librelay_leaf.so ./clib/

# Build for ARM32 (older devices)
CGO_ENABLED=1 GOOS=android GOARCH=arm GOARM=7 \
  CC=$TOOLCHAIN/bin/armv7a-linux-androideabi21-clang \
  go build -buildmode=c-shared -o dist/android_arm/librelay_leaf.so ./clib/
```

### Step 2: Copy Library to Android Project
```bash
# Copy to jniLibs folder
cp dist/android_arm64/librelay_leaf.so android/java/lib/jniLibs/arm64-v8a/
cp dist/android_arm/librelay_leaf.so android/java/lib/jniLibs/armeabi-v7a/
```

### Step 3: Add to Gradle
```gradle
android {
    sourceSets {
        main {
            jniLibs.srcDirs = ['lib/jniLibs']
        }
    }
}
```

### Step 4: Run Android Project
```bash
# Open in Android Studio
cd sdk/android/java/example

# Or build via command line
./gradlew assembleDebug
./gradlew installDebug
```

### Usage in Java
```java
import com.relayleaf.RelayLeaf;

RelayLeaf client = new RelayLeaf(true);
client.setPartnerId("your-partner-id");
client.start();

RelayStats stats = client.getStats();
Log.d("Relay", "Connected: " + stats.connected);

client.stop();
client.destroy();
```

## SDK Structure (After Build)

```
android/
├── java/           # JNI bindings
│   ├── lib/
│   │   ├── RelayLeaf.java
│   │   └── jniLibs/
│   │       ├── arm64-v8a/librelay_leaf.so
│   │       └── armeabi-v7a/librelay_leaf.so
│   └── example/
├── kotlin/         # Kotlin bindings
└── README.md
```

## Integration

### Gradle

```gradle
android {
    sourceSets {
        main {
            jniLibs.srcDirs = ['lib/jniLibs']
        }
    }
}
```

### Java Usage

```java
import com.relayleaf.RelayLeaf;

RelayLeaf client = new RelayLeaf(true);
client.setPartnerId("your-partner-id");
client.start();

RelayStats stats = client.getStats();
Log.d("Relay", "Connected: " + stats.connected);

client.stop();
client.destroy();
```

## Languages Supported

- Java (JNI)
- Kotlin (JNI)

## License

MIT License
