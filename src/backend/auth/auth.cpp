#include <iostream>
#include "backend/auth/HandleRequest.h"
#include "socket/LocalSocket.h"
#include <nlohmann/json.hpp>
#include <csignal>
#include "config/Config.h"

using json = nlohmann::json;

bool need_to_exit = false;

oink_judge::socket::LocalSocket sock(oink_judge::config::Config::config()["ports"]["auth"]);

void handle_signal(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "Shutting down server..." << std::endl;
        need_to_exit = true;
        sock.close();
        exit(0);
    }
}

int32_t main() {
    signal(SIGINT, handle_signal);   // Ctrl+C
    signal(SIGTERM, handle_signal);  // kill

    while (!need_to_exit) {
        oink_judge::socket::ClientSocket csocket = sock.accept();

        json request;
        try {
            request = csocket.receive_json();
        } catch (const std::exception &e) {
            std::cerr << "Failed to receive request: " << e.what() << std::endl;
            continue;
        }

        json response = oink_judge::backend::auth::handle_client(request);

        try {
            csocket.send_json(response);
        } catch (const std::exception &e) {
            std::cerr << "Failed to send response: " << e.what() << std::endl;
        }
    }
}

