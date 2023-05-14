#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *OpenFOAM-9* to HPC-NOW cluster.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
source /etc/profile
tmp_log=/tmp/hpcmgr_install.log

time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING OpenFOAM-9" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
ls /hpc_apps/OpenFOAM/OpenFOAM-9/platforms/linux*/bin/*Foam >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  foam9check=`ls /hpc_apps/OpenFOAM/OpenFOAM-9/platforms/linux*/bin/*Foam | wc -l` 
  if [ $foam9check -gt $((80)) ]; then
    cat /etc/profile | grep of9 >> /dev/null 2>&1
    if [ $? -ne 0 ]; then
      echo -e "alias of9='source $APP_ROOT/OpenFOAM/of9.sh'" >> /etc/profile
    fi
    echo -e "[ -INFO- ] It seems $foam9check OpenFOAM9 binaries are in place."
    echo -e "[ -INFO- ] If you REALLY want to rebuild, please move the previous binaries to other folders and retry. Exit now." 
    exit
  fi
fi
echo -e "[ -INFO- ] Cleaning up processes..."
ps -aux | grep OpenFOAM-9/wmake | cut -c 9-15 | xargs kill -9 >> /dev/null 2>&1
if [[ -n $1 && $1 = 'rebuild' ]]; then
  echo -e "[ -WARN- ] The previously OpenFOAM and Third-party folder will be removed and re-created."
  rm -rf $APP_ROOT/OpenFOAM/OpenFOAM-9
  rm -rf $APP_ROOT/OpenFOAM/ThirdParty-9 
fi
#source /opt/environment-modules/init/bash
hpcmgr install envmod >> $tmp_log 2>&1
module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] OpenFOAM will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] OpenFOAM will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] OpenFOAM will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile OpenFOAM. Will start installing gcc-12.1.0 which may take long time."
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
      module load gcc-12.2.0
    fi
  fi
fi
module ava -t | grep mpich >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep mpich | tail -n1 | awk '{print $1}'`
  module load $mpi_version
  echo -e "[ -INFO- ] OpenFOAM will be built with $mpi_version."
else
  module ava -t | grep ompi >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
    module load $mpi_version
    echo -e "[ -INFO- ] OpenFOAM will be built with $mpi_version."
  else
    echo -e "[ -INFO- ] No MPI version found. Installing mpich-4.0.2 now ..."
    hpcmgr install mpich4 >> $tmp_log 2>&1
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Failed to install mpich. Exit now."
      exit
    fi
    mpi_version=mpich-4.0.2
    module load $mpi_version
  fi
fi
echo -e "[ START: ] $time_current Building OpenFOAM-9 now ... "
echo -e "[ START: ] $time_current Building OpenFOAM-9 now ... " >> $logfile
mkdir -p $APP_ROOT/OpenFOAM
echo -e "[ STEP 1 ] $time_current Downloading & extracting source packages ..."
echo -e "[ STEP 1 ] $time_current Downloading & extracting source packages ..." >> $logfile
if [ ! -f $APP_ROOT/OpenFOAM/OpenFOAM-9.zip ]; then
  wget ${URL_PKGS}OpenFOAM-9.zip -q -O $APP_ROOT/OpenFOAM/OpenFOAM-9.zip
fi  
if [ ! -f $APP_ROOT/OpenFOAM/ThirdParty-9.zip ]; then
  wget ${URL_PKGS}ThirdParty-9.zip -q -O $APP_ROOT/OpenFOAM/ThirdParty-9.zip
fi
if [ ! -d $APP_ROOT/OpenFOAM/OpenFOAM-9 ]; then
  cd $APP_ROOT/OpenFOAM && unzip -q OpenFOAM-9.zip
  mv OpenFOAM-9-master OpenFOAM-9
fi
if [ ! -d $APP_ROOT/OpenFOAM/ThirdParty-9 ]; then
  cd $APP_ROOT/OpenFOAM && unzip -q ThirdParty-9.zip
  mv ThirdParty-9-master ThirdParty-9
fi
echo -e "[ -INFO- ] Removing the tarballs are not recommended. If you want to rebuild OF9, please remove the folder OpenFOAM-9 & ThirdParty-9."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 2 ] $time_current Removing previously-built binaries ..."
echo -e "[ STEP 2 ] $time_current Removing previously-built binaries ..." >> $logfile
rm -rf $APP_ROOT/OpenFOAM/OpenFOAM-9/platforms/*
rm -rf $APP_ROOT/OpenFOAM/ThirdParty-9/platforms/* 
$APP_ROOT/OpenFOAM/ThirdParty-9/Allclean >> $APP_ROOT/OpenFOAM/Build_OF.log 2>&1
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Compiling started ..."
export MPI_ROOT=/hpc_apps/$mpi_version
export MPI_ARCH_FLAGS="-DMPICH_SKIP_MPICXX"
export MPI_ARCH_INC="-I$MPI_ROOT/include"
export MPI_ARCH_LIBS="-L$MPI_ROOT/lib -lmpi"
if [ ! -f  $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings-orig ]; then
  cp $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings-orig
else
  /bin/cp $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings-orig $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings 
fi
if [ ! -f $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c-orig ]; then
  cp $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c-orig
else
  /bin/cp $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c-orig $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c
fi
if [ ! -f $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig ]; then
  cp $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++ $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig
else
  /bin/cp  $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++-orig $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++
fi
if [ ! -f $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc-orig ]; then
  cp $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc-orig
else
  /bin/cp $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc-orig $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc
fi
if [ $gcc_vnum -gt 10 ]; then
  cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC"  >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    cpu_model=`cat /proc/cpuinfo | grep "model name" | grep "AMD EPYC" | head -n1 | awk '{print $6}'`
    cpu_gen=${cpu_model: -1}
    if [ $cpu_gen = '3' ]; then
      line1=`sed = $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      line2=`sed = $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CXXFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      sed -i "${line1} s/-fPIC'/-fPIC -march=znver3'/g" $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings
      sed -i "${line2} s/-fPIC -std/-fPIC -march=znver3 -std/g" $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings
      sed -i 's/-fPIC/-fPIC -march=znver3/g' $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver3/g' $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++
    elif [ $cpu_gen = '2' ]; then
      line1=`sed = $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CFLAGS | tail -n1 | awk '{print $1}' | awk -F":" '{print $1}'`
      line2=`sed = $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings | sed 'N;s/\n/:/' | sed -n '/x86_64)/,+20p' | grep CXXFLAGS | tail -n1 | awk '{print $1}' |  awk -F":" '{print $1}'`
      sed -i "${line1} s/-fPIC'/-fPIC -march=znver2'/g" $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings
      sed -i "${line2} s/-fPIC'/-fPIC -march=znver2'/g" $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/config.sh/settings
      sed -i 's/-fPIC/-fPIC -march=znver2/g' $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c
      sed -i 's/-fPIC/-fPIC -march=znver2/g' $APP_ROOT/OpenFOAM/OpenFOAM-9/wmake/rules/linux64Gcc/c++
    fi
  fi
fi    
sed -i 's/export WM_MPLIB=SYSTEMOPENMPI/export WM_MPLIB=SYSTEMMPI/g' $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc
. $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc
echo -e "[ -INFO- ] Building OpenFOAM in progress ... It takes really long time (for example, 2.5 hours with 8 vCPUs).\n[ -INFO- ] Please check the log files: Build_OF.log."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ STEP 3 ] $time_current Started compiling source codes ..." >> $logfile
$APP_ROOT/OpenFOAM/OpenFOAM-9/Allwmake -j$NUM_PROCESSORS > $APP_ROOT/OpenFOAM/Build_OF.log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Building OpenFOAM-9 failed. Please check the Build_OF.log and retry later. Exit now."
  exit
fi
if [ $systemgcc = 'true' ]; then
  echo -e "#! /bin/bash\nmodule purge\nexport MPI_ROOT=/hpc_apps/$mpi_version\nexport MPI_ARCH_FLAGS=\"-DMPICH_SKIP_MPICXX\"\nexport MPI_ARCH_INC=\"-I\$MPI_ROOT/include\"\nexport MPI_ARCH_LIBS=\"-L\$MPI_ROOT/lib -lmpi\"\nmodule load $mpi_version\nsource $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc\necho \"OpenFOAM9 with $mpi_version and system gcc: $gcc_v is ready for running.\"" > $APP_ROOT/OpenFOAM/of9.sh
else
  echo -e "#! /bin/bash\nmodule purge\nexport MPI_ROOT=/hpc_apps/$mpi_version\nexport MPI_ARCH_FLAGS=\"-DMPICH_SKIP_MPICXX\"\nexport MPI_ARCH_INC=\"-I\$MPI_ROOT/include\"\nexport MPI_ARCH_LIBS=\"-L\$MPI_ROOT/lib -lmpi\"\nmodule load $mpi_version\nmodule load $gcc_v\nsource $APP_ROOT/OpenFOAM/OpenFOAM-9/etc/bashrc\necho \"OpenFOAM9 with $mpi_version and $gcc_v is ready for running.\"" > $APP_ROOT/OpenFOAM/of9.sh
fi

cat /etc/profile | grep of9 >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias of9='source $APP_ROOT/OpenFOAM/of9.sh'" >> /etc/profile
fi
echo -e "[ -DONE- ] Congratulations! OpenFOAM9 with $mpi_version and $gcc_v has been built."
time_current=`date "+%Y-%m-%d %H:%M:%S"`
echo -e "[ -DONE- ] $time_current Congratulations! OpenFOAM9 with $mpi_version and $gcc_v has been built." >> $logfile