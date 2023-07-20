#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenFOAM-v2112* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/usr/hpc-now/.public_apps.reg"
private_app_registry="/usr/hpc-now/.private_apps.reg"
tmp_log="/tmp/hpcmgr_install_of2112_${current_user}.log"

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_cache="/hpc_apps/.cache/"
  of_cache="/root/OpenFOAM/"
  envmod_root="/hpc_apps/envmod/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_cache="/hpc_apps/${current_user}_apps/.cache/"
  of_cache="/home/${current_user}/OpenFOAM/"
  envmod_root="/hpc_apps/envmod/${current_user}_env/"
fi

mkdir -p $app_cache
mkdir -p $of_cache

of_root="${app_root}OpenFOAM/"

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing binaries and libraries, this may take minutes ..."
  rm -rf ${of_root}OpenFOAM-v2112
  rm -rf ${of_root}ThirdParty-v2112
  echo -e "[ -INFO- ] Updating envrionment variables ..."
  if [ $current_user = 'root' ]; then
    sed -i '/of2112.sh/d' /etc/profile
    sed -i '/< of2112 >/d' ${public_app_registry}
  else
    sed -i '/of2112.sh/d' $HOME/.bashrc
    sed -e "/< of2112 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] OpenFOAM-v2112 has been removed successfully."
  exit 0
fi

echo -e "[ -INFO- ] Cleaning up processes..."
ps -aux | grep OpenFOAM-v2112/wmake | cut -c 9-15 | xargs kill -9 >> /dev/null 2>&1
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
gcc_version=`gcc --version | head -n1`
gcc_vnum=`echo $gcc_version | awk '{print $3}' | awk -F"." '{print $1}'`
echo -e "[ -INFO- ] Using GNU Compiler Collections - ${gcc_version}."
echo -e "[ -INFO- ] Detecting MPICH Libraries ..."
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
  echo -e "[ -INFO- ] Building MPICH Libraries now ..."
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
echo -e "[ -INFO- ] Using MPICH Libraries - ${mpi_env}."

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Building OpenFOAM-v2112 now ... "
mkdir -p ${of_root}
rm -rf ${of_root}*

echo -e "[ STEP 1 ] $time_current Downloading & extracting source packages ..."

if [ ! -f ${app_cache}OpenFOAM-v2112.tgz ]; then
  wget ${url_pkgs}OpenFOAM-v2112.tgz -O ${app_cache}OpenFOAM-v2112.tgz -o ${tmp_log}
fi  
if [ ! -f ${app_cache}ThirdParty-v2112.tgz ]; then
  wget ${url_pkgs}ThirdParty-v2112.tgz -O ${app_cache}ThirdParty-v2112.tgz -o ${tmp_log}
fi
cd ${app_cache}
tar zvxf OpenFOAM-v2112.tgz -C ${of_cache} >> $tmp_log
tar zvxf ThirdParty-v2112.tgz -C ${of_cache} >> $tmp_log 
if [ ! -f ${of_cache}ThirdParty-v2112/sources/ADIOS2-2.6.0.zip ]; then
  wget ${url_pkgs}ADIOS2-2.6.0.zip -O ${of_cache}ThirdParty-v2112/sources/ADIOS2-2.6.0.zip >> $tmp_log 2>&1
fi
#cd ${of_cache}ThirdParty-v2112/sources && rm -rf ADIOS2-2.6.0 && unzip ADIOS2-2.6.0.zip >> $tmp_log 2>&1
if [ ! -f ${of_cache}ThirdParty-v2112/sources/metis-5.1.0.tar.gz ]; then
  wget ${url_pkgs}metis-5.1.0.tar.gz -O ${of_cache}ThirdParty-v2112/sources/metis-5.1.0.tar.gz -q
fi
#cd ${of_cache}ThirdParty-v2112/sources && rm -rf metis-5.1.0 && tar zvxf metis-5.1.0.tar.gz >> $tmp_log 2>&1
echo -e "[ -INFO- ] Removing the tarballs are not recommended. If you want to rebuild of2112, please remove the folder OpenFOAM-v2112 & ThirdParty-v2112."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 2 ] $time_current Removing previously-built binaries ..."
#rm -rf ${of_cache}OpenFOAM-v2112/platforms/*
#rm -rf ${of_cache}ThirdParty-v2112/platforms/* 
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Compiling started ..."
if [ ! -f  ${of_cache}OpenFOAM-v2112/etc/config.sh/settings-orig ]; then
  cp ${of_cache}OpenFOAM-v2112/etc/config.sh/settings ${of_cache}OpenFOAM-v2112/etc/config.sh/settings-orig
