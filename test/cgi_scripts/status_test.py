#!/usr/bin/env python3
"""
CGI script that tests custom status codes
Responds with status code based on query parameter
Usage: /cgi/status?code=404
"""
import os

# Get status code from query string
query = os.environ.get('QUERY_STRING', '')
status_code = 200

if 'code=' in query:
    try:
        status_code = int(query.split('code=')[1].split('&')[0])
    except:
        status_code = 400

print(f"Status: {status_code}")
print("Content-Type: text/html")
print()  # Empty line

print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Status Code Test</title></head>")
print("<body>")
print(f"<h1>Status Code: {status_code}</h1>")
print(f"<p>Query String: {query}</p>")
print("</body>")
print("</html>")
