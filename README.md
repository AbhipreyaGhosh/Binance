# Binance BTC Live Scalper — README

## Project Overview

This repository contains a small prototype for receiving real-time BTC market data from Binance, forwarding it between processes, and running a simple scalping/trading logic that can be observed in a web dashboard. It is a mix of Python, C++ and (commented) Node.js pieces used to prototype different IPC and networking approaches.

High-level flow options implemented in the repo:
- Python WebSocket -> UNIX socket -> C++ trading engine + HTTP dashboard (primary path)
- Python WebSocket -> UDP packet forwarder (alternate path, `packets.py`)
- C++ trading engine serves an HTTP dashboard (`index.html`) and a JSON API (`/data`)


## Architecture / Components

- `BTCUSD.py` — Python WebSocket client that subscribes to Binance ticker stream (`btcusdt@ticker`) and writes the latest price to a UNIX domain socket at `/tmp/btc_price_socket`. This is intended to feed the local C++ trading engine.
- `packets.py` — Python WebSocket client that receives trade messages from Binance (`btcusdt@trade`) and forwards raw messages over UDP to a configured IP and port. Useful if you want to forward messages to another machine (example target IP configured inside the file).
- `main.cpp` — Primary C++ trading engine.
  - Listens on UNIX socket `/tmp/btc_price_socket` for price updates (sent by `BTCUSD.py`).
  - Implements a simple entry/take-profit/stop-loss logic:
    - On first price: open a buy position.
    - Take profit at +1% from entry.
    - Stop loss at ~-0.13% from entry (using 1 - 0.0013).
    - Tracks `pnl`, `capital`, and `closed_trades`.
  - Serves an HTTP dashboard (uses `httplib.h`) on port `8080`:
    - GET `/` serves `index.html`.
    - GET `/data` returns live JSON with `ltp`, `buy_price`, `pnl`, `capital`, and `trades`.
    - POST `/squareoff` forces squaring off the current position.
- `scalper.cpp` — An alternative/older C++ variant (commented) that listens on the same UNIX socket and demonstrates sending data to Node via another UNIX socket `/tmp/btc_to_node_socket`.
- `server.js` — Node/Express-based server (commented out). When enabled it listens on `/tmp/btc_to_node_socket` for JSON from C++ and serves the `public/` frontend and `/data` endpoint for the browser.
- `index.html` — Frontend dashboard that polls `/data` and displays live LTP, buy price, capital, P&L and trade history boxes; includes a `Square Off` button that POSTs `/squareoff`.
- `httplib.h` — Header-only C++ HTTP server library used by `main.cpp` (must remain in the project root or be found by include path).


## What each file does (detailed)

- `BTCUSD.py`
  - Connects to `wss://stream.binance.com:9443/ws` and subscribes to `btcusdt@ticker`.
  - On ticker messages, parses JSON, extracts `c` (close price) and writes the price string to UNIX socket `/tmp/btc_price_socket`.
  - Designed as the live price feeder for the C++ engine.

- `packets.py`
  - Connects to Binance WebSocket `wss://stream.binance.com:9443/ws/btcusdt@trade`.
  - For each incoming trade message it prints it and forwards the raw message over UDP to the `target_ip` and `target_port` configured at the top of the file.
  - Use-case: forward trade stream to a remote machine or service that consumes UDP packets.

- `main.cpp`
  - Creates an AF_UNIX server socket bound to `/tmp/btc_price_socket` and accepts price strings.
  - `process_price(double price)` implements trading rules and updates `TradeState`.
  - Starts a `httplib::Server` on port `8080` and exposes endpoints used by the frontend.
  - Combines socket listener and HTTP server using threads (HTTP runs in separate thread).

- `scalper.cpp`
  - Similar logic to `main.cpp` but contains examples of sending results to Node via `/tmp/btc_to_node_socket`.
  - Mostly commented-out reference code.

- `server.js`
  - Example Node/Express server that listens on `/tmp/btc_to_node_socket` for data from C++.
  - Serves the `public/` static frontend and exposes `/data` for the browser to poll.
  - This file is commented out; it is provided as an alternate approach if you want the dashboard via Node instead of C++.

- `index.html` (frontend)
  - Polls `/data` every 300ms to update the UI.
  - Shows LTP, buy price, capital and P&L.
  - Renders a visual history of closed trades and a `Square Off` button that POSTs `/squareoff`.


## Requirements

