#!/bin/bash
#SBATCH --job-name=tsunami
#SBATCH --output=tsunami.output
#SBATCH --error=tsunami.error
#SBATCH --partition=s_hadoop
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --cpus-per-task=72

# Load necessary modules
module load tools/python/3.8
module load compiler/gcc/11.2.0
module load compiler/intel/2020-Update2
python3.8 -m pip install --user scons

date
cd /beegfs/ri26lit/tsunami_lab
scons
./build/tsunami_lab chile_500m.json -t