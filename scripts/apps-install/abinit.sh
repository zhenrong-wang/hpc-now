# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *ABINIT-9.6.2* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`

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
  rm -rf ${app_root}abinit
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< abinit >/d' $public_app_registry
    sed -i '/abinit.env=/d' /etc/profile
  else
    sed -e "/< abinit > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
    sed -i '/abinit.env=/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] ABINIT has been removed successfully."
  exit 0
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] $time_current Software: ABINIT-9.6.2"
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: GNU Compiler Collections - GCC version ..."
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
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: MPI versions ..."
ompi_vers=('ompi4' 'ompi3')
ompi_code=('ompi-4.1.2' 'ompi-3.1.6')
for i in $(seq 0 1)
do
	grep "< ${ompi_vers[i]} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load ${ompi_code[i]}
    mpi_root="/hpc_apps/${ompi_code[i]}/"
    mpi_env="${ompi_code[i]}"
    break
  fi
  if [ $current_user != 'root' ]; then
    grep "< ${ompi_vers[i]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      module load ${current_user}_env/${ompi_code[i]}
      mpi_root="${app_root}${ompi_code[i]}/"
      mpi_env="${current_user}_env/ompi-4.1.2"
      break
    fi
  fi
done

mpirun --version >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  hpcmgr install ompi4 >> ${2}
  if [ $current_user = 'root' ]; then
    module load ompi-4.1.2
    mpi_env=ompi-4.1.2
  else
    module load ${current_user}_env/ompi-4.1.2
    mpi_env="${current_user}_env/ompi-4.1.2"
  fi
  mpi_root="${app_root}ompi-4.1.2/"
fi
echo -e "[ -INFO- ] Using MPI Libraries - ${mpi_env}."

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: OpenBLAS Libraries ..."
hpcmgr install openblas >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build OpenBLAS. Exit now."
  exit 11
fi
grep "< openblas >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  openblas_lib="/hpc_apps/openblas/lib"
  openblas_env=openblas
else
  openblas_lib="${app_root}openblas/lib"
  openblas_env=${current_user}_env/openblas
fi

abinit_root="${app_root}abinit/"
mkdir -p $abinit_root
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: libXC libraries ..."
if [ ! -f ${abinit_root}libxc-4.3.4/lib/libxcf90.la ]; then
  if [ ! -f ${app_cache}libxc-4.3.0.tar.gz ]; then
    wget ${url_pkgs}libxc-4.3.4.tar.gz -O ${app_cache}libxc-4.3.4.tar.gz -o ${2}
  fi
  tar zvxf ${app_cache}libxc-4.3.4.tar.gz -C ${app_extract_cache} >> ${2}
  cd ${app_extract_cache}libxc-4.3.4
  ./configure --prefix=${abinit_root}libxc-4.3.4 >> ${2} 2>&1
  make -j$num_processors >> ${2}
  make install >> ${2}
fi

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: FFTW-3.3.8 Libraries ..."
if [[ ! -f ${abinit_root}fftw-3.3.8/lib/libfftw3.a || ! -f ${abinit_root}fftw-3.3.8/lib/libfftw3_mpi.la ]]; then
  if [ ! -f ${app_cache}fftw-3.3.8.tar.gz ]; then
    wget ${url_pkgs}fftw-3.3.8.tar.gz -O ${app_cache}fftw-3.3.8.tar.gz -o ${2}
  fi
  tar zvxf ${app_cache}fftw-3.3.8.tar.gz -C ${app_extract_cache} >> ${2}
  cd ${app_extract_cache}fftw-3.3.8
  ./configure --prefix=${abinit_root}fftw-3.3.8 --enable-single --enable-mpi --enable-threads --enable-shared >> ${2} 2>&1
  make -j$num_processors >> ${2}
  make install >> ${2}

  cd ${app_extract_cache}fftw-3.3.8
  ./configure --prefix=${abinit_root}fftw-3.3.8 --enable-mpi --enable-threads --enable-shared >> ${2} 2>&1
  make -j$num_processors >> ${2}
  make install >> ${2}
