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

To build WebAssembly, compile with Emscripten (I used version 1.38.16):  
```
emcc -std=c++17 -O3 -DNDEBUG -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=7 -s PROXY_TO_PTHREAD=1 -s TOTAL_MEMORY=1073741824 -o t.js main.cpp
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
Native code shows about 3 speed up when sorting 10,000,000 floats on 4 threads. When running 8 threads on a 4-core CPU, oversubscription constitutes itself in slight slowdown. WebAssembly running in Chrome 72 shows decent performance with a single thread (about 50% slower than native code), but gets slower executing multi-threaded WebAssembly:  
![Comparison](/doc/results.png)