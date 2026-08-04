
- Remove builtin prefixes
- add DEFINE MAIN FILE in main.cpp
- On configure add bodySize to be set to bodySize max to be used as a limit when chunked

// Implement a compact function so that if necessary, moves cookie to the end of query so that path + query + cookie < 8192

// Implement state actions to Request so that it switches between reading and writing seamlessly
	// It needs to set the epoll variables and confirm upon entry that it can write
	// With chunked transfer encoding, it will constantly switch between read and write

// Implement method actions to Request so that it can GET, POST, DELETE, CGI or ERROR
	// This involves returning a HTTP Header with the appropriate status code and the payload

// Create a separate Parser class that handles the reading of the header only
	// It is supposed to output a struct containing all of the relevant metadata
	// It does not own the buffer

// Whenever status is deduced, it already loads the string into output buffer index 9 e.g. (HTTP/1.1 _)

// Fake envp added to config for easy insertion

# I/O Model

For `POST` requests, write into a temporary file instead of overwriting the target directly. Once the upload is complete:

1. Delete the original file.
2. Move the temporary file into its place.

## Data Flow Terminology

| Operation  | Source / Destination |
| ---------- | -------------------- |
| Read from  | File descriptor      |
| Write to   | File descriptor      |
| Read into  | Buffer               |
| Write from | Buffer               |

---

# Request Body Decoding

## Normal Body

1. Write from **Client Output** to the **Target FD**.

## Chunked Body

1. Dechunk from **Client Output** into a **stack scratch buffer**.
2. Write from the **stack scratch buffer** to the **Target FD**.
3. Copy any remaining bytes into **Client Input** and compact the buffer.

---

# Entry and Exit Points

## Entry Point

0. Read from **Client FD** into **Client Output**.

## Exit Point

* Write from **Client Input** to **Client FD**.

---

# Initial Assumption

The HTTP header has just finished reading and has been parsed as valid.

---

# Request Flows

## POST

1. Decode from **Client FD** into **File FD**.
2. Build a response header in **Client Input**.

## CGI

1. Shift the **Client Output** offset by `4096`.
2. Decode from **Client FD** into **CGI IN**.
3. Read from **CGI OUT** into **Client Output**.
4. Once the CGI header is found, prepend the HTTP response header to **Client Output**.

## DELETE

1. Delete the target using **File FD**.
2. Build a response header in **Client Input**.

## GET

1. Build the response header in **Client Input**.
2. Read from **File FD** into **Client Input**.

---

# Failure Conditions

A request fails when:

* The body size differs from the expected size.
* The body size exceeds `maxBodySize`.
* Writing to **File FD** fails.
* Deleting the target file fails.
* Opening or retrieving the target file fails.
