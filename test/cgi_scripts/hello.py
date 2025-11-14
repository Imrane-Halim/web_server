#!/usr/bin/env python3
import datetime
import sys

sys.stdout.write("Content-Type: text/html\r\n")
sys.stdout.write("\r\n")

current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

print(f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Python CGI</title>
    <style>
        * {{
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }}
        
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: #f5f5f5;
            min-height: 100vh;
            padding: 40px 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }}
        
        .container {{
            background: white;
            border-radius: 4px;
            padding: 48px;
            max-width: 500px;
            width: 100%;
            box-shadow: 0 1px 3px rgba(0, 0, 0, 0.1);
            text-align: center;
        }}
        
        .status {{
            display: inline-flex;
            align-items: center;
            padding: 8px 16px;
            background: #f0fdf4;
            color: #166534;
            border: 1px solid #bbf7d0;
            border-radius: 3px;
            font-size: 12px;
            font-weight: 500;
            margin-bottom: 32px;
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }}
        
        .status::before {{
            content: "";
            width: 8px;
            height: 8px;
            background: #22c55e;
            border-radius: 50%;
            margin-right: 8px;
        }}
        
        h1 {{
            color: #1a1a1a;
            font-size: 32px;
            font-weight: 700;
            letter-spacing: -0.02em;
            margin-bottom: 16px;
        }}
        
        .time {{
            color: #737373;
            font-size: 14px;
            font-family: 'SF Mono', Monaco, 'Courier New', monospace;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="status">CGI Active</div>
        <h1>Hello World!</h1>
        <div class="time">{current_time}</div>
    </div>
</body>
</html>""")