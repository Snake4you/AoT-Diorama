#pragma once

#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Titan Diorama Control Center</title>
    <style>
        :root {
            --bg-color: #0b0c10;
            --card-bg: rgba(22, 26, 37, 0.7);
            --border-color: rgba(255, 255, 255, 0.08);
            --text-color: #c5c6c7;
            --text-white: #ffffff;
            --primary: #66fcf1;
            --primary-dark: #45a29e;
            --secondary: #ffab00;
            --success: #00e676;
            --danger: #ff1744;
            --warning: #ffeb3b;
            --font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-color);
            font-family: var(--font-family);
            padding: 20px;
            min-height: 100vh;
            display: flex;
            flex-direction: column;
            background-image: radial-gradient(circle at top right, rgba(102, 252, 241, 0.05), transparent 60%),
                              radial-gradient(circle at bottom left, rgba(255, 171, 0, 0.03), transparent 60%);
        }

        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 15px 25px;
            background: var(--card-bg);
            backdrop-filter: blur(10px);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            margin-bottom: 20px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.3);
        }

        .header-title h1 {
            color: var(--text-white);
            font-size: 24px;
            font-weight: 600;
            letter-spacing: 0.5px;
            text-shadow: 0 0 10px rgba(102, 252, 241, 0.2);
        }

        .header-status {
            display: flex;
            align-items: center;
            gap: 15px;
        }

        .badge {
            padding: 6px 12px;
            border-radius: 20px;
            font-size: 13px;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            display: inline-flex;
            align-items: center;
            gap: 6px;
        }

        .badge-ws {
            background: rgba(255, 23, 68, 0.15);
            color: var(--danger);
            border: 1px solid var(--danger);
        }

        .badge-ws.connected {
            background: rgba(0, 230, 118, 0.15);
            color: var(--success);
            border: 1px solid var(--success);
        }

        .badge-state {
            background: rgba(102, 252, 241, 0.15);
            color: var(--primary);
            border: 1px solid var(--primary);
            box-shadow: 0 0 8px rgba(102, 252, 241, 0.2);
        }

        .dashboard-grid {
            display: grid;
            grid-template-columns: 1.2fr 1fr;
            gap: 20px;
            flex-grow: 1;
        }

        @media (max-width: 1024px) {
            .dashboard-grid {
                grid-template-columns: 1fr;
            }
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(10px);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 20px;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.2);
            display: flex;
            flex-direction: column;
            gap: 20px;
        }

        .card-title {
            color: var(--text-white);
            font-size: 18px;
            font-weight: 600;
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 10px;
            margin-bottom: 5px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        /* Action Buttons */
        .controls-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }

        .btn {
            padding: 12px 20px;
            border-radius: 8px;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            border: none;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 8px;
            user-select: none;
        }

        .btn-start {
            background: var(--success);
            color: #000;
        }

        .btn-start:hover {
            box-shadow: 0 0 15px var(--success);
            transform: translateY(-2px);
        }

        .btn-stop {
            background: #455a64;
            color: var(--text-white);
        }

        .btn-stop:hover {
            background: #546e7a;
            box-shadow: 0 0 15px rgba(84, 110, 122, 0.4);
            transform: translateY(-2px);
        }

        .btn-estop {
            grid-column: span 2;
            background: var(--danger);
            color: var(--text-white);
            padding: 16px;
            font-size: 18px;
            box-shadow: 0 0 10px rgba(255, 23, 68, 0.3);
            animation: pulse-red 2s infinite;
        }

        .btn-estop:hover {
            box-shadow: 0 0 25px var(--danger);
            transform: translateY(-2px);
            animation: none;
        }

        @keyframes pulse-red {
            0% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0.4); }
            70% { box-shadow: 0 0 0 12px rgba(255, 23, 68, 0); }
            100% { box-shadow: 0 0 0 0 rgba(255, 23, 68, 0); }
        }

        /* Telemetry Elements */
        .telemetry-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }

        .telemetry-item {
            background: rgba(0, 0, 0, 0.25);
            border-radius: 8px;
            padding: 15px;
            border: 1px solid var(--border-color);
            position: relative;
            overflow: hidden;
        }

        .telemetry-item::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 4px;
            height: 100%;
        }

        .telemetry-item.m1::before { background: var(--primary); }
        .telemetry-item.m2::before { background: var(--secondary); }

        .telemetry-label {
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            color: #888;
        }

        .telemetry-value {
            font-size: 24px;
            font-weight: 700;
            color: var(--text-white);
            margin: 5px 0;
            font-family: monospace;
        }

        .progress-bar-container {
            width: 100%;
            height: 6px;
            background: rgba(255, 255, 255, 0.05);
            border-radius: 3px;
            overflow: hidden;
            margin-top: 8px;
        }

        .progress-bar {
            height: 100%;
            width: 0%;
            transition: width 0.1s linear;
        }

        .m1 .progress-bar { background: var(--primary); }
        .m2 .progress-bar { background: var(--secondary); }

        /* Oscilloscope Charts */
        .chart-container {
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            border: 1px solid var(--border-color);
            padding: 10px;
            height: 150px;
            position: relative;
        }

        canvas {
            width: 100%;
            height: 100%;
            display: block;
        }

        /* Config Form */
        .tabs {
            display: flex;
            border-bottom: 1px solid var(--border-color);
            margin-bottom: 15px;
        }

        .tab-btn {
            background: none;
            border: none;
            padding: 10px 15px;
            color: var(--text-color);
            cursor: pointer;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s ease;
            position: relative;
        }

        .tab-btn:hover {
            color: var(--text-white);
        }

        .tab-btn.active {
            color: var(--primary);
        }

        .tab-btn.active::after {
            content: '';
            position: absolute;
            bottom: -1px;
            left: 0;
            width: 100%;
            height: 2px;
            background: var(--primary);
        }

        .tab-content {
            display: none;
            flex-direction: column;
            gap: 15px;
        }

        .tab-content.active {
            display: flex;
        }

        .form-group {
            display: flex;
            flex-direction: column;
            gap: 6px;
        }

        .form-group label {
            font-size: 13px;
            color: var(--text-white);
            font-weight: 500;
        }

        .input-row {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
        }

        input[type="text"], input[type="password"], input[type="number"], select {
            background: rgba(0, 0, 0, 0.3);
            border: 1px solid var(--border-color);
            border-radius: 6px;
            padding: 10px 12px;
            color: var(--text-white);
            font-size: 14px;
            transition: all 0.3s ease;
        }

        input:focus, select:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 5px rgba(102, 252, 241, 0.3);
        }

        .checkbox-group {
            display: flex;
            align-items: center;
            gap: 10px;
            cursor: pointer;
        }

        .checkbox-group input {
            cursor: pointer;
            width: 16px;
            height: 16px;
            accent-color: var(--primary);
        }

        .btn-save {
            background: var(--primary);
            color: #000;
            margin-top: 15px;
        }

        .btn-save:hover {
            box-shadow: 0 0 15px var(--primary);
            transform: translateY(-2px);
        }

        /* State Indicator */
        .state-visualizer {
            display: flex;
            align-items: center;
            gap: 12px;
            background: rgba(0, 0, 0, 0.2);
            padding: 12px 15px;
            border-radius: 8px;
            border: 1px solid var(--border-color);
        }

        .state-dot {
            width: 14px;
            height: 14px;
            border-radius: 50%;
            background: var(--text-color);
        }

        .state-dot.POWER_ON { background: var(--primary); animation: breathe 1.5s infinite alternate; }
        .state-dot.HOMING { background: var(--secondary); animation: blink 0.5s infinite; }
        .state-dot.IDLE { background: #3f51b5; animation: breathe 2s infinite alternate; }
        .state-dot.SHOW_ACTIVE { background: var(--success); animation: breathe 1s infinite alternate; }
        .state-dot.ESTOP { background: var(--danger); animation: blink 0.2s infinite; }

        @keyframes breathe {
            0% { transform: scale(0.9); opacity: 0.5; box-shadow: 0 0 0 rgba(255,255,255,0); }
            100% { transform: scale(1.1); opacity: 1; box-shadow: 0 0 10px currentColor; }
        }

        @keyframes blink {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.2; }
        }

        .limit-status {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 13px;
        }
        
        .limit-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #555;
        }
        
        .limit-dot.active {
            background: var(--danger);
            box-shadow: 0 0 8px var(--danger);
        }

        .btn-reset-estop {
            background: #ffc107;
            color: #000;
        }
        .btn-reset-estop:hover {
            box-shadow: 0 0 15px #ffc107;
        }

        /* Jog Button custom styling */
        .btn-jog {
            padding: 8px 12px;
            font-size: 13px;
            border-radius: 6px;
            font-weight: 700;
            box-shadow: 0 2px 4px rgba(0,0,0,0.3);
            text-transform: uppercase;
        }

        .btn-jog:active {
            transform: scale(0.95);
            box-shadow: inset 0 2px 4px rgba(0,0,0,0.5);
        }

        .btn-led-toggle, .btn-sw-toggle {
            transition: all 0.2s ease;
        }
    </style>
