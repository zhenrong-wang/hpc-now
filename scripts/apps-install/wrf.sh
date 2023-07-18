#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *WRF & WPS -4.4* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
tmp_log=/tmp/hpcmgr_install.log
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING WRF-4.4" >> ${logfile}

yum -y install cmake cmake3 tcsh >> /dev/null 2>&1 #WRF needs cmake and csh
echo -e "[ -INFO- ] Software: WRF & WPS - Version 4.4."
if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi
if [ -f /hpc_apps/WRF/wrf.exe ]; then
  echo -e "[ -INFO- ] It seems the wrf.exe is already in place (/hpc_apps/WRF/wrf.exe). If you do want to rebuild it, please delete it and retry this command."
  echo -e "[ -INFO- ] Exit now."
  cat /etc/profile | grep "alias wrfenv=" >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias wrfenv='source /hpc_apps/WRF/wrfenv.sh'" >> /etc/profile
  fi
  exit
fi
echo -e "[ -INFO- ] Checking and installing prerequisitions: netCDF & MPI ... "
mkdir -p /opt/packs
hpcmgr install ncdf4 >> $tmp_log
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
  echo -e "[ -INFO- ] WRF will be built with $mpi_version."
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

echo -e "[ -INFO- ] Checking and installing prerequisitions: jasper ..."
if [[ ! -d /hpc_apps/jasper/lib || ! -d /hpc_apps/jasper/include ]]; then
  if [ ! -f /opt/packs/jasper-1.900.1.tar.gz ]; then
    wget ${URL_PKGS}jasper-1.900.1.tar.gz -O /opt/packs/jasper-1.900.1.tar.gz -q
  fi
  cd /opt/packs && rm -rf jasper-1.900.1 && tar zxf jasper-1.900.1.tar.gz
  cd /opt/packs/jasper-1.900.1 && ./configure --prefix=/hpc_apps/jasper >> $tmp_log 2>&1
  make -j4 >> $tmp_log 2>&1
  make install >> $tmp_log 2>&1
fi

echo -e "[ STEP 1 ] Downloading WRF source code packages ... "
if [ ! -f /opt/packs/wrf-latest.tar.gz ]; then
  wget ${URL_PKGS}wrf-latest.tar.gz -O /opt/packs/wrf-latest.tar.gz -q
fi
echo -e "[ STEP 2 ] Extracting WRF source code tarball ..."
cd /opt/packs && rm -rf /opt/packs/WRF && tar zxf wrf-latest.tar.gz
cd /opt/packs/WRF
export NETCDF=/hpc_apps/netcdf4
export NETCDF_classic=1
export HDF5=/hpc_apps/hdf5-1.10.9
export PHDF5=/hpc_apps/hdf5-1.10.9
export JASPERLIB=/hpc_apps/jasper/lib/
export JASPERINC=/hpc_apps/jasper/include/

echo -e "[ STEP 3 ] Configuring and building WRF from the source code ..."
echo 35 | ./configure >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS wrf >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_b_wave >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_convrad >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_esmf_exp >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_fire >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_grav2d_x >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_heldsuarez >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_hill2d_x >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_les >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_quarter_ss >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_real >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_scm_xy >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_seabreeze2d_x >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_squall2d_x >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_squall2d_y >> $tmp_log 2>&1
./compile -j $NUM_PROCESSORS em_tropical_cyclone >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build WRF. Please check the log file for details. Exit now."
  exit
fi
mkdir -p /hpc_apps/WRF
cp -r /opt/packs/WRF/main/*.exe /hpc_apps/WRF/
if [ $systemgcc = 'false' ]; then
  echo -e "#! /bin/bash\nmodule purge\nmodule load $gcc_v\nmodule load $mpi_version\nexport PATH=/hpc_apps/WRF:\$PATH" > /hpc_apps/WRF/wrfenv.sh
else
  echo -e "#! /bin/bash\nmodule purge\nmodule load $mpi_version\nexport PATH=/hpc_apps/WRF:\$PATH" > /hpc_apps/WRF/wrfenv.sh
fi
cat /etc/profile | grep "alias wrfenv=" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias wrfenv='source /hpc_apps/WRF/wrfenv.sh'" >> /etc/profile
fi

echo -e "[ STEP 4 ] Building WPS now ..."
if [ ! -f /opt/packs/wps-latest.tar.gz ]; then
  wget ${URL_PKGS}wps-latest.tar.gz -O /opt/packs/wps-latest.tar.gz -q
fi
cd /opt/packs/ && rm -rf WPS && tar zxf wps-latest.tar.gz
cd /opt/packs/WPS
sed -i 's# NETCDFF=" "# NETCDFF="-lnetcdff -lgomp"#' configure # In order to build WPS
echo 3 | ./configure >> $tmp_log 2>&1
cp configure.wps configure.wps.bkup
sed -i 's@-L$(NETCDF)/lib -lnetcdff -lnetcdf@-L$(NETCDF)/lib -lnetcdff -lnetcdf -fopenmp@g' configure.wps # In order to build WPS
./compile >> $tmp_log 2>&1

if [ $? -ne 0 ]; then
  echo -e "[ -WARN- ] WRF has been successfully built, but WPS encountered some problems. Exit now."
  echo -e "WRF with $gcc_v & $mpi_version is ready for running." >> /hpc_apps/WRF/wrfenv.sh
  exit
fi
cp -r /opt/packs/WPS /hpc_apps/
echo -e "export PATH=/hpc_apps/WPS:/hpc_apps/WPS/util:\$PATH\necho -e \"WRF & WPS with $mpi_version and $gcc_v are ready for running.\"" >> /hpc_apps/WRF/wrfenv.sh

echo -e "[ -DONE- ] Congratulations! WRF & WPS with $mpi_version and $gcc_v has been built."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current Congratulations! WRF & WPS with $mpi_version and $gcc_v has been built." >> $logfile