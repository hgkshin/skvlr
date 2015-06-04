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
    UNUSED(key);
    UNUSED(value);
    UNUSED(curr_cpu);
    return;
}

void EmptySkvlr::db_put(const int key, int value, int curr_cpu)
{
    UNUSED(key);
    UNUSED(value);
    UNUSED(curr_cpu);
    return;
}
