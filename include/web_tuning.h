#ifndef WEB_TUNING_H
#define WEB_TUNING_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "config.h"

extern MapProfile profiles[3];
extern int selectedMap;
extern Preferences pref; 

extern void triggerStart();
extern void triggerStop();
extern void triggerCalib();

WebServer server(80);

const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
    <meta charset="UTF-8">
    <title>ESP32 Line Control</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, monospace; background: #121212; color: #e4e4e7; padding: 12px; font-size: 14px; }
        .container { max-width: 360px; margin: 0 auto; }
        
        .box { background: #1e1e24; border: 1px solid #2d2d35; border-radius: 6px; padding: 12px; margin-bottom: 10px; }
        .box-title { font-size: 12px; font-weight: 700; text-transform: uppercase; color: #a1a1aa; letter-spacing: 0.5px; margin-bottom: 10px; border-bottom: 1px solid #2d2d35; padding-bottom: 4px; }
        
        .row { display: flex; align-items: center; justify-content: space-between; margin-bottom: 8px; }
        label { font-size: 13px; color: #a1a1aa; font-weight: 500; }
        input, select { width: 55%; padding: 6px 8px; background: #121212; border: 1px solid #3f3f46; border-radius: 4px; color: #38bdf8; font-family: monospace; font-size: 14px; text-align: right; }
        input:focus, select:focus { border-color: #38bdf8; outline: none; }
        
        .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-bottom: 8px; }
        button { width: 100%; padding: 10px; border: none; border-radius: 4px; font-weight: 600; font-size: 13px; cursor: pointer; }
        
        .btn-start { background: #166534; color: #f0fdf4; }
        .btn-stop { background: #991b1b; color: #fef2f2; }
        .btn-calib { background: #d97706; color: #fff; }
        .btn-save { background: #2563eb; color: #fff; margin-top: 4px; }
        
        button:active { opacity: 0.8; }
        #status { font-family: monospace; font-size: 12px; color: #71717a; text-align: center; margin-top: 8px; min-height: 16px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="box">
            <div class="box-title">Lệnh nhanh</div>
            <div class="btn-grid">
                <button class="btn-start" onclick="sendCmd('/start')">START</button>
                <button class="btn-stop" onclick="sendCmd('/stop')">STOP</button>
            </div>
            <button class="btn-calib" onclick="sendCmd('/calib')">CALIB CẢM BIẾN</button>
        </div>

        <div class="box">
            <div class="box-title">Thông số PID</div>
            <div class="row">
                <label>Profile</label>
                <select id="mapSelect">
                    <option value="0">MAP 1</option>
                    <option value="1" selected>MAP 2</option>
                    <option value="2">MAP 3</option>
                </select>
            </div>
            <div class="row"><label>Kp</label><input type="number" id="kp" step="0.001"></div>
            <div class="row"><label>Kd</label><input type="number" id="kd" step="0.005"></div>
            <div class="row"><label>Base Speed</label><input type="number" id="speed" step="5"></div>
            <div class="row"><label>kSlow</label><input type="number" id="kslow" step="0.001"></div>
            
            <button class="btn-save" onclick="saveValues()">LƯU CẤU HÌNH</button>
            <div id="status">Ready</div>
        </div>
    </div>

    <script>
        function sendCmd(p) {
            fetch(p).then(r => r.text()).then(d => { document.getElementById('status').innerText = d; });
        }
        function saveValues() {
            let m = document.getElementById('mapSelect').value;
            let kp = document.getElementById('kp').value;
            let kd = document.getElementById('kd').value;
            let sp = document.getElementById('speed').value;
            let ks = document.getElementById('kslow').value;
            fetch(`/set?m=${m}&kp=${kp}&kd=${kd}&sp=${sp}&ks=${ks}`)
                .then(r => r.text())
                .then(d => { document.getElementById('status').innerText = d; });
        }
    </script>
</body>
</html>
)rawliteral";

void saveProfilesToFlash() {
    pref.begin("line_p", false);
    for (int i = 0; i < 3; i++) {
        String p = "m" + String(i);
        pref.putFloat((p + "kp").c_str(), profiles[i].kp);
        pref.putFloat((p + "kd").c_str(), profiles[i].kd);
        pref.putInt((p + "sp").c_str(), profiles[i].maxBaseSpeed);
        pref.putFloat((p + "ks").c_str(), profiles[i].kSlow);
    }
    pref.end();
}

void loadProfilesFromFlash() {
    pref.begin("line_p", true);
    for (int i = 0; i < 3; i++) {
        String p = "m" + String(i);
        profiles[i].kp = pref.getFloat((p + "kp").c_str(), profiles[i].kp);
        profiles[i].kd = pref.getFloat((p + "kd").c_str(), profiles[i].kd);
        profiles[i].maxBaseSpeed = pref.getInt((p + "sp").c_str(), profiles[i].maxBaseSpeed);
        profiles[i].kSlow = pref.getFloat((p + "ks").c_str(), profiles[i].kSlow);
    }
    pref.end();
}

void handleRoot() { server.send(200, "text/html", HTML_PAGE); }

void handleSet() {
    if (server.hasArg("m")) {
        int m = server.arg("m").toInt();
        if (m >= 0 && m < 3) {
            selectedMap = m;
            if (server.hasArg("kp")) profiles[m].kp = server.arg("kp").toFloat();
            if (server.hasArg("kd")) profiles[m].kd = server.arg("kd").toFloat();
            if (server.hasArg("sp")) profiles[m].maxBaseSpeed = server.arg("sp").toInt();
            if (server.hasArg("ks")) profiles[m].kSlow = server.arg("ks").toFloat();

            saveProfilesToFlash();

            server.send(200, "text/plain", "[OK] Map " + String(m + 1) + " Saved");
            return;
        }
    }
    server.send(400, "text/plain", "[ERR] Invalid Args");
}

void handleStartCmd() { triggerStart(); server.send(200, "text/plain", "[CMD] START"); }
void handleStopCmd()  { triggerStop();  server.send(200, "text/plain", "[CMD] STOP"); }
void handleCalibCmd() { triggerCalib(); server.send(200, "text/plain", "[CMD] CALIB"); }

void setupWebTuning() {
    WiFi.softAP("XE_LINE_TUNING", "");
    server.on("/", handleRoot);
    server.on("/set", handleSet);
    server.on("/start", handleStartCmd);
    server.on("/stop", handleStopCmd);
    server.on("/calib", handleCalibCmd);
    server.begin();
}

void handleWebTuning() { server.handleClient(); }

#endif