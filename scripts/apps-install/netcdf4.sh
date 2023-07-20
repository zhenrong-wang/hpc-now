#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *netCDF-c-4.9.0, netcdf-fortran-4.5.3* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_netcdf4_${current_user}.log"

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
  rm -rf ${app_root}netcdf4
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}netcdf4
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< netcdf4 >/d' $public_app_registry
  else
    sed -e "/< netcdf4 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] NetCDF has been removed successfully."
  exit 0
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] $time_current SOFTWARE: netCDF-c-4.9.0, netcdf-fortran-4.5.3"

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
gcc_version=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_version | awk '{print $3}' | awk -F"." '{print $1}'`

grep "< hdf5 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  hdf5_root="/hpc_apps/hdf5-1.10.9/"
else
  if [ $current_user != 'root' ]; then
    grep "< hdf5 > < ${current_user} >" ${private_app_registry}
    if [ $? -eq 0 ]; then
      hdf5_root="${app_root}hdf5-1.10.9/"
    else
      hpcmgr install hdf5 >> $tmp_log
      hdf5_root="${app_root}hdf5-1.10.9/"
    fi
  else
    hpcmgr install hdf5 >> $tmp_log
    hdf5_root="${app_root}hdf5-1.10.9/"
  fi
fi

mpi_root=`grep mpi_root ${hdf5_root} | awk '{print $2}'`
zlib_path=`grep zlib ${hdf5_root} | awk '{print $2}'`
cppflags="-I${hdf5_root}include -I${zlib_path}include -I${mpi_root}include"
ldflags="-L${hdf5_root}lib -L${zlib_path}lib"
#yum -y install m4 >> $tmp_log 2>&1
#yum -y install libxml2-devel >> $tmp_log 2>&1
echo -e "[ -INFO- ] netCDF-C and netCDF-Fortran will be built with GNU Compiler Collections."
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -f ${app_cache}netcdf-c-4.9.0.zip ]; then
  wget ${url_pkgs}netcdf-c-4.9.0.zip -O ${app_cache}netcdf-c-4.9.0.zip -o $tmp_log
fi
if [ ! -f ${app_cache}netcdf-fortran-4.5.3.tar.gz ]; then
  wget ${url_pkgs}netcdf-fortran-4.5.3.tar.gz -O ${app_cache}netcdf-fortran-4.5.3.tar.gz -o $tmp_log
fi
unzip -o ${app_cache}netcdf-c-4.9.0.zip -d ${app_extract_cache} >> $tmp_log
tar zvxf ${app_cache}netcdf-fortran-4.5.3.tar.gz -C ${app_extract_cache} >> $tmp_log

echo -e "[ STEP 1 ] Building netCDF-C-4.9.0 & netCDF-fortran-4.5.3 ... This step usually takes minutes."

cd ${app_extract_cache}netcdf-c-4.9.0
CPPFLAGS="${cppflags}" LDFLAGS="${ldflags}" CC=gcc ./configure --prefix=${app_root}netcdf4 >> $tmp_log
make -j$num_processors >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build netCDF-C-4.9.0. Please check the log file for more details. Exit now."
  exit 3
fi
make install >> $tmp_log
echo -e "[ -INFO- ] netCDF-C-4.9.0 has been built. Installing netCDF-fortran-4.5.3 now ..."
cd ${app_extract_cache}netcdf-fortran-4.5.3
CPPFLAGS="-I${app_root}netcdf4/include" LDFLAGS="-L${app_root}netcdf4/lib" ./configure --prefix=${app_root}netcdf4 >> $tmp_log
make -j$num_processors >> $tmp_log
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build netCDF-fortran-4.5.3. Please check the log file for more details. Exit now."
  exit 3
fi
make install >> $tmp_log
echo -e "[ -INFO- ] netCDF-C-4.9.0 and netCDF-fortran-4.5.3 has been built from the source code."
echo -e "[ STEP 2 ] Setting up system environments now ..."
echo -e "#%Module1.0\nprepend-path PATH ${app_root}netcdf4/bin" > ${envmod_root}netcdf4
echo -e "prepend-path LD_LIBRARY_PATH ${app_root}netcdf4/lib" >> ${envmod_root}netcdf4
echo -e "prepend-path C_INCLUDE_PATH ${app_root}netcdf4/include" >> ${envmod_root}netcdf4
if [ $current_user = 'root' ]; then
  echo -e "< netcdf4 >" >> $public_app_registry
else
  echo -e "< netcdf4 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] netCDF-C-4.9.0 and netCDF-fortran-4.5.3 has been successfully installed to your cluster." 