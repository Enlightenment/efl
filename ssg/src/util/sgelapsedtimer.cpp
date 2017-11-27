#include"sgelapsedtimer.h"

void SGElapsedTimer::start()
{
    clock = std::chrono::high_resolution_clock::now();
    m_valid = true;
}

double SGElapsedTimer::restart()
{
    double elapsedTime = elapsed();
    start();
    return elapsedTime;
}

double SGElapsedTimer::elapsed() const
{
    if (!isValid()) return 0;
    return std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now()-clock).count();
}

bool SGElapsedTimer::hasExpired(double time)
{
    double elapsedTime = elapsed();
    if (elapsedTime > time)
        return true;
    return false;
}

