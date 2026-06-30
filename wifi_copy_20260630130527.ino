import network

ssid = "Team9"
password = "12345678"

ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid=ssid, password=password)

while not ap.active():
    pass

print("Access Point Started")
print("SSID:", ssid)
print("IP Address:", ap.ifconfig()[0])