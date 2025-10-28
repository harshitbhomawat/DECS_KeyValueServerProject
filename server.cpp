#include <iostream>
#include <unordered_map>
#include <mutex>
#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

// Simple in-memory database
std::unordered_map<std::string, std::string> kv_store;
std::mutex kv_mutex; // To handle concurrent access

int main() {
    httplib::Server server;

    // Root endpoint
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Key-Value Server Running!", "text/plain");
    });

    // POST /create
    server.Post("/create", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string key = body["key"];
        std::string value = body["value"];

        {
            std::lock_guard<std::mutex> lock(kv_mutex);
            kv_store[key] = value;
        }

        res.set_content("SUCCESS: Key stored", "text/plain");
    });

    // GET /read/<key>
    server.Get(R"(/read/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];

        std::string value;
        bool found = false;

        {
            std::lock_guard<std::mutex> lock(kv_mutex);
            auto it = kv_store.find(key);
            if (it != kv_store.end()) {
                found = true;
                value = it->second;
            }
        }

        if (found) {
            res.set_content("VALUE: " + value, "text/plain");
        } else {
            res.status = 404;
            res.set_content("ERROR: Key not found", "text/plain");
        }
    });

    // DELETE /delete/<key>
    server.Delete(R"(/delete/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string key = req.matches[1];

        bool deleted = false;
        {
            std::lock_guard<std::mutex> lock(kv_mutex);
            deleted = kv_store.erase(key) > 0;
        }

        if (deleted) {
            res.set_content("SUCCESS: Key deleted", "text/plain");
        } else {
            res.status = 404;
            res.set_content("ERROR: Key not found", "text/plain");
        }
    });

    std::cout << "Server running at http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);

    return 0;
}
