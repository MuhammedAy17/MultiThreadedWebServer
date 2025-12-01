# Concurrent Web Server (C)

A multi-threaded HTTP web server written in C.  
Uses a thread pool, synchronized request queue, and several overload-handling policies.

## Features
- Handles HTTP GET requests (static & dynamic)
- Thread pool for concurrent request processing
- Request queue with mutex + condition variables
- Overload policies: block, dt (drop-tail), dh (drop-head), random, bf (block-flush)
- Per-thread statistics (static, dynamic, total)

## Build
```bash
gcc -o server server.c request.c Queue.c Thread.c segel.c -lpthread
