import websocket
import json
import socket

def on_message(ws, message):
    data = json.loads(message)
    symbol = data.get('s')
    price = data.get('c')
    print(f"📈 {symbol} Price: {price}")

    # Send price to Unix socket
    if price is not None:
        try:
            client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            client.connect("/tmp/btc_price_socket")
            client.sendall(str(price).encode('utf-8'))
            client.close()
        except Exception as e:
            print("Socket error:", e)

def on_open(ws):
    payload = {
        "method": "SUBSCRIBE",
        "params": [
            "btcusdt@ticker"  # You can add more pairs like "ethusdt@ticker"
        ],
        "id": 1
    }
    ws.send(json.dumps(payload))

def on_error(ws, error):
    print("Error:", error)

def on_close(ws, close_status_code, close_msg):
    print("WebSocket closed")

if __name__ == "__main__":
    socket_url = "wss://stream.binance.com:9443/ws"

    ws = websocket.WebSocketApp(socket_url,
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
    ws.run_forever()


