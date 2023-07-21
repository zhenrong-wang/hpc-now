#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *Lammps* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi
tmp_log="/tmp/hpcmgr_install_lammps_${current_user}.log"

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`

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
  rm -rf ${app_root}lammps
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< lammps >/d' $public_app_registry
  else
    sed -e "/< lammps > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] LAMMPS has been removed successfully."
  exit 0
fi

mkdir -p ${app_root}lammps
rm -rf ${app_root}lammps/*
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] ${time_current} Installing LAMMPS Started ..."
echo -e "[ -INFO- ] By *default*, Lammps will be built with 'yes-most', g++ and the latest version of MPICH found in the cluster."
echo -e "[ -INFO- ] If you'd like to build your own version of Lammps, please modify this script."
echo -e "[ -INFO- ] You can also go to the source code and make as you preferred. But this is not recommended."
echo -e "[ -INFO- ] Detecting GNU Compiler Collection ..."
gcc_vers=('gcc12' 'gcc9' 'gcc8' 'gcc4')
gcc_code=('gcc-12.1.0' 'gcc-9.5.0' 'gcc-8.2.0' 'gcc-4.9.2')
systemgcc='true'
if [ ! -z $centos_ver ] && [ $centos_ver -eq 7 ]; then
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

echo -e "[ -INFO- ] Checking and Installing the MPI Libraries ... "
mpi_vers=('mpich4' 'mpich3')
mpi_code=('mpich-4.0.2' 'mpich-3.2.1')
for i in $(seq 0 1)
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
  hpcmgr install mpich4 >> ${tmp_log}
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
hpcmgr install fftw3 >> $tmp_log
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

# Install adios2 now
echo -e "[ -INFO- ] Checking and Installing ADIOS2 now ... "
if [ ! -f ${app_cache}ADIOS2-master.zip ]; then
  wget ${url_pkgs}ADIOS2-master.zip -O ${app_cache}ADIOS2-master.zip -o $tmp_log
fi
unzip -o ${app_cache}ADIOS2-master.zip -d ${app_extract_cache} >> $tmp_log
cd ${app_extract_cache}
rm -rf ADIOS2 && mv ADIOS2-master ADIOS2
mkdir -p adios2-build && cd adios2-build
cmake -DCMAKE_INSTALL_PREFIX=${app_root}lammps/ADIOS2 ../ADIOS2  >> $tmp_log 2>&1
make -j$num_processors >> $tmp_log 2>&1
make install >> $tmp_log 2>&1
export PATH=${app_root}lammps/ADIOS2/bin:$PATH
export LD_LIBRARY_PATH=${app_root}lammps/ADIOS2/lib64:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=${app_root}lammps/ADIOS2/include:$C_INCLUDE_PATH

echo -e "[ -INFO- ] Downloading and extracting source files ..."
if [ ! -f ${app_cache}lammps.zip ]; then
  wget ${url_pkgs}lammps-develop.zip -O ${app_cache}lammps.zip -o $tmp_log
fi
unzip -o -q ${app_cache}lammps.zip -d ${app_extract_cache} >> $tmp_log
if [ ! -f ${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig ]; then 
  /bin/cp ${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich ${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig
else
  /bin/cp ${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich.orig ${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich
fi

echo -e "[ -INFO- ] Optimizing compile configurations ..."
makefile_g_mpich=${app_extract_cache}lammps-develop/src/MAKE/OPTIONS/Makefile.g++_mpich
cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC"  >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  cpu_model=`cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC" | head -n1`
  cpu_gen=${cpu_model: -1}
  if [ $cpu_gen = '3' ]; then
    sed -i 's/-g -O3/-g -O3 -march=znver3/g' $makefile_g_mpich
  elif [ $cpu_gen = '2' ]; then
    sed -i 's/-g -O3/-g -O3 -march=znver2/g' $makefile_g_mpich
  fi
fi
sed -i "s@MPI_INC = @MPI_INC = -I${mpi_root}include@g" $makefile_g_mpich
sed -i "s@MPI_PATH =@MPI_PATH = -L${mpi_root}lib@g" $makefile_g_mpich
sed -i "s@MPI_LIB =@MPI_LIB = -lmpi -lmpich -lmpicxx@g" $makefile_g_mpich
sed -i "s@FFT_INC =@FFT_INC = -I${fftw3_root}include@g" $makefile_g_mpich
sed -i "s@FFT_PATH =@FFT_PATH = -L${fftw3_root}lib@g" $makefile_g_mpich
sed -i "s@FFT_LIB =@FFT_LIB = -lfftw3@g" $makefile_g_mpich
echo -e "[ -INFO- ] Start compiling Lammps: lmp_g++_mpich now, this step may take minutes ..."
cd  ${app_extract_cache}lammps-develop/src
make yes-most >> $tmp_log 2>&1
make -j$num_processors g++_mpich >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build LAMMPS. Please check the log file for details. Exit now."
  exit 7
fi

/bin/cp ${app_extract_cache}lammps-develop/src/lmp_g++_mpich ${app_root}lammps/
echo -e "#! /bin/bash\nmodule purge" > ${app_root}lammps/lammps_environment.sh
if [ $systemgcc = 'false' ]; then
  echo -e "module load ${gcc_env}" >> ${app_root}lammps/lammps_environment.sh
fi
echo -e "module load ${mpi_env}\nmodule load ${fftw3_env}" >> ${app_root}lammps/lammps_environment.sh
echo -e "export PATH=${app_root}lammps:${app_root}lammps/ADIOS2/bin:\$PATH" >> ${app_root}lammps/lammps_environment.sh 
echo -e "export LD_LIBRARY_PATH=${app_root}lammps/ADIOS2/lib64:\$LD_LIBRARY_PATH" >> ${app_root}lammps/lammps_environment.sh
echo -e "export C_INCLUDE_PATH=${app_root}lammps/ADIOS2/include:\$C_INCLUDE_PATH" >> ${app_root}lammps/lammps_environment.sh
echo -e "echo -e \"LAMMPS is ready for running.\""  >> ${app_root}lammps/lammps_environment.sh

if [ $current_user = 'root' ]; then
  grep "alias lmpenv" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias lmpenv='source ${app_root}lammps/lammps_environment.sh'" >> /etc/profile
  fi
  echo -e "< lammps >" >> $public_app_registry
else
  grep "alias lmpenv" $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias lmpenv='source ${app_root}lammps/lammps_environment.sh'" >> $HOME/.bashrc
  fi
  echo -e "< lammps > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] Lammps: lmp_g++_mpich has been built to /hpc_apps/lammps ."