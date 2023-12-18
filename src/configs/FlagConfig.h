/**
 * @author Bohdan Babii, Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Configuration that holds all information for flags.
 **/
#ifndef TSUNAMI_LAB_FLAG_CONFIG_H
#define TSUNAMI_LAB_FLAG_CONFIG_H

#include <cstdlib>
#include <string>

#include "../constants.h"

namespace tsunami_lab {
    namespace configs {
        class FlagConfig;
    }
}  // namespace tsunami_lab

class tsunami_lab::configs::FlagConfig {
   private:
    bool m_useCheckPoint = false;
    bool m_useTiming = false;
    bool m_useIO = true;

   public:
    /**
     * Constructs a flag configuration object.
     *
     */
    FlagConfig() {
    }

    /**
     * @brief Gets the useCheckPoint flag.
     *
     * @return wether checkpoint should be used.
     */
    bool useCheckPoint() {
        return m_useCheckPoint;
    }

    /**
     * @brief Set the useCheckPoint flag.
     *
     * @param i_value value of the useCheckPoint flag.
     */
    void setUseCheckPoint(bool i_value) {
        m_useCheckPoint = i_value;
    }

    /**
     * @brief Gets the useTiming flag.
     *
     * @return wether timing should be used.
     */
    bool useTiming() {
        return m_useTiming;
    }

    /**
     * @brief Set the useTiming flag.
     *
     * @param i_value value of the useTiming flag.
     */
    void setUseTiming(bool i_value) {
        m_useTiming = i_value;
    }

    bool useIO() {
        return m_useIO;
    }

    void setUseIO(bool i_value) {
        m_useIO = i_value;
    }
};

#endif