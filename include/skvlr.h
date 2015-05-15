#include <map>

class Skvlr {

    Svklr(const std::string &name, int num_cores);
    ~Svklr();

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
        semaphore sema;
    };

    const std::string name;
    const int num_cores;

    // Matrix of request queues.
};
