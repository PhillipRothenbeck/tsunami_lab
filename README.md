Tsunami Lab
==========================

This is the initial code of the Tsunami Lab taught at Friedrich Schiller University Jena.
Further information is available from: https://scalable.uni-jena.de/opt/tsunami/

Our Website with the project report is available via Github pages: https://philliprothenbeck.github.io/tsunami_lab/

### Requirements
- Python3
- C++11 (or higher)
- SCons
- Doxygen
- git


### Setup Project

Clone the project and initate/update submodules:
```
$ git clone https://github.com/PhillipRothenbeck/tsunami_lab.git
$ cd tsunami_lab
$ git submodule init
$ git submodule update 
```

### Build the Project

The primary building command is:

```
$ scons <mode> <CXX>
```

But there are different flags you can specify to customize the build.
If no flag is set, the respective default value is used.

#### Options for `<mode>`

The `mode` flag can be used to specify the mode in which the project is compiled. 
Depending on the mode, there may be advantages and disadvantages (e.g. easier debugging, but overall much slower).
If no `mode` flag is used, the default value is `mode=release`.

- `release`: build in release mode, default value
- `release+san`: build in release mode and activate sanitizers
- `debug`: build in debug mode
- `debug+san`: build in debug mode and avtivate sanitizers

#### Options for `<CXX>`

The `CXX` flag determines which compiler is used. 
If not further defined, the default value is `CXX=g++`.

- `g++`: GNU C++ Compiler.
- `icpc`: Intel C++ Compiler.

Example with debug mode and intel Compiler:

```
$ scons mode=debug CXX=icpc
```

### Run the Code
Run unit tests from /tsunami_lab/ with:
```
$ ./build/tests
```
Run simulation from /tsunami_lab/ with:
```
$ ./build/tsunami_lab/ <config_file>.json
```
The `config_file.json` argument is used to pass the name of the JSON config file on to the program. 
The config needs to be located in the /tsunami_lab/res/configs/ directory.

#### Structure of a config file

Each argument in the following list can be in a 'config file', but does not have to be. 
If an argument does not appear, the respective default value is used. 
Furthermore, the arguments are not bound to any order, i.e. they can be interchanged. 

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

- :code:`boundaryCond`: string, that defines the boundary conditions (two characters in one dimension, four in two dimensions)

- :code:`setup`: string, that defines the used setup
