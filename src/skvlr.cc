#include <iostream>

#include "skvlr.h"

Skvlr::Skvlr(const std::string &name, int num_cores)
    : name(name), num_cores(num_cores)
{
    // Create KV store if no directory exists.
    //
    // Open files

    // Initialize matrix

    // Spawn threads, pin to cores
}

Skvlr::~Skvlr()
{
    /* Empty */
}
int Skvlr::db_get(const int key)
{
  std::cout << "db_get: " << key << std::endl;
    return -1;
}

void Skvlr::db_put(const int key, const int value)
{
  std::cout << "db_put: " << key << ": " << value << std::endl;
    /* Empty */
}
