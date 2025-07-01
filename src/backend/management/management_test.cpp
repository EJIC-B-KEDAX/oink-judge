#include <iostream>
#include "backend/management/Problem.h"
#include "socket/LocalSocket.h"
#include <nlohmann/json.hpp>
#include <csignal>
#include "backend/management/ICPCProblem.h"
#include "config/Config.h"

using json = nlohmann::json;

bool need_to_exit = false;

oink_judge::socket::LocalSocket sock(oink_judge::config::Config::instance().get_port("management"));

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

    oink_judge::backend::management::ICPCProblem problem("4");

    while (!need_to_exit) {
        oink_judge::socket::ClientSocket csocket = sock.accept();

        json request;
        try {
            request = csocket.receive_json();
        } catch (const std::exception &e) {
            std::cerr << "Failed to receive request: " << e.what() << std::endl;
            continue;
        }

        json response;

        if (request["type"] == "get_score") {
            std::string username = request["username"];

            response["score"] = problem.get_participant_score(username);
        } else if (request["type"] == "test_submission") {
            std::string submission_id = request["submission_id"];
            problem.handle_submission(submission_id);

            response["oink"] = "oink";
        }

        try {
            csocket.send_json(response);
        } catch (const std::exception &e) {
            std::cerr << "Failed to send response: " << e.what() << std::endl;
        }
    }
}

