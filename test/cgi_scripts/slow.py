#!/usr/bin/env python3
"""
CGI script that simulates a slow response for timeout testing
Usage: /cgi/slow?sleep=5 (sleeps for 5 seconds)
"""
import os
import time

# Get sleep duration from query string
query = os.environ.get('QUERY_STRING', '')
sleep_time = 2  # default

if 'sleep=' in query:
    try:
        sleep_time = int(query.split('sleep=')[1].split('&')[0])
    except:
        sleep_time = 2

print("Content-Type: text/html")
print()  # Empty line

print("<!DOCTYPE html>")
print("<html>")
print("<head><title>Slow CGI Test</title></head>")
print("<body>")
print(f"<h1>Sleeping for {sleep_time} seconds...</h1>")
print("</body>")
print("</html>")

# Flush to send headers immediately
import sys
sys.stdout.flush()

# Sleep
time.sleep(sleep_time)
