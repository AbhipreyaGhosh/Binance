// #include <iostream>
// #include <sys/socket.h>
// #include <sys/un.h>
// #include <unistd.h>
// #include <cstring>
// #include <sstream>     
// #include <iomanip>

// double entry_price = 0.0;
// bool in_position = false;
// double capital = 106000.0;
// double per_trade_amount = 106000.0;
// double take_profit_pct = 0.01;
// double stop_loss_pct = 0.0013;

// void send_to_node(const std::string& message) {
//     const char* out_socket_path = "/tmp/btc_to_node_socket";  // <-- Match this in Node.js
//     int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
//     if (sock_fd < 0) {
//         perror("C++ socket (to Node.js)");
//         return;
//     }

//     struct sockaddr_un addr;
//     memset(&addr, 0, sizeof(addr));
//     addr.sun_family = AF_UNIX;
//     strncpy(addr.sun_path, out_socket_path, sizeof(addr.sun_path) - 1);

//     if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//         perror("C++ connect (to Node.js)");
//         close(sock_fd);
//         return;
//     }

//     send(sock_fd, message.c_str(), message.size(), 0);
//     close(sock_fd);
// }


// void process_price(double price) {
//     std::cout << "📈 Price: " << price << std::endl;

//     double buy_price = entry_price;
//     double pnl = 0.0;

//     if (!in_position) {
//         entry_price = price;
//         in_position = true;
//         std::cout << "🟢 BUY at " << entry_price << std::endl;
//     } else {
//         double tp = entry_price * (1 + take_profit_pct);
//         double sl = entry_price * (1 - stop_loss_pct);

//         if (price >= tp) {
//             double profit = (price - entry_price) * (per_trade_amount / entry_price);
//             capital += profit;
//             pnl = profit;
//             std::cout << "🎯 TAKE PROFIT HIT at " << price << " (+" << profit << ")" << std::endl;
//             std::cout << "💰 Capital: ₹" << capital << std::endl;
//             in_position = false;
//         } else if (price <= sl) {
//             double loss = (price - entry_price) * (per_trade_amount / entry_price);
//             capital += loss;
//             pnl = loss;
//             std::cout << "🛑 STOP LOSS HIT at " << price << " (" << loss << ")" << std::endl;
//             std::cout << "💰 Capital: ₹" << capital << std::endl;
//             in_position = false;
//         } else {
//             // unrealized P&L
//             pnl = (price - entry_price) * (per_trade_amount / entry_price);
//         }
//     }

//     // Construct JSON manually
//     std::ostringstream oss;
//     oss << std::fixed << std::setprecision(2)
//         << "{"
//         << "\"ltp\":" << price << ","
//         << "\"buy_price\":" << entry_price << ","
//         << "\"pnl\":" << pnl << ","
//         << "\"capital\":" << capital
//         << "}";

//     send_to_node(oss.str()+ "\n");
// }


// int main() {
//     const char* socket_path = "/tmp/btc_price_socket";
//     int server_fd, client_fd;
//     struct sockaddr_un addr;

//     unlink(socket_path);

//     server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
//     if (server_fd < 0) {
//         perror("socket");
//         return 1;
//     }

//     memset(&addr, 0, sizeof(addr));
//     addr.sun_family = AF_UNIX;
//     strcpy(addr.sun_path, socket_path);

//     if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
//         perror("bind");
//         return 1;
//     }

//     if (listen(server_fd, 5) == -1) {
//         perror("listen");
//         return 1;
//     }

//     std::cout << "📡 Waiting for price feed on socket: " << socket_path << std::endl;

//     while (true) {
//         client_fd = accept(server_fd, nullptr, nullptr);
//         if (client_fd == -1) {
//             perror("accept");
//             continue;
//         }

//         char buf[128];
//         int bytes_read = read(client_fd, buf, sizeof(buf) - 1);
//         if (bytes_read > 0) {
//             buf[bytes_read] = '\0';
//             try {
//                 double price = std::stod(buf);
//                 process_price(price);
//                 //send_to_node(std::to_string(price));
//             } catch (...) {
//                 std::cerr << "❌ Invalid price: " << buf << std::endl;
//             }
//         }

//         close(client_fd);
//     }

//     close(server_fd);
//     return 0;
// }


