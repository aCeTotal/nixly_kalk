#pragma once

#include <cstdio>

inline int failures = 0;

inline void check(bool ok, const char *what)
{
    if (ok)
        return;
    ++failures;
    std::fprintf(stderr, "FAIL: %s\n", what);
}
