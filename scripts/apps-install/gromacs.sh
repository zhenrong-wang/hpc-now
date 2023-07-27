#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Gromacs* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`

if [ ! -z $CENTOS_VERSION ] && [ $CENTOS_VERSION -eq 7 ]; then
  echo -e "[ FATAL: ] GROMACS on CentOS 7.x is not natively supported by App Store. But you can manually build and run it."
  exit 3
fi

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_cache="/hpc_apps/.cache/"
  app_extract_cache="/root/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
  app_extract_cache="/home/${current_user}/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/${current_user}_env/"
fi
mkdir -p ${app_cache}
mkdir -p ${app_extract_cache}

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the binaries and libraries ..."
  rm -rf ${app_root}gromacs2022
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< gromacs >/d' $public_app_registry
    sed -i '/gromacs.env=/d' /etc/profile
  else
    sed -e "/< gromacs > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
    sed -i '/gromacs.env=/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] ABINIT has been removed successfully."
  exit 0
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] $time_current INSTALLING GROMACS-version 2022-2 now."
echo -e "[ -INFO- ] By *default*, GROMACS will be built with fftw3. if you'd like to build with MKL or its own fftpack, please modify the scripts."
echo -e "[ -INFO- ] Detecting GNU Compiler Collection ..."
gcc_vers=('gcc12' 'gcc9' 'gcc8' 'gcc4')
gcc_code=('gcc-12.1.0' 'gcc-9.5.0' 'gcc-8.2.0' 'gcc-4.9.2')
systemgcc='true'
if [ ! -z $CENTOS_VERSION ] && [ $CENTOS_VERSION = '7' ]; then
  for i in $(seq 0 3)
  do
	  grep "< ${gcc_vers[i]} >" $public_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      module load ${gcc_code[i]}
      gcc_env="${gcc_code[i]}"
      systemgcc='false'
      break
    fi
    if [ $current_user != 'root' ]; then
      grep "< ${gcc_vers[i]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        module load ${current_user}_apps/${gcc_code[i]}
        gcc_env="${current_user}_env/${gcc_code[i]}"
        systemgcc='false'
        break
      fi
    fi
  done
else
  grep "< ${gcc_vers[0]} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load ${gcc_code[0]}
    gcc_env="${gcc_code[0]}"
    systemgcc='false'
  else
    if [ $current_user != 'root' ]; then
      grep "< ${gcc_vers[0]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
      if [ $? -eq 0 ]; then
        module load ${current_user}_env/${gcc_code[0]}
        gcc_env="${current_user}_env/${gcc_code[0]}"
        systemgcc='false'
      fi
    fi
  fi
fi
gcc_v=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
echo -e "[ -INFO- ] Using GNU Compiler Collections - ${gcc_v}."

echo -e "[ -INFO- ] Detecting MPI Libraries ..."
mpi_vers=('mpich4' 'mpich3' 'ompi4' 'ompi3')
mpi_code=('mpich-4.0.2' 'mpich-3.2.1' 'ompi-4.1.2' 'ompi-3.1.6')
for i in $(seq 0 3)
do
	grep "< ${mpi_vers[i]} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load ${mpi_code[i]}
    mpi_root="/hpc_apps/${mpi_code[i]}/"
    mpi_env="${mpi_code[i]}"
    break
  fi
  if [ $current_user != 'root' ]; then
    grep "< ${mpi_vers[i]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      module load ${current_user}_env/${mpi_code[i]}
      mpi_root="${app_root}${mpi_code[i]}/"
      mpi_env="${current_user}_env/${mpi_code[i]}"
      break
    fi
  fi
done
mpirun --version >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] Building MPI Libraries now ..."
  hpcmgr install mpich4 >> ${2}
  if [ $current_user = 'root' ]; then
    module load mpich-4.0.2
    mpi_env="mpich-4.0.2"
  else
    module load ${current_user}_env/mpich-4.0.2
    mpi_env="${current_user}_env/mpich-4.0.2"
  fi
  mpi_root="${app_root}mpich-4.0.2/"
fi
echo -e "[ -INFO- ] Using MPI Libraries - ${mpi_env}."

echo -e "[ -INFO- ] Checking and Installing the fftw3 libraries ... "
hpcmgr install fftw3 >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build fftw3. Exit now."
  exit 5
fi
grep "< fftw3 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mdule load fftw3
  fftw3_env="fftw3"
  fftw3_root="/hpc_apps/fftw3/"
else
  module load ${current_user}_env/fftw3
  fftw3_env="${current_user}_env/fftw3"
  fftw3_root="/hpc_apps/${current_user}_apps/fftw3/"
fi

echo -e "[ -INFO- ] Downloading and extracting source files ..."
if [ ! -f ${app_cache}gromacs-2022.2.tar.gz ]; then
  wget ${url_pkgs}gromacs-2022.2.tar.gz -O ${app_cache}gromacs-2022.2.tar.gz -o ${2}
fi
tar zvxf ${app_cache}gromacs-2022.2.tar.gz -C ${app_extract_cache} >> ${2}
echo -e "[ -INFO- ] Building GROMACS-2022 now...This step may take minutes."
cd ${app_extract_cache}gromacs-2022.2
rm -rf build && mkdir -p build && cd build
cmake .. -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DGMX_MPI=on -DCMAKE_INSTALL_PREFIX=${app_root}gromacs2022 -DGMX_FFT_LIBRARY=fftw3 -DFFTW_LIBRARY=${fftw3_root}lib/libfftw3.a -DFFTW_INCLUDE_DIR=${fftw3_root}include -DGMX_DOUBLE=on >> ${2} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] CMake processing error. Exit now."
  exit 7
fi
make -j $num_processors >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build GROMACS. Please check the log file for details. Exit now."
  exit 9
fi
make install >> ${2}
if [ $current_user = 'root' ]; then
  grep "alias gromacs.env" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias gromacs.env='module load ${fftw3_env} && source ${app_root}gromacs2022/bin/GMXRC'" >> /etc/profile
  fi
  echo -e "< gromacs >" >> $public_app_registry
else
  grep "alias gromacs.env" $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias gromacs.env='module load ${fftw3_env} && source ${app_root}gromacs2022/bin/GMXRC'" >> $HOME/.bashrc
  fi
  echo -e "< gromacs > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] Congratulations! GROMACS-2022-2 has been built."
echo -e "|          Please run 'gromacs.env' command to load the environment before using GROMACS."