- Python 3.8+ (for `BTCUSD.py` and `packets.py`)
  - Python package: `websocket-client` (install via pip)
- C++ compiler (g++/clang) with pthread support to build `main.cpp`.
- `httplib.h` (present in repo) — header-only server implementation used by `main.cpp`.
- (Optional) Node.js + npm if you plan to use `server.js` + `public/` frontend.


## Quick Setup & Run

Below are the minimal steps to get the prototype running locally using the C++ HTTP dashboard.

1) Install Python dependency:

```bash
python3 -m pip install websocket-client
```

2) Start the Python price feeder (`BTCUSD.py`) in one terminal:

```bash
python3 BTCUSD.py
```

This creates the UNIX socket writer to `/tmp/btc_price_socket` and will log prices.

3) Build and run the C++ engine (`main.cpp`) in another terminal:

```bash
# Compile (GNU/Clang)
g++ -std=c++17 main.cpp -o binance_scalper -pthread

# Run
./binance_scalper
```

Notes:
- Ensure `httplib.h` is present in the same folder (it is included in this repo).
- If compilation fails due to missing headers (e.g., `<vector>`), add them or use a modern C++ standard flag (`-std=c++17`). The provided `main.cpp` uses `std::vector` in `TradeState`.

4) Open the dashboard in your browser:

```
http://localhost:8080
```

You should see live updates in the UI. Use the `Square Off` button to force the engine to close an active position.


## Alternate path — UDP forwarder

If you prefer to forward raw trade messages via UDP instead of the UNIX socket feeder, run:

```bash
python3 packets.py
```

Configure `target_ip` and `target_port` at the top of `packets.py` to point at the receiver.


## Optional Node dashboard

To use the Node.js server approach (if you prefer Node instead of the C++ HTTP server):

1. Install dependencies:

```bash
npm install
```

2. Edit `scalper.cpp` (or other producer) to `connect()` to `/tmp/btc_to_node_socket` and send JSON:
- `scalper.cpp` contains a commented `send_to_node()` helper showing how to send JSON to the Node socket.

3. Start the Node server (after uncommenting/adjusting `server.js`):

```bash
node server.js
```

4. Visit `http://localhost:3000` (update port if changed in `server.js`).


## Troubleshooting & Tips

- If the Python client fails to connect, check network access to `stream.binance.com` and that your system allows outbound websocket/TLS connections.
- If C++ build fails due to missing includes, add standard headers (e.g., `#include <vector>`), and compile with `-std=c++17`.
- UNIX socket path: `/tmp/btc_price_socket`. Ensure no stale socket file exists (the C++ code calls `unlink()` but if you hit permission issues remove manually).
- If `index.html` shows `NaN` or UI errors, hit the `/data` endpoint directly in your browser to inspect the JSON.


## Improvements (suggestions & prioritized list)

1. Robust reconnection & backoff
   - Add reconnection logic and exponential backoff to WebSocket clients (`BTCUSD.py`, `packets.py`). Handle transient network errors and resumed feeds.

2. Message validation & parsing
   - Make JSON parsing defensive. Validate expected fields before using them.

3. Configuration file / CLI flags
   - Move constants (socket paths, ports, IPs, thresholds) to a single `config.json` or environment variables.

4. Error handling & logging
   - Replace prints with a proper logging framework (Python `logging`, C++ spdlog or similar) and persist logs for debugging.

5. Unit tests & integration tests
   - Add tests for `process_price()` logic and end-to-end tests with a mocked WebSocket stream.

6. Packaging & deployment
   - Provide `systemd` unit files or Docker images for reproducible deployment of the feeder and engine.

7. Security & hardening
   - Validate/limit incoming data sizes. If you expose any HTTP endpoints externally, add authentication.

8. Use official REST/WS libraries and typed messages
   - Consider using Binance SDKs with signed endpoints if order execution or authenticated feeds are needed in the future.

9. Replace polling with WebSockets on the frontend (optional)
   - Instead of polling `/data`, push updates from the engine to the browser via WebSocket for lower latency and reduced CPU cost.

10. Clean separation & modernization
   - Decide on a single IPC pattern (UNIX sockets vs UDP vs in-process) and remove commented legacy code. Convert prototypes to small services with clear contracts.


## Notes & Warnings

- This code is a learning / prototype project. It is not production-ready and contains minimal error handling. Do not use it to place real trades or as-is in live systems handling real funds.
- Timing, precision, network latency, and exchange rules are important for real trading systems — this repo demonstrates logic only.


---
