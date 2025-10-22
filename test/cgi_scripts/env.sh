#!/bin/bash
# CGI script to display all environment variables

echo "Content-Type: text/html"
echo ""
echo "<!DOCTYPE html>"
echo "<html>"
echo "<head><title>CGI Environment Variables</title></head>"
echo "<body>"
echo "<h1>CGI Environment Variables</h1>"
echo "<table border='1'>"
echo "<tr><th>Variable</th><th>Value</th></tr>"

# Print all environment variables
env | sort | while IFS='=' read -r key value; do
    echo "<tr><td>$key</td><td>$value</td></tr>"
done

echo "</table>"
echo "</body>"
echo "</html>"
