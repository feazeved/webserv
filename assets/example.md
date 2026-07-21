POST /upload HTTP/1.1
Host: localhost:8080
Content-Type: text/plain
Content-Length: 11
Connection: keep-alive

Hello world

POST /api/upload?overwrite=true&folder=reports HTTP/1.1
Host: localhost:8080
User-Agent: TestClient/1.0
Accept: application/json
Content-Type: application/json
Transfer-Encoding: chunked
Trailer: X-Body-Checksum
Cookie: session=abc123; theme=dark
Connection: keep-alive \r\n\r\n

10
{"name":"report.
11
txt","overwrite":
1c
true,"tags":["work","2026"]}
0
X-Body-Checksum: example-checksum
