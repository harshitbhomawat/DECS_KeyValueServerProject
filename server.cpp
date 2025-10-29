#include <iostream>
#include <unordered_map>
#include <mutex>
#include "httplib.h"
#include "json.hpp"
#include "database.h"
#include "cache.h"

using json = nlohmann::json;

// Simple in-memory database
std::unordered_map<std::string, std::string> kv_store;
std::mutex kv_mutex; // To handle concurrent access

int main() {

    // choose capacity 200 per your choice
    static LRUCache cache(200);


    if (!db_connect()) {
        std::cerr << "Failed to connect to DB!" << std::endl;
        return 1;
    }


    httplib::Server server;

    // Root endpoint
    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Key-Value Server Running!", "text/plain");
    });

    // POST /create
    /*server.Post("/create", [](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body);
        std::string key = body["key"];
        std::string value = body["value"];

        {
            std::lock_guard<std::mutex> lock(kv_mutex);
            kv_store[key] = value;
        }

        res.set_content("SUCCESS: Key stored", "text/plain");
    });*/
    
    server.Post("/create", [&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("key") || !req.has_param("value")) {
            std::cout<<"Missing Key/Value"<<std::endl;
            res.set_content("Missing key/value", "text/plain");
            return;
        }
        std::string key = req.get_param_value("key");
        std::string value = req.get_param_value("value");
        
        //std::cout<<"Post request to create key: "<<key<<", value: "<<value<<std::endl;

        if (!db_insert(key, value))
        {
            res.set_content("DB Error!", "text/plain");
            return;
        }
            
        res.set_content("Stored!", "text/plain");
        cache.put(key, value);
    });


    // GET /read/<key>
    /*server.Get(R"(/read/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
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
    });*/
    
    server.Get("/read", [&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("key")) {
            res.set_content("Missing key", "text/plain");
            return;
        }
        std::string key = req.get_param_value("key");
        std::string value;
        
        if (cache.get(key, value)) {
            //std::cout << "[CACHE HIT] key=" << key << std::endl;
            res.set_content(value, "text/plain");
            return;
        }

        if (db_get(key, value))
            res.set_content(value, "text/plain");
        else
            res.set_content("Not found", "text/plain");
    });


    // DELETE /delete/<key>
    /*server.Delete(R"(/delete/(\w+))", [](const httplib::Request& req, httplib::Response& res) {
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
    });*/
    
    server.Delete("/delete", [&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("key")) {
            res.set_content("Missing key", "text/plain");
            return;
        }
        std::string key = req.get_param_value("key");
        
        cache.erase(key);

        if (db_delete(key))
            res.set_content("Deleted!", "text/plain");
        else
            res.set_content("Not found or error", "text/plain");
    });


    std::cout << "Server running at http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    
    db_close();

    return 0;
}
