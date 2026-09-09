*This project has been created as part of the 42 curriculum by \<adeimlin>, \<feazeved> and \<wlucas-f>.*

# webserv

An HTTP/1.1 server written in C++98, plus a small multiplayer demo built on top
of it to prove the thing actually works in a browser.

## Description

The server reads an nginx-style configuration file and serves whatever it
describes: static files, directory listings, uploads, redirections and CGI. All
socket I/O goes through a single `epoll` loop and never blocks, and `fork` is
used for nothing except running CGI scripts.

To test it with something less boring than `curl`, we built **Club Penguin:
Webserv** — a mock of the old game. You log in, pick a region, and walk a
penguin around a room while everyone else on that region walks around with you.
Every part of it runs through the server: a Python CGI handles the login and
sets a session cookie, a Rust CGI keeps the room state, and the browser polls it
a few times a second.

The four regions are four `server` blocks on four ports, so "everyone on the
same server" is literally true — pick Europe in two browsers and you share a
room, pick different ones and you don't.

## Instructions

```bash
make
./build/webserv config/penguin.conf
```

Then open <http://127.0.0.1:8080/>. Type `http://` explicitly — if your browser
has an `https://` entry cached for that address it will try TLS and fail.

To see two penguins at once you need two cookie jars: a normal window and a
private one, different names, same region.

Other configs in `config/` cover the plainer cases (`default.conf`,
`twoServer.conf`, and `invalid/` for parser errors).

`make` also builds the Rust CGI. If `rustc` is missing it prints a warning and
skips it — the server still builds, only the game stops working.

Requires `python3` and `php-cgi` for the CGI examples. On Ubuntu:
`sudo apt install python3 php-cgi`.

## Features

- `GET`, `POST`, `DELETE`; file uploads; configurable error pages
- Multiple servers on multiple ports from one config file
- Per-route rules: allowed methods, root, index, autoindex, redirection, upload
  directory, CGI by extension
- Chunked request bodies, un-chunked before reaching a CGI
- CGI in three languages — Python, PHP and Rust — listed at `/cgis.html`
- Cookies and sessions, used for the game login and for remembering where your
  penguin was standing

## Technical choices

- **`epoll` over `poll`** — one call for every socket, and we get the ready list
  back instead of scanning the whole set each tick.
- **Header-only `.ipp` files** included from a single translation unit. It keeps
  the build to one compile step, at the cost of needing `-MMD -MP` for the
  dependency tracking to work at all.
- **A file as the game's database.** The CGI is a fresh process per request, so
  the room state has to live somewhere shared. It is a sub-kilobyte text file,
  read and written under an `O_EXCL` lock file and replaced with `rename(2)` so
  a reader never sees half of it. A real database would have been a dependency
  we can't install on the school machines, solving a problem we don't have.

## Resources

- RFC 9110 and RFC 9112 (HTTP semantics and HTTP/1.1)
- nginx docs, mostly for the config syntax and for comparing response headers
- `man` pages for `epoll`, `socket`, `fork`, `execve`, `rename`
