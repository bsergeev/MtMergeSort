# MtMergeSort
Exploration of WebAssembly multi-threading on plain merge sort

## Build
To build a native executable, run `build.sh` (on Windows, run it in _Git Bash_).  
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
While native code shows about 3.5 speed up when sorting 10,000,000 floats on 6 threads (4 first level and 2 second level sub-array merging), Chrome actiually gets twice as slow executing multi-threaded WebAssembly... After several runs, Chrome crashes. 