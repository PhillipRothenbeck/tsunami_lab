/**
 * @author Phillip Rothenbeck
 *
 * @section DESCRIPTION
 * Two-dimensional wave propagation patch.
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION_2D

#include "../../MPIKernel/MPIKernel.h"
#include "../../solvers/FWave.h"
#include "../WavePropagation.h"

namespace tsunami_lab {
    namespace patches {
        class WavePropagation2d;
    }
}  // namespace tsunami_lab

class tsunami_lab::patches::WavePropagation2d : public WavePropagation {
   private:
    //! current step which indicates the active values in the arrays below
    unsigned short m_step = 0;

    //! number of cells in x-direction discretizing the computational domain
    t_idx m_nCellsX = 0;

    //! number of cells in y-direction discretizing the computational domain
    t_idx m_nCellsY = 0;

    //! number of overall cells discretizing the computational domain including ghost cells
    t_idx m_nCellsAll = 0;

    //! water heights for the current and next time step for all cells
    alignas(8) t_real *m_h[2] = {nullptr, nullptr};

    //! momenta in x-direction for the current and next time step for all cells
    alignas(8) t_real *m_hu[2] = {nullptr, nullptr};

    //! momenta in y-direction for the current and next time step for all cells
    alignas(8) t_real *m_hv[2] = {nullptr, nullptr};

    //! bathymetries for all cells
    alignas(8) t_real *m_b = nullptr;

    //! parallel data like neighbour threads
    MPIKernel::ParallelData m_parallelData;

    //! set ghost cells by communicating with neighbours
    void setSweepGhostCells(t_real *i_height,
                            t_real *i_momentumX,
                            t_real *i_momentumY,
                            t_real *i_bathymetry,
                            int i_mode);

   public:
    /**
     * Constructs the 2d wave propagation solver.
     *
     * @param i_nCellsX number of cells in x-direction.
     * @param i_nCellsY number of cells in y-direction.
     * @param i_height array of height values for step 0 (including ghost cell space).
     * @param i_momentumX array of momentum values in x-direction for step 0 (including ghost cell space).
     * @param i_momentumY array of momentum values in y-direction for step 0 (including ghost cell space).
     * @param i_bathymetry array of bathymetry values for step 0 (including ghost cell space).
     **/
    WavePropagation2d(t_idx i_nCellsX,
                      t_idx i_mCellsY,
                      MPIKernel::ParallelData i_parallelData,
                      t_real *i_height,
                      t_real *i_momentumX,
                      t_real *i_momentumY,
                      t_real *i_bathymetry);

    /**
     * Destructor which frees all allocated memory.
     **/
    ~WavePropagation2d();

    /**
     * Calculate linear position from 2d coordinates.
     *
     * @param i_x x coordinate of queried point.
     * @param i_y y coordinate of queried point.
     */
    t_idx getIndex(t_idx i_x, t_idx i_y);

    /**
     * Performs a time step.
     *
     * @param i_scalingX scaling of the time step (dt / dx).
     * @param i_scalingY scaling of the time step (dt / dy).
     **/
    void timeStep(t_real i_scalingX,
                  t_real i_scalingY);

    /**
     * @brief copies the 4 corner cells of the water grid into the ghost corner cells.
     *
     * @param o_dataArray corresponding data array containing cell data, for example h or hu.
     */
    void copyCornerCells(t_real *o_dataArray);

    /**
     * @brief copies the data of each outter cell into the ghost cells according to axis alignment.
     *
     * @param i_axis array to determine which edge should be set. 0: x-Axis (-1, 1; left, right); 1: y-Axis (-1, 1: bottom, top).
     * @param o_dataArray corresponding data array containing cell data, for example h or hu.
     */
    void copyGhostCellsOutflow(short i_axis[2], t_real *o_dataArray);

    /**
     * @brief copies the data of each outter cell into the ghost cells according to axis alignment.
     *
     * @param i_axis array to determine which edge should be set. 0: x-Axis (-1, 1; left, right); 1: y-Axis (-1, 1: bottom, top).
     * @param i_value the value the cell should be set to.
     * @param o_dataArray corresponding data array containing cell data, for example h or hu.
     */
    void copyGhostCellsReflecting(short i_axis[2], t_real i_value, t_real *o_dataArray);

    /**
     * Sets the values of the ghost cells according to entered outflow boundary conditions.
     *
     * @param i_boundary defines the boundary condition.
     **/
    void setGhostCells(e_boundary *i_boundary);

    /**
     * Gets the stride in y-direction. x-direction is stride-1.
     *
     * @return stride in y-direction.
     **/
    t_idx getStride() {
        return m_nCellsX + 2;
    }

    /**
     * Gets cells' water heights.
     *
     * @return water heights.
     */
    t_real const *getHeight() {
        return m_h[m_step];
    }

    /**
     * Gets the cells' momenta in x-direction.
     *
     * @return momenta in x-direction.
     **/
    t_real const *getMomentumX() {
        return m_hu[m_step];
    }

    /**
     * Gets the cells' momenta in y-direction.
     *
     * @return momenta in y-direction.
     **/
    t_real const *getMomentumY() {
        return m_hv[m_step];
    }

    /**
     * Gets the cells bathymetries;
     *
     * @return bathymetries.
     */
    t_real const *getBathymetry() {
        return m_b;
    }

    /**
     * Sets the height of the cell to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_h water height.
     **/
    void setHeight(t_idx i_ix,
                   t_idx i_iy,
                   t_real i_h) {
        t_idx l_idx = getIndex(i_ix + 1, i_iy + 1);
        m_h[m_step][l_idx] = i_h;
    }

    /**
     * Sets the momentum in x-direction to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_hu momentum in x-direction.
     **/
    void setMomentumX(t_idx i_ix,
                      t_idx i_iy,
                      t_real i_hu) {
        t_idx l_idx = getIndex(i_ix + 1, i_iy + 1);
        m_hu[m_step][l_idx] = i_hu;
    }

    /**
     * Sets the momentum in y-direction to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_hv momentum in y-direction.
     **/
    void setMomentumY(t_idx i_ix,
                      t_idx i_iy,
                      t_real i_hv) {
        t_idx l_idx = getIndex(i_ix + 1, i_iy + 1);
        m_hv[m_step][l_idx] = i_hv;
    };

    /**
     * Sets the bathymetry to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_ix id of the cell in y-direction.
     * @param i_b bathymetry.
     **/
    void setBathymetry(t_idx i_ix,
                       t_idx i_iy,
                       t_real i_b) {
        t_idx l_idx = getIndex(i_ix + 1, i_iy + 1);
        m_b[l_idx] = i_b;
    }
};

#endif