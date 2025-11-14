#!/usr/bin/env python3
"""
CGI script that echoes back POST data and request information
"""
import os
import sys


# Write HTTP headers manually with CRLF endings
sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("Cache-Control: no-cache\r\n")
sys.stdout.write("\r\n")  # End of headers

# Now print the HTML body
print("""<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CGI Echo Test</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: #f5f5f5;
            min-height: 100vh;
            padding: 40px 20px;
        }
        .container {
            max-width: 900px;
            margin: 0 auto;
        }
        header {
            background: white;
            border-radius: 4px;
            padding: 30px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
            margin-bottom: 32px;
        }
        .status {
            display: inline-flex;
            align-items: center;
            padding: 8px 16px;
            background: #f0fdf4;
            color: #166534;
            border: 1px solid #bbf7d0;
            border-radius: 3px;
            font-size: 12px;
            font-weight: 500;
            margin-bottom: 20px;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }
        .status::before {
            content: "";
            width: 8px;
            height: 8px;
            background: #22c55e;
            border-radius: 50%;
            margin-right: 8px;
            animation: pulse 2s ease-in-out infinite;
        }
        @keyframes pulse {
            0%, 100% {
                opacity: 1;
            }
            50% {
                opacity: 0.5;
            }
        }
        h1 {
            color: #1a1a1a;
            font-size: 32px;
            font-weight: 700;
            letter-spacing: -0.02em;
            margin-bottom: 8px;
        }
        .section {
            background: white;
            border-radius: 4px;
            padding: 32px;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
            margin-bottom: 24px;
        }
        .section-title {
            font-size: 13px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: #404040;
            margin-bottom: 20px;
            padding-bottom: 12px;
            border-bottom: 1px solid #e5e5e5;
        }
        .info-box {
            background: #fafafa;
            border: 1px solid #e5e5e5;
            border-radius: 3px;
            padding: 20px;
            font-family: 'SF Mono', Monaco, 'Courier New', monospace;
            font-size: 13px;
            color: #1a1a1a;
            line-height: 1.6;
            overflow-x: auto;
        }
        .empty-state {
            color: #737373;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <div class="status">Request Received</div>
            <h1>CGI Echo Test</h1>
        </header>""")

# Display environment variables
print("""        <div class="section">
            <div class="section-title">Environment Variables</div>
            <div class="info-box">""")

for key in sorted(os.environ.keys()):
    if key.startswith('HTTP_') or key in [
        'REQUEST_METHOD', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH',
        'SCRIPT_NAME', 'SCRIPT_FILENAME', 'SERVER_NAME', 'SERVER_PORT',
        'GATEWAY_INTERFACE', 'SERVER_PROTOCOL'
    ]:
        print(f"{key}: {os.environ[key]}<br>")

print("""            </div>
        </div>""")

# Display POST data
print("""        <div class="section">
            <div class="section-title">POST Data</div>
            <div class="info-box">""")

content_length = os.environ.get('CONTENT_LENGTH', '0')
if content_length and content_length != '0':
    post_data = sys.stdin.read(int(content_length))
    print(f"{post_data}")
else:
    print('<span class="empty-state">No POST data received</span>')

print("""            </div>
        </div>""")

# Display query string
print("""        <div class="section">
            <div class="section-title">Query String</div>
            <div class="info-box">""")

query_string = os.environ.get('QUERY_STRING', '')
if query_string:
    print(f"{query_string}")
else:
    print('<span class="empty-state">No query string</span>')

print("""            </div>
        </div>
    </div>
</body>
</html>""")