# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

# This script is used by 'hpcmgr' command to build *WRF & WPS -4.4* to HPC-NOW cluster.

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
wrf_root="${app_root}WRF/"
wps_root="${app_root}WPS/"

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the binaries and libraries ..."
  rm -rf $wrf_root
  rm -rf $wps_root
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< wrf >/d' $public_app_registry
    sed -i '/wrf.env=/d' /etc/profile
  else
    sed -e "/< wrf > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
    sed -i '/wrf.env=/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] WRF & WPS has been removed successfully."
  exit 0
fi

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] ${time_current} Software: WRF & WPS - Version 4.4."
mkdir -p ${wrf_root} && rm -rf ${wrf_root}*
mkdir -p ${wps_root} && rm -rf ${wps_root}*
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
mpi_vers=('ompi4' 'ompi3')
mpi_code=('ompi-4.1.2' 'ompi-3.1.6')
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
  hpcmgr install ompi4 >> ${2}
  if [ $current_user = 'root' ]; then
    module load ompi-4.1.2
    mpi_env="ompi-4.1.2"
  else
    module load ${current_user}_env/ompi-4.1.2
    mpi_env="${current_user}_env/ompi-4.1.2"
  fi
  mpi_root="${app_root}ompi-4.1.2/"
fi
echo -e "[ -INFO- ] Using MPI Libraries - ${mpi_env}."
echo -e "[ -INFO- ] Checking and installing prerequisitions: netCDF"
hpcmgr install netcdf4 >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build/install netcdf4. Exit now."
  exit 3
fi
grep "< netcdf4 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load netcdf4
  netcdf4_root="/hpc_apps/netcdf4/"
  netcdf4_env="netcdf4"
else
  module load ${current_user}_env/netcdf4
  netcdf4_root="${app_root}netcdf4/"
  netcdf4_env="${current_user}_env/netcdf4"
fi
grep "< hdf5 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load hdf5-1.10.9
  hdf5_root="/hpc_apps/hdf5-1.10.9/"
  hdf5_env="hdf5-1.10.9"
else
  module load ${current_user}_env/hdf5-1.10.9
  hdf5_root="${app_root}hdf5-1.10.9/"
  hdf5_env="${current_user}_env/hdf5-1.10.9"
fi

echo -e "[ -INFO- ] Checking and installing prerequisitions: jasper ..."
if [ ! -f ${app_cache}jasper-1.900.1.tar.gz ]; then
  wget ${url_pkgs}jasper-1.900.1.tar.gz -O ${app_cache}jasper-1.900.1.tar.gz -o ${2}
fi
tar zvxf ${app_cache}jasper-1.900.1.tar.gz -C ${app_extract_cache} >> ${2}
cd ${app_extract_cache}jasper-1.900.1
./configure --prefix=${wrf_root}jasper >> ${2} 2>&1
make -j$num_processors >> ${2} 2>&1
make install >> ${2} 2>&1

echo -e "[ STEP 1 ] Downloading WRF source code packages ... "
if [ ! -f ${app_cache}wrf-latest.tar.gz ]; then
  wget ${url_pkgs}wrf-latest.tar.gz -O ${app_cache}wrf-latest.tar.gz -o ${2}
fi
echo -e "[ STEP 2 ] Extracting WRF source code tarball ..."
tar zvxf ${app_cache}wrf-latest.tar.gz -C ${app_extract_cache} >> ${2}
cd ${app_extract_cache}WRF
export NETCDF=${netcdf4_root}
export NETCDF_classic=1
export HDF5=${hdf5_root}
export PHDF5=${hdf5_root}
export JASPERLIB=${wrf_root}jasper/lib/
export JASPERINC=${wrf_root}jasper/include/

