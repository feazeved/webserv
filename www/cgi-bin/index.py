#!/usr/bin/env python3
import os
import sys

def main():
    cookie_header = os.environ.get("HTTP_COOKIE", "")

    if "cp_session=" not in cookie_header:
        print("Status: 302 Found")
        print("Location: /login.html")
        print("Content-Type: text/html; charset=utf-8")
        print()
        print('<html><head><meta http-equiv="refresh" content="0;url=/login.html"></head><body>Redirecting to login...</body></html>')
        sys.exit(0)

    print("Content-Type: text/html; charset=utf-8")
    print()

    game_file_path = os.path.join(os.path.dirname(__file__), "..", "game.html")
    if not os.path.exists(game_file_path):
        game_file_path = "./game.html"

    try:
        with open(game_file_path, "r", encoding="utf-8") as f:
            sys.stdout.write(f.read())
    except OSError:
        print("<h1>500 Internal Server Error: game.html not found</h1>")

if __name__ == "__main__":
    main()
