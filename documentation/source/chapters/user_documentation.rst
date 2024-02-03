.. _User_Documentation:

User Documentation
==================

Welcome to the Tsunami Lab project! 
This quick start guide will show you how to set up and use the tsunami simulator.

Getting Started
---------------
Depending on which version of the Tsunami Simulator you are using (normal or MPI-parallelized solver), 
there are differences in how the solver is started and used. 
If the normal version of the solver is used, please refer to `running the normal version`_ on how to run it.
Otherwise please refer to `running the mpi version`_ on how to run the solver with open MPI.

Prerequisites
^^^^^^^^^^^^^^^
Before you can begin, ensure that you have the following prerequisites installed and set up on your system:

* Python3
* C++11 (or higher)
* SCons
* Doxygen
* git

Downloading the Project
^^^^^^^^^^^^^^^^^^^^^^^

#. Open your terminal or command prompt.

#. Change to the directory in which the project is to be saved.

#. Run the following command to clone the project repository from GitHub:

    .. code-block:: bash
        
        git clone https://github.com/PhillipRothenbeck/tsunami_lab.git

#. Download and Update all submodules

    .. code-block:: bash
        
        cd tsunami_lab
        git submodules init
        git submodules update.


Compiling Doxygen
^^^^^^^^^^^^^^^^^

#. Open your terminal or command prompt.

#. Navigate to the project's root directory where the Doxygen configuration file (Doxyfile.in) is located.

#. Run the following command to generate the documentation:

    .. code-block::

        doxygen Doxyfile.in

#. Once the documentation generation is complete, you can open the documentation in your browser of choice by opening the following URL:

    .. code-block::

        file:///path/to/tsunami-lab-project/_build/html/index.html


Building the Project
---------------------

To compile the Tsunami Lab Project, you have various options and flags to choose from. 
The primary building command is :code:`scons`, and you can specify different flags to customize the build.
If no flag is set, the respective default value is used.

**mode:**

The :code:`mode` flag determines the mode in which the project is compiled. 
Depending on the mode, there may be advantages and disadvantages (e.g. easier debugging, but overall much slower).

If no :code:`mode` flag is used, the default value is :code:`mode=release`.

#. :code:`release`: Optimized release mode.
#. :code:`release+san`: Release mode with sanitizers.
#. :code:`debug`: Debug mode.
#. :code:`debug+san`: Debug mode with sanitizers.

**CXX:**

The :code:`CXX` flag can be used to specify which compiler is used. 
If not further defined, the default value is :code:`CXX=g++`.

#. :code:`g++`: GNU C++ Compiler.
#. :code:`icpc`: Intel C++ Compiler.

To build the project with default values, navigate to the project's root directory and run the following command:

.. code-block::

    scons


To compile the project with a specific mode and compiler, use the mode/CXX flag as follows:

.. code-block::
 
    scons mode=release CXX=g++


.. _ch:Running_the_project:

Running the Project
-------------------
When running the project, several flags can be used to influence the behavior of the solver. 
There is no predetermined order and they can all be used at the same time.

#. :code:`-t`: Activate Time Measurement.
#. :code:`-nio`: Deactivate I/O Output.

.. _running the normal version:

Running the normal version
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Make sure that the current terminal is located in the /tsunami_lab/ directory.

To execute the test files, use the following command:

.. code-block::

    ./build/tests

To execute the project, use the following command with the appropriate flags:

.. code-block::

    ./build/tsunami_lab <config_file.json>

The :code:`config_file.json` argument is used to pass the name of the JSON config 
file on to the program. The config needs to be located in the /tsunami_lab/res/configs/ 
directory.

.. _running the mpi version:

Running the MPI-parallelized version
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When running the MPI version, you must use a batch script.
In this script, computing time must be allocated on a compute node, the project must be built, compiled and then run.

Example batch script tsunami_lab_mpi.sh:

