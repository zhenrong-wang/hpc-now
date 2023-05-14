#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *HDF5-1.10.9* to HPC-NOW cluster.

tmp_log=/tmp/hpcmgr_install.log
if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

echo -e "\n# $time_current SOFTWARE: HDF5-1.10.9"

if [ -f /hpc_apps/hdf5-1.10.9/bin/h5pcc ]; then
  echo -e "[ -INFO- ] It seems HDF5-1.10.9 is already in place (/hpc_apps/hdf5-1.10.9)."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please remove the previous folder and retry. Exit now."
  cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LD_LIBRARY_PATH=/hpc_apps/hdf5-1.10.9/lib:\$LD_LIBRARY_PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export PATH=/hpc_apps/hdf5-1.10.9/bin:\$PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export C_INCLUDE_PATH=/hpc_apps/hdf5-1.10.9/include:\$C_INCLUDE_PATH" >> /etc/profile
  fi
  exit 
fi

if [ ! -f /hpc_apps/zlib-1.2.13/lib/libz.so ]; then
  echo -e "[ -INFO- ] HDF5 requires zlib-1.2.13, which is not found in current cluster. Installing now ... "
  hpcmgr install zlib >> $tmp_log
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING HDF5-1.10.9" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
gcc_version=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_version | awk '{print $3}' | awk -F"." '{print $1}'`

hpcmgr install envmod >> $tmp_log 2>&1

module ava -t | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
  module purge
  module load $mpi_version
  echo -e "[ -INFO- ] HDF5-1.10.9 will be built with $mpi_version."
else
  echo -e "[ -INFO- ] No MPI version found, installing OpenMPI-4.1.2 now..."
  hpcmgr install ompi4 >> ${tmp_log}.ompi
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install OpenMPI-4.1.2. Installation abort. Please check the log file for details. Exit now."
    exit
  else
    echo -e "[ -INFO- ] OpenMPI-4.1.2 has been successfully built."
    mpi_version=ompi-4.1.2
    module purge
    module load $mpi_version
  fi
fi

echo -e "[ -INFO- ] HDF5-1.10.9 will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/hdf5-1.10.9.tar.gz ]; then
  wget ${URL_PKGS}hdf5-1.10.9.tar.gz -q -O /opt/packs/hdf5-1.10.9.tar.gz
fi
rm -rf /opt/packs/hdf5-1.10.9 && tar zxf /opt/packs/hdf5-1.10.9.tar.gz -C /opt/packs/ >> $tmp_log 2>&1
echo -e "[ STEP 1 ] Building HDF5-1.10.9 ... This step usually takes seconds."
#yum -y install zlib-devel >> $tmp_log 2>&1
#cd /opt/packs/hdf5-1.10.9 && make clean >> $tmp_log 2>&1
cd /opt/packs/hdf5-1.10.9 && CXXFLAGS="-I/hpc_apps/$mpi_version/include" LDFLAGS="-L/hpc_apps/$mpi_version/lib -lmpi_cxx" ./configure --prefix=/hpc_apps/hdf5-1.10.9 CC=/hpc_apps/$mpi_version/bin/mpicc --enable-parallel --enable-shared --enable-fortran --enable-cxx --enable-hl --with-zlib=/hpc_apps/zlib-1.2.13 --enable-unsupported >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build HDF5-1.10.9. Please check the log file for more details. Exit now."
  exit
fi
echo -e "[ -INFO- ] HDF5-1.10.9 has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LD_LIBRARY_PATH=/hpc_apps/hdf5-1.10.9/lib:\$LD_LIBRARY_PATH" >> /etc/profile
fi
cat /etc/profile | grep "PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export PATH=/hpc_apps/hdf5-1.10.9/bin:\$PATH" >> /etc/profile
fi
cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/hdf5-1.10.9" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export C_INCLUDE_PATH=/hpc_apps/hdf5-1.10.9/include:\$C_INCLUDE_PATH" >> /etc/profile
fi
source /etc/profile
echo -e "[ -DONE- ] HDF5-1.10.9 has been successfully installed to your cluster." 