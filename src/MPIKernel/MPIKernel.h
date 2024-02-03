/**
 * @author Phillip Rothenbeck (phillip.rothenbeck AT uni-jena.de)
 * @author Moritz Rätz (moritz.raetz AT uni-jena.de)
 *
 * @section DESCRIPTION
 * routines and data structure
 **/
#ifndef TSUNAMI_LAB_MPI_KERNEL
#define TSUNAMI_LAB_MPI_KERNEL

#include <mpi.h>

#include "../constants.h"

namespace tsunami_lab {
    class MPIKernel;
}

class tsunami_lab::MPIKernel {
   public:
    /**
     * @brief Datastructure to hold all important data for MPI communication.
     *
     */
    struct ParallelData {
        //! number of processes in communicator
        int size;
        //! identifier of respective process
        int rank;
        //! identifiers of the respecive neighbours
        int up, down, left, right;
        //! dimensions of cartesian process grid
        int xDim, yDim;
        //! current communicator (world/cartesian)
        MPI_Comm communicator;
        //! requests for subgrid assignment communication
        MPI_Request subgridRequest[4];
        //! requests for first border communication (left/up)
        MPI_Request firstRequest[8];
        //! requests for second border communication (right/down)
        MPI_Request secondRequest[8];
        //! data type for communicating horizontal borders (rows)
        MPI_Datatype horizontalBorder;
        //! data type for communicating vertical borders (columns)
        MPI_Datatype verticalBorder;
        //! data type for subgrid assignment communication
        MPI_Datatype subgrid;

        ParallelData() : size(1), rank(0), up(-2), down(-2), left(-2), right(-2){};
    };

    /**
     * @brief Datastructure to hold important information about the global and local subgrid.
     *
     */
    struct GridData {
        // number of cells in x direction in a subgrid (local domain)
        t_idx localNX;
        // number of cells in y direction in a subgrid (local domain)
        t_idx localNY;
        // number of cells in x direction in the whole grid (global domain)
        t_idx globalNX;
        // number of cells in y direction in the whole grid (global domain)
        t_idx globalNY;
    };

    /**
     * @brief Fetch all information about the MPI environment by creating a
     * cartesian communicator and retrieving the data from it. Also create all
     * needed data types.
     *
     * @param i_globalNX number of cells of the whole grid (global domain) in x-direction.
     * @param i_globalNY number of cells of the whole grid (global domain) in y-direction.
     * @param o_parallelData struct that is supposed to be filled with the communication data.
     * @param o_grid struct that is supposed to be filled with the grid information.
     */
    static void init(t_idx i_globalNX,
                     t_idx i_globalNY,
                     ParallelData *o_parallelData,
                     GridData *o_grid);

    /**
     * @brief Fill the ParallelData structure with data.
     *
     * @param i_localNX number of cells of the subgrid (local domain) in x-direction.
     * @param i_localNY number of cells of the subgrid (local domain) in y-direction.
     * @param i_dimension partition of processes in x and y direction.
     * @param o_parallelData structure to be filled.
     */
    static void initParallelData(t_idx i_localNX,
                                 t_idx i_localNY,
                                 int *i_dimension,
                                 ParallelData *o_parallelData);

    /**
     * @brief Fill the GridData structure with the given information about the global and local domain.
     *
     * @param i_localNX number of cells of the subgrid (local domain) in x-direction.
     * @param i_localNY number of cells of the subgrid (local domain) in y-direction.
     * @param i_globalNX number of cells of the whole grid (global domain) in x-direction.
     * @param i_globalNY number of cells of the whole grid (global domain) in y-direction.
     * @param o_grid structure to be filled.
     */
    static void initGridData(t_idx i_localNX,
                             t_idx i_localNY,
                             t_idx i_globalNX,
                             t_idx i_globalNY,
                             GridData *o_grid);

    /**
     * @brief Free the MPI data types created for the simulation.
     *
     * @param o_parallelData datastructure to hold the information of the MPI process.
     */
    static void freeTypes(ParallelData *o_parallelData);
};

#endif