.. code-block:: c++

    #!/bin/bash
    #SBATCH --job-name=tsunami
    #SBATCH --output=tsunami.out
    #SBATCH --error=tsunami.err
    #SBATCH --partition=s_hadoop
    #SBATCH --nodes=1
    #SBATCH --ntasks=1
    #SBATCH --time=10:00:00
    #SBATCH --cpus-per-task=72

    # Load necessary modules
    module load tools/python/3.8
    module load compiler/gcc/11.2.0
    module load compiler/intel/2020-Update2
    module load mpi/openmpi/4.1.2-gcc-10.2.0
    python3.8 -m pip install --user scons

    date
    cd /beegfs/ri26lit/tsunami_lab
    scons CXX=mpic++
    mpirun -n 5 ./build/tsunami_lab chile_10000m.json -t


By default, a batch script named :code:`tsunami_lab_mpi.sh` should be located in the tsunami_lab/scripts directory.

Execute it with the following command in the terminal while you are in the said directory:

.. code-block::

    sbatch tsunami_lab_mpi.sh


The :code:`config_file.json` argument is used to pass the name of the JSON config 
file on to the program. The config needs to be located in the /tsunami_lab/res/configs/ 
directory.


Structure of a config file
^^^^^^^^^^^^^^^^^^^^^^^^^^

**List of arguments:** 

- :code:`dimension`: integer, that indicates the dimensions of a simulation

- :code:`nx`: integer, that defines the number of cells in x-direction

- :code:`ny`: integer, that defines the number of cells in y-direction

- :code:`xLen`: float, that defines the length in x-direction 

- :code:`yLen`: float, that defines the length in y-direction 

- :code:`epicenterOffsetX`: float, that defines the Offset of the computational domain in x-direction

- :code:`epicenterOffsetY`: float, that defines the Offset of the computational domain in y-direction

- :code:`bathymetryFileName`: string, that defines the name of the bathymetry input file

- :code:`displacementFileName`: string, that defines the name of the displacement input file

- :code:`simTime`: float, duration of a simulation (not wall time)

- :code:`boundaryCond`: string, that defines the boundary conditions

There are some differences between one and two dimensional simulations:

* 1d: "OO", "RR", "RO", "OR"; 
* 2d: "OOOO", "ROOO", "OROO", "OORO" (...) 

(each letter represents a direction, in order: East, North, West, South)
if "OO" is entered for 2d, it is filled with "OOOO" as the default value ("RR" = "RROO")

- :code:`setup`: string, that defines the used setup

Depending on the dimensionality, different setups can be used:

* 1d: DamBreak, RareRare, ShockShock, SubciriticalFlow, SupercriticalFlow, TunsamiEvent, Sanitize, CustomSetup

* 2d: DamBreak, ArtificialTsunamiEvent, TsunamiEvent


.. _ch:Troubleshooting:

Troubleshooting
---------------

If you encounter any issues while working with the Tsunami Lab Project, please refer to the following common issues and contact information for assistance.

Common issues
^^^^^^^^^^^^^

* Compilation Errors:
    If you encounter compilation errors, ensure that you have all the necessary prerequisites installed, as mentioned in the Prerequisites section. Verify that you have the correct versions of Python, C++, SCons, CMake, and Doxygen.

    Double-check the compilation flags and options specified in the Compiling the Code section.

* Doxygen Documentation:
    If you face issues generating Doxygen documentation, make sure you have executed the correct command mentioned in the Compiling Doxygen section. Ensure that the Doxyfile.in is present in the project directory.

* Running the Project:
    If you encounter errors while running the project, ensure that you have provided the correct command-line flags as explained in the Compiling the Project section. Check for any typos or incorrect inputs.
    Verify that the required input files (e.g., Middle_states or dummy_middle_states) are present in the designated folders as mentioned in the project flags.

Contact Information
^^^^^^^^^^^^^^^^^^^

If you are unable to resolve your issue or face any other difficulties not mentioned here, please feel free to contact the Tsunami Lab Project maintainers for assistance. You can reach out to us at the following email addresses:

* Phillip Rothenbeck: phillip.rothenbeck@uni-jena.de

* Marek Sommerfeld: marek.sommerfeld@uni-jena.de

* Moritz Rätz: moritz.raetz@uni-jena.de

We are here to help you with any questions or problems you may encounter while using the Tsunami Lab Project. Please don't hesitate to reach out, and we will do our best to assist you.