</head>
<body>

    <header>
        <div class="header-title">
            <h1>Titan Diorama</h1>
        </div>
        <div class="header-status">
            <div id="limit-indicator" class="limit-status">
                <div id="limit-dot" class="limit-dot"></div>
                <span>Limit Switch</span>
            </div>
            <div id="ws-badge" class="badge badge-ws">Disconnected</div>
            <div id="state-badge" class="badge badge-state">UNKNOWN</div>
        </div>
    </header>

    <div class="dashboard-grid">
        
        <!-- TELEMETRY, CONTROLS & JOG MODE -->
        <div style="display:flex; flex-direction:column; gap:20px;">
            
            <!-- Main Telemetry Card -->
            <div class="card">
                <div class="card-title">
                    Control & Live Telemetry
                    <span id="show-time-left" style="font-size:14px; font-weight:normal; color:#888;"></span>
                </div>

                <!-- State visualization -->
                <div class="state-visualizer">
                    <div id="state-dot" class="state-dot"></div>
                    <div style="display:flex; flex-direction:column;">
                        <span style="font-size:11px; text-transform:uppercase; color:#888;">System State</span>
                        <span id="state-text" style="color:var(--text-white); font-weight:600; font-size:15px;">POWER_ON</span>
                    </div>
                </div>

                <!-- Action buttons -->
                <div class="controls-row">
                    <button id="btn-start" class="btn btn-start" onclick="sendCommand('start')">Start Show</button>
                    <button id="btn-stop" class="btn btn-stop" onclick="sendCommand('stop')">Stop Show</button>
                    <button id="btn-reset-estop" class="btn btn-reset-estop" onclick="sendCommand('reset_estop')" style="display:none; grid-column: span 2;">Reset Emergency Stop</button>
                    <button id="btn-estop" class="btn btn-estop" onclick="sendCommand('estop')">Emergency Stop</button>
                </div>

                <!-- Readouts -->
                <div class="telemetry-row">
                    <div class="telemetry-item m1">
                        <span class="telemetry-label">Motor 1 Current</span>
                        <div id="m1-current-text" class="telemetry-value">0.0 mA</div>
                        <div class="progress-bar-container">
                            <div id="m1-bar" class="progress-bar"></div>
                        </div>
                    </div>
                    <div class="telemetry-item m2">
                        <span class="telemetry-label">Motor 2 Current</span>
                        <div id="m2-current-text" class="telemetry-value">0.0 mA</div>
                        <div class="progress-bar-container">
                            <div id="m2-bar" class="progress-bar"></div>
                        </div>
                    </div>
                </div>

                <!-- Charts -->
                <div>
                    <span class="telemetry-label" style="display:block; margin-bottom:8px;">Motor 1 Current Waveform (Limit: <span id="m1-limit-val">0</span> mA)</span>
                    <div class="chart-container">
                        <canvas id="m1-chart"></canvas>
                    </div>
                </div>
                <div>
                    <span class="telemetry-label" style="display:block; margin-bottom:8px;">Motor 2 Current Waveform (Limit: <span id="m2-limit-val">0</span> mA)</span>
                    <div class="chart-container">
                        <canvas id="m2-chart"></canvas>
                    </div>
                </div>
            </div>

            <!-- Jog Mode (Tippbetrieb) Card -->
            <div class="card" id="jog-card">
                <div class="card-title">
                    Tippbetrieb (Manual Controls)
                    <span style="font-size:12px; font-weight:normal; color:#888;">Only active in IDLE state</span>
                </div>
                
                <div style="display:flex; flex-direction:column; gap:15px;">
                    <!-- Motor 1 (Horizontal) Jog -->
                    <div style="border: 1px solid var(--border-color); padding: 12px; border-radius: 8px; background: rgba(0,0,0,0.15);">
                        <div style="font-weight:600; color:var(--text-white); margin-bottom:10px; display:flex; justify-content:space-between; align-items:center;">
                            <span>Motor 1 (Horizontal)</span>
                            <span id="m1-jog-status" style="font-size:11px; color:#888; text-transform:uppercase;">Idle</span>
                        </div>
                        <div style="display:flex; gap:12px; align-items:center;">
                            <button class="btn btn-jog" id="btn-jog-m1-fwd" 
                                    onmousedown="startJog(1, 1)" onmouseup="stopJog(1)" onmouseleave="stopJog(1)"
                                    ontouchstart="startJog(1, 1)" ontouchend="stopJog(1)" style="flex:1.2; background:var(--primary); color:#000;">FWD</button>
                            <button class="btn btn-jog" id="btn-jog-m1-rev" 
                                    onmousedown="startJog(1, -1)" onmouseup="stopJog(1)" onmouseleave="stopJog(1)"
                                    ontouchstart="startJog(1, -1)" ontouchend="stopJog(1)" style="flex:1.2; background:var(--primary); color:#000;">REV</button>
                            <div style="display:flex; flex-direction:column; gap:2px; width:100px;">
                                <label style="font-size:10px; color:#888; text-transform:uppercase;">Jog Speed</label>
                                <input type="number" id="m1_jog_speed_val" min="0" max="255" value="150" style="padding:6px; font-size:12px; text-align:center;">
                            </div>
                        </div>
                    </div>

                    <!-- Motor 2 (Linear Actuator) Jog -->
                    <div style="border: 1px solid var(--border-color); padding: 12px; border-radius: 8px; background: rgba(0,0,0,0.15);">
                        <div style="font-weight:600; color:var(--text-white); margin-bottom:10px; display:flex; justify-content:space-between; align-items:center;">
                            <span>Motor 2 (Linear Actuator)</span>
                            <span id="m2-jog-status" style="font-size:11px; color:#888; text-transform:uppercase;">Idle</span>
                        </div>
                        <div style="display:flex; gap:12px; align-items:center;">
                            <button class="btn btn-jog" id="btn-jog-m2-up" 
                                    onmousedown="startJog(2, 1)" onmouseup="stopJog(2)" onmouseleave="stopJog(2)"
                                    ontouchstart="startJog(2, 1)" ontouchend="stopJog(2)" style="flex:1.2; background:var(--secondary); color:#000;">UP</button>
                            <button class="btn btn-jog" id="btn-jog-m2-down" 
                                    onmousedown="startJog(2, -1)" onmouseup="stopJog(2)" onmouseleave="stopJog(2)"
                                    ontouchstart="startJog(2, -1)" ontouchend="stopJog(2)" style="flex:1.2; background:var(--secondary); color:#000;">DOWN</button>
                            <div style="display:flex; flex-direction:column; gap:2px; width:100px;">
                                <label style="font-size:10px; color:#888; text-transform:uppercase;">Jog Speed</label>
                                <input type="number" id="m2_jog_speed_val" min="0" max="255" value="120" style="padding:6px; font-size:12px; text-align:center;">
                            </div>
                        </div>
                    </div>

                    <!-- LED Segments Overrides -->
                    <div style="border: 1px solid var(--border-color); padding: 12px; border-radius: 8px; background: rgba(0,0,0,0.15);">
                        <div style="font-weight:600; color:var(--text-white); margin-bottom:10px;">LED Bereiche (Tippbetrieb Toggles)</div>
                        <div style="display:grid; grid-template-columns: repeat(3, 1fr); gap:8px;">
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-0" onclick="toggleManualLED(0)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Lower Door</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-1" onclick="toggleManualLED(1)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Fire Right</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-2" onclick="toggleManualLED(2)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Lower Books</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-3" onclick="toggleManualLED(3)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Fire Left</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-4" onclick="toggleManualLED(4)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Upper Books</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-5" onclick="toggleManualLED(5)" style="font-size:10px; padding:6px; background:#222; color:#fff;">Upper Frame</button>
                            <button class="btn btn-jog btn-led-toggle" id="btn-led-6" onclick="toggleManualLED(6)" style="font-size:10px; padding:6px; background:#222; color:#fff; grid-column: span 3;">Titan Eye</button>
                        </div>
                    </div>

                    <!-- Relay Switches Overrides -->
                    <div style="border: 1px solid var(--border-color); padding: 12px; border-radius: 8px; background: rgba(0,0,0,0.15);">
                        <div style="font-weight:600; color:var(--text-white); margin-bottom:10px;">Schaltausgänge (Tippbetrieb Toggles)</div>
                        <div style="display:flex; gap:12px;">
                            <button class="btn btn-jog btn-sw-toggle" id="btn-sw-1" onclick="toggleManualSwitch(1)" style="flex:1; background:#222; color:#fff; font-size:11px;">Switch Out 1</button>
                            <button class="btn btn-jog btn-sw-toggle" id="btn-sw-2" onclick="toggleManualSwitch(2)" style="flex:1; background:#222; color:#fff; font-size:11px;">Switch Out 2</button>
                        </div>
                    </div>
                </div>
            </div>

        </div>

        <!-- CONFIGURATION PANEL -->
        <div class="card">
            <div class="card-title">Configuration Parameters</div>
            
            <div class="tabs">
                <button class="tab-btn active" onclick="switchTab(event, 'tab-m1')">Motor 1</button>
                <button class="tab-btn" onclick="switchTab(event, 'tab-m2')">Motor 2</button>
                <button class="tab-btn" onclick="switchTab(event, 'tab-sw')">Switches & LEDs</button>
                <button class="tab-btn" onclick="switchTab(event, 'tab-wifi')">Network & System</button>
            </div>

            <form id="config-form" onsubmit="saveConfig(event)">
                <!-- TAB 1: Motor 1 Settings -->
                <div id="tab-m1" class="tab-content active">
                    <div class="form-group">
                        <label for="m1_speed">Motor 1 Speed (PWM 0-255)</label>
                        <input type="number" id="m1_speed" min="0" max="255" required>
                    </div>
                    <div class="form-group">
                        <label for="m1_jog_speed">Manual Jog Speed (Tippbetrieb PWM 0-255)</label>
                        <input type="number" id="m1_jog_speed" min="0" max="255" required>
                    </div>
                    <div class="form-group">
                        <label for="m1_current_limit">Current Limit (mA)</label>
                        <input type="number" id="m1_current_limit" min="0" max="5000" required>
                    </div>
                    <div class="form-group">
                        <label for="m1_blanking_time">Startup Blanking Time (ms)</label>
                        <input type="number" id="m1_blanking_time" min="0" max="2000" required>
                    </div>
                </div>

                <!-- TAB 2: Motor 2 Settings -->
                <div id="tab-m2" class="tab-content">
                    <div class="form-group">
                        <label for="m2_speed">Motor 2 Speed (PWM 0-255)</label>
                        <input type="number" id="m2_speed" min="0" max="255" required>
                    </div>
                    <div class="form-group">
                        <label for="m2_jog_speed">Manual Jog Speed (Tippbetrieb PWM 0-255)</label>
                        <input type="number" id="m2_jog_speed" min="0" max="255" required>
                    </div>
                    <div class="form-group">
                        <label for="m2_current_limit">Motor 2 Current Limit (mA)</label>
                        <input type="number" id="m2_current_limit" min="0" max="5000" required>
                    </div>
                    <div class="form-group">
                        <label for="m2_blanking_time">Startup Blanking Time (ms)</label>
                        <input type="number" id="m2_blanking_time" min="0" max="2000" required>
                    </div>
                    <div class="form-group">
                        <label for="m2_ramp_time">Acceleration/Deceleration Ramp (seconds)</label>
                        <input type="number" id="m2_ramp_time" step="0.1" min="0.0" max="10.0" required>
                    </div>
                    <div class="input-row">
                        <div class="form-group">
                            <label for="m2_run_time_up">Run Time Up (sec)</label>
                            <input type="number" id="m2_run_time_up" step="0.1" min="0.5" max="60.0" required>
                        </div>
                        <div class="form-group">
                            <label for="m2_run_time_down">Run Time Down (sec)</label>
                            <input type="number" id="m2_run_time_down" step="0.1" min="0.5" max="60.0" required>
                        </div>
                    </div>
                </div>

                <!-- TAB 3: Switches & LEDs -->
                <div id="tab-sw" class="tab-content">
                    <div style="border: 1px solid var(--border-color); padding:12px; border-radius:8px; display:flex; flex-direction:column; gap:12px;">
                        <label style="font-weight:600; color:var(--text-white);">Digital Switch Out 1 (Relays)</label>
                        <div class="checkbox-group">
                            <input type="checkbox" id="sw1_active">
                            <label for="sw1_active">Switch 1 Enabled during Show</label>
                        </div>
                        <div class="form-group">
                            <label for="sw1_polarity">Switch 1 Default Polarity</label>
                            <select id="sw1_polarity">
                                <option value="true">Normal (OUT1 HIGH / OUT2 LOW)</option>
                                <option value="false">Reversed (OUT1 LOW / OUT2 HIGH)</option>
                            </select>
                        </div>
                    </div>

                    <div style="border: 1px solid var(--border-color); padding:12px; border-radius:8px; display:flex; flex-direction:column; gap:12px;">
                        <label style="font-weight:600; color:var(--text-white);">Digital Switch Out 2 (Relays)</label>
                        <div class="checkbox-group">
                            <input type="checkbox" id="sw2_active">
                            <label for="sw2_active">Switch 2 Enabled during Show</label>
                        </div>
                        <div class="form-group">
                            <label for="sw2_polarity">Switch 2 Default Polarity</label>
                            <select id="sw2_polarity">
                                <option value="true">Normal (OUT1 HIGH / OUT2 LOW)</option>
                                <option value="false">Reversed (OUT1 LOW / OUT2 HIGH)</option>
                            </select>
                        </div>
                    </div>

                    <div style="border: 1px solid var(--border-color); padding:12px; border-radius:8px; display:flex; flex-direction:column; gap:12px; margin-top: 10px;">
                        <label style="font-weight:600; color:var(--text-white);">Segment Helligkeiten (0-255)</label>
                        
                        <div class="form-group">
                            <label for="led_br_0">Lower Door</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_0" min="0" max="255" style="flex:1;" oninput="updateBrLabel(0)">
                                <span id="led_br_val_0" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_1">Fire Right</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_1" min="0" max="255" style="flex:1;" oninput="updateBrLabel(1)">
                                <span id="led_br_val_1" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_2">Lower Books</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_2" min="0" max="255" style="flex:1;" oninput="updateBrLabel(2)">
                                <span id="led_br_val_2" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_3">Fire Left</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_3" min="0" max="255" style="flex:1;" oninput="updateBrLabel(3)">
                                <span id="led_br_val_3" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_4">Upper Books</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_4" min="0" max="255" style="flex:1;" oninput="updateBrLabel(4)">
                                <span id="led_br_val_4" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_5">Upper Frame</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_5" min="0" max="255" style="flex:1;" oninput="updateBrLabel(5)">
                                <span id="led_br_val_5" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                        <div class="form-group">
                            <label for="led_br_6">Titan Eye</label>
                            <div style="display:flex; gap:10px; align-items:center;">
                                <input type="range" id="led_br_6" min="0" max="255" style="flex:1;" oninput="updateBrLabel(6)">
                                <span id="led_br_val_6" style="width:30px; font-size:12px; text-align:right;">128</span>
                            </div>
                        </div>
                    </div>
                </div>

                <!-- TAB 4: Network & System -->
                <div id="tab-wifi" class="tab-content">
                    <div class="form-group">
                        <label for="wifi_ap_mode">WiFi Mode</label>
                        <select id="wifi_ap_mode" onchange="toggleWiFiFields()">
                            <option value="true">Access Point (ESP32 Network)</option>
                            <option value="false">Station Mode (Connect to Home WiFi)</option>
                        </select>
                    </div>
                    <div class="form-group">
                        <label for="wifi_ssid">WiFi SSID</label>
                        <input type="text" id="wifi_ssid" maxlength="31" required>
                    </div>
                    <div class="form-group" id="wifi-pass-group">
                        <label for="wifi_pass">WiFi Password</label>
                        <input type="password" id="wifi_pass" maxlength="63">
                    </div>
                    <div class="form-group">
                        <label for="show_run_time">Show Duration (seconds, 0 = Continuous)</label>
                        <input type="number" id="show_run_time" min="0" max="3600" required>
                    </div>
                </div>

                <button type="submit" class="btn btn-save">Save Settings</button>
            </form>
        </div>

    </div>

    <script>
        let ws;
        let m1History = Array(100).fill(0);
        let m2History = Array(100).fill(0);
        let configData = {};
        let jogInterval = null;
        let manualLEDsState = Array(7).fill(false);
        let manualSwitchesState = Array(2).fill(false);

        // Canvas oscillators setup
        const canvasM1 = document.getElementById('m1-chart');
        const canvasM2 = document.getElementById('m2-chart');
        const ctxM1 = canvasM1.getContext('2d');
        const ctxM2 = canvasM2.getContext('2d');

        function resizeCanvas() {
            canvasM1.width = canvasM1.parentElement.clientWidth;
            canvasM1.height = canvasM1.parentElement.clientHeight;
            canvasM2.width = canvasM2.parentElement.clientWidth;
            canvasM2.height = canvasM2.parentElement.clientHeight;
            drawChart(ctxM1, m1History, '#66fcf1', 500); // 500mA max scale
            drawChart(ctxM2, m2History, '#ffab00', 500); // 500mA max scale
        }

        window.addEventListener('resize', resizeCanvas);

        // Connect to WebSocket
        function connect() {
            const host = window.location.hostname || '192.168.4.1';
            const wsBadge = document.getElementById('ws-badge');
            
            wsBadge.innerText = 'Connecting...';
            wsBadge.className = 'badge badge-ws';

            ws = new WebSocket(`ws://${host}/ws`);

            ws.onopen = () => {
                wsBadge.innerText = 'Connected';
                wsBadge.className = 'badge badge-ws connected';
                resizeCanvas();
            };

            ws.onclose = () => {
                wsBadge.innerText = 'Disconnected';
                wsBadge.className = 'badge badge-ws';
                setTimeout(connect, 2000);
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);
                    if (data.t === 'tel') {
                        updateTelemetry(data);
                    } else if (data.t === 'cfg') {
                        populateConfig(data);
                    } else if (data.t === 'ack') {
                        alert('Settings saved successfully!');
                    }
                } catch (e) {
                    console.error('JSON Parse error', e);
                }
            };
        }

        function updateTelemetry(data) {
            // Update numerical values
            document.getElementById('m1-current-text').innerText = `${data.m1_i.toFixed(1)} mA`;
            document.getElementById('m2-current-text').innerText = `${data.m2_i.toFixed(1)} mA`;
            
            // Limit switch
            const limitDot = document.getElementById('limit-dot');
            if (data.lim) {
                limitDot.classList.add('active');
            } else {
                limitDot.classList.remove('active');
            }

            // Update Progress bars
            const m1Limit = parseFloat(document.getElementById('m1-limit-val').innerText) || 1200;
            const m2Limit = parseFloat(document.getElementById('m2-limit-val').innerText) || 1500;
            const m1Percentage = Math.min((data.m1_i / m1Limit) * 100, 100);
            const m2Percentage = Math.min((data.m2_i / m2Limit) * 100, 100);
            document.getElementById('m1-bar').style.width = `${m1Percentage}%`;
            document.getElementById('m2-bar').style.width = `${m2Percentage}%`;

            // System State
            const state = data.state;
            document.getElementById('state-badge').innerText = state;
            
            const stateDot = document.getElementById('state-dot');
            stateDot.className = 'state-dot ' + state;
            
            document.getElementById('state-text').innerText = state;

            // Handle Show Time Remaining
            const timeLeftText = document.getElementById('show-time-left');
            if (state === 'SHOW_ACTIVE' && data.time > 0) {
                timeLeftText.innerText = `Time Left: ${data.time.toFixed(1)}s`;
            } else {
                timeLeftText.innerText = '';
            }

            // Action button states
            const startBtn = document.getElementById('btn-start');
            const stopBtn = document.getElementById('btn-stop');
            const estopBtn = document.getElementById('btn-estop');
            const resetEstopBtn = document.getElementById('btn-reset-estop');

            if (state === 'ESTOP') {
                startBtn.disabled = true;
                stopBtn.disabled = true;
                estopBtn.style.display = 'none';
                resetEstopBtn.style.display = 'block';
            } else {
                startBtn.disabled = (state === 'HOMING');
                stopBtn.disabled = (state === 'IDLE');
                estopBtn.style.display = 'block';
                resetEstopBtn.style.display = 'none';
            }

            // Manage manual Jog Mode buttons: only enable in IDLE state
            const isIdle = (state === 'IDLE');
            const jogButtons = document.querySelectorAll('.btn-jog');
            const jogInputs = [document.getElementById('m1_jog_speed_val'), document.getElementById('m2_jog_speed_val')];
            
            jogButtons.forEach(btn => {
                btn.disabled = !isIdle;
                if (!isIdle) {
                    btn.style.opacity = '0.4';
                    btn.style.cursor = 'not-allowed';
                } else {
                    btn.style.opacity = '1';
                    btn.style.cursor = 'pointer';
                }
            });
            jogInputs.forEach(input => {
                input.disabled = !isIdle;
            });

            // Update manual LED buttons highlights
            if (data.m_leds) {
                manualLEDsState = data.m_leds;
                for (let i = 0; i < 7; i++) {
                    const btn = document.getElementById(`btn-led-${i}`);
                    if (data.m_leds[i]) {
                        btn.style.background = 'var(--primary)';
                        btn.style.color = '#000';
                        btn.style.boxShadow = '0 0 10px rgba(102, 252, 241, 0.4)';
                        btn.style.borderColor = 'var(--primary)';
                    } else {
                        btn.style.background = '#222';
                        btn.style.color = '#fff';
                        btn.style.boxShadow = 'none';
                        btn.style.borderColor = 'rgba(255,255,255,0.08)';
                    }
                }
            }

            // Update manual Switch buttons highlights
            if (data.m_sw) {
                manualSwitchesState = data.m_sw;
                for (let i = 1; i <= 2; i++) {
                    const btn = document.getElementById(`btn-sw-${i}`);
                    if (data.m_sw[i - 1]) {
                        btn.style.background = 'var(--success)';
                        btn.style.color = '#000';
                        btn.style.boxShadow = '0 0 10px rgba(0, 230, 118, 0.4)';
                        btn.style.borderColor = 'var(--success)';
                    } else {
                        btn.style.background = '#222';
                        btn.style.color = '#fff';
                        btn.style.boxShadow = 'none';
                        btn.style.borderColor = 'rgba(255,255,255,0.08)';
                    }
                }
            }

            // Feed chart buffers
            m1History.push(data.m1_i);
            m1History.shift();
            m2History.push(data.m2_i);
            m2History.shift();

            // Draw oscilloscope graphs
            drawChart(ctxM1, m1History, '#66fcf1', 500); // 500mA max scale
            drawChart(ctxM2, m2History, '#ffab00', 500); // 500mA max scale
        }

        // Tippbetrieb Jog functions for Motors
        function startJog(motor, dir) {
            const state = document.getElementById('state-badge').innerText;
            if (state !== 'IDLE') return;

            if (ws && ws.readyState === WebSocket.OPEN) {
                const speedInputId = motor === 1 ? 'm1_jog_speed_val' : 'm2_jog_speed_val';
                const speed = parseInt(document.getElementById(speedInputId).value) || 120;
                const statusSpan = document.getElementById(motor === 1 ? 'm1-jog-status' : 'm2-jog-status');
                
                statusSpan.innerText = dir === 1 ? (motor === 1 ? 'FWD' : 'UP') : (motor === 1 ? 'REV' : 'DOWN');
                statusSpan.style.color = motor === 1 ? 'var(--primary)' : 'var(--secondary)';

                // Send immediate packet
                ws.send(JSON.stringify({ cmd: 'jog', motor: motor, dir: dir, speed: speed }));

                // Keepalive jog heartbeat repeats every 300ms to override ESP32 safety timeout
                clearInterval(jogInterval);
                jogInterval = setInterval(() => {
                    ws.send(JSON.stringify({ cmd: 'jog', motor: motor, dir: dir, speed: speed }));
                }, 300);
            }
        }

        function stopJog(motor) {
            clearInterval(jogInterval);
            const statusSpan = document.getElementById(motor === 1 ? 'm1-jog-status' : 'm2-jog-status');
            statusSpan.innerText = 'Idle';
            statusSpan.style.color = '#888';

            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ cmd: 'jog_stop', motor: motor }));
            }
        }

        // Tippbetrieb toggle functions for LEDs & Switches
        function toggleManualLED(segIndex) {
            const state = document.getElementById('state-badge').innerText;
            if (state !== 'IDLE') return;

            const nextState = !manualLEDsState[segIndex];
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ cmd: 'toggle_manual_led', seg: segIndex, state: nextState }));
            }
        }

        function toggleManualSwitch(swIndex) {
            const state = document.getElementById('state-badge').innerText;
            if (state !== 'IDLE') return;

            const nextState = !manualSwitchesState[swIndex - 1];
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ cmd: 'toggle_manual_sw', sw: swIndex, state: nextState }));
            }
        }

        function drawChart(ctx, history, color, maxVal) {
            const w = ctx.canvas.width;
            const h = ctx.canvas.height;
            ctx.clearRect(0, 0, w, h);

            // Draw grid
            ctx.strokeStyle = 'rgba(255,255,255,0.04)';
            ctx.lineWidth = 1;
            for (let i = 0; i < w; i += 30) {
                ctx.beginPath();
                ctx.moveTo(i, 0);
                ctx.lineTo(i, h);
                ctx.stroke();
            }
            for (let i = 0; i < h; i += 30) {
                ctx.beginPath();
                ctx.moveTo(0, i);
                ctx.lineTo(w, i);
                ctx.stroke();
            }

            // Draw trace line
            ctx.strokeStyle = color;
            ctx.lineWidth = 2;
            ctx.shadowBlur = 4;
            ctx.shadowColor = color;
            ctx.beginPath();

            const step = w / (history.length - 1);
            for (let i = 0; i < history.length; i++) {
                const x = i * step;
                const y = h - (history[i] / maxVal) * h;
                if (i === 0) {
                    ctx.moveTo(x, y);
                } else {
                    ctx.lineTo(x, y);
                }
            }
            ctx.stroke();
            ctx.shadowBlur = 0; // reset shadow
        }

        function populateConfig(data) {
            configData = data;
            document.getElementById('m1_speed').value = data.m1_speed;
            document.getElementById('m1_jog_speed').value = data.m1_jog_speed;
            document.getElementById('m1_jog_speed_val').value = data.m1_jog_speed;
            document.getElementById('m1_current_limit').value = data.m1_current_limit;
            document.getElementById('m1-limit-val').innerText = data.m1_current_limit;
            document.getElementById('m1_blanking_time').value = data.m1_blanking_time;

            document.getElementById('m2_speed').value = data.m2_speed;
            document.getElementById('m2_jog_speed').value = data.m2_jog_speed;
            document.getElementById('m2_jog_speed_val').value = data.m2_jog_speed;
            document.getElementById('m2_current_limit').value = data.m2_current_limit;
            document.getElementById('m2-limit-val').innerText = data.m2_current_limit;
            document.getElementById('m2_blanking_time').value = data.m2_blanking_time;
            document.getElementById('m2_ramp_time').value = data.m2_ramp_time;
            document.getElementById('m2_run_time_up').value = data.m2_run_time_up;
            document.getElementById('m2_run_time_down').value = data.m2_run_time_down;

            document.getElementById('sw1_active').checked = data.sw1_active;
            document.getElementById('sw1_polarity').value = data.sw1_polarity.toString();
            document.getElementById('sw2_active').checked = data.sw2_active;
            document.getElementById('sw2_polarity').value = data.sw2_polarity.toString();

            if (data.led_seg_brightness) {
                for (let i = 0; i < 7; i++) {
                    document.getElementById(`led_br_${i}`).value = data.led_seg_brightness[i];
                    document.getElementById(`led_br_val_${i}`).innerText = data.led_seg_brightness[i];
                }
            }

            document.getElementById('wifi_ap_mode').value = data.wifi_ap_mode.toString();
            document.getElementById('wifi_ssid').value = data.wifi_ssid;
            document.getElementById('wifi_pass').value = data.wifi_pass || '';
            document.getElementById('show_run_time').value = data.show_run_time;

            toggleWiFiFields();
            resizeCanvas();
        }

        function toggleWiFiFields() {
            const apMode = document.getElementById('wifi_ap_mode').value === 'true';
            const passGroup = document.getElementById('wifi-pass-group');
            if (apMode) {
                document.getElementById('wifi_pass').required = false;
            } else {
                document.getElementById('wifi_pass').required = true;
            }
        }

        function switchTab(evt, tabId) {
            const contents = document.getElementsByClassName('tab-content');
            for (let i = 0; i < contents.length; i++) {
                contents[i].classList.remove('active');
            }
            const buttons = document.getElementsByClassName('tab-btn');
            for (let i = 0; i < buttons.length; i++) {
                buttons[i].classList.remove('active');
            }
            document.getElementById(tabId).classList.add('active');
            evt.currentTarget.classList.add('active');
        }

        function sendCommand(cmd) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ cmd: cmd }));
            }
        }

        function saveConfig(event) {
            event.preventDefault();
            if (!ws || ws.readyState !== WebSocket.OPEN) {
                alert('WebSocket not connected. Cannot save config.');
                return;
            }

            const sendObj = {
                cmd: 'save_config',
                wifi_ssid: document.getElementById('wifi_ssid').value,
                wifi_pass: document.getElementById('wifi_pass').value,
                wifi_ap_mode: document.getElementById('wifi_ap_mode').value === 'true',
                m1_speed: parseInt(document.getElementById('m1_speed').value),
                m1_jog_speed: parseInt(document.getElementById('m1_jog_speed').value),
                m1_current_limit: parseInt(document.getElementById('m1_current_limit').value),
                m1_blanking_time: parseInt(document.getElementById('m1_blanking_time').value),
                m2_speed: parseInt(document.getElementById('m2_speed').value),
                m2_jog_speed: parseInt(document.getElementById('m2_jog_speed').value),
                m2_current_limit: parseInt(document.getElementById('m2_current_limit').value),
                m2_blanking_time: parseInt(document.getElementById('m2_blanking_time').value),
                m2_ramp_time: parseFloat(document.getElementById('m2_ramp_time').value),
                m2_run_time_up: parseFloat(document.getElementById('m2_run_time_up').value),
                m2_run_time_down: parseFloat(document.getElementById('m2_run_time_down').value),
                sw1_active: document.getElementById('sw1_active').checked,
                sw1_polarity: document.getElementById('sw1_polarity').value === 'true',
                sw2_active: document.getElementById('sw2_active').checked,
                sw2_polarity: document.getElementById('sw2_polarity').value === 'true',
                led_seg_brightness: [
                    parseInt(document.getElementById('led_br_0').value),
                    parseInt(document.getElementById('led_br_1').value),
                    parseInt(document.getElementById('led_br_2').value),
                    parseInt(document.getElementById('led_br_3').value),
                    parseInt(document.getElementById('led_br_4').value),
                    parseInt(document.getElementById('led_br_5').value),
                    parseInt(document.getElementById('led_br_6').value)
                ],
                show_run_time: parseInt(document.getElementById('show_run_time').value)
            };

            ws.send(JSON.stringify(sendObj));
        }

        function updateBrLabel(index) {
            const val = document.getElementById(`led_br_${index}`).value;
            document.getElementById(`led_br_val_${index}`).innerText = val;
        }

        // Init
        window.onload = () => {
            connect();
            setTimeout(resizeCanvas, 500);
        };
    </script>
</body>
</html>
)rawhtml";
