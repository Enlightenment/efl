#ifndef SGELAPSEDTIMER_H
#define SGELAPSEDTIMER_H

#include "sgglobal.h"
#include<chrono>

class SG_EXPORT SGElapsedTimer
{
public:
    SGElapsedTimer():m_valid(false){}
    double elapsed() const;
    bool hasExpired(double millsec);
    void start();
    double restart();
    inline bool isValid() const {return m_valid;}
private:
    std::chrono::high_resolution_clock::time_point clock;
    bool m_valid;
};
#endif // SGELAPSEDTIMER_H
