# Architecture

The project owns its HTML parser, text layout and browser UI code in C. `network.c` uses WinHTTP with default certificate validation; `http.c` parses HTTP headers for unit-tested transport logic. The intended dataflow is URL -> WinHTTP GET -> bounded body -> HTML parser -> layout -> GDI.

The current network request implementation has a 1 MiB body limit and 15-second timeouts. HTTPS depends on the system WinHTTP/SChannel stack; it is not Internet Explorer, but it is not a bundled TLS implementation. The next UI integration must move fetch work to a worker thread to avoid blocking the Win32 message loop.

No JavaScript, CSS layout, images, DOM mutation, extensions, sandbox or multiprocess isolation exists.
