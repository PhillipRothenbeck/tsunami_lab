.. _ch:Task_7:

Week Report 7
=============

Checkpoints
-----------

In order for the checkpoints to fulfill their purpose, they must contain certain information. 

In our case, this information is:

- all configurations made in the old config.json
- h, hu and hv at the last timestep
- how long the simulation has been running

The name of the old config file is given through ... then the simulation is set up normal und so

All other parameters are written to the checkpoint file as an array (just like in the simulation). In this way, we can load the values directly from the file into our array when we set up the new simulation.
In the same way, the corresponding timestep is stored.

Since the solver is started by specifying a config file, ...

.. code-block:: c++

    Implementation of checkpoint in ConfigLoader

.. code-block:: c++

    Implementation of checkpoint in NetCDF

We have decided that checkpoints should be set at individually definable intervals. The interval can be set in the config file.

We do not delete old checkpoint files using the code, as this is eventually done by hand.

Coarse Output
-------------

To obtain a coarse output, several cells in a :math:`k \times k` square must be combined into one cell. This works by iterating over the grid with a :math:`k \times k` filter. (similar to a blur filter in image processing)
But it raises the question of what happens to cells that do not exist because they are outside the domain. 
If :math:`k = 5` then there are not enough ghost cells at the edge to combine the cells. In this case ... 

.. code-block:: c++

    Implementation of coarse output

Simulation of 2011 Tohoku with coarse Output
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Simulation of the 2011 Tohoku M 9.1 Tsunami Event is started with a grid size of 50m and a coarse Output modifer :math:`k` of ...

.. code-block:: c++

    Tohoku config file bitte danke


Visualization of Tohoku 2011

.. video:: ../_static/assignment_7/
  :autoplay:
  :loop:
  :height: 300
  :width: 650


Individual Member Contributions
--------------------------------

This week LeChuck did all the work, he deserves most of the credit.