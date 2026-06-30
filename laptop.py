import socket
import struct
import threading
import tkinter as tk
from tkintermapview import TkinterMapView

# --- NETWORK CONFIG ---
UDP_IP = "0.0.0.0"
UDP_PORT = 5005

class SOSMapApp:
    def __init__(self, root):
        self.root = root
        self.root.title("🚨 Live ESP32 SOS Dashboard")
        self.root.geometry("900x700")

        # Dictionary to keep track of active markers { "NODE_01": marker_object }
        self.active_markers = {}

        # 1. Initialize Map Widget
        self.map_widget = TkinterMapView(self.root, width=900, height=700, corner_radius=0)
        self.map_widget.pack(fill="both", expand=True)

        # Set default view (e.g., center of the map, zoom level 1-19)
        self.map_widget.set_position(53.290, -6.362672) # Default to TU Campus
        self.map_widget.set_zoom(16)

        # 2. Start the Background UDP Network Listener Thread
        self.listener_thread = threading.Thread(target=self.udp_listener, daemon=True)
        self.listener_thread.start()

    def udp_listener(self):
        """Runs in a separate background thread to listen for UDP bytes."""
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        sock.bind((UDP_IP, UDP_PORT))
        
        print(f"📡 UDP Map Sync Active on Port {UDP_PORT}...")

        while True:
            try:
                data, addr = sock.recvfrom(1024)
                if len(data) == 24:
                    # Unpack: 16 bytes string, 4 bytes float lat, 4 bytes float lon
                    device_id_raw, lat, lon = struct.unpack('<16sff', data)
                    device_id = device_id_raw.decode('utf-8').strip('\x00')
                    
                    print(f"📥 Received broadcast from {device_id} -> Lat: {lat}, Lon: {lon}")
                    
                    # Schedule map update to run safely on Tkinter's main UI thread
                    self.root.after(0, self.update_map_marker, device_id, lat, lon)
                else:
                    print("nigga")
            except Exception as e:
                print(f"Error parsing packet: {e}")

    def update_map_marker(self, device_id, lat, lon):
        """Updates an existing marker or creates a new one on OpenStreetMap."""
        
        if device_id in self.active_markers:
            self.active_markers[device_id].delete()
            print(f"🔄 Updating location for {device_id}")
        else:
            print(f"📍 New device detected! Creating marker for {device_id}")

        # Create a new marker on the OpenStreetMap canvas
        # The text parameter creates a label hovering above or clickable on the pin
        new_marker = self.map_widget.set_marker(
            lat, 
            lon, 
            text=f"🚨 SOS: {device_id}",
            marker_color_circle="black",
            marker_color_path="red" # Distinct visual warning color
        )

        self.active_markers[device_id] = new_marker
        # Automatically pan/center the map to focus on the newly distressed node
        self.map_widget.set_position(lat, lon)


# --- MAIN EXECUTION ---
if __name__ == "__main__":
    root = tk.Tk()
    app = SOSMapApp(root)
    root.mainloop()