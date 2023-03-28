#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Lammps* to HPC-NOW cluster.

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
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING Lammps" >> ${logfile}
echo -e "INSTALLING Lammps now."
tmp_log=/tmp/hpcmgr_install.log
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
if [ -d $APP_ROOT/lammps ]; then
  flag=`ls -ll $APP_ROOT/lammps | grep lmp | wc -l`
  if [ $flag -gt $((0)) ]; then
    echo -e "[ -WARN- ] There is(are) already binary(ies) in $APP_ROOT/lammps. Previously built binary(ies) might be override."
    echo -e "[ -WARN- ] You can press keyboard 'Ctrl C' to stop current building process"
    echo -ne "[ -WAIT- ] | "
    for i in $( seq 1 10)
    do
      sleep 1
      echo -ne "> $((11-i)) "
    done
    echo -e "> |"
  fi
fi
echo -e "[ -INFO- ] By *default*, Lammps will be built with 'yes-most', g++ and the latest version of MPICH found in the cluster."
echo -e "[ -INFO- ] If you'd like to build your own version of Lammps, please contact us by email info@hpc-now.com, or contact your technical support."
echo -e "[ -INFO- ] You can also go to the source code in $APP_ROOT/lammps-develop and make as you preferred. But this is not recommended."
echo -e "[ -INFO- ] Checking the Compilers now ..."
hpcmgr install envmod >> $tmp_log 2>&1
module ava -t | grep mpich >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep mpich | tail -n1 | awk '{print $1}'`
  module load $mpi_version
  echo -e "[ -INFO- ] Lammps will be built with $mpi_version."
else
  echo -e "[ -INFO- ] No MPICH version found. Installing MPICH-4.0.2 now ..."
  hpcmgr install mpich4 >> $tmp_log 2>&1
  mpi_version=mpich-4.0.2
  module load $mpi_version
fi

module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] Lammps will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] Lammps will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] Lammps will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile Lammps. Will start installing gcc-12.1.0 which may take long time."
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
      module load gcc-12.1.0
    fi
  fi
fi

# Install fftw3 if not found in the cluster
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

# Install adios2 now
if [ ! -f /usr/local/bin/adios2-config ]; then
  echo -e "[ -INFO- ] Building adios2 now ... "
  if [ ! -f /opt/packs/ADIOS2-master.zip ]; then
    wget ${URL_PKGS}ADIOS2-master.zip -q -O /opt/packs/ADIOS2-master.zip
  fi
  cd /opt/packs && unzip -q -o ADIOS2-master.zip && rm -rf ADIOS2 && mv ADIOS2-master ADIOS2
  mkdir -p adios2-build && cd adios2-build
  cmake ../ADIOS2 >> $tmp_log 2>${logfile}
  make -j$NUM_PROCESSORS >> $tmp_log 2>${logfile}
  make install >> $tmp_log 2>${logfile}
else
  echo -e "[ -INFO- ] ADIOS2 is already in place."
fi

echo -e "[ -INFO- ] Downloading and extracting source files ..."
if [ ! -f /opt/packs/lammps.zip ]; then
  wget ${URL_PKGS}lammps-develop.zip -O /opt/packs/lammps.zip -q
fi
cd /opt/packs && unzip -o -q lammps.zip
if [ ! -f /opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig ]; then 
  /bin/cp /opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich /opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig
else
  /bin/cp /opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig /opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich
fi

echo -e "[ -INFO- ] Optimizing compile configurations ..."
MAKEFILE_G_MPICH=/opt/packs/lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich
cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC"  >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  cpu_model=`cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC" | head -n1`
  cpu_gen=${cpu_model: -1}
  if [ $cpu_gen = '3' ]; then
    sed -i 's/-g -O3/-g -O3 -march=znver3/g' $MAKEFILE_G_MPICH
  elif [ $cpu_gen = '2' ]; then
    sed -i 's/-g -O3/-g -O3 -march=znver2/g' $MAKEFILE_G_MPICH
  fi
fi
sed -i "s@MPI_INC = @MPI_INC = -I$APP_ROOT/$mpi_version/include@g" $MAKEFILE_G_MPICH
sed -i "s@MPI_PATH =@MPI_PATH = -L$APP_ROOT/$mpi_version/lib@g" $MAKEFILE_G_MPICH
sed -i "s@MPI_LIB =@MPI_LIB = -lmpi -lmpich -lmpicxx@g" $MAKEFILE_G_MPICH
sed -i "s@FFT_INC =@FFT_INC = -I$APP_ROOT/fftw3/include@g" $MAKEFILE_G_MPICH
sed -i "s@FFT_PATH =@FFT_PATH = -L$APP_ROOT/fftw3/lib@g" $MAKEFILE_G_MPICH
sed -i "s@FFT_LIB =@FFT_LIB = -lfftw3@g" $MAKEFILE_G_MPICH

echo -e "[ -INFO- ] Start compiling Lammps: lmp_g++_mpich now, this step may take minutes ..."
cd  /opt/packs/lammps-develop/src
make yes-most >> $tmp_log 2>${logfile}
make -j$NUM_PROCESSORS g++_mpich >> $tmp_log 2>${logfile}
if [ ! -d $APP_ROOT/lammps ]; then
  mkdir -p $APP_ROOT/lammps
fi
/bin/cp /opt/packs/lammps-develop/src/lmp_g++_mpich $APP_ROOT/lammps/

if [ $systemgcc = 'true' ]; then
  echo -e "#! /bin/bash\nmodule purge\nmodule load $mpi_version\nmodule load lammps\nmodule load fftw3\necho -e \"Lammps is ready for running.\"" > $APP_ROOT/lammps/lammps_environment.sh
else
  echo -e "#! /bin/bash\nmodule purge\nmodule load $mpi_version\nmodule load lammps\nmodule load $gcc_v\nmodule load fftw3\necho -e \"Lammps is ready for running.\"" > $APP_ROOT/lammps/lammps_environment.sh
fi
chmod +x $APP_ROOT/lammps/lammps_environment.sh
cat /etc/profile | grep "alias lmpenv" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias lmpenv='source $APP_ROOT/lammps/lammps_environment.sh'" >> /etc/profile
fi  
echo -e "[ -DONE- ] Lammps: lmp_g++_mpich has been built to /hpc_apps/lammps ."
echo -e "[ -DONE- ] Lammps: lmp_g++_mpich has been built to /hpc_apps/lammps ." >> ${logfile}