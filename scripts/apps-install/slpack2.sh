#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *ScaLAPACK-Latest* to HPC-NOW cluster.

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
  echo -e "[ -INFO- ] Removing binaries and libraries ..."
  rm -rf ${app_root}scalapack
  echo -e "[ -INFO- ] Removing environment module file ..."
  rm -rf ${envmod_root}scalapack
  echo -e "[ -INFO- ] Updating the registry ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< slpack2 >/d' $public_app_registry
  else
    sed -e "/< slpack2 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] ScaLAPACK-Latest has been removed successfully."
  exit 0
fi

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

grep "< lapack311 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load lapack-3.11
  lapack_lib="/hpc_apps/lapack-3.11"
else
  if [ $current_user = 'root' ]; then
    hpcmgr install lapack311 >> ${2}
    module load lapack-3.11
    lapack_lib="/hpc_apps/lapack-3.11"
  else
    grep "< lapack311 > < $current_user >" $private_app_registry >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      hpcmgr install lapack-3.11 >> ${2}
    fi
    module load ${current_user}_env/lapack-3.11
    lapack_lib="${app_root}lapack-3.11"
  fi
fi

echo -e "[ -INFO- ] Detecting MPI Libraries ..."
mpi_vers=('ompi4' 'ompi3' 'mpich4' 'mpich3')
mpi_code=('ompi-4.1.2' 'ompi-3.1.6' 'mpich-4.0.2' 'mpich-3.2.1')
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

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Started building ScaLAPACK-Latest."
echo -e "[ START: ] Downloading and Extracting source code ..."

if [ ! -f ${app_cache}scalapack-master.zip ]; then
  wget ${url_pkgs}scalapack-master.zip -O ${app_cache}scalapack-master.zip -o ${2}
fi
unzip -o ${app_cache}scalapack-master.zip -d ${app_extract_cache} >> ${2}
rm -rf ${app_root}scalapack
rm -rf ${app_extract_cache}scalapack
mv ${app_extract_cache}scalapack-master ${app_extract_cache}scalapack
echo -e "[ STEP 1 ] Building ScaLAPACK ... This step usually takes seconds."
cd ${app_extract_cache}scalapack
/bin/cp SLmake.inc.example SLmake.inc
if [ $gcc_vnum -gt 10 ]; then
  sed -i 's@FCFLAGS       = -O3@FCFLAGS       = -O3 -fallow-argument-mismatch -fPIC@g' ${app_extract_cache}scalapack/SLmake.inc
else
  sed -i 's@FCFLAGS       = -O3@FCFLAGS       = -O3 -fPIC@g' ${app_extract_cache}scalapack/SLmake.inc
fi
sed -i "s@BLASLIB       = -lblas@BLASLIB       = -L${lapack_lib} -lrefblas -lcblas@g" ${app_extract_cache}scalapack/SLmake.inc
sed -i "s@LAPACKLIB     = -llapack@LAPACKLIB     = -L${lapack_lib} -llapack.a -llapacke -ltmglib@g" ${app_extract_cache}scalapack/SLmake.inc
cd ${app_extract_cache}scalapack
make lib >> ${2}
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ScaLAPACK. Please check the log file for more details. Exit now."
  exit 3
fi
rm -rf ${app_root}scalapack
mkdir -p ${app_root}scalapack
/bin/cp ${app_extract_cache}scalapack/libscalapack.a ${app_root}scalapack/
echo -e "#%Module1.0\nprepend-path LIBRARY_PATH ${app_root}scalapack\n" > ${envmod_root}scalapack
if [ $current_user = 'root' ]; then
  echo -e "< slpack2 >" >> $public_app_registry
else
  echo -e "< slpack2 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -INFO- ] ScaLAPACK-Latest has been built from the source code."