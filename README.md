# skvlr
Scalable multi-core KV store

To compile, run it on a high core Ubuntu machine (such as corn or AWS)

```bash
# Fetch Hoard dependencies
> git submodule update --init --recursive

# Build SKVLR
> make

# Run the tests
> ./bin/test

# Run the profiler
> ./bin/profiler

# Run the watch demo
> ./bin/watch_demo
```
