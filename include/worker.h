class Worker {

    Worker(const int fd, const int worker_id, std::map<int, int> data);
    ~Worker();

 private:

    void handle_get(request &req);
    void handle_put(request &req);

    const int fd;
    const int worker_id;
    std::map<int, int> data;
};
