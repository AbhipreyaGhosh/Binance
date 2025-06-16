// const express = require('express');
// const net = require('net');
// const fs = require('fs');
// const path = require('path');

// const app = express();
// const PORT = 3000;
// const SOCKET_PATH = '/tmp/btc_to_node_socket';

// let latestData = {
//   ltp: 0,
//   buy_price: 0,
//   pnl: 0,
//   capital: 0
// };

// // Remove stale socket
// if (fs.existsSync(SOCKET_PATH)) fs.unlinkSync(SOCKET_PATH);

// // Handle incoming data from C++ via Unix socket
// const unixServer = net.createServer((socket) => {
//   socket.on('data', (data) => {
//     const message = data.toString().trim();
//     try {
//       const json = JSON.parse(message);
//       if (
//         typeof json.ltp === 'number' &&
//         typeof json.buy_price === 'number' &&
//         typeof json.pnl === 'number' &&
//         typeof json.capital === 'number'
//       ) {
//         latestData = { ...json };
//       }
//     } catch (err) {
//       // Invalid JSON, ignore
//     }
//   });
// });

// unixServer.listen(SOCKET_PATH, () => {
//   console.log(`🛠️ UNIX socket listening at ${SOCKET_PATH}`);
// });

// // Serve frontend
// app.use(express.static(path.join(__dirname, 'public')));

// // API for frontend polling
// app.get('/data', (req, res) => {
//   res.json(latestData);
// });

// app.listen(PORT, () => {
//   console.log(`🚀 Dashboard available at http://localhost:${PORT}`);
// });
