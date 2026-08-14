import json
import os
import sys
from http.server import BaseHTTPRequestHandler, SimpleHTTPRequestHandler, HTTPServer

STATIC_DIR = sys.argv[1] if len(sys.argv) > 1 else "."

state = {
	"wlucas-f": {"x": 10, "y": 50},
	"igomes-d": {"x": 100, "y": 80}
}

class SimpleBackendHandler(SimpleHTTPRequestHandler):

    def log_message(self, format, *args):
        pass

    def _send_cors_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET,POST,OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "x-api-key,Content-Type")

    def send_dict_response(self, d):
        self.wfile.write(bytes(json.dumps(d), "utf8"))

    def do_GET(self):
        if self.path == '/state':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self._send_cors_headers()
            self.end_headers()
            self.send_dict_response([{"username": k, **state[k]} for k in state])
        else:
            SimpleHTTPRequestHandler.do_GET(self)


    def do_POST(self):
        if self.path == '/update':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))
            if "x" not in data:
                state.pop(data["username"])
            else:
                state[data["username"]] = {"x": data["x"], "y": data["y"]}
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self._send_cors_headers()
            self.end_headers()
            self.send_dict_response([{"username": k, **state[k]} for k in state])
        else:
            self.send_error(404, "Endpoint Not Found")

    def do_OPTIONS(self):
        self.send_response(200)
        self._send_cors_headers()
        self.end_headers()

def run_server(port=8081):
    os.chdir(STATIC_DIR)
    
    server_address = ('0.0.0.0', port)
    httpd = HTTPServer(server_address, SimpleBackendHandler)
    print(f"Serving static files from: {os.path.abspath(STATIC_DIR)}")
    print(f"Backend server actively listening on port {port}...")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down server.")
        httpd.server_close()

if __name__ == '__main__':
    run_server()
