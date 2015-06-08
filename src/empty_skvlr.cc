#include "empty_skvlr.h"

EmptySkvlr::EmptySkvlr()
{
    /* Empty */
}

EmptySkvlr::~EmptySkvlr()
{
    /* Empty */
}

void EmptySkvlr::db_get(const int key, int *value, int curr_cpu)
{
    assert(curr_cpu >= 0);
    auto& map = maps[curr_cpu];
    map.map_lock.lock();
    auto it = map.map.find(key);
    if (it == map.map.end()) {
      *value = 0;
    } else {
      *value = it->second;
    }
    map.map_lock.unlock();
}

void EmptySkvlr::db_put(const int key, int value, int curr_cpu)
{
    assert(curr_cpu >= 0);
    auto& map = maps[curr_cpu];
    map.map_lock.lock();
    map.map[key] = value;
    map.map_lock.unlock();
}
