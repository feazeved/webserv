#!/usr/bin/env python3
import sys

print("Status: 302 Found")
print("Location: /login.html")
print("Set-Cookie: cp_session=; Path=/; Max-Age=0")
print("Content-Type: text/html; charset=utf-8")
print()
print('<html><head><meta http-equiv="refresh" content="0;url=/login.html"></head><body>Logging out...</body></html>')
