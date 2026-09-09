#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs, quote

SERVER_PORTS = {
    "america": 8001,
    "africa":  8002,
    "europe":  8003,
    "oceania": 8004,
}

def read_body():
    try:
        length = int(os.environ.get("CONTENT_LENGTH", "0") or 0)
    except ValueError:
        length = 0
    return sys.stdin.buffer.read(length) if length > 0 else b""

def back_to_login():
    print("Status: 302 Found")
    print("Location: /login.html?error=1")
    print("Content-Type: text/html; charset=utf-8")
    print()
    print("<html><body>Redirecting to login...</body></html>")
    sys.exit(0)

def main():
    if os.environ.get("REQUEST_METHOD", "GET") != "POST":
        back_to_login()

    body = read_body().decode("utf-8", errors="replace")
    form = parse_qs(body)

    username = form.get("username", [""])[0].strip()
    server_choice = form.get("dropdown", [""])[0].strip().lower()
    accepted_terms = form.get("terms", [""])[0] == "on"

    if not username or not accepted_terms:
        back_to_login()

    port = SERVER_PORTS.get(server_choice, os.environ.get("SERVER_PORT", 8081))
    host_env = os.environ.get("HTTP_HOST", os.environ.get("SERVER_NAME", "localhost"))
    host = host_env.split(":")[0]

    redirect_url = f"http://{host}:{port}/index.html"

    print("Status: 302 Found")
    print(f"Location: {redirect_url}")
    print(f"Set-Cookie: cp_session={quote(username)}; Path=/; Max-Age=3600")
    print("Content-Type: text/html; charset=utf-8")
    print()
    print(f'<html><head><meta http-equiv="refresh" content="0;url=/index.html"></head><body>Logged in as {username}. Redirecting...</body></html>')

if __name__ == "__main__":
    main()
