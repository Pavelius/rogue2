#include <time.h>

unsigned long start_random_seed;

void waitcputime(unsigned v) {
    timespec req = {};
    req.tv_sec = v / 1000;
    req.tv_nsec = (v%1000)*1000000;
    nanosleep(&req, 0);
}

unsigned long getcputime() {
    timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000ULL +
           ts.tv_nsec / 1000000ULL;
}

unsigned randomseed() {
    return time(0);
}
