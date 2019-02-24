# MtMergeSort
Exploration of WebAssembly multi-threading on plain merge sort

## Build
To build a native executable, run `build.sh` (on Windows, run it in _Git Bash_) or invoke your compiler directly, e.g. with sanitizers:
```
g++ src/main.cpp -std=c++17 -g -O0 -lpthread s -fsanitize=thread
```
or  
```
g++ src/main.cpp -std=c++17 -g -O0 -lpthread s -fsanitize=address
```

To build WebAssembly, compile with Emscripten (I used version 1.38.16). For single-threaded version, build as:  
```
emcc -std=c++17 -Os -DNDEBUG  --llvm-lto 1 -s FILESYSTEM=0 -s TOTAL_MEMORY=1073741824 -o t.js main.cpp
```
and for multi-threaded runs it is necessary to enable filesystem emulation (for some reason, it is required even without memory init file) and set threading related options, e.g. for 4 threads:  
```
 emcc -std=c++17 -Os --llvm-lto 1 -s FILESYSTEM=1 -s -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=7 -s PROXY_TO_PTHREAD=1 -DNDEBUG -s TOTAL_MEMORY=1073741824 --memory-init-file 0 -o t.js main.cpp
```
Then create `index.html`, e.g.  
```html
<!DOCTYPE html>
<html>
  <title>Threads test</title>
  <body>
    <script src="t.js"></script>
  </body>
</html>
```
and serve it with some HTTP server, e.g. `http-server` that can be installed with _npm_:  
```
npm i -g http-server
```
Then open `localhost:8080` in Chrome with enabled WebAssembly thread support, as described here:  
https://developers.google.com/web/updates/2018/10/wasm-threads  
and look in the Console (F12).

## Results
Native code shows about 3x speed up when sorting 10,000,000 floats on 4 threads. When running 8 threads on a 4-core CPU, oversubscription constitutes itself in slight slowdown. WebAssembly running in Chrome 72 shows the expected performance with a single thread (1.5...2 times as slow as the native code), but gets slower executing multi-threaded WebAssembly:  
![Comparison](/doc/results.png)
Sorting on a single thread takes native code 1.80 seconds, WebAssembly 3.1...3.3 seconds, manually written JavaScript 4.69 seconds, and Kotlin/JVM (Kotlin 1.3.21 with JRE 1.8) 2.2 seconds. Splitting the work between several threads causes the sort time in WebAssembly to inscrease, this tendency is worse in Chrome than in Firefox.