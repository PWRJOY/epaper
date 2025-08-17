#pragma once

// static const char wifi_config_html[] =
// "<!DOCTYPE html>"
// "<html>"
// "<head><meta charset='utf-8'><title>ESP32 WiFi 配网</title>"
// "<style>body{font-family:Arial;text-align:center;margin-top:50px;}input{padding:10px;margin:10px;width:200px;}button{padding:10px 20px;}</style>"
// "</head>"
// "<body>"
// "<h2>ESP32 WiFi 配网</h2>"
// "<form action='/wifi' method='post'>"
// "<input type='text' name='ssid' placeholder='WiFi 名称' required><br>"
// "<input type='password' name='pass' placeholder='WiFi 密码' required><br>"
// "<button type='submit'>连接</button>"
// "</form>"
// "</body>"
// "</html>";


static const char wifi_config_html[] = R"=====(
<html>
<head><meta charset='utf-8'><title>ESP32 WiFi 配网</title>
<style>body{font-family:Arial;text-align:center;margin-top:50px;}input{padding:10px;margin:10px;width:200px;}button{padding:10px 20px;}</style>
</head>
<body>
<h2>ESP32 WiFi 配网</h2>
<form action='/wifi' method='post'>
<input type='text' name='ssid' placeholder='WiFi 名称' required><br>
<input type='password' name='pass' placeholder='WiFi 密码' required><br>
<button type='submit'>连接</button>
</form>
</body>
</html>
)=====";