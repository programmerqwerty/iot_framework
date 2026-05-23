
#include "webserver_station.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "station_0";
const char *password = "123456789";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>SmartHome Station</title>
<style>
*, *::before, *::after {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

:root {
  --bg: #f0f2f5;
  --surface: #ffffff;
  --surface-alt: #f7f8fa;
  --primary: #3a7bd5;
  --primary-light: #e8f0fe;
  --primary-dark: #2a5ba0;
  --accent: #00b894;
  --accent-light: #e6faf5;
  --warn: #e17055;
  --warn-light: #fdecea;
  --text: #2d3436;
  --text-secondary: #636e72;
  --text-muted: #b2bec3;
  --border: #dfe6e9;
  --shadow: 0 2px 12px rgba(0,0,0,0.06);
  --shadow-lg: 0 8px 32px rgba(0,0,0,0.10);
  --radius: 14px;
  --radius-sm: 8px;
  --nav-height: 60px;
  --transition: 0.3s ease;
}

html {
  scroll-behavior: smooth;
  scroll-padding-top: calc(var(--nav-height) + 16px);
}

body {
  font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
  background: linear-gradient(160deg, #d4c4b0 0%%, #ddd2c3 30%%, #e8dfd4 55%%, #ddd0c0 80%%, #d1c1ab 100%%);
  background-attachment: fixed;
  color: var(--text);
  line-height: 1.6;
  min-height: 100vh;
}

.navbar {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  height: var(--nav-height);
  background: linear-gradient(135deg, #d4c4b0 0%%, #ddd2c3 40%%, #e5dbd0 100%%);
  border-bottom: 1px solid var(--border);
  display: flex;
  align-items: center;
  padding: 0 24px;
  z-index: 1000;
  box-shadow: 0 1px 8px rgba(0,0,0,0.04);
}

.navbar .logo {
  display: flex;
  align-items: center;
  gap: 10px;
  font-weight: 700;
  font-size: 1.15rem;
  color: var(--primary);
  text-decoration: none;
  flex-shrink: 0;
}

.logo-icon {
  width: 32px;
  height: 32px;
  background: var(--primary);
  border-radius: 50%%;
}

.nav-links {
  display: flex;
  gap: 4px;
  margin-left: auto;
  overflow-x: auto;
}

.nav-links a {
  text-decoration: none;
  color: var(--text-secondary);
  font-size: 0.88rem;
  font-weight: 500;
  padding: 8px 14px;
  border-radius: var(--radius-sm);
  white-space: nowrap;
  transition: background var(--transition), color var(--transition);
}

.nav-links a:hover,
.nav-links a.active {
  background: var(--primary-light);
  color: var(--primary);
}

.menu-toggle {
  display: none;
  background: none;
  border: none;
  cursor: pointer;
  margin-left: auto;
  padding: 8px;
}

.menu-toggle span {
  display: block;
  width: 22px;
  height: 2px;
  background: var(--text);
  margin: 5px 0;
  border-radius: 2px;
  transition: var(--transition);
}

.main {
  max-width: 1100px;
  margin: 0 auto;
  padding: calc(var(--nav-height) + 24px) 20px 60px;
}

.section-title {
  font-size: 1.35rem;
  font-weight: 700;
  margin-bottom: 20px;
  color: var(--text);
  display: flex;
  align-items: center;
  gap: 10px;
}

.section-title .icon {
  width: 36px;
  height: 36px;
  border-radius: 10px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.1rem;
  flex-shrink: 0;
}

.card {
  background: var(--surface);
  border-radius: var(--radius);
  padding: 24px;
  box-shadow: var(--shadow);
  border: 1px solid var(--border);
  transition: box-shadow var(--transition);
}

.card:hover {
  box-shadow: var(--shadow-lg);
}

#dashboard {
  margin-bottom: 36px;
}

.dashboard-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  flex-wrap: wrap;
  gap: 16px;
  margin-bottom: 24px;
}

.greeting h1 {
  font-size: 1.6rem;
  font-weight: 700;
}

.greeting p {
  color: var(--text-secondary);
  font-size: 0.95rem;
  margin-top: 4px;
}

.status-badge {
  display: flex;
  align-items: center;
  gap: 8px;
  background: var(--accent-light);
  color: var(--accent);
  padding: 8px 16px;
  border-radius: 20px;
  font-size: 0.85rem;
  font-weight: 600;
}

.status-dot {
  width: 8px;
  height: 8px;
  background: var(--accent);
  border-radius: 50%%;
  animation: pulse-dot 2s ease-in-out infinite;
}

@keyframes pulse-dot {
  0%%, 100%% { opacity: 1; transform: scale(1); }
  50%% { opacity: 0.5; transform: scale(1.3); }
}

.quick-stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 16px;
}

.stat-card {
  background: var(--surface);
  border-radius: var(--radius);
  padding: 20px;
  box-shadow: var(--shadow);
  border: 1px solid var(--border);
  display: flex;
  align-items: center;
  gap: 16px;
}

.stat-icon {
  width: 48px;
  height: 48px;
  border-radius: 12px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.3rem;
  flex-shrink: 0;
}

