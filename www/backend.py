import json
import os
import sys
import time
import http.server
from urllib.parse import parse_qs, quote

PORT = 8081
STATIC_DIR = sys.argv[1] if len(sys.argv) > 1 else "."

# {"username": {"x": int, "y": int, "last_seen": float}}
state = {}

def cleanup_ghosts():
    """Remove players who haven't sent a heartbeat/update in over 3 seconds."""
    now = time.time()
    ghosts = [user for user, info in state.items() if now - info.get("last_seen", 0) > 3]
    for user in ghosts:
        del state[user]

class FullGameAndCGIServer(http.server.CGIHTTPRequestHandler):
    cgi_directories = ["/cgi-bin"]

    def log_message(self, format, *args):
        pass

    def _send_cors_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "x-api-key,Content-Type")

    def send_dict_response(self, d):
        self.wfile.write(bytes(json.dumps(d), "utf8"))

    def do_GET(self):
        path = self.path.split("?")[0]

        if path in ["/", "/index.html"]:
            self.path = "/cgi-bin/index.py"
            super().do_GET()
            return

        if path == "/state":
            cleanup_ghosts()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self._send_cors_headers()
            self.end_headers()
            self.send_dict_response([{"username": k, "x": v["x"], "y": v["y"]} for k, v in state.items()])
            return

        super().do_GET()

    def do_POST(self):
        path = self.path.split("?")[0]

        if path == "/update":
            content_length = int(self.headers.get("Content-Length", 0))
            post_data = self.rfile.read(content_length)

            try:
                data = json.loads(post_data.decode("utf-8"))
            except Exception:
                data = {}

            username = data.get("username")
            if username:
                if "x" not in data or "y" not in data:
                    state.pop(username, None)
                else:
                    state[username] = {
                        "x": int(data["x"]),
                        "y": int(data["y"]),
                        "last_seen": time.time()
                    }

            cleanup_ghosts()
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self._send_cors_headers()
            self.end_headers()
            self.send_dict_response([{"username": k, "x": v["x"], "y": v["y"]} for k, v in state.items()])
            return

        super().do_POST()

    def do_OPTIONS(self):
        self.send_response(200)
        self._send_cors_headers()
        self.end_headers()

def run_server():
    os.chdir(STATIC_DIR)
    os.makedirs("./cgi-bin", exist_ok=True)
    server_address = ("0.0.0.0", PORT)

    httpd = http.server.HTTPServer(server_address, FullGameAndCGIServer)

    print(f"Serving static files from: {os.path.abspath(STATIC_DIR)}")
    print(f"Server listening on http://localhost:{PORT}...")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down server.")
        httpd.server_close()

if __name__ == "__main__":
    run_server()
