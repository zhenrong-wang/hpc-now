#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *ScaLAPACK-Latest* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

tmp_log=/tmp/hpcmgr_install.log

if [[ ! -f /hpc_apps/lapack-3.11/libcblas.a || ! -f /hpc_apps/lapack-3.11/liblapack.a ]]; then
  echo -e "[ FATAL: ] You need to build LAPACK in advance. Please exit and run 'hpcmgr install lapk311' command and retry. Exit now."
  exit
fi
module ava -t | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
  module purge
  module load $mpi_version
  echo -e "[ -INFO- ] ScaLAPACK will be built with $mpi_version."
else
  module ava -t | grep mpich >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mpi_version=`module ava -t | grep mpich | tail -n1 | awk '{print $1}'`
    module purge
    module load $mpi_version
    echo -e "[ -INFO- ] ScaLAPACK will be built with $mpi_version. However, we recommend OpenMPI for compiling ScaLAPACK."
  else
    echo -e "[ FATAL: ] No MPI version found. Please install mpi first. You can run 'hpcmgr install mpich3/mpich4/ompi3/ompi4' and retry. Exit now."
    exit
  fi
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING ScaLAPACK-Latest" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
gcc_version=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_version | awk '{print $3}' | awk -F"." '{print $1}'`

echo -e "\n# $time_current SOFTWARE: ScaLAPACK-Latest"

if [ -f /hpc_apps/scalapack/libscalapack.a ]; then
  echo -e "[ -INFO- ] It seems ScaLAPACK-Latest libraries are already in place (/hpc_apps/scalapack)."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please remove the previous libraries and retry. Exit now."
  cat /etc/profile | grep "LIBRARY_PATH=/hpc_apps/scalapack" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LIBRARY_PATH=/hpc_apps/scalapack:\$LIBRARY_PATH" >> /etc/profile
  fi
  exit 
fi
echo -e "[ -INFO- ] ScaLAPACK-Latest will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/scalapack-master.zip ]; then
  wget ${URL_PKGS}scalapack-master.zip -q -O /opt/packs/scalapack-master.zip
fi
unzip /opt/packs/scalapack-master.zip -d /hpc_apps/ >> $tmp_log 2>&1
mv /hpc_apps/scalapack-master /hpc_apps/scalapack
echo -e "[ STEP 1 ] Building ScaLAPACK ... This step usually takes seconds."
cd /hpc_apps/scalapack && /bin/cp SLmake.inc.example SLmake.inc
if [ $gcc_vnum -gt 10 ]; then
  sed -i 's@FCFLAGS       = -O3@FCFLAGS       = -O3 -fallow-argument-mismatch -fPIC@g' /hpc_apps/scalapack/SLmake.inc
else
  sed -i 's@FCFLAGS       = -O3@FCFLAGS       = -O3 -fPIC@g' /hpc_apps/scalapack/SLmake.inc
fi
sed -i 's@BLASLIB       = -lblas@BLASLIB       = -L/hpc_apps/lapack-3.11 -lrefblas -lcblas@g' /hpc_apps/scalapack/SLmake.inc
sed -i 's@LAPACKLIB     = -llapack@LAPACKLIB     = -L/hpc_apps/lapack-3.11 -llapack.a -llapacke -ltmglib@g' /hpc_apps/scalapack/SLmake.inc
cd /hpc_apps/scalapack/ && make lib >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ScaLAPACK. Please check the log file for more details. Exit now."
  exit
fi
echo -e "[ -INFO- ] ScaLAPACK-Latest has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
cat /etc/profile | grep "LIBRARY_PATH=/hpc_apps/scalapack" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LIBRARY_PATH=/hpc_apps/scalapack:\$LIBRARY_PATH" >> /etc/profile
fi
source /etc/profile
echo -e "[ -DONE- ] ScaLAPACK-Latest has been successfully installed to your cluster." 