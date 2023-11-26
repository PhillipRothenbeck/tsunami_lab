.. _ch:Task_5:

Week Report 5
=============

.. _ch:Task_5_1:

Implementation of NetCDF Output
-------------------------------

Implemented a :code:`init` function which initializes the dimensions nad it's variables.

.. code-block:: c++

    int tsunami_lab::io::NetCDF::init(t_real i_dxy,
                                  t_idx i_nx,
                                  t_idx i_ny,
                                  t_idx i_stride,
                                  t_real const *i_b) {
    m_dxy = i_dxy;
    m_nx = i_nx;
    m_ny = i_ny;
    m_stride = i_stride;
    m_b = i_b;

    int l_dimXId,
        l_dimYId, l_dimTimeId;
    int l_varXId, l_varYId, l_varTimeId, l_varBathymetryId, l_varHeightId, l_varMomentumXId, l_varMomentumYId;
    int l_nc_err;

    // create netCDF file
    l_nc_err = nc_create(m_fileName.c_str(), NC_CLOBBER, &m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    // define dimensions
    l_nc_err = nc_def_dim(m_ncId, "x", i_nx, &l_dimXId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_def_dim(m_ncId, "y", i_ny, &l_dimYId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_def_dim(m_ncId, "time", NC_UNLIMITED, &l_dimTimeId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    // define variables
    l_nc_err = nc_def_var(m_ncId, "x", NC_FLOAT, 1, &l_dimXId, &l_varXId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_def_var(m_ncId, "y", NC_FLOAT, 1, &l_dimYId, &l_varYId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_def_var(m_ncId, "time", NC_FLOAT, 1, &l_dimTimeId, &l_varTimeId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    int l_dimBathymetryIds[2] = {l_dimXId, l_dimYId};
    l_nc_err = nc_def_var(m_ncId, "bathymetry", NC_FLOAT, 2, l_dimBathymetryIds, &l_varBathymetryId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    int l_dimHeightIds[3] = {l_dimTimeId, l_dimXId, l_dimYId};
    l_nc_err = nc_def_var(m_ncId, "height", NC_FLOAT, 3, l_dimHeightIds, &l_varHeightId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    int l_dimMomentumXIds[3] = {l_dimTimeId, l_dimXId, l_dimYId};
    l_nc_err = nc_def_var(m_ncId, "momentum_x", NC_FLOAT, 3, l_dimMomentumXIds, &l_varMomentumXId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    int l_dimMomentumYIds[3] = {l_dimTimeId, l_dimXId, l_dimYId};
    l_nc_err = nc_def_var(m_ncId, "momentum_y", NC_FLOAT, 3, l_dimMomentumYIds, &l_varMomentumYId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_enddef(m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }
    
it also covers the creation of the x, y and bathymetry data.

.. code-block:: c++

	 // write x coordinates
    t_real *l_dataX = new t_real[m_nx];
    for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
        l_dataX[l_ix] = (l_ix + 0.5) * m_dxy;
    }
    l_nc_err = nc_put_var_float(m_ncId, l_varXId, l_dataX);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    // write y coordinates
    t_real *l_dataY = new t_real[m_ny];
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        l_dataY[l_iy] = (l_iy + 0.5) * m_dxy;
    }
    l_nc_err = nc_put_var_float(m_ncId, l_varYId, l_dataY);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    // write bathymetry
    t_real *l_bathymetry = new t_real[m_nx * m_ny];
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            l_bathymetry[l_ix + l_iy * m_nx] = i_b[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }
    l_nc_err = nc_put_var_float(m_ncId, l_varBathymetryId, l_bathymetry);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

finally it writes the data into the file and deletes it's pointers.

.. code-block:: c++

    l_nc_err = nc_close(m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    delete[] l_dataY;
    delete[] l_dataX;
    delete[] l_bathymetry;

    return 0;
}

The :code:`write` function writes continous data (i.e. the timestep, height an gets the variable Ids via the variable names.

.. code-block:: c++

	int tsunami_lab::io::NetCDF::write(t_real i_time,
                                   t_idx i_timeStep,
                                   t_real const *i_h,
                                   t_real const *i_hu,
                                   t_real const *i_hv) {
    int l_varTimeId, l_varHeightId, l_varMomentumXId, l_varMomentumYId;
    int l_nc_err;

    l_nc_err = nc_open(m_fileName.c_str(), NC_WRITE, &m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_inq_varid(m_ncId, "time", &l_varTimeId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_inq_varid(m_ncId, "height", &l_varHeightId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_inq_varid(m_ncId, "momentum_x", &l_varMomentumXId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_inq_varid(m_ncId, "momentum_y", &l_varMomentumYId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    l_nc_err = nc_put_var1_float(m_ncId, l_varTimeId, &i_timeStep, &i_time);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

It then appends the data to the variables and safes it.

.. code-block:: c++

	 size_t l_startp[3] = {i_timeStep, 0, 0};
    size_t l_countp[3] = {1, m_nx, m_ny};

    t_real *l_height = new t_real[m_nx * m_ny];
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            l_height[l_ix + l_iy * m_nx] = i_h[(l_iy + 1) * m_stride + (l_ix + 1)] + m_b[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }
    l_nc_err = nc_put_vara_float(m_ncId, l_varHeightId, l_startp, l_countp, l_height);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    t_real *l_momentumX = new t_real[m_nx * m_ny];
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            l_momentumX[l_ix + l_iy * m_nx] = i_hu[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }
    l_nc_err = nc_put_vara_float(m_ncId, l_varMomentumXId, l_startp, l_countp, l_momentumX);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    t_real *l_momentumY = new t_real[m_nx * m_ny];
    for (t_idx l_iy = 0; l_iy < m_ny; l_iy++) {
        for (t_idx l_ix = 0; l_ix < m_nx; l_ix++) {
            l_momentumY[l_ix + l_iy * m_nx] = i_hv[(l_iy + 1) * m_stride + (l_ix + 1)];
        }
    }
    l_nc_err = nc_put_vara_float(m_ncId, l_varMomentumYId, l_startp, l_countp, l_momentumY);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    delete[] l_height;
    delete[] l_momentumX;
    delete[] l_momentumY;

    l_nc_err = nc_close(m_ncId);
    if (l_nc_err != NC_NOERR) {
        std::cout << "Error: " << nc_strerror(l_nc_err) << std::endl;
        return 1;
    }

    return 0;
}


Implementation of Artificial Tsunami setup
-------------------------------------------

Implemented an Tsunami setup with constant bathymetry and artificial displacement in the computational domain :math:`[10\text{km}] \times [10\text{km}]`.
The displacement happens in a :math:`[1\text{km}] \times [1\text{km}]` square in the center of our domain.

.. math::

    h &= 100 \\
    hu &= 0 \\
    hv &= 0 \\
    b  &= \begin{cases}
               -100 + d, & \text{ if } x,y <= |500|\\
               -100, & \text{ else }.
             \end{cases}

.. math:: \text{d}: [-500, +500] \times [-500, +500] \rightarrow \mathbb{R}

.. math::

   \begin{aligned}
       \text{d}(x, y) & = & 5 \cdot f(x)g(y) \\
       \text{f}(x) & = & \sin\left(\left(\frac{x}{500}+1\right) \cdot \pi\right) \\
       \text{g}(y) & = & -\left(\frac{y}{500}\right)^2 + 1
     \end{aligned}

Hence a new setup (ArtificialTsunami2d) was implemented:

.. code-block:: c++

  tsunami_lab::setups::ArtificialTsunami2d::ArtificialTsunami2d(t_real in_simLenX,
                                                                t_real in_simLenY) {
      m_simLenX = in_simLenX;
      m_simLenY = in_simLenY;
  }

  tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getHeight(t_real,
                                                                          t_real) const {
      // max(-bathymetry, delta(20m)) -> bathymetry constant -100 -> height constant 100
      return 100;
  }

  tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumX(t_real,
                                                                            t_real) const {
      return 0;
  }

  tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getMomentumY(t_real,
                                                                            t_real) const {
      return 0;
  }

  tsunami_lab::t_real tsunami_lab::setups::ArtificialTsunami2d::getBathymetry(t_real in_x,
                                                                              t_real in_y) const {
      t_real displacement = 0;
      t_real pi = 3.14159265358979323846;
      // displacement = 5 * g(x) * f(y)
      displacement = 5 * sin(((in_x / 500) + 1) * pi) * (-((in_y / 500) * (in_y / 500)) + 1);

      // theory: 10km x 10km -> mid at 5000m. if |midx - in_x| <= 500 return -100 + displacement, else -100

      t_real midx = (m_simLenX / 2);
      t_real midy = (m_simLenY / 2);

      if (abs(midx - in_x) <= 500 && abs(midy - in_y) <= 500) {
          return (-100 + displacement);
      } else {
          return -100;
      }
  }

Visualization of the two-dimensional artificial Tsunami Event:

.. figure:: ../_static/video_folder/assignment_5/
  :width: 600px

Implementation of NetCDF Input
-------------------------------

Implemented the NetCDF file input through a new :code:`read` function.

First, the file paths are initialized:

.. code-block:: c++

  int tsunami_lab::io::NetCDF::read(std::string i_nameBathymetry,
                                    std::string i_nameDisplacements,
                                    t_idx *o_bathymetryDimX,
                                    t_idx *o_bathymetryDimY,
                                    t_real *&o_bathymetryPosX,
                                    t_real *&o_bathymetryPosY,
                                    t_real *&o_bathymetry,
                                    t_idx *o_dispDimX,
                                    t_idx *o_dispDimY,
                                    t_real *&o_dispPosX,
                                    t_real *&o_dispPosY,
                                    t_real *&o_displacements) {
      // add res path
      i_nameBathymetry = "./res/" + i_nameBathymetry;
      i_nameDisplacements = "./res/" + i_nameDisplacements;
      int l_ncIDBathymetry, l_ncIDDisplacements;
      int l_nc_err = nc_open(i_nameBathymetry.c_str(), 0, &l_ncIDBathymetry);

the bathymetry CDF file is opened and all values to be read are initialized:

.. code-block:: c++

    // open bathymetry file
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not open file: " << i_nameBathymetry << std::endl;
        return 1;
    }

    // get dimensions
    std::size_t l_xDim, l_yDim;
    l_nc_err = nc_inq_dimlen(l_ncIDBathymetry, 0, &l_xDim);
    l_nc_err = nc_inq_dimlen(l_ncIDBathymetry, 1, &l_yDim);

    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could get the size of a dimension in bathymetry." << std::endl;
        return 1;
    }

    *o_bathymetryDimX = (t_idx)l_xDim;
    *o_bathymetryDimY = (t_idx)l_yDim;
    o_bathymetry = new tsunami_lab::t_real[l_xDim * l_yDim];
    o_bathymetryPosX = new tsunami_lab::t_real[l_xDim];
    o_bathymetryPosY = new tsunami_lab::t_real[l_yDim];

and finally the values are read from the file:

.. code-block:: c++

    // get variable ids
    int l_varIDx, l_varIDy, l_varIDz;
    l_nc_err = nc_inq_varid(l_ncIDBathymetry, "x", &l_varIDx);
    l_nc_err = nc_inq_varid(l_ncIDBathymetry, "y", &l_varIDy);
    l_nc_err = nc_inq_varid(l_ncIDBathymetry, "z", &l_varIDz);

    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could find variable in bathymetry." << std::endl;
        return 1;
    }

    // read position of bathymetry in grid

    l_nc_err = nc_get_var_float(l_ncIDBathymetry, l_varIDx, &o_bathymetryPosX[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable x" << std::endl;
        return 1;
    }

    l_nc_err = nc_get_var_float(l_ncIDBathymetry, l_varIDy, &o_bathymetryPosY[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable y" << std::endl;
        return 1;
    }

    // read bathymetry value itself

    l_nc_err = nc_get_var_float(l_ncIDBathymetry, l_varIDz, o_bathymetry);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable z" << std::endl;
        return 1;
    }

The same two steps happen for the displacement netCDF file.

Open displacement file and initialize values to be read:

.. code-block:: c++

    l_nc_err = nc_open(i_nameDisplacements.c_str(), 0, &l_ncIDDisplacements);

    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not open file: " << i_nameBathymetry << std::endl;
        return 1;
    }

    l_nc_err = nc_inq_dimlen(l_ncIDDisplacements, 0, &l_xDim);
    l_nc_err = nc_inq_dimlen(l_ncIDDisplacements, 1, &l_yDim);

    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could get the size of a dimension in displacements." << std::endl;
        return 1;
    }

    *o_dispDimX = (t_idx)l_xDim;
    *o_dispDimY = (t_idx)l_yDim;
    o_displacements = new tsunami_lab::t_real[l_xDim * l_yDim];
    o_dispPosX = new tsunami_lab::t_real[l_xDim];
    o_dispPosY = new tsunami_lab::t_real[l_yDim];

Read values from displacement netCDF:

.. code-block:: c++
  
    // get variable ids
    l_nc_err = nc_inq_varid(l_ncIDDisplacements, "x", &l_varIDx);
    l_nc_err = nc_inq_varid(l_ncIDDisplacements, "y", &l_varIDy);
    l_nc_err = nc_inq_varid(l_ncIDDisplacements, "z", &l_varIDz);

    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could find variable in displacements." << std::endl;
        return 1;
    }

    // read position of displacement in grid

    l_nc_err = nc_get_var_float(l_ncIDDisplacements, l_varIDx, &o_dispPosX[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable x" << std::endl;
        return 1;
    }

    l_nc_err = nc_get_var_float(l_ncIDDisplacements, l_varIDy, &o_dispPosY[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable y" << std::endl;
        return 1;
    }

    // read bathymetry value itself

    l_nc_err = nc_get_var_float(l_ncIDDisplacements, l_varIDz, &o_displacements[0]);
    if (l_nc_err != NC_NOERR) {
        std::cerr << "Could not load data from variable z" << std::endl;
        return 1;
    }

Implementation of two-dimensional Tsunami Event
-----------------------------------------------

In the non-artificial version of the two-dimensional Tsunami Event there is a slight change to the initial values.

.. math::

   \begin{split}
       h  &= \begin{cases}
               \max( -b_\text{in}, \delta), &\text{if } b_\text{in} < 0 \\
               0, &\text{else}
             \end{cases}\\
       hu &= 0\\
       hv &= 0\\
       b  &= \begin{cases}
               \min(b_\text{in}, -\delta) + d_\text{in}, & \text{ if } b_\text{in} < 0\\
               \max(b_\text{in}, \delta) + d_\text{in}, & \text{ else}.
             \end{cases}
   \end{split}

The user is able to make various settings in the config .json file (such as simulation time, boundary conditions, resolution of cells, ...) :

.. code-block:: c++

  {
      "dimension": 2,
      "nx": 500,
      "ny": 500,
      "xLen": 5000.0,
      "yLen": 5000.0,
      "simTime": 5000,
      "boundaryCond": "OO",
      "setup": "TsunamiEvent"
  }

The setup itself receives most of the parameters from the netCDF input file:

.. code-block:: c++

  tsunami_lab::setups::TsunamiEvent2d::TsunamiEvent2d(t_real *in_bathymetry,
                                                      t_real *in_rawX_bathymetry,
                                                      t_idx in_dimX_bathymetry,
                                                      t_real *in_rawY_bathymetry,
                                                      t_idx in_dimY_bathymetry,
                                                      t_real *in_displacement,
                                                      t_real *in_rawX_displacement,
                                                      t_idx in_dimX_displacement,
                                                      t_real *in_rawY_displacement,
                                                      t_idx in_dimY_displacement) {
      m_bathymetry = in_bathymetry;
      m_rawX_bathymetry = in_rawX_bathymetry;
      m_dimX_bathymetry = in_dimX_bathymetry;
      m_rawY_bathymetry = in_rawY_bathymetry;
      m_dimY_bathymetry = in_dimY_bathymetry;
      m_displacement = in_displacement;
      m_rawX_displacement = in_rawX_displacement;
      m_dimX_displacement = in_dimX_displacement;
      m_rawY_displacement = in_rawY_displacement;
      m_dimY_displacement = in_dimY_displacement;
  }

To ensure that the resolution of our simulation does not collide with the given grid coordinates, a conversion of the x,y input values at a certain time step into x,y grid coordinates from the input file is performed.

Therefore, the index of the nearest value in the given grid coordinates (e.g. :code:`m_rawX_bathymetry`) is found. 

These indices (e.g. :code:`nearestValueX`) are then used to calculate the position of the corresponding bathymetry value:

.. code-block:: c++

  tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getHeight(t_real in_x,
                                                                     t_real in_y) const {
      // convert scaled x,y to given values from netCDF
      // simple unoptimzed solution: iterate over array and store index of nearest Value
      int nearestValueX = 0;
      int nearestValueY = 0;
      for (t_idx i = 1; i < m_dimX_bathymetry; i++) {
          if (fabs(m_rawX_bathymetry[i] - in_x) < fabs(m_rawX_bathymetry[nearestValueX] - in_x)) {
              nearestValueX = i;
          }
      }

      for (t_idx i = 1; i < m_dimY_bathymetry; i++) {
          if (fabs(m_rawX_bathymetry[i] - in_y) < fabs(m_rawX_bathymetry[nearestValueY] - in_y)) {
              nearestValueY = i;
          }
      }

      // calculate new converted pos index of bathymetry
      t_idx newBathymetryIndex = nearestValueY * m_dimX_bathymetry + nearestValueX;

      if (m_bathymetry[newBathymetryIndex] < 0) {
          return (-m_bathymetry[newBathymetryIndex] < 20) ? 20 : -m_bathymetry[newBathymetryIndex];
      }

      return 0;
  }

The displacement data follow the same procedure in terms of the grid coordinates, but differ in the use of the computational domain.
The raw input grid coordinates are used in an if condition to constrain the calculation of the bathymetry in the displacement area.

Therefore, the smallest and largest values in the x and y directions are used to check whether you are in the displacement range or not:

.. code-block:: c++

  tsunami_lab::t_real tsunami_lab::setups::TsunamiEvent2d::getBathymetry(t_real in_x,
                                                                         t_real in_y) const {
      // variables for displacement condition
      t_real smallestX = m_rawX_displacement[0];
      t_real biggestX = m_rawX_displacement[m_dimX_displacement - 1];
      t_real smallestY = m_rawY_displacement[0];
      t_real biggestY = m_rawY_displacement[m_dimX_displacement - 1];

      nearestValueX = 0;
      nearestValueY = 0;

      // if in domain of displacement look for nearest value
      if (in_x >= smallestX && in_x <= biggestX) {
          for (t_idx i = 1; i < m_dimX_displacement; i++) {
              if (fabs(m_rawX_displacement[i] - in_x) < fabs(m_rawX_displacement[nearestValueX] - in_x)) {
                  nearestValueX = i;
              }
          }
      }

      if (in_y >= smallestY && in_y <= biggestY) {
          for (t_idx i = 1; i < m_dimY_displacement; i++) {
              if (fabs(m_rawY_displacement[i] - in_y) < fabs(m_rawY_displacement[nearestValueY] - in_y)) {
                  nearestValueY = i;
              }
          }
      }

      // new converted pos index displacement
      t_idx newDisplacementIndex = nearestValueY * m_dimX_displacement + nearestValueX;

      // if in computational domain of displacement -> update displacement to value, else displacement = 0
      t_real displacement = 0;
      if (in_x >= smallestX && in_x <= biggestX) {
          if (in_y >= smallestY && in_y <= biggestY) {
              displacement = m_displacement[newDisplacementIndex];
          }
      }

      if (m_bathymetry[newBathymetryIndex] < 0) {
          return !(m_bathymetry[newBathymetryIndex] < -20) ? -20 + displacement : m_bathymetry[newBathymetryIndex] + displacement;
      } else {
          return (m_bathymetry[newBathymetryIndex] < 20) ? 20 + displacement : m_bathymetry[newBathymetryIndex] + displacement;
      }
  }

Visualization of the two-dimensional tsunami event with the netCDF files provided on the website:

.. figure:: ../_static/video_folder/assignment_5/
  :width: 600px



Individual Member Contributions
--------------------------------

Phillip Rothenbeck: configured netCDF library integration, implemented netCDF Input

Marek Sommerfeld: implemented netCDF Output

Moritz Rätz: implemented ArtificialTsunami2d and TsunamiEvent2d setups, project report