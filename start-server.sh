#!/bin/bash

echo "========================================"
echo "Starting Local Web Server..."
echo "========================================"
echo ""
echo "Server will start on: http://localhost:8000"
echo ""
echo "To access from other devices on the same WiFi:"
echo "1. Find your computer's IP address (shown below)"
echo "2. On other devices, open: http://YOUR_IP:8000"
echo ""
echo "Press Ctrl+C to stop the server"
echo ""
echo "========================================"
echo ""

# Get local IP address (Linux/Mac)
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    IP=$(hostname -I | awk '{print $1}')
elif [[ "$OSTYPE" == "darwin"* ]]; then
    IP=$(ipconfig getifaddr en0 || ipconfig getifaddr en1)
fi

if [ ! -z "$IP" ]; then
    echo "Your IP Address: $IP"
    echo "Access URL: http://$IP:8000/electricity-theft.html"
    echo ""
fi

echo "========================================"
echo ""

# Start Python HTTP server
python3 -m http.server 8000








