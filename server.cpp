#include <iostream>
#include <unordered_map>
#include <mutex>
#include "httplib.h"
#include "database.h"
#include "cache.h"
#include <sstream>


int main() {

    // setting cache capacity 400 
    static LRUCache cache(20000);


    if (!db_connect()) {
        std::cerr << "Failed to connect to DB!" << std::endl;
        return 1;
    }

    httplib::Server server;

    // int threads = std::thread::hardware_concurrency();
    // if (threads == 0) threads = 4;
    int threads = 64;
    server.new_task_queue = [threads] {
        return new httplib::ThreadPool(threads);
    };

    server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        res.set_content("Key-Value Server Running!", "text/plain");
    });
    
    server.Post("/create", [&](const httplib::Request& req, httplib::Response& res) {
    
        std::string key, value;
        if (req.has_param("key") && req.has_param("value")) {
            key = req.get_param_value("key");
            value = req.get_param_value("value");
        } else {
            res.set_content("Missing key/value", "text/plain");
            return;
        }
        /*
        //curl -X POST "http://localhost:8080/create?key=testKey&value=testValue"
        if (req.has_param("key") && req.has_param("value")) {
            key = req.get_param_value("key");
            value = req.get_param_value("value");
        }
        
        //curl -X POST -d "key=name&value=harshit2" http://localhost:8080/create
        if (req.has_form_data("key") && req.has_form_data("value")) {
            key = req.get_form_value("key");
            value = req.get_form_value("value");
        }*/

        if (key.empty() || value.empty()) {
            res.status = 400;
            res.set_content("Missing key or value", "text/plain");
            return;
        }

        if (!db_insert(key, value)) {
            res.status = 500;
            res.set_content("DB Error!", "text/plain");
            return;
        }

        cache.put(key, value);
        res.set_content("Stored!", "text/plain");
    });
    
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
        {
            cache.put(key, value);
            res.set_content(value, "text/plain");
        }
        else
            res.set_content("Not found key in DB", "text/plain");
    });
    
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

    server.Get("/stats", [&](const httplib::Request& req, httplib::Response& res) {
        // fetch stats
        uint64_t hits = cache.hits();
        uint64_t misses = cache.misses();

        // reset after reading
        cache.reset_stats();

        // Produce JSON manually (no json library needed)
        std::ostringstream out;
        out << "{ \"hits\": " << hits << ", \"misses\": " << misses << " }";

        res.set_content(out.str(), "application/json");
    });

    server.Post("/flush", [&](const httplib::Request &req, httplib::Response &res) {
        bool ok = db_flush();
        cache.clear();

        if (!ok) {
            res.status = 500;
            res.set_content("Failed to flush DB\n", "text/plain");
            return;
        }

        res.status = 200;
        res.set_content("Flushed DB + Cache\n", "text/plain");
    });



    std::cout << "Server running at http://localhost:8080\n";
    server.listen("0.0.0.0", 8080);
    
    db_close();

    return 0;
}
