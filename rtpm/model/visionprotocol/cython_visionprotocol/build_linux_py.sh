#!/bin/bash
script_dir="$(dirname "$0")"
cd $script_dir 
ls $pwd

echo "========================================================================================="
echo "======>>>>  2. Build cPython-x86_64-linux-gnu.so "
echo "========================================================================================="
sleep 1

cp ../build/linux-x86_64/libvisionprotocol.so* .

# Build cPython-x86_64-linux-gnu.so
source ~/miniforge3/etc/profile.d/conda.sh

conda activate py38
python setup_linux.py build_ext --inplace
conda deactivate
echo "========================================================================================="
sleep 0.5

conda activate py39
python setup_linux.py build_ext --inplace
conda deactivate
echo "========================================================================================="
sleep 0.5

conda activate py310
python setup_linux.py build_ext --inplace
conda deactivate
echo "========================================================================================="
sleep 0.5

conda activate py311
python setup_linux.py build_ext --inplace
conda deactivate
echo "========================================================================================="
sleep 0.5

conda activate py312
python setup_linux.py build_ext --inplace
conda deactivate
echo "========================================================================================="
sleep 0.5
