#!/bin/bash
#SBATCH --job-name=tsunami
#SBATCH --output=tsunami.output_vtune_mpi
#SBATCH --error=tsunami.error_vtune_mpi
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
mpirun -n 10 vtune -collect hotspots -trace-mpi -r ./vtune_out/r004hs ./build/tsunami_lab chile_1000m.json