# skvlr
Scalable multi-core KV store

To compile

```bash
# Fetch Hoard dependencies
git submodule update --init --recursive

# Build Hoard
cd Hoard/src
make linux-gcc-x86-64
cd -

# Build SKVLR
make

# Run the profiler
> ./bin/profiler
SKVLR LOG: Directory profiler/profiler_dump/profiler_basic_db doesn't exist, so we create it.
PROFILER LOG: 1: 3.9011e+07
PROFILER LOG: 2: 5.3674e+07
PROFILER LOG: 3: 7.57475e+07
PROFILER LOG: 4: 8.29957e+07
PROFILER LOG: 5: 9.86927e+07
PROFILER LOG: 6: 9.98329e+07
PROFILER LOG: 7: 1.07743e+08
PROFILER LOG: 8: 1.1265e+08
```
