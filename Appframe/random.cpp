#include "random.h"

thread_local std::mt19937 Random::randomEngine;
std::uniform_int_distribution<std::mt19937::result_type> Random::distribution;