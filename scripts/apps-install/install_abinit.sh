#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *ABINIT-9.6.2* to HPC-NOW cluster.

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING ABINIT" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
tmp_log=/tmp/hpcmgr_install.log

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

echo -e "[ -INFO- ] Software: ABINIT-9.6.2"

mkdir -p $APP_ROOT/abinit
hpcmgr install envmod >> $tmp_log 2>&1
source /etc/profile

if [ -f $APP_ROOT/abinit/build/bin/abinit ]; then
  echo -e "[ -INFO- ] It seems ABINIT binaries are in place."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please move the previous binaries to other folders and retry. Exit now.\n"
  cat /etc/profile | grep abinit >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias abinitenv='source ${APP_ROOT}/abinit/abinit_env.sh'" >> /etc/profile
  fi
fi

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: GNU Compiler Collections - GCC version ..."
module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] ABINIT-9.6.2 will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] ABINIT-9.6.2 will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] ABINIT-9.6.2 will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile ABINIT-9.6.2. Will start installing gcc-12.1.0 which may take long time."
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

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: MPI versions ..."
module ava -t | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
  module purge
  module load $mpi_version
  echo -e "[ -INFO- ] ABINIT-9.6.2 will be built with $mpi_version."
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
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: OpenBLAS Libraries ..."
hpcmgr install oblas >> $tmp_log 2>&1
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: libXC libraries ..."
if [ ! -f $APP_ROOT/libxc-4.3.4/lib/libxcf90.la ]; then
  if [ ! -f /opt/packs/libxc-4.3.0.tar.gz ]; then
    wget ${URL_PKGS}libxc-4.3.4.tar.gz -O /opt/packs/libxc-4.3.4.tar.gz -q
  fi
  cd /opt/packs/ && rm -rf libxc-4.3.4 && tar zxf libxc-4.3.4.tar.gz && cd libxc-4.3.4
  ./configure --prefix=$APP_ROOT/libxc-4.3.4 >> $tmp_log 2>&1
  make -j$NUM_PROCESSORS >> $tmp_log 2>&1 && make install >> $tmp_log 2>&1
fi

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: FFTW-3.3.8 Libraries ..."
if [[ ! -f $APP_ROOT/abinit/fftw-3.3.8/lib/libfftw3.a || ! -f $APP_ROOT/abinit/fftw-3.3.8/lib/libfftw3_mpi.la ]]; then
  if [ ! -f /opt/packs/fftw-3.3.8.tar.gz ]; then
    wget ${URL_PKGS}fftw-3.3.8.tar.gz -O /opt/packs/fftw-3.3.8.tar.gz -q
  fi
  cd /opt/packs && rm -rf fftw-3.3.8 && tar zxf fftw-3.3.8.tar.gz && cd fftw-3.3.8
  ./configure --prefix=$APP_ROOT/abinit/fftw-3.3.8 --enable-single --enable-mpi --enable-threads --enable-shared >> $tmp_log 2>&1
  make -j$NUM_PROCESSORS >> $tmp_log 2>&1
  make install >> $tmp_log 2>&1

  cd /opt/packs/fftw-3.3.8
  ./configure --prefix=$APP_ROOT/abinit/fftw-3.3.8 --enable-mpi --enable-threads --enable-shared >> $tmp_log 2>&1 
  make -j$NUM_PROCESSORS >> $tmp_log 2>&1
  make install >> $tmp_log 2>&1
fi
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: HDF5 ..."
hpcmgr install hdf5 >> $tmp_log 2>&1
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: netCDF4 ..."
hpcmgr install ncdf4 >> $tmp_log 2>&1
pip install configurator >> $tmp_log 2>&1

echo -e "[ -INFO- ] Now, finally, we can start building ABINIT-9.6.2 ..."
echo -e "[ STEP 1 ] Downloading and extracing source code ..."
if [ ! -f /opt/packs/abinit-9.6.2.tar.gz ]; then
  wget ${URL_PKGS}abinit-9.6.2.tar.gz -O /opt/packs/abinit-9.6.2.tar.gz -q
fi

if [ $systemgcc = 'false' ]; then
  module load $gcc_v
fi

echo -e "[ STEP 2 ] Configuring and Compiling in progress, this step usually takes minutes ..."
cd /opt/packs && rm -rf abinit-9.6.2 && tar zxf abinit-9.6.2.tar.gz
rm -rf $APP_ROOT/abinit/build && cd abinit-9.6.2 && mkdir $APP_ROOT/abinit/build
FC_LDFLAGS_EXTRA="-L${APP_ROOT}/openblas/lib -lopenblas -L${APP_ROOT}/abinit/fftw-3.3.8/lib -lfftw3_mpi -lfftw3f_mpi -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads" ./configure --prefix=$APP_ROOT/abinit/build --with-mpi=$APP_ROOT/$mpi_version --with-hdf5=$APP_ROOT/hdf5-1.10.9 --with-fftw3=$APP_ROOT/abinit/fftw-3.3.8 --with-libxc=$APP_ROOT/libxc-4.3.4 --with-netcdf=$APP_ROOT/netcdf4 >> ${tmp_log} 2>&1
make -j$NUM_PROCESSORS >> $tmp_log 2>&1 && make install >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ABINIT. Please check the log file for details. Exit now."
  exit
else
  echo -e "[ -DONE- ] ABINIT-9.6.2 has been sucessfully built to your cluster. You need to run command 'abinitenv' before using it."
  if [ $systemgcc = 'true' ]; then
    echo -e "# !/bin/bash\nmodule purge\nmodule load ${mpi_version}\nexport LD_LIBRARY_PATH=${APP_ROOT}/netcdf4/lib:${APP_ROOT}/abinit/fftw-3.3.8/lib:${APP_ROOT}/openblas/lib:\$LD_LIBRARY_PATH\nexport PATH=${APP_ROOT}/abinit/build/bin:\$PATH\necho -e \"ABINIT-9.6.2 with ${mpi_version} is ready for running.\"" > $APP_ROOT/abinit/abinit_env.sh
  else
    echo -e "# !/bin/bash\nmodule purge\nmodule load ${gcc_v}\nmodule load ${mpi_version}\nexport LD_LIBRARY_PATH=${APP_ROOT}/netcdf4/lib:${APP_ROOT}/abinit/fftw-3.3.8/lib:${APP_ROOT}/openblas/lib:\$LD_LIBRARY_PATH\nexport PATH=${APP_ROOT}/abinit/build/bin:\$PATH\necho -e \"ABINIT-9.6.2 with ${mpi_version} is ready for running.\"" > $APP_ROOT/abinit/abinit_env.sh
  fi
  cat /etc/profile | grep abinit >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias abinitenv='source ${APP_ROOT}/abinit/abinit_env.sh'" >> /etc/profile
  fi
fi