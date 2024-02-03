/**
 * @author Bohdan Babii
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 *
 * @section DESCRIPTION
 * Simulator class that runs all simulations.
 **/
#ifndef TSUNAMI_LAB_SIMULATOR_SIMULATOR
#define TSUNAMI_LAB_SIMULATOR_SIMULATOR

#include <cstdlib>
#include <string>

#include "../MPIKernel/MPIKernel.h"
#include "../configs/SimConfig.h"
#include "../constants.h"
#include "../io/Csv/Csv.h"
#include "../io/NetCDF/NetCDF.h"
#include "../patches/2d/WavePropagation2d.h"
#include "../setups/Setup.h"
#include "../timer.h"

namespace tsunami_lab {
    class Simulator;
}

class tsunami_lab::Simulator {
   private:
	 //! simulation configuration
    configs::SimConfig m_simConfig;
	 //! MPI data
    MPIKernel::ParallelData m_parallelData;
	 //! grid dimensions
    tsunami_lab::MPIKernel::Grid m_grid;

    t_idx m_nx, m_ny;
    t_real m_dxy, m_dt, m_scalingX, m_scalingY, m_endTime;
    long int m_localSize;

    alignas(8) t_real *m_height, *m_momentumX, *m_momentumY, *m_bathymetry;

	 /**
	  * @brief sends data from process 0 to the other processes.
	  * 
	  * @param i_setup data stored in setup.
	  * @param i_timer timer helper class.
	  */
    void sendData(setups::Setup *i_setup, Timer *i_timer);

	 /**
	  * @brief recieve data from process 0.
	  * 
	  * @param i_timer timer helper class.
	  */
    void recieveData(Timer *i_timer);

   public:
    /**
     * @brief starts and runs the simulation.
     *
     * @param i_setup data stored in setup.
     * @param i_simConfig configuration of the simulation.
     * @param i_parallelData data for MPI usage.
     * @param i_grid dimensions of the grid.
     */
    void runSimulation(tsunami_lab::setups::Setup *i_setup,
                       tsunami_lab::configs::SimConfig i_simConfig,
                       tsunami_lab::MPIKernel::ParallelData i_parallelData,
                       tsunami_lab::MPIKernel::Grid i_grid);
};

#endif