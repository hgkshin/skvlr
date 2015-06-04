#include <map>

#include "unskvlr.h"

Unskvlr::Unskvlr()
{
    /* Empty */
}

Unskvlr::~Unskvlr()
{
    /* Empty */
}

void Unskvlr::db_get(const int key, int *value, int curr_cpu)
{
    UNUSED(curr_cpu);
    data_lock.lock();
    *value = data[key];
    data_lock.unlock();
}

void Unskvlr::db_put(const int key, int value, int curr_cpu)
{
    UNUSED(curr_cpu);
    data_lock.lock();
    data[key] = value;
    data_lock.unlock();
}
