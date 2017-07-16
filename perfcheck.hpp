#pragma once

#include <cmath>
#include <iostream>

// To use, put PERF_INIT somewhere in the class, and call
// PERF_ADD on variables that should count as samples.

#define USE_PERF

#ifdef USE_PERF
#define PERF_INIT StreamStat __SS_PERF__;
#define PERF_CLEAR __SS_PERF__.clear();
#define PERF_ADD( x ) __SS_PERF__.add(x);
#define PERF_LOG \
std::cout << "---------------------------------------" << std::endl; \
std::cout << __FILE__ << "(" << __LINE__ << ")::" << __FUNCTION__ << std::endl; \
std::cout << "---------------------------------------" << std::endl; \
std::cout << "[Stats]" << std::endl; \
std::cout << "Samples: " << __SS_PERF__.n << std::endl; \
std::cout << "Mean: " << __SS_PERF__.mean() << std::endl; \
std::cout << "Variance: " << __SS_PERF__.variance() << std::endl; \
std::cout << "Standard Deviation: " << __SS_PERF__.sd() << std::endl; \
std::cout << std::endl;

#else
#define PERF_INIT
#define PERF_CLEAR
#define PERF_ADD( x )
#define PERF_LOG
#endif


// Calculate numerically stable moving average and variance (and stddev).
// Algorithm (Welford's method) is found here:
// https://www.johndcook.com/blog/standard_deviation/
struct StreamStat {
    void clear() {
        n = 0;
    }

    void add( double x ) {
        ++n;

        if( n == 1 ) {
            m_new = x;
            v_new = 0;
        } else {
            m_new = m_old + ( x - m_old ) / n;
            v_new = v_old + ( x - m_old ) * ( x - m_new );
        }

        m_old = m_new;
        v_old = v_new;
    }

    double mean() {
        return m_new;
    }

    double variance() {
        return n > 1 ? v_new / (n - 1) : v_new;
    }

    double sd() {
        return sqrt( variance() );
    }

    double m_old, m_new;
    double v_old, v_new;
    int n = 0;
};

