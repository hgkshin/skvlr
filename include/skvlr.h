#include <map>
#include <mutex>
#include <string>
#include <queue>

#pragma once

class Skvlr {
  friend class Worker;
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Blocking
    int db_get(const int key);

    // Non-blocking
    void db_put(const int key, const int value);

 private:

    enum RequestType { GET, PUT };

    struct request {
        int key;
        int value;
        RequestType type;
      std::mutex mtx;
    };

    const std::string name;
    const int num_cores;

    std::queue<struct request> **request_matrix;
    std::mutex **request_matrix_locks;
};