fi
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: HDF5 ..."
hpcmgr install hdf5 >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build HDF5. Exit now."
  exit 13
fi
grep "< hdf5 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  hdf5_root="/hpc_apps/hdf5-1.10.9"
  hdf5_env=hdf5-1.10.9
else
  hdf5_root="${app_root}hdf5-1.10.9"
  hdf5_env="${current_user}_env/hdf5-1.10.9"
fi

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: netCDF4 ..."
hpcmgr install netcdf4 >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build netcdf4. Exit now."
  exit 15
fi
grep "< netcdf4 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  netcdf_root="/hpc_apps/netcdf4"
  netcdf_env=netcdf4
else
  netcdf_root="${app_root}netcdf4"
  netcdf_env="${current_user}_env/netcdf4"
fi
pip install configurator >> ${2}

echo -e "[ -INFO- ] Now, finally, we can start building ABINIT-9.6.2 ..."
echo -e "[ STEP 1 ] Downloading and extracing source code ..."
if [ ! -f ${app_cache}abinit-9.6.2.tar.gz ]; then
  wget ${url_pkgs}abinit-9.6.2.tar.gz -O ${app_cache}abinit-9.6.2.tar.gz -o ${2}
fi
echo -e "[ STEP 2 ] Configuring and Compiling in progress, this step usually takes minutes ..."
tar zvxf ${app_cache}abinit-9.6.2.tar.gz -C ${app_extract_cache} >> ${2}
rm -rf ${abinit_root}build
mkdir -p ${abinit_root}build
cd ${app_extract_cache}abinit-9.6.2
FC_LDFLAGS_EXTRA="-L${openblas_lib} -lopenblas -L${abinit_root}fftw-3.3.8/lib -lfftw3_mpi -lfftw3f_mpi -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads" ./configure --prefix=${abinit_root}build --with-mpi=${mpi_root} --with-hdf5=${hdf5_root} --with-fftw3=${abinit_root}fftw-3.3.8 --with-libxc=${abinit_root}libxc-4.3.4 --with-netcdf=${netcdf_root} >> ${2} 2>&1
make -j$num_processors >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ABINIT. Please check the log file for details. Exit now."
  exit 1
fi
make install >> ${2} 2>&1
echo -e "# !/bin/bash\nmodule purge" > ${abinit_root}abinit_env.sh
if [ ${systemgcc} = 'false' ]; then
  echo -e "module load ${gcc_env}" >> ${abinit_root}abinit_env.sh
fi
echo -e "module load ${mpi_env}" >> ${abinit_root}abinit_env.sh
echo -e "module load ${openblas_env}" >> ${abinit_root}abinit_env.sh
echo -e "module load ${hdf5_env}" >> ${abinit_root}abinit_env.sh
echo -e "module load ${netcdf_env}" >> ${abinit_root}abinit_env.sh
echo -e "export LD_LIBRARY_PATH=${abinit_root}fftw-3.3.8/lib:\$LD_LIBRARY_PATH" >> ${abinit_root}abinit_env.sh
echo -e "export PATH=${abinit_root}build/bin:\$PATH" >> ${abinit_root}abinit_env.sh
echo -e "echo -e \"ABINIT is ready for running.\"" >> ${abinit_root}abinit_env.sh

if [ $current_user = 'root' ]; then
  grep abinit /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias abinit.env='source ${abinit_root}abinit_env.sh'" >> /etc/profile
  fi
  echo -e "< abinit >" >> $public_app_registry
else
  grep abinit $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias abinit.env='source ${abinit_root}abinit_env.sh'" >> $HOME/.bashrc
  fi
  echo -e "< abinit > < $current_user >" >> $private_app_registry
fi
echo -e "[ -DONE- ] ABINIT-9.6.2 has been sucessfully built to your cluster. You need to run command 'abinit.env' before using it."