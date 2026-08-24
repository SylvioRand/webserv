# webserv

**An HTTP/1.1 web server written from scratch in C++98 — no external libraries.**

Score: 125/125 (mandatory + bonus) — 42 Antananarivo

---

## What this is

A working web server. Not a wrapper around an existing one, not a toy that echoes a fixed string: a
server that parses real HTTP/1.1 requests from real browsers, serves static files, runs CGI scripts,
accepts uploads, and holds many simultaneous connections open without threads and without blocking.

The 42 curriculum forbids external libraries, so every layer is implemented here: the socket setup, the
event loop, the HTTP parser, the response builder, the configuration file parser.

## Features

- **Non-blocking, single-threaded event loop.** One `poll()` call drives every socket. No request can
  stall another, and no operation on a file descriptor happens outside the multiplexer.
- **Multiple servers, ports and virtual hosts** resolved by `Host` header, in one process.
- **Methods:** `GET`, `POST`, `DELETE`.
- **Chunked transfer encoding**, both directions.
- **File uploads** to a configurable directory.
- **CGI execution** — the server forks, wires up the pipes and environment, and streams the script's
  output back without blocking the loop.
- **NGINX-inspired configuration file**: `listen`, `server_name`, `root`, `index`, `error_page`,
  `client_max_body_size`, per-location method restrictions, autoindex, redirects.
- **Custom error pages** and correct status-code semantics.
- Accurate `Content-Length` / `Content-Type` handling, keep-alive, and graceful client disconnection.

## Build and run

```sh
make
./webserv config/default.conf
```

Then open `http://localhost:8080`.

## Architecture

```
main
 └── Config parser        reads the .conf, builds the server blocks
 └── Server sockets       one listening fd per host:port
 └── Event loop (poll)
      ├── accept()        new client → Connection object
      ├── read            feed bytes into the per-connection request parser
      ├── parse           request line → headers → body (chunked or content-length)
      ├── route           match server block, then location block
      ├── handle          static file · upload · delete · CGI
      └── write           stream the response back, non-blocking
```

The parser is a **state machine per connection**, because with non-blocking sockets a request arrives in
arbitrary fragments. There is no guarantee that headers arrive in one read, or that a read stops at a
message boundary. Every connection therefore carries its own parse state and buffer.

## The parts that were genuinely hard

**Never blocking, anywhere.** The rule sounds simple and it constrains everything. A single `read()` on a
socket outside the poll loop, or a CGI script that writes more than a pipe buffer while nobody drains it,
and the whole server freezes. Getting CGI right — forking, plumbing both pipe ends into the event loop,
reaping the child without ever calling a blocking `waitpid` — was the hardest part of the project.

**Partial writes.** A large response does not leave in one `send()`. The connection has to remember how
much of the response it has already written and resume exactly there when the socket is writable again.

**Trusting nothing from the client.** Malformed request lines, absurd `Content-Length` values, headers
without a colon, bodies larger than the configured maximum, connections that vanish mid-request. Every one
of them has to produce a correct status code instead of a crash.

## What I took away from it

HTTP is a much more forgiving protocol to read about than to implement. The specification is full of
"SHOULD" clauses that real browsers interpret in their own way, and the only reliable test is pointing
Chrome, curl and `siege` at the server and watching what actually happens.

More broadly, this project is where non-blocking I/O stopped being an abstraction to me. Once you have
built an event loop by hand, `epoll`, `libuv` and Node's runtime stop being magic.

---

*Built at [42 Antananarivo](https://42antananarivo.mg/), where the curriculum bans frameworks and external
libraries so you have to understand the machinery underneath.*
