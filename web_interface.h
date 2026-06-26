// =====================================================
// web_interface.h - Web Dashboard HTML & API Endpoints
// =====================================================
// Generates responsive HTML dashboard and JSON API responses

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "config.h"
#include "timing.h"
#include "state_machine.h"

// ===== HTML Dashboard (stored in PROGMEM to save RAM) =====
const char htmlPage[] PROGMEM = R"rawhtml(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="3">
<title>Smart Parking — Slot 101</title>
<link href="https://fonts.googleapis.com/css2?family=DM+Mono:wght@400;500&family=Syne:wght@700;800&family=Inter:wght@400;500&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #0a0c10;
    --surface: #111318;
    --surface2: #181b22;
    --border: #1e2230;
    --accent-green: #00e87a;
    --accent-red: #ff3b5c;
    --accent-blue: #3b82f6;
    --text: #e8eaf0;
    --text-dim: #6b7280;
    --text-mid: #9ca3af;
    --glow-green: 0 0 24px rgba(0,232,122,0.25);
    --glow-red: 0 0 24px rgba(255,59,92,0.25);
  }

  * { box-sizing: border-box; margin: 0; padding: 0; }

  body {
    font-family: 'Inter', sans-serif;
    background: var(--bg);
    color: var(--text);
    min-height: 100vh;
    padding: 24px 16px 60px;
  }

  body::before {
    content: '';
    position: fixed;
    inset: 0;
    background-image:
      linear-gradient(rgba(59,130,246,0.03) 1px, transparent 1px),
      linear-gradient(90deg, rgba(59,130,246,0.03) 1px, transparent 1px);
    background-size: 40px 40px;
    pointer-events: none;
    z-index: 0;
  }

  .wrap {
    max-width: 760px;
    margin: 0 auto;
    position: relative;
    z-index: 1;
  }

  .header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    margin-bottom: 32px;
    padding-bottom: 20px;
    border-bottom: 1px solid var(--border);
  }
  .header-left h1 {
    font-family: 'Syne', sans-serif;
    font-size: 1.6rem;
    font-weight: 800;
    letter-spacing: -0.5px;
    color: var(--text);
  }
  .header-left p {
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--text-dim);
    margin-top: 4px;
    letter-spacing: 0.08em;
    text-transform: uppercase;
  }
  .live-dot {
    display: flex;
    align-items: center;
    gap: 8px;
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .live-dot::before {
    content: '';
    width: 8px; height: 8px;
    border-radius: 50%;
    background: var(--accent-green);
    box-shadow: 0 0 8px var(--accent-green);
    animation: pulse 1.5s infinite;
  }
  @keyframes pulse {
    0%,100% { opacity: 1; }
    50% { opacity: 0.3; }
  }

  .status-card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 20px;
    padding: 36px 32px;
    margin-bottom: 28px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 24px;
    position: relative;
    overflow: hidden;
    transition: box-shadow 0.4s ease;
  }
  .status-card.occupied {
    border-color: rgba(255,59,92,0.3);
    box-shadow: var(--glow-red);
  }
  .status-card.available {
    border-color: rgba(0,232,122,0.3);
    box-shadow: var(--glow-green);
  }
  .status-card::after {
    content: '';
    position: absolute;
    top: -40px; right: -40px;
    width: 140px; height: 140px;
    border-radius: 50%;
    opacity: 0.06;
  }
  .status-card.occupied::after { background: var(--accent-red); }
  .status-card.available::after { background: var(--accent-green); }

  .status-left {}
  .slot-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.7rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.12em;
    margin-bottom: 12px;
  }
  .status-badge {
    display: inline-flex;
    align-items: center;
    gap: 10px;
    padding: 10px 22px;
    border-radius: 100px;
    font-family: 'Syne', sans-serif;
    font-weight: 700;
    font-size: 1rem;
    letter-spacing: 0.02em;
  }
  .status-badge.occupied {
    background: rgba(255,59,92,0.12);
    color: var(--accent-red);
    border: 1.5px solid rgba(255,59,92,0.35);
  }
  .status-badge.available {
    background: rgba(0,232,122,0.1);
    color: var(--accent-green);
    border: 1.5px solid rgba(0,232,122,0.3);
  }
  .status-badge .dot {
    width: 9px; height: 9px;
    border-radius: 50%;
  }
  .status-badge.occupied .dot { background: var(--accent-red); box-shadow: 0 0 6px var(--accent-red); }
  .status-badge.available .dot { background: var(--accent-green); box-shadow: 0 0 6px var(--accent-green); animation: pulse 1.5s infinite; }

  .status-sub {
    font-size: 0.82rem;
    color: var(--text-dim);
    margin-top: 12px;
  }

  .status-right {
    text-align: right;
  }
  .car-count-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .car-count-num {
    font-family: 'Syne', sans-serif;
    font-size: 3rem;
    font-weight: 800;
    color: var(--text);
    line-height: 1;
    margin-top: 4px;
  }

  .session-bar {
    background: var(--surface);
    border: 1px solid rgba(255,59,92,0.2);
    border-left: 3px solid var(--accent-red);
    border-radius: 12px;
    padding: 14px 20px;
    margin-bottom: 28px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    gap: 12px;
  }
  .session-bar-label {
    font-family: 'DM Mono', monospace;
    font-size: 0.72rem;
    color: var(--accent-red);
    text-transform: uppercase;
    letter-spacing: 0.1em;
  }
  .session-bar-time {
    font-family: 'DM Mono', monospace;
    font-size: 0.85rem;
    color: var(--text-mid);
  }

  .section-head {
    display: flex;
    align-items: baseline;
    gap: 10px;
    margin-bottom: 16px;
  }
  .section-head h2 {
    font-family: 'Syne', sans-serif;
    font-size: 1.1rem;
    font-weight: 700;
    color: var(--text);
  }
  .section-head .count-pill {
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    background: var(--surface2);
    border: 1px solid var(--border);
    color: var(--text-dim);
    padding: 2px 10px;
    border-radius: 100px;
    letter-spacing: 0.05em;
  }

  .table-wrap {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 16px;
    overflow: hidden;
  }
  table {
    width: 100%;
    border-collapse: collapse;
  }
  thead th {
    background: var(--surface2);
    padding: 13px 18px;
    text-align: left;
    font-family: 'DM Mono', monospace;
    font-size: 0.68rem;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.1em;
    border-bottom: 1px solid var(--border);
    font-weight: 500;
  }
  tbody td {
    padding: 14px 18px;
    font-size: 0.85rem;
    border-bottom: 1px solid var(--border);
    color: var(--text-mid);
    vertical-align: middle;
  }
  tbody tr:last-child td { border-bottom: none; }
  tbody tr:hover td { background: var(--surface2); }

  .car-id {
    font-family: 'DM Mono', monospace;
    font-size: 0.8rem;
    color: var(--accent-blue);
    background: rgba(59,130,246,0.1);
    border: 1px solid rgba(59,130,246,0.2);
    padding: 3px 10px;
    border-radius: 6px;
    display: inline-block;
  }
  .time-val {
    font-family: 'DM Mono', monospace;
    font-size: 0.78rem;
    color: var(--text-mid);
  }
  .dur-val {
    font-family: 'DM Mono', monospace;
    font-size: 0.8rem;
    color: var(--text);
    background: var(--surface2);
    border: 1px solid var(--border);
    padding: 3px 10px;
    border-radius: 6px;
    display: inline-block;
  }

  .empty-row td {
    text-align: center;
    color: var(--text-dim);
    font-size: 0.85rem;
    padding: 40px 18px;
    font-style: italic;
  }

  .footer {
    text-align: center;
    margin-top: 40px;
    font-family: 'DM Mono', monospace;
    font-size: 0.65rem;
    color: var(--text-dim);
    letter-spacing: 0.08em;
    text-transform: uppercase;
  }

  @media(max-width: 520px) {
    .status-card { flex-direction: column; align-items: flex-start; }
    .status-right { text-align: left; }
    .header { flex-direction: column; align-items: flex-start; gap: 12px; }
  }
