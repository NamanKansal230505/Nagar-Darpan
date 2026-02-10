@echo off
echo ========================================
echo Starting Local Web Server...
echo ========================================
echo.
echo Server will start on: http://localhost:8000
echo.
echo To access from other devices on the same WiFi:
echo 1. Find your computer's IP address (shown below)
echo 2. On other devices, open: http://YOUR_IP:8000
echo.
echo Press Ctrl+C to stop the server
echo.
echo ========================================
echo.

:: Get local IP address
for /f "tokens=2 delims=:" %%a in ('ipconfig ^| findstr /i "IPv4"') do (
    set IP=%%a
    set IP=!IP: =!
    echo Your IP Address: !IP!
    echo Access URL: http://!IP!:8000/electricity-theft.html
    echo.
)
echo ========================================
echo.

:: Start Python HTTP server
python -m http.server 8000








