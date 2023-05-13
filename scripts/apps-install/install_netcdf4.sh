#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *netCDF-c-4.9.0, netcdf-fortran-4.5.3* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING netCDF-c-4.9.0, netcdf-fortran-4.5.3" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`

echo -e "\n# $time_current SOFTWARE: netCDF-c-4.9.0, netcdf-fortran-4.5.3"

if [[ -f /hpc_apps/netcdf4/bin/nc-config && -f /hpc_apps/netcdf4/bin/nf-config ]]; then
  echo -e "[ -INFO- ] It seems netCDF binaries are already in place (/hpc_apps/netcdf4)."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please remove the previous folder and retry. Exit now.\n"
  cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib:\$LD_LIBRARY_PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "PATH=/hpc_apps/netcdf4/bin" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export PATH=/hpc_apps/netcdf4/bin:\$PATH" >> /etc/profile
  fi
  cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/netcdf4/include" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export C_INCLUDE_PATH=/hpc_apps/netcdf4/include:\$C_INCLUDE_PATH" >> /etc/profile
  fi
  exit 
fi

hpcmgr install envmod >> $tmp_log 2>&1
source /etc/profile

module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] The software will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] The software will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] The software will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile the software. Will start installing gcc-12.1.0 which may take long time."
      echo -e "[ -WARN- ] You can press keyboard 'Ctrl C' to stop current building process."
      echo -ne "[ -WAIT- ] |--> "
      for i in $( seq 1 10)
      do
	      sleep 1
        echo -ne "$((11-i))--> "
      done
      echo -e "|\n[ -INFO- ] Building gcc-12.1.0 now ..."
      hpcmgr install gcc12 >> ${tmp_log}
      gcc_v=gcc-12.1.0
      gcc_vnum=12
      systemgcc='false'
      module load gcc-12.2.0
    fi
  fi
fi

module ava -t | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
  module purge
  module load $mpi_version
  echo -e "[ -INFO- ] netCDF4 will be built with $mpi_version."
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

if [ ! -f /hpc_apps/hdf5-1.10.9/bin/h5pcc ]; then
  echo -e "[ -INFO- ] HDF5 not found. Installing hdf5-1.10.9 now ..."
  hpcmgr install hdf5 >> ${tmp_log}.hdf5
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to build hdf5-1.10.9. Please check the log file for more details. Exit now."
    exit
  fi
fi

yum -y install m4 >> $tmp_log 2>&1
yum -y install libxml2-devel >> $tmp_log 2>&1

echo -e "[ -INFO- ] netCDF-C and netCDF-Fortran will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/netcdf-c-4.9.0.zip ]; then
  wget ${URL_PKGS}netcdf-c-4.9.0.zip -q -O /opt/packs/netcdf-c-4.9.0.zip
fi
if [ ! -f /opt/packs/netcdf-fortran-4.5.3.tar.gz ]; then
  wget ${URL_PKGS}netcdf-fortran-4.5.3.tar.gz -q -O /opt/packs/netcdf-fortran-4.5.3.tar.gz
fi
rm -rf /opt/packs/netcdf-c-4.9.0 && rm -rf /opt/packs/netcdf-fortran-4.5.3
unzip -q -o /opt/packs/netcdf-c-4.9.0.zip -d /opt/packs >> $tmp_log 2>&1
tar zxf /opt/packs/netcdf-fortran-4.5.3.tar.gz -C /opt/packs/ >> $tmp_log 2>&1

echo -e "[ STEP 1 ] Building netCDF-C-4.9.0 & netCDF-fortran-4.5.3 ... This step usually takes seconds."

cd /opt/packs/netcdf-c-4.9.0 && CPPFLAGS='-I/hpc_apps/hdf5-1.10.9/include -I/hpc_apps/zlib-1.2.13/include -I/hpc_apps/'$mpi_version'/include' LDFLAGS='-L/hpc_apps/hdf5-1.10.9/lib -L/hpc_apps/zlib-1.2.13/lib' CC=gcc ./configure --prefix=/hpc_apps/netcdf4 >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build netCDF-C-4.9.0. Please check the log file for more details. Exit now."
  exit
fi
make install >> $tmp_log 2>&1
echo -e "[ -INFO- ] netCDF-C-4.9.0 has been built. Installing netCDF-fortran-4.5.3 now ..."
cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib:\$LD_LIBRARY_PATH" >> /etc/profile
fi
source /etc/profile
cd /opt/packs/netcdf-fortran-4.5.3 && CPPFLAGS='-I/hpc_apps/netcdf4/include' LDFLAGS='-L/hpc_apps/netcdf4/lib' ./configure --prefix=/hpc_apps/netcdf4 >> $tmp_log 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build netCDF-fortran-4.5.3. Please check the log file for more details. Exit now."
  exit
fi
make install >> $tmp_log 2>&1
echo -e "[ -INFO- ] netCDF-C-4.9.0 and netCDF-fortran-4.5.3 has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
cat /etc/profile | grep "LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LD_LIBRARY_PATH=/hpc_apps/netcdf4/lib:\$LD_LIBRARY_PATH" >> /etc/profile
fi
cat /etc/profile | grep "PATH=/hpc_apps/netcdf4/bin" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export PATH=/hpc_apps/netcdf4/bin:\$PATH" >> /etc/profile
fi
cat /etc/profile | grep "C_INCLUDE_PATH=/hpc_apps/netcdf4/include" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export C_INCLUDE_PATH=/hpc_apps/netcdf4/include:\$C_INCLUDE_PATH" >> /etc/profile
fi
source /etc/profile
echo -e "[ -DONE- ] netCDF-C-4.9.0 and netCDF-fortran-4.5.3 has been successfully installed to your cluster." 