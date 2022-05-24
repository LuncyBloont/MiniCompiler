#pragma once
#include <iostream>

#ifndef NDEBUG
#define LOG(v) std::cerr << v
#define PERR(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(v)
#define PERR(...)
#endif