/**
 * @author Bohdan Babii
 *
 * @section DESCRIPTION
 * Base class of the wave propagation patches.
 **/
#ifndef TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION
#define TSUNAMI_LAB_PATCHES_WAVE_PROPAGATION

#include <string>

#include "../constants.h"

namespace tsunami_lab {
    namespace patches {
        class WavePropagation;
    }
}  // namespace tsunami_lab

class tsunami_lab::patches::WavePropagation {
   public:
    /**
     * Virtual destructor for base class.
     **/
    virtual ~WavePropagation(){};

    /**
     * Performs a time step.
     *
     * @param i_scalingX scaling of the time step (dt / dx).
     * @param i_scalingY scaling of the time step (dt / dy).
     **/
    virtual void timeStep(t_real i_scalingX,
                          t_real i_scalingY) = 0;

    /**
     * Sets the values of the ghost cells according to entered outflow boundary conditions.
     *
     * @param i_boundary defines the boundary condition.
     **/
    virtual void setGhostCells(e_boundary *i_boundary) = 0;

    /**
     * Gets the stride in y-direction. x-direction is stride-1.
     *
     * @return stride in y-direction.
     **/
    virtual t_idx getStride() = 0;

    /**
     * Gets cells' water heights.
     *
     * @return water heights.
     */
    virtual t_real const *getHeight() = 0;

    /**
     * Gets the cells' momenta in x-direction.
     *
     * @return momenta in x-direction.
     **/
    virtual t_real const *getMomentumX() = 0;

    /**
     * Gets the cells' momenta in y-direction.
     *
     * @return momenta in y-direction.
     **/
    virtual t_real const *getMomentumY() = 0;

    /**
     * Gets the cells bathymetries;
     *
     * @return bathymetries.
     */
    virtual t_real const *getBathymetry() = 0;

    /**
     * Sets the height of the cell to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_h water height.
     **/
    virtual void setHeight(t_idx i_ix,
                           t_idx i_iy,
                           t_real i_h) = 0;

    /**
     * Sets the momentum in x-direction to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_hu momentum in x-direction.
     **/
    virtual void setMomentumX(t_idx i_ix,
                              t_idx i_iy,
                              t_real i_hu) = 0;

    /**
     * Sets the momentum in y-direction to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_iy id of the cell in y-direction.
     * @param i_hv momentum in y-direction.
     **/
    virtual void setMomentumY(t_idx i_ix,
                              t_idx i_iy,
                              t_real i_hv) = 0;

    /**
     * Sets the bathymetry to the given value.
     *
     * @param i_ix id of the cell in x-direction.
     * @param i_ix id of the cell in y-direction.
     * @param i_b bathymetry.
     **/
    virtual void setBathymetry(t_idx i_ix,
                               t_idx i_iy,
                               t_real i_b) = 0;
};

#endif