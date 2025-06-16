// #include "httplib.h"
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <thread>
// #include <atomic>
// #include <mutex>
// #include <cstring>
// #include <unistd.h>
// #include <sys/socket.h>
// #include <sys/un.h>

// struct TradeState
// {
//     std::mutex mtx;
//     double entry_price = 0.0;
//     bool in_position = false;
//     double ltp = 0.0;
//     double buy_price = 0.0;
//     double pnl = 0.0;
//     double capital = 190000.0;
//     double per_trade_amount = 190000.0;
// };

// TradeState state;

// // Process logic from received price
// void process_price(double price)
// {
//     std::lock_guard<std::mutex> lock(state.mtx);
//     state.ltp = price;

//     if (!state.in_position)
//     {
//         state.entry_price = price;
//         state.buy_price = price;
//         state.in_position = true;
//         std::cout << "🟢 BUY at " << price << std::endl;
//     }
//     else
//     {
//         double tp = state.entry_price * 1.01;
//         double sl = state.entry_price * (1 - 0.0013);
//         double profit_or_loss = (price - state.entry_price) * (state.per_trade_amount / state.entry_price);
//         state.pnl = profit_or_loss;
//         state.capital = 190000.0 + profit_or_loss;

//         if (price >= tp)
//         {
//             std::cout << "🎯 TAKE PROFIT: " << price << " (+" << profit_or_loss << ")" << std::endl;
//             state.in_position = false;
//         }
//         else if (price <= sl)
//         {
//             std::cout << "🛑 STOP LOSS: " << price << " (" << profit_or_loss << ")" << std::endl;
//             state.in_position = false;
//         }
//     }
// }

// // Thread to receive from Python over UNIX socket
// void socket_listener()
// {
//     const char *socket_path = "/tmp/btc_price_socket";
//     int server_fd, client_fd;
//     struct sockaddr_un addr;

//     unlink(socket_path);
//     server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
//     if (server_fd < 0)
//     {
//         perror("socket");
//         exit(1);
//     }

//     memset(&addr, 0, sizeof(addr));
//     addr.sun_family = AF_UNIX;
//     strcpy(addr.sun_path, socket_path);

//     if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
//     {
//         perror("bind");
//         exit(1);
//     }

//     if (listen(server_fd, 5) == -1)
//     {
//         perror("listen");
//         exit(1);
//     }

//     std::cout << "📡 Listening for price feed on " << socket_path << std::endl;

//     while (true)
//     {
//         client_fd = accept(server_fd, nullptr, nullptr);
//         if (client_fd == -1)
//         {
//             perror("accept");
//             continue;
//         }

//         char buf[128];
//         int bytes_read = read(client_fd, buf, sizeof(buf) - 1);
//         if (bytes_read > 0)
//         {
//             buf[bytes_read] = '\0';
//             try
//             {
//                 double price = std::stod(buf);
//                 process_price(price);
//             }
//             catch (...)
//             {
//                 std::cerr << "❌ Invalid price received: " << buf << std::endl;
//             }
//         }

//         close(client_fd);
//     }

//     close(server_fd);
// }

// // Serve HTML + live JSON API
// void start_http_server()
// {
//     httplib::Server svr;

//     svr.Get("/", [](const httplib::Request &, httplib::Response &res)
//             {
//                 std::ifstream file("index.html");
//                 if (file)
//                 {
//                     std::stringstream buffer;
//                     buffer << file.rdbuf();
//                     res.set_content(buffer.str(), "text/html");
//                 }
//                 else
//                 {
//                     res.status = 500;
//                     res.set_content("Failed to load index.html", "text/plain");
//                 }
//             });

//     svr.Get("/data", [](const httplib::Request &, httplib::Response &res)
//             {
//         std::lock_guard<std::mutex> lock(state.mtx);
//         std::ostringstream json;
//         json << "{";
//         json << "\"ltp\":" << state.ltp << ",";
//         json << "\"buy_price\":" << state.buy_price << ",";
//         json << "\"pnl\":" << state.pnl << ",";
//         json << "\"capital\":" << state.capital;
//         json << "}";
//         res.set_content(json.str(), "application/json"); });

//     std::cout << "✅ HTTP dashboard: http://localhost:8080" << std::endl;
//     svr.listen("0.0.0.0", 8080);
// }

// int main()
// {
//     std::thread http_thread(start_http_server);
//     socket_listener(); // Blocking call
//     http_thread.join();
//     return 0;
// }

