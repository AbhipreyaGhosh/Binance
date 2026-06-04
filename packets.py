import websocket
import socket

ws_url = "wss://stream.binance.com:9443/ws/btcusdt@trade"
target_ip = "172.20.10.2"   # Ubuntu system IP
target_port = 9001          # UDP port

udp_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def on_message(ws, message):
    print("Received from Binance:", message)
    udp_sock.sendto(message.encode(), (target_ip, target_port))
    print("Sent packet to", target_ip, target_port)

def on_error(ws, error):
    print("WebSocket error:", error)

def on_close(ws, close_status_code, close_msg):
    print("WebSocket closed")

def on_open(ws):
    print("WebSocket connection opened")

if __name__ == "__main__":
    ws = websocket.WebSocketApp(
        ws_url,
        on_open=on_open,
        on_message=on_message,
        on_error=on_error,
        on_close=on_close
    )
    ws.run_forever()