echo -e "[ STEP 3 ] Configuring and building WRF from the source code ..."
echo 35 | ./configure >> ${2} 2>&1
./compile -j $num_processors wrf >> ${2} 2>&1
./compile -j $num_processors em_b_wave >> ${2} 2>&1
./compile -j $num_processors em_convrad >> ${2} 2>&1
./compile -j $num_processors em_esmf_exp >> ${2} 2>&1
./compile -j $num_processors em_fire >> ${2} 2>&1
./compile -j $num_processors em_grav2d_x >> ${2} 2>&1
./compile -j $num_processors em_heldsuarez >> ${2} 2>&1
./compile -j $num_processors em_hill2d_x >> ${2} 2>&1
./compile -j $num_processors em_les >> ${2} 2>&1
./compile -j $num_processors em_quarter_ss >> ${2} 2>&1
./compile -j $num_processors em_real >> ${2} 2>&1
./compile -j $num_processors em_scm_xy >> ${2} 2>&1
./compile -j $num_processors em_seabreeze2d_x >> ${2} 2>&1
./compile -j $num_processors em_squall2d_x >> ${2} 2>&1
./compile -j $num_processors em_squall2d_y >> ${2} 2>&1
./compile -j $num_processors em_tropical_cyclone >> ${2} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build WRF. Please check the log file for details. Exit now."
  exit 5
fi
cp -r ${app_extract_cache}WRF/main/*.exe ${wrf_root}
echo -e "#! /bin/bash\nmodule purge" > ${wrf_root}wrfenv.sh
if [ $systemgcc = 'false' ]; then
  echo -e "module load $gcc_env" >> ${wrf_root}wrfenv.sh
fi
echo -e "module load ${mpi_env}\nexport PATH=${wrf_root}:\$PATH" >> ${wrf_root}wrfenv.sh
echo -e "module load ${netcdf4_env}\nmodule load ${hdf5_env}" >> ${wrf_root}wrfenv.sh
echo -e "export LD_LIBRARY_PATH=${wrf_root}jasper/lib:\$LD_LIBRARY_PATH" >> ${wrf_root}wrfenv.sh
echo -e "[ STEP 4 ] Building WPS now ..."
if [ ! -f ${app_cache}wps-latest.tar.gz ]; then
  wget ${url_pkgs}wps-latest.tar.gz -O ${app_cache}wps-latest.tar.gz -o ${2}
fi
tar zvxf ${app_cache}wps-latest.tar.gz -C ${app_extract_cache} >> ${2}
cd ${app_extract_cache}WPS
sed -i 's# NETCDFF=" "# NETCDFF="-lnetcdff -lgomp"#' configure # In order to build WPS
echo 3 | ./configure >> ${2} 2>&1
cp configure.wps configure.wps.bkup
sed -i 's@-L$(NETCDF)/lib -lnetcdff -lnetcdf@-L$(NETCDF)/lib -lnetcdff -lnetcdf -fopenmp@g' configure.wps # In order to build WPS
./compile >> ${2} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -WARN- ] WRF has been successfully built, but WPS encountered some problems. Exit now."
  echo -e "WRF with $gcc_v & $mpi_env is ready for running." >> ${wrf_root}wrfenv.sh
else
  cp -r ${app_extract_cache}WPS/* ${wps_root}
  echo -e "export PATH=${wps_root}:${wps_root}util:\$PATH\necho -e \"WRF & WPS with $gcc_v & $mpi_env are ready for running.\"" >> ${wrf_root}wrfenv.sh
fi

if [ $current_user = 'root' ]; then
  grep "alias wrf.env" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias wrf.env='source ${wrf_root}wrfenv.sh'" >> /etc/profile
  fi
  echo -e "< wrf >" >> $public_app_registry
else
  grep "alias wrf.env" $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias wrf.env='source ${wrf_root}wrfenv.sh'" >> $HOME/.bashrc
  fi
  echo -e "< wrf > < ${current_user} >" >> $private_app_registry
fi
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current Congratulations! WRF & WPS with $gcc_v & $mpi_env has been built."