.stat-icon.water { background: #e8f4fd; color: #2196f3; }
.stat-icon.temp { background: #fff3e0; color: #ff9800; }
.stat-icon.devices { background: #e8f5e9; color: #4caf50; }
.stat-icon.alerts { background: #fce4ec; color: #e91e63; }

.stat-info h3 {
  font-size: 1.4rem;
  font-weight: 700;
  line-height: 1.2;
}

.stat-info p {
  font-size: 0.82rem;
  color: var(--text-secondary);
  margin-top: 2px;
}

#water {
  margin-bottom: 36px;
}

.water-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}

.meter-display {
  grid-column: 1 / -1;
}

.meter-visual {
  display: flex;
  align-items: center;
  justify-content: center;
  flex-direction: column;
  gap: 24px;
  padding: 32px 24px;
}

.odometer-wrapper {
  position: relative;
}

.odometer-label {
  text-align: center;
  font-size: 0.85rem;
  color: var(--text-secondary);
  margin-bottom: 12px;
  font-weight: 500;
  letter-spacing: 0.5px;
  text-transform: uppercase;
}

.odometer {
  display: flex;
  gap: 4px;
  background: #1a1a2e;
  padding: 16px 20px;
  border-radius: 12px;
  box-shadow: inset 0 4px 12px rgba(0,0,0,0.3), 0 4px 16px rgba(0,0,0,0.15);
  position: relative;
}

.odometer::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 50%%;
  background: rgba(255,255,255,0.03);
  border-radius: 12px 12px 0 0;
  pointer-events: none;
}

.odo-digit {
  width: 44px;
  height: 64px;
  background: linear-gradient(180deg, #16213e 0%%, #0f3460 100%%);
  border-radius: 6px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 2.2rem;
  font-weight: 700;
  color: #e0e0e0;
  font-family: 'Consolas', 'Courier New', monospace;
  position: relative;
  overflow: hidden;
  box-shadow: inset 0 1px 3px rgba(0,0,0,0.4);
}

.odo-digit::after {
  content: '';
  position: absolute;
  top: 50%%;
  left: 0;
  right: 0;
  height: 1px;
  background: rgba(0,0,0,0.3);
}

.odo-digit.decimal {
  color: #e74c3c;
  background: linear-gradient(180deg, #1a1a2e 0%%, #2d1b33 100%%);
}

.odo-separator {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 12px;
  font-size: 2rem;
  color: #e74c3c;
  font-weight: 700;
}

.odo-unit {
  display: flex;
  align-items: center;
  margin-left: 8px;
  font-size: 0.85rem;
  color: #7f8fa6;
  font-weight: 600;
  letter-spacing: 1px;
}

.flow-indicator {
  display: flex;
  align-items: center;
  gap: 16px;
  padding: 16px 24px;
  background: var(--surface-alt);
  border-radius: 10px;
  width: 100%%;
  max-width: 400px;
}

.flow-ring {
  width: 52px;
  height: 52px;
  border-radius: 50%%;
  border: 4px solid var(--border);
  position: relative;
  flex-shrink: 0;
}

.flow-ring-fill {
  position: absolute;
  inset: -4px;
  border-radius: 50%%;
  border: 4px solid transparent;
  border-top-color: var(--primary);
  animation: spin-flow 1.5s linear infinite;
}

.flow-ring-fill.inactive {
  animation: none;
  border-top-color: var(--text-muted);
}

@keyframes spin-flow {
  to { transform: rotate(360deg); }
}

.flow-info .flow-rate {
  font-size: 1.15rem;
  font-weight: 700;
}

.flow-info .flow-label {
  font-size: 0.78rem;
  color: var(--text-secondary);
}

.usage-card h3 {
  font-size: 1rem;
  font-weight: 600;
  margin-bottom: 16px;
}

.usage-bars {
  display: flex;
  flex-direction: column;
  gap: 14px;
}

.usage-bar-item {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.usage-bar-header {
  display: flex;
  justify-content: space-between;
  font-size: 0.84rem;
}

.usage-bar-header span:first-child {
  color: var(--text-secondary);
}

.usage-bar-header span:last-child {
  font-weight: 600;
}

.usage-bar-track {
  height: 8px;
  background: var(--bg);
  border-radius: 4px;
  overflow: hidden;
}

.usage-bar-fill {
  height: 100%%;
  border-radius: 4px;
  transition: width 1.5s ease;
  width: 0;
}

.usage-bar-fill.blue { background: linear-gradient(90deg, #74b9ff, #0984e3); }
.usage-bar-fill.teal { background: linear-gradient(90deg, #81ecec, #00b894); }
.usage-bar-fill.orange { background: linear-gradient(90deg, #ffeaa7, #fdcb6e); }
.usage-bar-fill.pink { background: linear-gradient(90deg, #fd79a8, #e84393); }

.billing-card h3 {
  font-size: 1rem;
  font-weight: 600;
  margin-bottom: 16px;
}

.billing-rows {
  display: flex;
  flex-direction: column;
  gap: 12px;
}

.billing-row {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 14px;
  background: var(--surface-alt);
  border-radius: var(--radius-sm);
  font-size: 0.9rem;
}

.billing-row .label {
  color: var(--text-secondary);
}

.billing-row .value {
  font-weight: 700;
}

.billing-row.total {
  background: var(--primary-light);
  color: var(--primary-dark);
  font-weight: 600;
}

.billing-row.total .value {
  font-size: 1.1rem;
}

#sensors {
  margin-bottom: 36px;
}

.sensors-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: 16px;
}

.sensor-card {
  padding: 20px;
  position: relative;
  overflow: hidden;
}

.sensor-card .sensor-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 14px;
}

.sensor-name {
  font-size: 0.85rem;
  color: var(--text-secondary);
  font-weight: 500;
}

.sensor-status {
  width: 8px;
  height: 8px;
  border-radius: 50%%;
  background: var(--accent);
}

.sensor-status.offline {
  background: var(--text-muted);
}

.sensor-value {
  font-size: 2rem;
  font-weight: 700;
  line-height: 1.2;
  margin-bottom: 4px;
}

.sensor-unit {
  font-size: 0.9rem;
  font-weight: 400;
  color: var(--text-secondary);
}

.sensor-change {
  font-size: 0.78rem;
  margin-top: 8px;
  display: flex;
  align-items: center;
  gap: 4px;
}

.sensor-change.up { color: var(--warn); }
.sensor-change.down { color: var(--accent); }
.sensor-change.neutral { color: var(--text-muted); }

.sparkline {
  display: flex;
  align-items: flex-end;
  gap: 2px;
  height: 32px;
  margin-top: 12px;
}

.sparkline .bar {
  flex: 1;
  background: var(--primary-light);
  border-radius: 2px 2px 0 0;
  min-width: 4px;
  transition: height 0.8s ease;
}

.sparkline .bar.accent { background: rgba(0, 184, 148, 0.2); }
.sparkline .bar.warn { background: rgba(225, 112, 85, 0.2); }

#devices {
  margin-bottom: 36px;
}

.devices-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 16px;
}

.device-card {
  padding: 20px;
  text-align: center;
  cursor: pointer;
  transition: transform var(--transition), box-shadow var(--transition);
}

.device-card:active {
  transform: scale(0.97);
}

.device-icon {
  width: 56px;
  height: 56px;
  margin: 0 auto 12px;
  border-radius: 16px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 1.4rem;
  transition: background var(--transition);
}

.device-icon.on {
  background: var(--primary-light);
  color: var(--primary);
}

.device-icon.off {
  background: var(--bg);
  color: var(--text-muted);
}

.device-name {
  font-size: 0.9rem;
  font-weight: 600;
  margin-bottom: 4px;
}

.device-state {
  font-size: 0.78rem;
  color: var(--text-secondary);
  margin-bottom: 12px;
}

.toggle {
  width: 44px;
  height: 24px;
  background: var(--border);
  border-radius: 12px;
  position: relative;
  cursor: pointer;
  margin: 0 auto;
  transition: background var(--transition);
}

.toggle.on {
  background: var(--primary);
}

.toggle .knob {
  width: 20px;
  height: 20px;
  background: #fff;
  border-radius: 50%%;
  position: absolute;
  top: 2px;
  left: 2px;
  transition: transform var(--transition);
  box-shadow: 0 1px 4px rgba(0,0,0,0.15);
}

.toggle.on .knob {
  transform: translateX(20px);
}

#network {
  margin-bottom: 36px;
}

.network-grid {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: 20px;
}

.network-info h3 {
  font-size: 1rem;
  font-weight: 600;
  margin-bottom: 16px;
}

.info-rows {
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.info-row {
  display: flex;
  justify-content: space-between;
  padding: 8px 0;
  border-bottom: 1px solid var(--border);
  font-size: 0.88rem;
}

.info-row:last-child { border-bottom: none; }

.info-row .label { color: var(--text-secondary); }
.info-row .value { font-weight: 600; }

.connected-list {
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.connected-device {
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 10px 14px;
  background: var(--surface-alt);
  border-radius: var(--radius-sm);
}

.connected-device .device-dot {
  width: 10px;
  height: 10px;
  border-radius: 50%%;
  background: var(--accent);
  flex-shrink: 0;
}

.connected-device .device-details {
  flex: 1;
  min-width: 0;
}

.connected-device .device-details .name {
  font-size: 0.88rem;
  font-weight: 600;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.connected-device .device-details .ip {
  font-size: 0.76rem;
  color: var(--text-secondary);
  font-family: 'Consolas', monospace;
}

.connected-device .signal {
  display: flex;
  align-items: flex-end;
  gap: 2px;
  height: 16px;
  flex-shrink: 0;
}

.connected-device .signal span {
  width: 3px;
  background: var(--accent);
  border-radius: 1px;
}

#log {
  margin-bottom: 36px;
}

.log-entries {
  display: flex;
  flex-direction: column;
  gap: 0;
}

.log-entry {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 12px 16px;
  border-bottom: 1px solid var(--border);
  font-size: 0.86rem;
  transition: background var(--transition);
}

.log-entry:last-child {
  border-bottom: none;
}

.log-entry:hover {
  background: var(--surface-alt);
}

.log-time {
  color: var(--text-muted);
  font-family: 'Consolas', monospace;
  font-size: 0.78rem;
  flex-shrink: 0;
  min-width: 64px;
  padding-top: 2px;
}

.log-type {
  width: 8px;
  height: 8px;
  border-radius: 50%%;
  flex-shrink: 0;
  margin-top: 6px;
}

.log-type.info { background: var(--primary); }
.log-type.success { background: var(--accent); }
.log-type.warning { background: #fdcb6e; }
.log-type.error { background: var(--warn); }

.log-message {
  color: var(--text-secondary);
  line-height: 1.4;
}

.footer {
  text-align: center;
  padding: 32px 20px;
  color: var(--text-muted);
  font-size: 0.82rem;
  border-top: 1px solid var(--border);
  margin-top: 20px;
}

.footer strong {
  color: var(--text-secondary);
}

.css-icon {
  display: inline-block;
  position: relative;
}

.icon-water {
  width: 18px;
  height: 18px;
  background: #2196f3;
  border-radius: 50%% 50%% 50%% 0;
  transform: rotate(-45deg);
  display: inline-block;
}

.icon-text { font-size: 1.1rem; }

@media (max-width: 768px) {
  .nav-links {
    display: none;
    position: absolute;
    top: var(--nav-height);
    left: 0;
    right: 0;
    background: var(--surface);
    flex-direction: column;
    padding: 12px;
    border-bottom: 1px solid var(--border);
    box-shadow: var(--shadow);
  }

  .nav-links.open {
    display: flex;
  }

  .menu-toggle {
    display: block;
  }

  .water-grid {
    grid-template-columns: 1fr;
  }

  .network-grid {
    grid-template-columns: 1fr;
  }

  .odo-digit {
    width: 34px;
    height: 50px;
    font-size: 1.6rem;
  }

  .odometer {
    padding: 12px 14px;
  }

  .greeting h1 {
    font-size: 1.3rem;
  }
}

@media (max-width: 480px) {
  .quick-stats {
    grid-template-columns: 1fr;
  }

  .devices-grid {
    grid-template-columns: 1fr 1fr;
  }

  .sensors-grid {
    grid-template-columns: 1fr;
  }
}

.fade-in {
  opacity: 0;
  transform: translateY(16px);
  transition: opacity 0.5s ease, transform 0.5s ease;
}

.fade-in.visible {
  opacity: 1;
  transform: translateY(0);
}

</style>
</head>
<body>

<nav class="navbar">
  <a href="#" class="logo">
    <div class="logo-icon"></div>
    SmartHome Station
  </a>
  <div class="nav-links" id="navLinks">
    <a href="#dashboard" class="active">Dashboard</a>
    <a href="#water">Water Meter</a>
    <a href="#sensors">Sensors</a>
    <a href="#devices">Devices</a>
    <a href="#network">Network</a>
    <a href="#log">Activity Log</a>
  </div>
  <button class="menu-toggle" id="menuToggle" aria-label="Toggle menu">
    <span></span><span></span><span></span>
  </button>
</nav>
<main class="main">
  <section id="dashboard" class="fade-in">
    <div class="dashboard-header">
      <div class="greeting">
        <h1>Welcome to your Smart Home</h1>
        <p id="dateTime">Loading...</p>
      </div>
      <div class="status-badge">
        <span class="status-dot"></span>
        Station Online
      </div>
    </div>
    <div class="quick-stats">
      <div class="stat-card">
        <div class="stat-icon water">
          <span class="icon-water"></span>
        </div>
        <div class="stat-info">
          <h3 id="statWaterToday">0.00 m&sup3;</h3>
          <p>Water used today</p>
        </div>
      </div>
      <div class="stat-card">
        <div class="stat-icon temp">
          <span class="icon-text">&#9832;</span>
        </div>
        <div class="stat-info">
          <h3 id="statTemp">-- &deg;C</h3>
          <p>Indoor temperature</p>
        </div>
      </div>
	  <div class="stat-card">
        <div class="stat-icon temp">
          <span class="icon-text">&#9832;</span>
        </div>
        <div class="stat-info">
          <h3 id="statMoisture">-- &deg;C</h3>
          <p>Moisture</p>
        </div>
      </div>
      <div class="stat-card">
        <div class="stat-icon devices">
          <span class="icon-text">&#9881;</span>
        </div>
        <div class="stat-info">
          <h3 id="statDevices">0 / 0</h3>
          <p>Devices active</p>
        </div>
      </div>
      <div class="stat-card">
        <div class="stat-icon alerts">
          <span class="icon-text">&#9888;</span>
        </div>
        <div class="stat-info">
          <h3 id="statAlerts">0</h3>
          <p>Active alerts</p>
        </div>
      </div>
    </div>
  </section>
  <section id="water" class="fade-in">
    <h2 class="section-title">
      <span class="icon" style="background:#e8f4fd;color:#2196f3;">
        <span class="icon-water"></span>
      </span>
      Water Meter
    </h2>
    <div class="water-grid">
      <div class="card meter-display">
        <div class="meter-visual">
          <div class="odometer-wrapper">
            <div class="odometer-label">Total Consumption</div>
            <div class="odometer">
              <div class="odo-digit" id="d0">0</div>
              <div class="odo-digit" id="d1">0</div>
              <div class="odo-digit" id="d2">0</div>
              <div class="odo-digit" id="d3">0</div>
              <div class="odo-digit" id="d4">0</div>
              <div class="odo-separator">.</div>
              <div class="odo-digit decimal" id="d5">0</div>
              <div class="odo-digit decimal" id="d6">0</div>
              <div class="odo-digit decimal" id="d7">0</div>
              <span class="odo-unit">m&sup3;</span>
            </div>
          </div>
          <div class="flow-indicator">
            <div class="flow-ring">
              <div class="flow-ring-fill" id="flowRing"></div>
            </div>
            <div class="flow-info">
              <div class="flow-rate" id="flowRate">0.00 L/min</div>
              <div class="flow-label">Current flow rate</div>
            </div>
          </div>
        </div>
      </div>
      <div class="card usage-card">
        <h3>Monthly Usage</h3>
        <div class="usage-bars">
          <div class="usage-bar-item">
            <div class="usage-bar-header">
              <span>This month</span>
              <span id="usageThisMonth">0.00 m&sup3;</span>
            </div>
            <div class="usage-bar-track">
              <div class="usage-bar-fill blue" id="barThisMonth" style="width:0%%"></div>
            </div>
          </div>
          <div class="usage-bar-item">
            <div class="usage-bar-header">
              <span>Last month</span>
              <span id="usageLastMonth">0.00 m&sup3;</span>
            </div>
            <div class="usage-bar-track">
              <div class="usage-bar-fill teal" id="barLastMonth" style="width:0%%"></div>
            </div>
          </div>
          <div class="usage-bar-item">
            <div class="usage-bar-header">
              <span>Average (6 mo)</span>
              <span id="usageAvg">0.00 m&sup3;</span>
            </div>
            <div class="usage-bar-track">
              <div class="usage-bar-fill orange" id="barAvg" style="width:0%%"></div>
            </div>
          </div>
          <div class="usage-bar-item">
            <div class="usage-bar-header">
              <span>Daily average</span>
              <span id="usageDaily">0.00 m&sup3;</span>
            </div>
            <div class="usage-bar-track">
              <div class="usage-bar-fill pink" id="barDaily" style="width:0%%"></div>
            </div>
          </div>
        </div>
      </div>
      <div class="card billing-card">
        <h3>Billing Estimate</h3>
        <div class="billing-rows">
          <div class="billing-row">
            <span class="label">Cold water usage</span>
            <span class="value" id="billCold">0.00 m&sup3;</span>
          </div>
          <div class="billing-row">
            <span class="label">Cold water rate</span>
            <span class="value">15.0 KZT / m&sup3;</span>
          </div>
          <div class="billing-row">
            <span class="label">Hot water usage</span>
            <span class="value" id="billHot">0.00 m&sup3;</span>
          </div>
          <div class="billing-row">
            <span class="label">Hot water rate</span>
            <span class="value">32.0 KZT / m&sup3;</span>
          </div>
          <div class="billing-row">
            <span class="label">Service fee</span>
            <span class="value">20.0 KZT</span>
          </div>
          <div class="billing-row total">
            <span class="label">Estimated total</span>
            <span class="value" id="billTotal">0.00 KZT</span>
          </div>
        </div>
      </div>
    </div>
  </section>
  <section id="sensors" class="fade-in">
    <h2 class="section-title">
      <span class="icon" style="background:#fff3e0;color:#ff9800;">
        <span class="icon-text">&#9832;</span>
      </span>
      Sensors
    </h2>
    <div class="sensors-grid" id="sensorsGrid">
      <!-- filled by JS -->
    </div>
  </section>
  <section id="devices" class="fade-in">
    <h2 class="section-title">
      <span class="icon" style="background:#e8f5e9;color:#4caf50;">
        <span class="icon-text">&#9881;</span>
      </span>
      Devices
    </h2>
    <div class="devices-grid" id="devicesGrid">
      <!-- filled by JS -->
    </div>
  </section>
  <section id="network" class="fade-in">
    <h2 class="section-title">
      <span class="icon" style="background:#ede7f6;color:#7c4dff;">
        <span class="icon-text">&#9776;</span>
      </span>
      Network
    </h2>
    <div class="network-grid">
      <div class="card network-info">
        <h3>Station Info</h3>
        <div class="info-rows">
          <div class="info-row">
            <span class="label">SSID</span>
            <span class="value">SmartHome_AP</span>
          </div>
          <div class="info-row">
            <span class="label">IP Address</span>
            <span class="value">192.168.4.1</span>
          </div>
          <div class="info-row">
            <span class="label">Subnet</span>
            <span class="value">255.255.255.0</span>
          </div>
          <div class="info-row">
            <span class="label">Mode</span>
            <span class="value">Access Point</span>
          </div>
          <div class="info-row">
            <span class="label">Channel</span>
            <span class="value">6 (2.4 GHz)</span>
          </div>
          <div class="info-row">
            <span class="label">Uptime</span>
            <span class="value" id="uptime">1d 14h 2m</span>
          </div>
          <div class="info-row">
            <span class="label">Firmware</span>
            <span class="value">v1.0.0</span>
          </div>
        </div>
      </div>
      <div class="card network-info">
        <h3>Connected Devices</h3>
        <div class="connected-list" id="connectedList">
          <!-- filled by JS -->
        </div>
      </div>
    </div>
  </section>
  <section id="log" class="fade-in">
    <h2 class="section-title">
      <span class="icon" style="background:#e3f2fd;color:#1976d2;">
        <span class="icon-text">&#9998;</span>
      </span>
      Activity Log
    </h2>
    <div class="card">
      <div class="log-entries" id="logEntries">
        <!-- filled by JS -->
      </div>
    </div>
  </section>
</main>
<footer class="footer">
  <strong>SmartHome IoT Station</strong> &mdash; University Project &copy; 2026<br>
  IOT-2301 Yuriy Kalinkin
</footer>

<script>
(function() {
  'use strict';
  var menuToggle = document.getElementById('menuToggle');
  var navLinks = document.getElementById('navLinks');

  menuToggle.addEventListener('click', function() {
    navLinks.classList.toggle('open');
  });

  navLinks.querySelectorAll('a').forEach(function(link) {
    link.addEventListener('click', function() {
      navLinks.classList.remove('open');
    });
  });

  var sections = document.querySelectorAll('section[id]');
  var navAnchors = navLinks.querySelectorAll('a');

  function highlightNav() {
    var scrollY = window.scrollY + 100;
    sections.forEach(function(section) {
      var top = section.offsetTop;
      var height = section.offsetHeight;
      var id = section.getAttribute('id');
      if (scrollY >= top && scrollY < top + height) {
        navAnchors.forEach(function(a) {
          a.classList.remove('active');
          if (a.getAttribute('href') === '#' + id) {
            a.classList.add('active');
          }
        });
      }
    });
  }

  window.addEventListener('scroll', highlightNav);

  var fadeEls = document.querySelectorAll('.fade-in');

  function checkFade() {
    fadeEls.forEach(function(el) {
      var rect = el.getBoundingClientRect();
      if (rect.top < window.innerHeight - 60) {
        el.classList.add('visible');
      }
    });
  }

  window.addEventListener('scroll', checkFade);
  checkFade();

  function updateDateTime() {
    var now = new Date();
    var options = { weekday: 'long', year: 'numeric', month: 'long', day: 'numeric' };
    var date = now.toLocaleDateString('en-US', options);
    var time = now.toLocaleTimeString('en-US', { hour: '2-digit', minute: '2-digit' });
    document.getElementById('dateTime').textContent = date + '  \u2022  ' + time;
  }

  updateDateTime();
  setInterval(updateDateTime, 30000);

  var waterTotal = 1247.385;
  var waterTodayBase = 0.187;
  var waterThisMonth = 8.42;
  var waterLastMonth = 11.65;
  var waterAvg6mo = 10.12;
  var waterDailyAvg = 0.34;
  var coldUsage = 5.21;
  var hotUsage = 3.21;
  var flowActive = true;
  var currentFlow = 4.8;

  var COLD_RATE = 15.0;
  var HOT_RATE = 32.0;
  var SERVICE_FEE = 20.0;

  function updateOdometer(val) {
    var str = val.toFixed(3);
    var parts = str.split('.');
    var intPart = parts[0].padStart(5, '0');
    var decPart = parts[1];

    for (var i = 0; i < 5; i++) {
      document.getElementById('d' + i).textContent = intPart[i];
    }
    for (var j = 0; j < 3; j++) {
      document.getElementById('d' + (5 + j)).textContent = decPart[j];
    }
  }

  updateOdometer(waterTotal);

  setInterval(function() {
    if (flowActive) {
      var increment = (currentFlow / 1000 / 60) * 2;
      waterTotal += increment;
      waterTodayBase += increment;
      updateOdometer(waterTotal);
    }
  }, 2000);

  function updateFlow() {
    if (flowActive) {
      var variation = (Math.random() - 0.5) * 0.6;
      var rate = Math.max(0.1, currentFlow + variation);
      document.getElementById('flowRate').textContent = rate.toFixed(2) + ' L/min';
      document.getElementById('flowRing').classList.remove('inactive');
    } else {
      document.getElementById('flowRate').textContent = '0.00 L/min';
      document.getElementById('flowRing').classList.add('inactive');
    }
  }

  setInterval(updateFlow, 3000);

  setInterval(function() {
    if (Math.random() < 0.15) {
      flowActive = !flowActive;
    }
  }, 10000);

  function updateWaterStats() {
    document.getElementById('statWaterToday').innerHTML = waterTodayBase.toFixed(2) + ' m&sup3;';
    document.getElementById('usageThisMonth').innerHTML = waterThisMonth.toFixed(2) + ' m&sup3;';
    document.getElementById('usageLastMonth').innerHTML = waterLastMonth.toFixed(2) + ' m&sup3;';
    document.getElementById('usageAvg').innerHTML = waterAvg6mo.toFixed(2) + ' m&sup3;';
    document.getElementById('usageDaily').innerHTML = waterDailyAvg.toFixed(2) + ' m&sup3;';

    var maxRef = 15;
    document.getElementById('barThisMonth').style.width = (waterThisMonth / maxRef * 100) + '%%';
    document.getElementById('barLastMonth').style.width = (waterLastMonth / maxRef * 100) + '%%';
    document.getElementById('barAvg').style.width = (waterAvg6mo / maxRef * 100) + '%%';
    document.getElementById('barDaily').style.width = (waterDailyAvg / 0.6 * 100) + '%%';

    document.getElementById('billCold').innerHTML = coldUsage.toFixed(2) + ' m&sup3;';
    document.getElementById('billHot').innerHTML = hotUsage.toFixed(2) + ' m&sup3;';
    var total = (coldUsage * COLD_RATE) + (hotUsage * HOT_RATE) + SERVICE_FEE;
    document.getElementById('billTotal').textContent = total.toFixed(2) + ' KZT';
  }

  setTimeout(updateWaterStats, 400);

  var sensorsData = [
    { name: 'Living Room Temp', value: 0.0, unit: '\u00B0C', change: '+0.3', dir: 'up', bars: [60,70,65,80,75,90,85,70,65,78] },
    { name: 'Humidity', value: 0.0, unit: '%%', change: '-2', dir: 'down', bars: [80,75,70,60,65,55,50,48,52,45] },
    { name: 'Water Pressure', value: 3.2, unit: 'bar', change: '0.0', dir: 'neutral', bars: [50,52,48,50,51,49,50,50,52,51] },
    { name: 'Water Temp (hot)', value: 54.1, unit: '\u00B0C', change: '+1.2', dir: 'up', bars: [45,50,55,60,58,65,62,70,68,72] },
    { name: 'Light Level', value: 340, unit: 'lux', change: '-80', dir: 'down', bars: [90,85,80,70,60,55,50,40,35,30] },
    { name: 'Air Quality', value: 42, unit: 'AQI', change: '+5', dir: 'up', bars: [30,35,32,38,40,42,38,45,40,42] }
  ];

  var sensorsGrid = document.getElementById('sensorsGrid');

  sensorsData.forEach(function(s) {
    var barClass = s.dir === 'up' ? 'warn' : (s.dir === 'down' ? 'accent' : '');
    var barsHtml = s.bars.map(function(h) {
      return '<div class="bar ' + barClass + '" style="height:' + h + '%%"></div>';
    }).join('');

    var html = '<div class="card sensor-card">' +
      '<div class="sensor-header">' +
        '<span class="sensor-name">' + s.name + '</span>' +
        '<span class="sensor-status"></span>' +
      '</div>' +
      '<span class="sensor-value">' + s.value + '<span class="sensor-unit"> ' + s.unit + '</span></span>' +
      '<div class="sensor-change ' + s.dir + '">' +
        (s.dir === 'up' ? '&#9650; ' : (s.dir === 'down' ? '&#9660; ' : '&#9644; ')) +
        s.change + ' from yesterday' +
      '</div>' +
      '<div class="sparkline">' + barsHtml + '</div>' +
    '</div>';
    sensorsGrid.insertAdjacentHTML('beforeend', html);
  });

  document.getElementById('statTemp').innerHTML = sensorsData[0].value + ' &deg;C';

  var devicesData = [
    { name: 'Main Valve', icon: '\u2B55', on: true },
    { name: 'Boiler', icon: '\u2668', on: true },
    { name: 'Pump', icon: '\u26A1', on: false },
    { name: 'Ventilation', icon: '\u2741', on: true },
    { name: 'Floor Heating', icon: '\u2600', on: false },
    { name: 'Garden Sprinkler', icon: '\u2744', on: false }
  ];

  var devicesGrid = document.getElementById('devicesGrid');

  devicesData.forEach(function(d, idx) {
    var state = d.on ? 'on' : 'off';
    var stateText = d.on ? 'Active' : 'Inactive';
    var html = '<div class="card device-card">' +
      '<div class="device-icon ' + state + '">' + d.icon + '</div>' +
      '<div class="device-name">' + d.name + '</div>' +
      '<div class="device-state" id="devState' + idx + '">' + stateText + '</div>' +
      '<div class="toggle ' + state + '" id="devToggle' + idx + '" data-idx="' + idx + '">' +
        '<div class="knob"></div>' +
      '</div>' +
    '</div>';
    devicesGrid.insertAdjacentHTML('beforeend', html);
  });

  devicesData.forEach(function(d, idx) {
    var toggle = document.getElementById('devToggle' + idx);
    toggle.addEventListener('click', function() {
      devicesData[idx].on = !devicesData[idx].on;
      var isOn = devicesData[idx].on;
      toggle.classList.toggle('on', isOn);
      var icon = toggle.closest('.device-card').querySelector('.device-icon');
      icon.classList.toggle('on', isOn);
      icon.classList.toggle('off', !isOn);
      document.getElementById('devState' + idx).textContent = isOn ? 'Active' : 'Inactive';
      updateDeviceStat();
      addLogEntry(isOn ? 'success' : 'info', d.name + (isOn ? ' turned ON' : ' turned OFF'));
    });
  });

  function updateDeviceStat() {
    var active = devicesData.filter(function(d) { return d.on; }).length;
    document.getElementById('statDevices').textContent = active + ' / ' + devicesData.length;
  }

  updateDeviceStat();

  var connectedDevices = [
    { name: 'User Phone', ip: '192.168.4.2', signal: [4,8,12,16] },
    { name: 'Laptop', ip: '192.168.4.3', signal: [4,8,12,16] },
    { name: 'Tablet', ip: '192.168.4.5', signal: [4,8,10] }
  ];

  var connList = document.getElementById('connectedList');
  connectedDevices.forEach(function(cd) {
    var signalBars = cd.signal.map(function(h) {
      return '<span style="height:' + h + 'px"></span>';
    }).join('');
    var html = '<div class="connected-device">' +
      '<span class="device-dot"></span>' +
      '<div class="device-details">' +
        '<div class="name">' + cd.name + '</div>' +
        '<div class="ip">' + cd.ip + '</div>' +
      '</div>' +
      '<div class="signal">' + signalBars + '</div>' +
    '</div>';
    connList.insertAdjacentHTML('beforeend', html);
  });

  var uptimeSeconds = 86400 + 7200 + 2340;

  function updateUptime() {
    var d = Math.floor(uptimeSeconds / 86400);
    var h = Math.floor((uptimeSeconds %% 86400) / 3600);
    var m = Math.floor((uptimeSeconds %% 3600) / 60);
    document.getElementById('uptime').textContent = d + 'd ' + h + 'h ' + m + 'm';
    uptimeSeconds += 60;
  }

  updateUptime();
  setInterval(updateUptime, 60000);

  var logData = [
    { time: '14:32', type: 'success', msg: 'Water meter reading synced successfully' },
    { time: '14:28', type: 'info', msg: 'Boiler temperature stabilised at 54\u00B0C' },
    { time: '14:15', type: 'warning', msg: 'Water pressure slightly below normal (3.0 bar)' },
    { time: '13:50', type: 'info', msg: 'New device connected: User Phone (192.168.4.2)' },
    { time: '13:42', type: 'success', msg: 'Daily water usage report generated' },
    { time: '13:10', type: 'info', msg: 'Ventilation fan speed adjusted to 60%%' },
    { time: '12:55', type: 'error', msg: 'Garden sprinkler sensor unresponsive (timeout)' },
    { time: '12:30', type: 'info', msg: 'System health check passed \u2014 all wired connections OK' },
    { time: '12:00', type: 'success', msg: 'Hourly data snapshot saved to local storage' },
    { time: '11:45', type: 'info', msg: 'Floor heating schedule updated by user' }
  ];

  var logContainer = document.getElementById('logEntries');

  function renderLog() {
    logContainer.innerHTML = '';
    logData.forEach(function(entry) {
      var html = '<div class="log-entry">' +
        '<span class="log-time">' + entry.time + '</span>' +
        '<span class="log-type ' + entry.type + '"></span>' +
        '<span class="log-message">' + entry.msg + '</span>' +
      '</div>';
      logContainer.insertAdjacentHTML('beforeend', html);
    });
  }

  renderLog();

  function addLogEntry(type, msg) {
    var now = new Date();
    var timeStr = now.getHours().toString().padStart(2, '0') + ':' +
                  now.getMinutes().toString().padStart(2, '0');
    logData.unshift({ time: timeStr, type: type, msg: msg });
    if (logData.length > 20) logData.pop();
    renderLog();
  }

  document.getElementById('statAlerts').textContent = '1';
  
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  
  window.addEventListener('load', onLoad);
  
  function onLoad(event) {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
  }
  
  function onOpen(event) {
    console.log('Connection opened');
  }
  
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(onLoad, 2000);
  }
  
  function onMessage(event){
    if(event.data[0] == '1'){
      document.getElementById('statMoisture').innerHTML = event.data.substring(1, 5) + ' %%';
      document.getElementById('statTemp').innerHTML = event.data.substring(5, 9) + ' &deg;C';
    }else if(event.data[0] == '2'){
    }
  }
  
  setInterval(function() {
    /*
	sensorsData.forEach(function(s, idx) {
      var card = sensorsGrid.children[idx];
      if (!card) return;
      var variation = (Math.random() - 0.5) * 0.4;
      s.value = parseFloat((s.value + variation).toFixed(1));
      card.querySelector('.sensor-value').innerHTML = s.value +
        '<span class="sensor-unit"> ' + s.unit + '</span>';
    });
	*/
    //document.getElementById('statTemp').innerHTML = sensorsData[0].value + ' &deg;C';
  }, 5000);

})();
</script>

</body>
</html>

)rawliteral";

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT){
    /*
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0){
      ledState = !ledState;
    }
    */
  }
}

String processor(const String &var){
  return "!error!";
}

void onEventFunc(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  switch (type) {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%%u connected from %%s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void start_webserver_station(){
  Serial.print("creating wifi network");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("ip: ");
  Serial.println(WiFi.softAPIP());
  ws.onEvent(onEventFunc);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {request->send_P(200, "text/html", index_html, processor);});
  server.begin();
}

void loop_webserver_station(){
  ws.cleanupClients();
}

void webserver_station_textall(const char *line){
  ws.textAll(line);
}
