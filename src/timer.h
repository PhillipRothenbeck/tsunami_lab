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
    Timer() {
        start();
    }

    t_time now() {
        return std::chrono::high_resolution_clock::now();
    }

    void start() {
        m_startTime = now();
    }

    void printTime(std::string i_description = "") {
        std::chrono::duration<double> l_chronoTimeElapsed = now() - m_startTime;
		  if (i_description.compare("") != 0) {
			std::cout << i_description << std::endl;
		  }
        std::cout << "Took: " << l_chronoTimeElapsed.count() << "s" << std::endl;
        start();
    }
};

#endif