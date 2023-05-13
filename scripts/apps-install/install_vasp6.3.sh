#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *VASP-6.3.0* to HPC-NOW cluster.

tmp_log=/tmp/hpcmgr_install.log

echo -e "[ -INFO- ] Software: VASP-6.3.0\n"
echo -e "**************** IMPORTANT! ****************"
echo -e "*    VASP is non-free software licensed    *"
echo -e "*    by VASP Software GmbH located at      *"
echo -e "*    Sensengasse 8/12, A-1090, Vienna,     *"
echo -e "*    Austria. Shanghai HPC-NOW Technol-    *"
echo -e "*    -ogies Co., Ltd (HPC-NOW) doesn't     *"
echo -e "*    provide commercial license to you.    *"
echo -e "*    Please make sure you are licensed     *"
echo -e "*    by the software vendor or officia-    *" 
echo -e "*    -lly authorized resellers before      *"
echo -e "*    installation! HPC-NOW doesn't pro-    *"
echo -e "*    -vide any technical support on any    *"
echo -e "*    version(s) of VASP.                   *"
echo -e "********************************************\n"
echo -e "[ -INFO- ] If you are *NOT* sure whether you've been licensed. Please press keyboard 'Ctrl C' to stop current process."
echo -ne "[ -WAIT- ] |"
for i in $( seq 1 10)
do
  sleep 1
  echo -ne "..$((11-i)).."
done
echo -e "|"

if [[ -f /hpc_apps/vasp.6.3.0/bin/vasp_std || -f /hpc_apps/vasp.6.3.0/bin/vasp_gam || -f /hpc_apps/vasp.6.3.0/bin/vasp_ncl ]]; then
  echo -e "[ -INFO- ] It seems VASP-6.3.0 binaries are in place."
  echo -e "[ -INFO- ] If you REALLY want to rebuild, please move the previous binaries to other folders and retry. Exit now.\n"
  exit
fi
if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi
echo -e "[ -INFO- ] Checking Prerequisitions: MPI version ..."
hpcmgr install envmod >> $tmp_log 2>&1
module ava -t | grep ompi >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  mpi_version=`module ava -t | grep ompi | tail -n1 | awk '{print $1}'`
  module purge
  module load $mpi_version
  echo -e "[ -INFO- ] VASP will be built with $mpi_version."
else
  module ava -t | grep mpich >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    mpi_version=`module ava -t | grep mpich | tail -n1 | awk '{print $1}'`
    module purge
    module load $mpi_version
    echo -e "[ -INFO- ] VASP will be built with $mpi_version."
  else
    echo -e "[ -INFO- ] No MPI version found, installing OpenMPI-4.1.2 now..."
    hpcmgr install ompi4 >> ${tmp_log} 2>&1
    if [ $? -ne 0 ]; then
      echo -e "[ FATAL: ] Failed to install OpenMPI-4.1.2. Installation abort. Please check the log file for details. Exit now."
      exit
    else
      echo -e "[ -INFO- ] OpenMPI-4.1.2 has been successfully built."
      mpi_version=ompi-4.1.2
      module purge
      module load $mpi_version
    fi
  fi
fi
echo -e "[ -INFO- ] Checking Prerequisitions: GNU Compiler Collections - GCC version ..."
module ava -t | grep gcc-12.1.0 >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load gcc-12.1.0
  gcc_v=gcc-12.1.0
  gcc_vnum=12
  systemgcc='false'
  echo -e "[ -INFO- ] VASP will be built with GNU C Compiler: $gcc_v"
else
  module ava -t | grep gcc-8.2.0 >> /dev/null 2>&1
  if [ $? -eq 0 ]; then
    module load gcc-8.2.0
    gcc_v=gcc-8.2.0
    gcc_vnum=8
    systemgcc='false'
    echo -e "[ -INFO- ] VASP will be built with GNU C Compiler: $gcc_v"
  else
    gcc_v=`gcc --version | head -n1`
    gcc_vnum=`echo $gcc_v | awk '{print $3}' | awk -F"." '{print $1}'`
    systemgcc='true'
    echo -e "[ -INFO- ] VASP will be built with GNU C Compiler: $gcc_v"
    if [ $gcc_vnum -lt 8 ]; then
      echo -e "[ -WARN- ] Your gcc version is too old to compile VASP. Will start installing gcc-12.1.0 which may take long time."
      echo -e "[ -WARN- ] You can press keyboard 'Ctrl C' to stop current building process."
      echo -ne "[ -WAIT- ] |--> "
      for i in $( seq 1 10)
      do
	      sleep 1
        echo -ne "$((11-i))--> "
      done
      echo -e "|\n[ -INFO- ] Building gcc-12.1.0 now ..."
      hpcmgr install gcc12 >> ${tmp_log} 2>&1
      gcc_v=gcc-12.1.0
      gcc_vnum=12
      systemgcc='false'
      module load gcc-12.2.0
    fi
  fi
fi
echo -e "[ -INFO- ] Checking Prerequisitions: Libraries - LAPACK, ScaLAPACK, & FFTW3 ..."
if [[ ! -f /hpc_apps/lapack-3.11/libcblas.a || ! -f /hpc_apps/lapack-3.11/liblapack.a || ! -f /hpc_apps/lapack-3.11/liblapacke.a || ! -f /hpc_apps/lapack-3.11/librefblas.a || ! -f /hpc_apps/lapack-3.11/libtmglib.a ]]; then
  echo -e "[ -INFO- ] LAPACK lib(s) is(are) not in place, installing now ..."
  hpcmgr install lapk311 >> ${tmp_log} 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install LAPACK-3.11.0. Installation abort. Please check the log file for details. Exit now."
    exit
  else
    echo -e "[ -INFO- ] LAPACK-3.11.0 has been successfully built."
  fi
