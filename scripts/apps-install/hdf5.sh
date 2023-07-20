#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *HDF5-1.10.9* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_hdf5_${current_user}.log"

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
  echo -e "[ -INFO- ] Removing binaries and libraries ..."
  rm -rf ${app_root}hdf5-1.10.9
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}hdf5-1.10.9
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< hdf5 >/d' $public_app_registry
  else
    sed -e gcc-12.1.0"/< hdf5 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] HDF5-1.10.9 has been removed successfully."
  exit 0
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] $time_current SOFTWARE: HDF5-1.10.9"
echo -e "[ -INFO- ] HDF5 requires zlib-1.2.13. Installing now ... "
grep "< zlib >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  zlib_path="/hpc_apps/zlib-1.2.13/"
else
  if [ $current_user = 'root' ]; then
    hpcmgr install zlib >> $tmp_log
  else
    grep "< zlib > < ${current_user} >" $private_app_registry >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      hpcmgr install zlib >> $tmp_log
    fi
  fi
  zlib_path="${app_root}zlib-1.2.13/"
fi

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

ompi_vers=('ompi4' 'ompi3')
ompi_code=('ompi-4.1.2' 'ompi-3.1.6')
for i in $(seq 0 1)
do
	grep "< ${ompi_vers[i]} >" $public_app_registry >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load ${ompi_code[i]}
    mpi_root="/hpc_apps/${ompi_code[i]}/"
    cxxflags="-I${mpi_root}include"
    ldflags="-L${mpi_root}lib -lmpi_cxx"
    cc_path="${mpi_root}bin/mpicc"
    break
  fi
  if [ $current_user != 'root' ]; then
    grep "< ${ompi_vers[i]} > < $current_user >" $private_app_registry >> /dev/null 2>&1
    if [ $? -eq 0 ]; then
      module load ${current_user}/${ompi_code[i]}
      mpi_root="${app_root}${ompi_code[i]}/"
      cxxflags="-I${mpi_root}include"
      ldflags="-L${mpi_root}lib -lmpi_cxx"
      cc_path="${mpi_root}bin/mpicc"
      break
    fi
  fi
done

mpirun --version >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  hpcmgr install ompi4 >> $tmp_log
  if [ $current_user = 'root' ]; then
    module load ompi-4.1.2
  else
    module load ${current_user}/ompi-4.1.2
  fi
  mpi_root="${app_root}ompi-4.1.2/"
  cxxflags="-I${mpi_root}include"
  ldflags="-L${mpi_root}lib -lmpi_cxx"
  cc_path="${mpi_root}bin/mpicc"
fi

echo -e "[ -INFO- ] HDF5-1.10.9 will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -f ${app_cache}hdf5-1.10.9.tar.gz ]; then
  wget ${url_pkgs}hdf5-1.10.9.tar.gz -O ${app_cache}hdf5-1.10.9.tar.gz -o $tmp_log
fi
tar zvxf ${app_cache}hdf5-1.10.9.tar.gz -C ${app_extract_cache} >> $tmp_log
echo -e "[ STEP 1 ] Building HDF5-1.10.9 ... This step usually takes seconds."
#yum -y install zlib-devel >> $tmp_log 2>&1
#cd /opt/packs/hdf5-1.10.9 && make clean >> $tmp_log 2>&1
cd ${app_extract_cache}hdf5-1.10.9
CXXFLAGS="${cxxflags}" LDFLAGS="${ldflags}" ./configure --prefix=${app_root}hdf5-1.10.9 CC=${cc_path} --enable-parallel --enable-shared --enable-fortran --enable-cxx --enable-hl --with-zlib=${zlib_path} --enable-unsupported >> $tmp_log
make -j${num_processors} >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build HDF5-1.10.9. Please check the log file for more details. Exit now."
  exit 3
fi
echo -e "[ STEP 2 ] Installing HDF5-1.10.9 ... This step usually takes seconds."
make install >> $tmp_log
echo -e "[ STEP 3 ] Setting up system environments now ..."
echo -e "#%Module1.0\nprepend-path PATH ${app_root}hdf5-1.10.9/bin" > ${envmod_root}hdf5-1.10.9
echo -e "prepend-path LD_LIBRARY_PATH ${app_root}hdf5-1.10.9/lib" >> ${envmod_root}hdf5-1.10.9
echo -e "prepend-path C_INCLUDE_PATH ${app_root}hdf5-1.10.9/include" >> ${envmod_root}hdf5-1.10.9
if [ $current_user = 'root' ]; then
  echo -e "< hdf5 >" >> $public_app_registry
else
  echo -e "< hdf5 > < ${current_user} >" >> $private_app_registry
fi
echo -e "zlib ${zlib_path}" > ${app_root}build_info.txt
echo -e "mpi_root ${mpi_root}" >> ${app_root}build_info.txt
echo -e "[ -DONE- ] HDF5-1.10.9 has been successfully installed to your cluster."