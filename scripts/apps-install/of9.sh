#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenFOAM-9* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi

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
  rm -rf ${of_root}OpenFOAM-9
  rm -rf ${of_root}ThirdParty-9
  echo -e "[ -INFO- ] Updating envrionment variables ..."
  if [ $current_user = 'root' ]; then
    sed -i '/of9.env/d' /etc/profile
    sed -i '/< of9 >/d' ${public_app_registry}
  else
    sed -i '/of9.env/d' $HOME/.bashrc
    sed -e "/< of9 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
  fi
  echo -e "[ -INFO- ] OpenFOAM9 has been removed successfully."
  exit 0
fi

echo -e "[ -INFO- ] Cleaning up processes..."
ps -aux | grep OpenFOAM-9/wmake | cut -c 9-15 | xargs kill -9 >> /dev/null 2>&1
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

time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ START: ] $time_current Building OpenFOAM-9 now ... "
mkdir -p ${of_root}
rm -rf ${of_root}*
echo -e "[ -INFO- ] $time_current Downloading & extracting source packages ..."
if [ ! -f ${app_cache}OpenFOAM-9.zip ]; then
  wget ${url_pkgs}OpenFOAM-9.zip -O ${app_cache}OpenFOAM-9.zip -o ${2}
fi  
if [ ! -f ${app_cache}ThirdParty-9.zip ]; then
  wget ${url_pkgs}ThirdParty-9.zip -O ${app_cache}ThirdParty-9.zip -o ${2}
fi
cd ${app_cache}
unzip -o OpenFOAM-9.zip -d ${of_cache} >> ${2}
unzip -o ThirdParty-9.zip -d ${of_cache} >> ${2}
cd ${of_cache}
rm -rf OpenFOAM-9 && rm -rf ThirdParty-9
mv OpenFOAM-9-master OpenFOAM-9 && mv ThirdParty-9-master ThirdParty-9
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -INFO- ] $time_current Compiling started ..."
export MPI_ROOT=${mpi_root}
echo "${mpi_env}" | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  export MPI_ARCH_FLAGS="-DOMPI_SKIP_MPICXX"
else
  export MPI_ARCH_FLAGS="-DMPICH_SKIP_MPICXX"
fi
export MPI_ARCH_INC="-I$MPI_ROOT/include"
export MPI_ARCH_LIBS="-L$MPI_ROOT/lib -lmpi"
if [ ! -f  ${of_cache}OpenFOAM-9/etc/config.sh/settings-orig ]; then
  cp ${of_cache}OpenFOAM-9/etc/config.sh/settings ${of_cache}OpenFOAM-9/etc/config.sh/settings-orig
else
  /bin/cp ${of_cache}OpenFOAM-9/etc/config.sh/settings-orig ${of_cache}OpenFOAM-9/etc/config.sh/settings 
fi
if [ ! -f ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c-orig ]; then
  cp ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c-orig
else
  /bin/cp ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c-orig ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c
fi
if [ ! -f ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig ]; then
  cp ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++ ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig
else
  /bin/cp  ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++
fi
if [ ! -f ${of_cache}OpenFOAM-9/etc/bashrc-orig ]; then
  cp ${of_cache}OpenFOAM-9/etc/bashrc ${of_cache}OpenFOAM-9/etc/bashrc-orig
else
  /bin/cp ${of_cache}OpenFOAM-9/etc/bashrc-orig ${of_cache}OpenFOAM-9/etc/bashrc
fi
if [ $gcc_vnum -gt 10 ]; then
  cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC"  >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    cpu_model=`cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC" | head -n1 | awk '{print $6}'`
    cpu_gen=${cpu_model: -1}
    if [ $cpu_gen = '3' ]; then
      line1=`sed = ${of_cache}OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      line2=`sed = ${of_cache}OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CXXFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      sed -i "${line1} s/-fPIC'/-fPIC -march=znver3'/g" ${of_cache}OpenFOAM-9/etc/config.sh/settings
      sed -i "${line2} s/-fPIC -std/-fPIC -march=znver3 -std/g" ${of_cache}OpenFOAM-9/etc/config.sh/settings
      sed -i 's/-fPIC/-fPIC -march=znver3/g' ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver3/g' ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++
    elif [ $cpu_gen = '2' ]; then
      line1=`sed = ${of_cache}OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      line2=`sed = ${of_cache}OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CXXFLAGS | tail -n1 | awk '{print $1}' |  awk -F":" '{print $1}'`
      sed -i "${line1} s/-fPIC'/-fPIC -march=znver2'/g" ${of_cache}OpenFOAM-9/etc/config.sh/settings
      sed -i "${line2} s/-fPIC'/-fPIC -march=znver2'/g" ${of_cache}OpenFOAM-9/etc/config.sh/settings
      sed -i 's/-fPIC/-fPIC -march=znver2/g' ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver2/g' ${of_cache}OpenFOAM-9/wmake/rules/linux64Gcc/c++
    fi
  fi
fi    
sed -i 's/export WM_MPLIB=SYSTEMOPENMPI/export WM_MPLIB=SYSTEMMPI/g' ${of_cache}OpenFOAM-9/etc/bashrc
. ${of_cache}OpenFOAM-9/etc/bashrc
echo -e "[ -INFO- ] Building OpenFOAM in progress ... It takes really long time (for example, 2.5 hours with 8 vCPUs)"
echo -e "[ -INFO- ] Please check the log files: Build_OF9.log."
nohup ${of_cache}OpenFOAM-9/Allwmake -j$num_processors > ${of_cache}Build_OF9.log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Building OpenFOAM-9 failed. Please check the Build_OF9.log and retry later. Exit now."
  exit
fi
echo -e "[ -INFO- ] Copying files ..."
rsync -a --info=progress2 ${of_cache} ${of_root}
echo -e "#! /bin/bash\nmodule purge" > ${of_root}of9.sh
echo -e "export MPI_ROOT=\"${MPI_ROOT}\"" >> ${of_root}of9.sh
echo -e "export MPI_ARCH_FLAGS=\"${MPI_ARCH_FLAGS}\"" >> ${of_root}of9.sh
echo -e "export MPI_ARCH_INC=\"${MPI_ARCH_INC}\"" >> ${of_root}of9.sh
echo -e "export MPI_ARCH_LIBS=\"${MPI_ARCH_LIBS}\"" >> ${of_root}of9.sh
echo -e "module load ${mpi_env}" >> ${of_root}of9.sh
if [ $systemgcc = 'false' ]; then
  echo -e "module load ${gcc_env}" >> ${of_root}of9.sh
fi 
echo -e "source ${of_root}OpenFOAM-9/etc/bashrc" >> ${of_root}of9.sh
echo -e "echo -e \"OpenFOAM9 with ${mpi_env} and ${gcc_v} is ready for running.\"" >> ${of_root}of9.sh
if [ $current_user = 'root' ]; then
  grep of9 /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias of9.env='source ${of_root}of9.sh'" >> /etc/profile
  fi
  echo -e "< of9 >" >> $public_app_registry
else
  grep of9 $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias of9.env='source ${of_root}of9.sh'" >> $HOME/.bashrc
  fi
  echo -e "< of9 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] Congratulations! OpenFOAM9 with ${mpi_env} and ${gcc_v} has been built."