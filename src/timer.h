/**
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Helper class for timing.
 **/
#ifndef TSUNAMI_LAB_TIMER_H
#define TSUNAMI_LAB_TIMER_H

#include <chrono>
#include <iostream>
#include <string>

class Timer {
   private:
    typedef std::chrono::high_resolution_clock::time_point t_time;
    t_time m_startTime;

   public:
    /**
     * @brief timer constructer, runs timer->start() when init
     */
    Timer() {
        start();
    }

    /**
     * @brief helper function to get current time
     */
    t_time now() {
        return std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief resets timer to current time
     */
    void start() {
        m_startTime = now();
    }

    /**
     * @brief prints out time took since last reset, resets time afterwards
     */
    void printTime(std::string i_description = "") {
        std::chrono::duration<double> l_chronoTimeElapsed = now() - m_startTime;
        std::cout << "Took: " << l_chronoTimeElapsed.count() << "s";
        if (i_description.compare("") != 0) {
            std::cout << ", Reason: " << i_description << std::endl;
        }
        start();
    }
};

#endif