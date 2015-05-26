#include <map>
#include <mutex>
#include <string>

#include "semaphore.h"

#pragma once

class Skvlr {
    friend class Worker;

public:
    Skvlr(const std::string &name, int num_cores);
    ~Skvlr();

    // Blocking
    int db_get(const int key);

    // Non-blocking
    void db_put(const int key, const int value);

private:
    enum RequestType { GET, PUT };
    enum RequestStatus { PENDING, SUCCESS, ERROR };

    struct request {
        int key;
        int value;
        RequestType type;
        RequestStatus status;
        Semaphore sema;
    };

    const std::string name;
    const int num_cores;

    // Matrix of request queues.
};