fi
if [ ! -f /hpc_apps/scalapack/libscalapack.a ]; then
  echo -e "[ -INFO- ] ScaLAPACK lib is not in place, installing now ..."
  hpcmgr install slpack2 >> ${tmp_log}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install ScaLAPACK. Installation abort. Please check the log file for details. Exit now."
    exit
  else
    echo -e "[ -INFO- ] ScaLAPACK library has been successfully built."
  fi
fi

if [ ! -d /hpc_apps/openblas/lib/ ]; then
  echo -e "[ -INFO- ] OpenBLAS lib is not in place, installing now ..."
  hpcmgr install oblas >> ${tmp_log} 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install OpenBLAS. Installation abort. Please check the log file for details. Exit now."
    exit
  else
    echo -e "[ -INFO- ] OpenBLAS library has been successfully built."
  fi
fi

if [[ ! -d /hpc_apps/fftw3/lib || ! -d /hpc_apps/fftw3/include ]]; then
  echo -e "[ -INFO- ] FFTW-3 lib is not in place, installing now ... "
  hpcmgr install fftw3 >> ${tmp_log} 2>&1
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install fftw3. Installation abort. Please check the log file for details. Exit now."
    exit
  else
    echo -e "[ -INFO- ] fftw3 library has been successfully built."
  fi
fi
echo -e "[ -INFO- ] Prerequisition Checked. Will start building VASP-6.3.0."

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_PKGS=${URL_ROOT}packages/
time_current=`date "+%Y-%m-%d %H:%M:%S"`
logfile=/var/log/hpcmgr_install.log && echo -e "\n# $time_current INSTALLING VASP-6.3.0" >> ${logfile}
APP_ROOT=/hpc_apps
NUM_PROCESSORS=`cat /proc/cpuinfo| grep "processor"| wc -l`
echo -e "[ START: ] Downloading and Extracting source code ..."
if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi
if [ ! -f /opt/packs/vasp.6.3.0.zip ]; then
  wget ${URL_PKGS}vasp.6.3.0.zip -q -O /opt/packs/vasp.6.3.0.zip
fi
rm -rf $APP_ROOT/vasp.6.3.0 && unzip -q /opt/packs/vasp.6.3.0.zip -d $APP_ROOT
echo -e "[ -INFO- ] Building binaries from the source code ... This step usually take minutes..."
cd $APP_ROOT/vasp.6.3.0 && /bin/cp arch/makefile.include.gnu makefile.include
sed -i 's@OPENBLAS_ROOT ?= /path/to/your/openblas/installation@OPENBLAS_ROOT = /hpc_apps/openblas@g' $APP_ROOT/vasp.6.3.0/makefile.include
sed -i 's@SCALAPACK_ROOT ?= /path/to/your/scalapack/installation@SCALAPACK_ROOT = /hpc_apps/scalapack@g' $APP_ROOT/vasp.6.3.0/makefile.include
sed -i 's@SCALAPACK   = -L$(SCALAPACK_ROOT)/lib -lscalapack@SCALAPACK   = -L$(SCALAPACK_ROOT) -lscalapack@g' $APP_ROOT/vasp.6.3.0/makefile.include
sed -i 's@FFTW_ROOT  ?= /path/to/your/fftw/installation@FFTW_ROOT  = /hpc_apps/fftw3@g' $APP_ROOT/vasp.6.3.0/makefile.include
echo -e "CPP_OPTIONS += -DLAPACK36" >> $APP_ROOT/vasp.6.3.0/makefile.include
echo -e "! routines replaced in LAPACK >=3.6\n#ifdef LAPACK36\n#define DGEGV DGGEV\n#endif" >> $APP_ROOT/vasp.6.3.0/src/symbol.inc
cd $APP_ROOT/vasp.6.3.0 && rm -rf build/*
make std >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_std. Please check the log file and retry. Exit now."
  exit
fi
echo -e "[ -INFO- ] vasp_std built."
make gam >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_gam. Please check the log file and retry. Exit now."
  exit
fi
echo -e "[ -INFO- ] vasp_gam built."
make ncl >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_ncl. Please check the log file and retry. Exit now."
  exit
fi
chmod +x /hpc_apps/vasp.6.3.0/bin/*
echo -e "[ -INFO- ] vasp_ncl built." && cd $APP_ROOT
if [ $systemgcc = 'true' ]; then
  echo -e "#! /bin/bash\nmodule purge\nmodule load $mpi_version\nmodule load fftw3\nmodule load openblas\nexport PATH=/hpc_apps/vasp.6.3.0/bin:\$PATH\necho \"VASP 6.3.0 is ready for running\"" > $APP_ROOT/vasp.6.3.0/vasp6.3.sh
else
  echo -e "#! /bin/bash\nmodule purge\nmodule load $mpi_version\nmodule load $gcc_v\nmodule load fftw3\nmodule load openblas\nexport PATH=/hpc_apps/vasp.6.3.0/bin:\$PATH\necho \"VASP 6.3.0 is ready for running\"" > $APP_ROOT/vasp.6.3.0/vasp6.3.sh
fi
cat /etc/profile | grep "alias vasp63=" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "alias vasp63='source $APP_ROOT/vasp.6.3.0/vasp6.3.sh'" >> /etc/profile
fi
source /etc/profile
echo -e "[ -DONE- ] VASP-6.3.0 has been successfully installed to your cluster." 