</style>
</head>
<body>
<div class="wrap">

  <div class="header">
    <div class="header-left">
      <h1>&#x1F17F; Smart Parking</h1>
      <p>Slot #101 &nbsp;&bull;&nbsp; Live Monitor</p>
    </div>
    <div class="live-dot">Auto-refresh 3s</div>
  </div>

  <div class="status-card STATUS_CLASS">
    <div class="status-left">
      <div class="slot-label">Current Status</div>
      <div class="status-badge STATUS_CLASS">
        <span class="dot"></span>
        STATUS_TEXT
      </div>
      <div class="status-sub">STATUS_DESC</div>
    </div>
    <div class="status-right">
      <div class="car-count-label">Total Visits</div>
      <div class="car-count-num">VISIT_COUNT</div>
    </div>
  </div>

SESSION_BAR

  <div class="section-head"><h2>Parking History</h2><span class="count-pill">RECORD_COUNT records</span></div>
  <div class="table-wrap"><table><thead><tr>
  <th>Car</th><th>Entry Time</th><th>Exit Time</th><th>Duration</th>
  </tr></thead><tbody>
HISTORY_ROWS
  </tbody></table></div>

  <div class="footer">Smart Parking System &nbsp;&bull;&nbsp; ESP32 &nbsp;&bull;&nbsp; Only sessions &ge;10s are recorded</div>