#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

struct TradeState
{
    std::mutex mtx;
    double entry_price = 0.0;
    bool in_position = false;
    double ltp = 0.0;
    double buy_price = 0.0;
    double pnl = 0.0;
    double capital = 190000.0;
    double per_trade_amount = 190000.0;
    std::vector<double> closed_trades;
};

TradeState state;

// Process logic from received price
void process_price(double price)
{
    std::lock_guard<std::mutex> lock(state.mtx);
    state.ltp = price;

    if (!state.in_position)
    {
        state.entry_price = price;
        state.buy_price = price;
        state.in_position = true;
        std::cout << "🟢 BUY at " << price << std::endl;
    }
    else
    {
        double tp = state.entry_price * 1.01;
        double sl = state.entry_price * (1 - 0.0013);
        double profit_or_loss = (price - state.entry_price) * (state.per_trade_amount / state.entry_price);
        state.pnl = profit_or_loss;
        state.capital = 190000.0 + profit_or_loss;

        if (price >= tp)
        {
            std::cout << "🎯 TAKE PROFIT: " << price << " (+" << profit_or_loss << ")" << std::endl;
            state.in_position = false;
            state.closed_trades.push_back(profit_or_loss);
        }
        else if (price <= sl)
        {
            std::cout << "🛑 STOP LOSS: " << price << " (" << profit_or_loss << ")" << std::endl;
            state.in_position = false;
            state.closed_trades.push_back(profit_or_loss);
        }
        
    }
    
}

// Thread to receive from Python over UNIX socket
void socket_listener()
{
    const char *socket_path = "/tmp/btc_price_socket";
    int server_fd, client_fd;
    struct sockaddr_un addr;

    unlink(socket_path);
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_path);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 5) == -1)
    {
        perror("listen");
        exit(1);
    }

    std::cout << "📡 Listening for price feed on " << socket_path << std::endl;

    while (true)
    {
        client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        char buf[128];
        int bytes_read = read(client_fd, buf, sizeof(buf) - 1);
        if (bytes_read > 0)
        {
            buf[bytes_read] = '\0';
            try
            {
                double price = std::stod(buf);
                process_price(price);
            }
            catch (...)
            {
                std::cerr << "❌ Invalid price received: " << buf << std::endl;
            }
        }

        close(client_fd);
    }

    close(server_fd);
}

// Serve HTML + live JSON API
void start_http_server()
{
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request &, httplib::Response &res)
            {
                std::ifstream file("index.html");
                if (file)
                {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    res.set_content(buffer.str(), "text/html");
                }
                else
                {
                    res.status = 500;
                    res.set_content("Failed to load index.html", "text/plain");
                } });

    svr.Get("/data", [](const httplib::Request &, httplib::Response &res) {
    std::lock_guard<std::mutex> lock(state.mtx);
    std::ostringstream json;
    json << "{";
    json << "\"ltp\":" << state.ltp << ",";
    json << "\"buy_price\":" << state.buy_price << ",";
    json << "\"pnl\":" << state.pnl << ",";
    json << "\"capital\":" << state.capital << ",";
    json << "\"trades\":[";
    for (size_t i = 0; i < state.closed_trades.size(); ++i) {
        json << state.closed_trades[i];
        if (i != state.closed_trades.size() - 1)
            json << ",";
    }
    json << "]";
    json << "}";
    res.set_content(json.str(), "application/json");
});


    svr.Post("/squareoff", [](const httplib::Request &req, httplib::Response &res)
             {
    std::lock_guard<std::mutex> lock(state.mtx);

    if (state.in_position) {
        double profit_or_loss = (state.ltp - state.entry_price) * (state.per_trade_amount / state.entry_price);
        state.pnl = profit_or_loss;
        state.capital = 190000.0 + profit_or_loss;
        state.in_position = false;

        std::ostringstream out;
        out << "🔴 Position squared off at " << state.ltp << ", P&L: ₹" << profit_or_loss;
        std::cout << out.str() << std::endl;
        res.set_content(out.str(), "text/plain");
        state.closed_trades.push_back(profit_or_loss);
    } else {
        res.set_content("No active position to square off", "text/plain");
    } });

    std::cout << "✅ HTTP dashboard: http://localhost:8080" << std::endl;
    svr.listen("0.0.0.0", 8080);
}

int main()
{
    std::thread http_thread(start_http_server);
    socket_listener(); // Blocking call
    http_thread.join();
    return 0;
}
