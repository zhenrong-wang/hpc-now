#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Gromacs* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

#Make the packs path
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING GROMACS" >> ${logfile}
echo -e "[ -INFO- ] INSTALLING GROMACS-version 2022-2 now."
tmp_log=/tmp/hpcmgr_install.log
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
yum -y install python3-devel >> ${tmp_log} 2>&1

if [ -f $APP_ROOT/gromacs2022/bin/gmx_mpi_d ]; then
  echo -e "[ FATAL: ] It seems GROMACS mpi version is already in place. If you do want to rebuild it, please emtpy $APP_ROOT/gromacs2022/ folder and retry. Exit now."
  exit
fi

echo -e "[ -INFO- ] By *default*, GROMACS will be built with fftw3. if you'd like to build with MKL or its own fftpack, please contact us."
echo -e "[ -INFO- ] If you'd like to build your own version of GROMACS, please contact us by email info@hpc-now.com, or contact your technical support."
echo -e "[ -INFO- ] You can also go to the source code in /opt/packs/gromacs and build as you preferred. But this is not recommended."
echo -e "[ -INFO- ] Checking the Compilers now ..."
hpcmgr install envmod >> $tmp_log 2>&1
module ava -t | grep mpich >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep mpich | tail -n1 | awk '{print $1}'`
  module load $mpi_version
  echo -e "[ -INFO- ] GROMACS will be built with $mpi_version."
else
  module ava -t | grep ompi >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
    module load $mpi_version
    echo -e "[ -INFO- ] GROMACS will be built with $mpi_version. However, we recommend MPICH-4 for compiling GROMACS."
  else
    echo -e "[ -WARN- ] No MPI version found. Installing MPICH-4 ..."
    hpcmgr install mpich4 >> $tmp_log 2>&1
    mpi_version=mpich-4.0.2
    module load $mpi_version
  fi
fi

module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] GROMACS will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] GROMACS will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] GROMACS will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile Gromacs. Will start installing gcc-12.1.0 which may take long time."
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

CENTOS_VER=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ $CENTOS_VER -eq 7 ]; then
  echo -e "[ -INFO- ] CentOS Version 7.x detected. Installing cmake3 & Python 3.8 now ..."
  yum -y install cmake3 -q >> $tmp_log 2>&1
  rm -rf /bin/cmake
  ln -s /bin/cmake3 /bin/cmake
  ls -ll /bin/python3 | grep python3.8 >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    if [ ! -f /opt/packs/Python-3.8.9.tar.xz ]; then
      wget ${URL_PKGS}Python-3.8.9.tar.xz -O /opt/packs/Python-3.8.9.tar.xz -q
    fi
    rm -rf /opt/packs/Python-3.8.9
    tar xf /opt/packs/Python-3.8.9.tar.xz -C /opt/packs
    cd /opt/packs/Python-3.8.9 && ./configure --prefix=/hpc_apps/python-3.8.9 >> $tmp_log 2>&1
    make -j $NUM_PROCESSORS >> $tmp_log 2>&1 && make install >> $tmp_log 2>&1
    if [ $? -ne 0 ]; then
      ehco -e  "[ FATAL: ] Failed to install Python-3.8.9. Exit now."
      exit
    fi
    cat /etc/profile | grep "/hpc_apps/python-3.8.9/bin" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "export PATH=/hpc_apps/python-3.8.9/bin:\$PATH" >> /etc/profile
    fi
    cat /etc/profile | grep "/hpc_apps/python-3.8.9/lib" >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "export LD_LIBRARY_PATH=/hpc_apps/python-3.8.9/lib:\$LD_LIBRARY_PATH" >> /etc/profile
    fi
    rm -rf /bin/python3 && ln -s /hpc_apps/python-3.8.9/bin/python3.8 /bin/python3
    source /etc/profile
  fi
fi

if [ ! -f $APP_ROOT/fftw3/bin/fftw-wisdom ]; then
  echo -e "[ -INFO- ] Building fftw3 now ... "
  rm -rf /hpc_apps/fftw3
  if [ ! -f /opt/packs/fftw-3.3.10.tar.gz ]; then
    wget ${URL_PKGS}fftw-3.3.10.tar.gz -q -O /opt/packs/fftw-3.3.10.tar.gz
  fi
  cd /opt/packs && tar zxf fftw-3.3.10.tar.gz
  cd /opt/packs/fftw-3.3.10 && ./configure --prefix=$APP_ROOT/fftw3 --enable-sse2 --enable-avx --enable-avx2 --enable-shared >> $tmp_log 2>${logfile}
  make -j$NUM_PROCESSORS >> $tmp_log 2>${logfile}
  make install >> $tmp_log 2>${logfile}
else
  echo -e "[ -INFO- ] fftw3 is already in place."
fi

echo -e "[ -INFO- ] Downloading and extracting source files ..."
if [ ! -f /opt/packs/gromacs-2022.2.tar.gz ]; then
  wget ${URL_PKGS}gromacs-2022.2.tar.gz -O /opt/packs/gromacs-2022.2.tar.gz -q
fi
rm -rf /opt/packs/gromacs-2022.2
cd /opt/packs && tar zxf gromacs-2022.2.tar.gz
echo -e "[ -INFO- ] Building GROMACS-2022 to $APP_ROOT/gromacs2022 now...This step may take minutes."
cd /opt/packs/gromacs-2022.2 && mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGMX_MPI=on -DCMAKE_INSTALL_PREFIX=/hpc_apps/gromacs2022 -DGMX_FFT_LIBRARY=fftw3 -DFFTW_LIBRARY=/hpc_apps/fftw3/lib/libfftw3.a -DFFTW_INCLUDE_DIR=/hpc_apps/fftw3/include -DGMX_DOUBLE=on >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] CMake processing error. Exit now."
  exit
fi
make -j $NUM_PROCESSORS >> ${tmp_log} 2>${logfile}
make install >> ${tmp_log} 2>${logfile}
cat /etc/profile | grep "alias gmxenv" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias gmxenv='source $APP_ROOT/gromacs2022/bin/GMXRC'" >> /etc/profile
fi  
echo -e "[ -DONE- ] Congratulations! GROMACS-2022-2 has been build to $APP_ROOT/gromacs2022. Please run 'gmxenv' command to load the environment before using GROMACS."