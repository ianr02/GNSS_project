#!/usr/bin/env python3
"""
LifeLine - PC Alarm Log
Reads CSV lines "id,lat,lng,alarm" from the USB gateway ESP32 (which forwards
the ESP-NOW traffic over serial) and shows a live, timestamped alarm log.
"""

import serial
import threading
import queue
import time
from datetime import datetime
import tkinter as tk
from tkinter import ttk

# ---------------- CONFIG ----------------
SERIAL_PORT = "/dev/cu.usbmodemXXXX"   # <-- set to your gateway ESP's port (ls /dev/cu.*)
BAUD = 115200
PEER_TIMEOUT = 8.0                     # seconds without a packet => device shown offline
# ----------------------------------------

msg_queue = queue.Queue()


def parse_line(line):
    """Return (id, lat, lng, alarm) or None.
    Tolerates an optional 'RSSI -54 : ' style prefix from the gateway debug output."""
    line = line.strip()
    if not line:
        return None
    if ':' in line:                    # strip "RSSI -54 : " prefixes if present
        line = line.split(':')[-1]
    parts = line.split(',')
    if len(parts) != 4:
        return None
    try:
        return int(parts[0]), float(parts[1]), float(parts[2]), int(parts[3])
    except ValueError:
        return None


def serial_reader():
    """Background thread: read serial lines and push parsed messages to the queue."""
    while True:
        try:
            with serial.Serial(SERIAL_PORT, BAUD, timeout=1) as ser:
                print(f"Connected to {SERIAL_PORT}")
                while True:
                    raw = ser.readline().decode("utf-8", errors="ignore")
                    parsed = parse_line(raw)
                    if parsed:
                        msg_queue.put(parsed)
        except Exception as e:
            print(f"Serial error: {e} - retrying in 2 s")
            time.sleep(2)


class AlarmLogApp:
    def __init__(self, root):
        self.root = root
        root.title("LifeLine - Alarm Log")
        root.geometry("780x480")

        # per-device state
        self.last_alarm = {}     # id -> 0/1  (last known alarm state)
        self.last_seen = {}      # id -> timestamp of last packet
        self.active_row = {}     # id -> treeview row of its current ACTIVE alarm

        # --- live status bar (top) ---
        self.status_var = tk.StringVar(value="Waiting for data...")
        tk.Label(root, textvariable=self.status_var, font=("Menlo", 13),
                 anchor="w", padx=10, pady=8).pack(fill="x")

        # --- alarm log table ---
        cols = ("time", "device", "event", "position", "status")
        widths = (150, 70, 140, 200, 130)
        self.tree = ttk.Treeview(root, columns=cols, show="headings", height=16)
        for c, w in zip(cols, widths):
            self.tree.heading(c, text=c.capitalize())
            self.tree.column(c, width=w, anchor="w")
        self.tree.pack(fill="both", expand=True, padx=8, pady=8)
        self.tree.tag_configure("active", background="#ffd6d6")     # light red
        self.tree.tag_configure("resolved", background="#d6ffd6")   # light green

        # start polling the queue + refreshing the status bar
        self.root.after(100, self.process_queue)
        self.root.after(1000, self.refresh_status)

    def process_queue(self):
        while not msg_queue.empty():
            dev_id, lat, lng, alarm = msg_queue.get()
            self.last_seen[dev_id] = time.time()
            prev = self.last_alarm.get(dev_id, 0)

            # edge detection: only log on the transition, not on every packet
            if alarm == 1 and prev == 0:
                self.log_alarm_raised(dev_id, lat, lng)
            elif alarm == 0 and prev == 1:
                self.log_alarm_resolved(dev_id)

            self.last_alarm[dev_id] = alarm
        self.root.after(100, self.process_queue)

    def log_alarm_raised(self, dev_id, lat, lng):
        t = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        pos = f"{lat:.5f}, {lng:.5f}"
        row = self.tree.insert("", 0,
                               values=(t, f"D{dev_id}", "ALARM RAISED", pos, "ACTIVE"),
                               tags=("active",))
        self.active_row[dev_id] = row
        print(f"[{t}] ALARM from D{dev_id} @ {pos}")

    def log_alarm_resolved(self, dev_id):
        t = datetime.now().strftime("%H:%M:%S")
        row = self.active_row.get(dev_id)
        if row:
            vals = list(self.tree.item(row, "values"))
            vals[4] = f"RESOLVED {t}"
            self.tree.item(row, values=vals, tags=("resolved",))
            self.active_row.pop(dev_id, None)
        print(f"[{t}] Alarm from D{dev_id} resolved")

    def refresh_status(self):
        now = time.time()
        parts = []
        for dev_id in sorted(self.last_seen):
            online = (now - self.last_seen[dev_id]) < PEER_TIMEOUT
            alarm = self.last_alarm.get(dev_id, 0)
            state = "🚨 ALARM" if alarm else ("online" if online else "offline")
            parts.append(f"D{dev_id}: {state}")
        self.status_var.set("      ".join(parts) if parts else "Waiting for data...")
        self.root.after(1000, self.refresh_status)


if __name__ == "__main__":
    threading.Thread(target=serial_reader, daemon=True).start()
    root = tk.Tk()
    AlarmLogApp(root)
    root.mainloop()
