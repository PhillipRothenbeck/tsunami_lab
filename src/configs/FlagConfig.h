/**
 * @author Marek Sommerfeld (marek.sommerfeld AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Configuration that holds all information for flags.
 **/
#ifndef TSUNAMI_LAB_FLAG_CONFIG_H
#define TSUNAMI_LAB_FLAG_CONFIG_H

namespace tsunami_lab {
    namespace configs {
        class FlagConfig;
    }
}  // namespace tsunami_lab

class tsunami_lab::configs::FlagConfig {
   private:
    //! use timing flag
    bool m_useTiming = false;
    //! use IO flag
    bool m_useIO = true;

   public:
    /**
     * @brief Constructs a flag configuration object.
     *
     */
    FlagConfig() {
    }

    /**
     * @brief Gets the useTiming flag.
     *
     * @return true: timing should be used.
     * @return false: timing should not be used.
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

    /**
     * @brief Gets the useIO flag.
     *
     * @return true: output file should be written.
     * @return false: output file should not be written.
     */
    bool useIO() {
        return m_useIO;
    }

    /**
     * @brief Set the useIO flag.
     *
     * @param i_value value of the useIO flag.
     */
    void setUseIO(bool i_value) {
        m_useIO = i_value;
    }
};

#endif