else
  /bin/cp ${of_cache}OpenFOAM-v2112/etc/config.sh/settings-orig ${of_cache}OpenFOAM-v2112/etc/config.sh/settings 
fi
if [ ! -f ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c-orig ]; then
  cp ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c-orig
else
  /bin/cp ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c-orig ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c
fi
if [ ! -f ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++-orig ]; then
  cp ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++ ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++-orig
else
  /bin/cp  ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++-orig ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++
fi
if [ ! -f ${of_cache}OpenFOAM-v2112/etc/bashrc-orig ]; then
  cp ${of_cache}OpenFOAM-v2112/etc/bashrc ${of_cache}OpenFOAM-v2112/etc/bashrc-orig
else
  /bin/cp ${of_cache}OpenFOAM-v2112/etc/bashrc-orig ${of_cache}OpenFOAM-v2112/etc/bashrc
fi
if [ $gcc_vnum -gt 10 ]; then
  cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC"  >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    cpu_model=`cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC" | head -n1 | awk '{print $6}'`
    cpu_gen=${cpu_model: -1}
    if [ $cpu_gen = '3' ]; then
      sed -i 's/-fPIC/-fPIC -march=znver3/g' ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver3/g' ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++
    elif [ $cpu_gen = '2' ]; then
      sed -i 's/-fPIC/-fPIC -march=znver2/g' ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver2/g' ${of_cache}OpenFOAM-v2112/wmake/rules/linux64Gcc/c++
    fi
  fi
fi    
sed -i 's/export WM_MPLIB=SYSTEMOPENMPI/export WM_MPLIB=SYSTEMMPI/g' ${of_cache}OpenFOAM-v2112/etc/bashrc
export MPI_ROOT=${mpi_root}
echo "${mpi_env}" | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  export MPI_ARCH_FLAGS="-DOMPI_SKIP_MPICXX"
else
  export MPI_ARCH_FLAGS="-DMPICH_SKIP_MPICXX"
fi
export MPI_ARCH_INC="-I${mpi_root}include" 
export MPI_ARCH_LIBS="-L${mpi_root}lib -lmpi"
source ${of_cache}OpenFOAM-v2112/etc/bashrc
export FOAM_EXTRA_LDFLAGS="-L${of_cache}ThirdParty-v2112/platforms/linux64Gcc/fftw-3.3.10/lib -lfftw3"
echo -e "[ -INFO- ] Building OpenFOAM in progress ... It takes really long time (for example, 2.5 hours with 8 vCPUs)"
echo -e "[ -INFO- ] Please check the log files: Build_OF.log."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Started compiling source codes ..."
#module load $mpi_version
PATH=${mpi_root}bin:$PATH LD_LIBRARY_PATH=${mpi_root}lib:$LD_LIBRARY_PATH
${of_cache}ThirdParty-v2112/Allclean -build > ${of_cache}Build_OF.log 2>&1
${of_cache}OpenFOAM-v2112/Allwmake -j$num_processors >> ${of_cache}Build_OF.log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Building OpenFOAM-v2112 failed. Please check the Build_OF.log and retry later. Exit now."
  exit
fi
echo -e "[ -INFO- ] Copying files ..."
rsync -a --info=progress2 ${of_cache} ${of_root}

echo -e "#! /bin/bash\nmodule purge" > ${of_root}of2112.sh
echo -e "export MPI_ROOT=${MPI_ROOT}" >> ${of_root}of2112.sh
echo -e "export MPI_ARCH_FLAGS=${MPI_ARCH_FLAGS}" >> ${of_root}of2112.sh
echo -e "export MPI_ARCH_INC=${MPI_ARCH_INC}" >> ${of_root}of2112.sh
echo -e "export MPI_ARCH_LIBS=${MPI_ARCH_LIBS}" >> ${of_root}of2112.sh
echo -e "module load ${mpi_env}" >> ${of_root}of2112.sh
if [ $systemgcc = 'false' ]; then
  echo -e "module load ${gcc_env}" >> ${of_root}of2112.sh
fi 
echo -e "source ${of_root}OpenFOAM-9/etc/bashrc" >> ${of_root}of2112.sh
echo -e "echo -e \"OpenFOAM-v2112 with ${mpi_env} and ${gcc_version} is ready for running.\"" >> ${of_root}of2112.sh
if [ $current_user = 'root' ]; then
  grep of2112 /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias of2112='source ${of_root}of2112.sh'" >> /etc/profile
  fi
  echo -e "< of2112 >" >> $public_app_registry
else
  grep of2112 $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias of2112='source ${of_root}of2112.sh'" >> $HOME/.bashrc
  fi
  echo -e "< of2112 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] Congratulations! OpenFOAM-v2112 with ${mpi_env} and ${gcc_version} has been built."