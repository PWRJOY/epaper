#pragma once

static const char wifi_config_html[] = R"=====(
<html>
<head>
    <meta charset='utf-8'>
    <title>ESP32 WiFi 配网</title>
    <style>
        body {font-family:Arial;text-align:center;margin-top:30px;}
        .container {max-width:400px;margin:0 auto;padding:20px;}
        .wifi-list {width:100%;padding:10px;margin:10px 0;border:1px solid #ccc;border-radius:4px;}
        input {padding:10px;margin:10px 0;width:100%;box-sizing:border-box;border:1px solid #ccc;border-radius:4px;}
        button {padding:10px 20px;background-color:#4CAF50;color:white;border:none;border-radius:4px;cursor:pointer;}
        button:hover {background-color:#45a049;}
        .scan-btn {background-color:#2196F3;margin-bottom:20px;}
        .scan-btn:hover {background-color:#0b7dda;}
        .hidden {display:none;}
        .loading {margin:10px;color:#666;}
    </style>
</head>
<body>
    <div class="container">
        <h2>ESP32 WiFi 配网</h2>
        <button class="scan-btn" onclick="scanWiFi()">扫描附近WiFi</button>
        <div class="loading hidden" id="loading">正在扫描WiFi...</div>
        
        <select class="wifi-list" id="wifiSelect" onchange="updateSSID()">
            <option value="">-- 选择WiFi --</option>
            <!-- WiFi列表将通过JavaScript动态添加 -->
        </select><br>
        
        <input type="text" id="ssid" name="ssid" placeholder="WiFi名称（可手动输入）" required><br>
        <input type="password" name="pass" placeholder="WiFi密码" required><br>
        <button type="submit">连接</button>
    </div>

    <form id="wifiForm" action='/wifi' method='post'>
        <input type="hidden" name="ssid" id="formSsid">
        <input type="hidden" name="pass" id="formPass">
    </form>

    <script>
        // 扫描WiFi并更新列表
        function scanWiFi() {
            document.getElementById('loading').classList.remove('hidden');
            document.getElementById('wifiSelect').innerHTML = '<option value="">-- 扫描中... --</option>';
            
            // 从ESP32获取WiFi列表
            fetch('/scan')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('loading').classList.add('hidden');
                    const select = document.getElementById('wifiSelect');
                    select.innerHTML = '<option value="">-- 选择WiFi --</option>';
                    
                    if (data.networks && data.networks.length > 0) {
                        data.networks.forEach(network => {
                            const option = document.createElement('option');
                            option.value = network.ssid;
                            // 显示信号强度（RSSI）
                            option.textContent = `${network.ssid} (信号: ${getSignalStrength(network.rssi)})`;
                            select.appendChild(option);
                        });
                    } else {
                        select.innerHTML += '<option value="">未发现WiFi网络</option>';
                    }
                })
                .catch(error => {
                    document.getElementById('loading').classList.add('hidden');
                    console.error('扫描失败:', error);
                    alert('扫描WiFi失败，请重试');
                });
        }

        // 将RSSI转换为信号强度描述
        function getSignalStrength(rssi) {
            if (rssi >= -50) return "极好";
            if (rssi >= -60) return "良好";
            if (rssi >= -70) return "一般";
            if (rssi >= -80) return "较弱";
            return "差";
        }

        // 当选择WiFi时更新输入框
        function updateSSID() {
            const select = document.getElementById('wifiSelect');
            document.getElementById('ssid').value = select.value;
        }

        // 表单提交处理
        document.querySelector('button[type="submit"]').addEventListener('click', function(e) {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const pass = document.querySelector('input[type="password"]').value;
            
            if (ssid && pass) {
                document.getElementById('formSsid').value = ssid;
                document.getElementById('formPass').value = pass;
                document.getElementById('wifiForm').submit();
            }
        });
    </script>
</body>
</html>
)=====";