</div></body></html>)rawhtml";

// ===== Generate HTML Dashboard =====
String getHTML() {
    String html = String(htmlPage);
    
    // Status section
    html.replace("STATUS_CLASS", isOccupied ? "occupied" : "available");
    html.replace("STATUS_TEXT", isOccupied ? "OCCUPIED" : "AVAILABLE");
    html.replace("STATUS_DESC", isOccupied ? "&#x1F697; Vehicle detected in slot" : "&#x2705; Slot is free to park");
    html.replace("VISIT_COUNT", String(getTotalVisits()));
    
    // Session bar (only if occupied)
    String sessionBar = "";
    if (isOccupied) {
        unsigned long elapsed = getEpochNow() - sessionStartEpoch;
        sessionBar = "<div class='session-bar'>";
        sessionBar += "<div class='session-bar-label'>&#x23F1; Session in progress</div>";
        sessionBar += "<div class='session-bar-time'>Started: " + formatRealTime(sessionStartEpoch);
        sessionBar += " &nbsp;|&nbsp; Elapsed: " + formatDuration(elapsed) + "</div>";
        sessionBar += "</div>";
    }
    html.replace("SESSION_BAR", sessionBar);
    
    // History table
    String historyRows = "";
    auto recent = getRecentSessions(15);
    
    if (recent.empty()) {
        historyRows = "<tr class='empty-row'><td colspan='4'>No parking records yet &mdash; sessions under 10 seconds are not logged</td></tr>";
    } else {
        for (int i = recent.size() - 1; i >= 0; i--) {
            historyRows += "<tr>";
            historyRows += "<td><span class='car-id'>#" + String(recent[i].carID) + "</span></td>";
            historyRows += "<td><span class='time-val'>" + formatRealTime(recent[i].entryEpoch) + "</span></td>";
            historyRows += "<td><span class='time-val'>" + formatRealTime(recent[i].exitEpoch) + "</span></td>";
            historyRows += "<td><span class='dur-val'>" + formatDuration(recent[i].duration) + "</span></td>";
            historyRows += "</tr>";
        }
    }
    
    html.replace("HISTORY_ROWS", historyRows);
    html.replace("RECORD_COUNT", String(getSessionCount()));
    
    return html;
}

// ===== Generate JSON API Response =====
String getJSON() {
    unsigned long currentEpoch = getEpochNow();
    unsigned long elapsedSession = isOccupied ? (currentEpoch - sessionStartEpoch) : 0;
    
    String json = "{";
    json += "\"occupied\":" + String(isOccupied ? "true" : "false") + ",";
    json += "\"total_visits\":" + String(getTotalVisits()) + ",";
    json += "\"history_records\":" + String(getSessionCount()) + ",";
    json += "\"current_epoch\":" + String(currentEpoch) + ",";
    json += "\"session_elapsed_seconds\":" + String(elapsedSession);
    json += "}";
    
    return json;
}

#endif // WEB_INTERFACE_H
