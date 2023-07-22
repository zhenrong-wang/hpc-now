#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *VASP-6.1.0* to HPC-NOW cluster.

current_user=`whoami`
public_app_registry="/hpc_apps/.public_apps.reg"
if [ $current_user != 'root' ]; then
  private_app_registry="/hpc_apps/${current_user}_apps/.private_apps.reg"
fi
tmp_log="/tmp/hpcmgr_install_vasp6.1_${current_user}.log"

url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo| grep "processor"| wc -l`
centos_ver=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`

if [ $current_user = 'root' ]; then
  app_root="/hpc_apps/"
  app_extract_cache="/root/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/"
else
  app_root="/hpc_apps/${current_user}_apps/"
  app_extract_cache="/home/${current_user}/.app_extract_cache/"
  envmod_root="/hpc_apps/envmod/${current_user}_env/"
fi
mkdir -p ${app_extract_cache}
mkdir -p ${app_root}vasp.6.1.0 && rm -rf ${app_root}vasp.6.1.0/*

echo -e "\n****************** ! DISCLAIMER ! ******************"
if [ $1 != 'remove' ]; then
  echo -e "* Place the source code to ${app_root}vasp.6.1.0.zip."
fi
echo -e "* This script doesn't provide source code of VASP. "
echo -e "* You need to contact the vendor to get the codes."
echo -e "****************** ! DISCLAIMER ! ******************\n"

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing the binaries and libraries ..."
  rm -rf ${app_root}vasp.6.1.0
  echo -e "[ -INFO- ] Removing environment variables ..."
  if [ $current_user = 'root' ]; then
    sed -i '/< vasp6.1 >/d' $public_app_registry
    sed -i '/vasp6.1.env=/d' /etc/profile
  else
    sed -e "/< vasp6.1 > < ${current_user} >/d" $private_app_registry > /tmp/sed_${current_user}.tmp
    cat /tmp/sed_${current_user}.tmp > $private_app_registry
    rm -rf /tmp/sed_${current_user}.tmp
    sed -i '/vasp6.1.env=/d' $HOME/.bashrc
  fi
  echo -e "[ -INFO- ] VASP-6.1.0 has been removed successfully."
  exit 0
fi

if [ ! -f ${app_root}vasp.6.1.0.zip ]; then
  echo -e "[ FATAL: ] Source code ${app_root}vasp.6.1.0.zip not found. Exit now."
  exit 3
fi

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
ompi_vers=('ompi4' 'ompi3' 'mpich4' 'mpich3')
ompi_code=('ompi-4.1.2' 'ompi-3.1.6' 'mpich-4.0.2' 'mpich-3.2.1')
for i in $(seq 0 3)
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
  hpcmgr install ompi4 >> $tmp_log
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

echo -e "[ -INFO- ] Checking and Installing Prerequisitions: LAPACK-3.11.0 ..."
hpcmgr install lapack311 >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build lapack-3.11.0. Exit now."
  exit 5
fi
grep "< lapack311 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load lapack-3.11
  lapack_root="/hpc_apps/lapack-3.11/"
  lapack_env="lapack-3.11"
else
  module load ${current_user}_env/lapack-3.11
  lapack_root="/hpc_apps/${current_user}_apps/lapack-3.11/"
  lapack_env="${current_user}_env/lapack-3.11"
fi
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: ScaLAPACK ..."
hpcmgr install slpack2 >> $tmp_log 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build ScaLAPACK. Exit now."
  exit 7
fi
grep "< slpack2 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load scalapack
  scalapack_root="/hpc_apps/scalapack/"
  scalapack_env="scalapack"
else
  module load ${current_user}_env/scalapack
  scalapack_root="/hpc_apps/${current_user}_apps/scalapack/"
  scalapack_env="${current_user}_env/scalapack"
fi
echo -e "[ -INFO- ] Checking and Installing Prerequisitions: FFTW-3.3.10 ..."
hpcmgr install fftw3 >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build fftw3. Exit now."
  exit 7
fi
grep "< fftw3 >" $public_app_registry >> /dev/null 2>&1
if [ $? -eq 0 ]; then
  module load fftw3
  fftw3_root="/hpc_apps/fftw3/"
  fftw3_env="fftw3"
else
  module load ${current_user}_env/fftw3
  fftw3_root="/hpc_apps/${current_user}_apps/fftw3/"
  fftw3_env="${current_user}_env/fftw3"
fi
echo -e "[ -INFO- ] Prerequisition Checked. Will start building VASP-6.1.0."
echo -e "[ START: ] Extracting the source code at ${app_root}vasp.6.1.0.zip ..."
unzip -o ${app_root}vasp.6.1.0.zip -d ${app_extract_cache} >> $tmp_log
echo -e "[ -INFO- ] Building binaries from the source code, this step usually take minutes..."
cd ${app_extract_cache}vasp.6.1.0
/bin/cp arch/makefile.include.linux_gnu makefile.include
if [ $gcc_vnum -gt 10 ]; then
  sed -i 's/FFLAGS     = -w -march=native/FFLAGS     = -w -march=native -fPIC -fallow-argument-mismatch/g' ${app_extract_cache}vasp.6.1.0/makefile.include
else
  sed -i 's/FFLAGS     = -w -march=native/FFLAGS     = -w -march=native -fPIC/g' ${app_extract_cache}vasp.6.1.0/makefile.include
fi
sed -i "s@LIBDIR     = /opt/gfortran/libs/@LIBDIR     = ${lapack_root}@g" ${app_extract_cache}vasp.6.1.0/makefile.include
sed -i "s@SCALAPACK  = -L\$(LIBDIR) -lscalapack \$(BLACS)@SCALAPACK  = -L${scalapack_root} -lscalapack \$(BLACS)@g" ${app_extract_cache}vasp.6.1.0/makefile.include
sed -i "s@FFTW       ?= /opt/gfortran/fftw-3.3.6-GCC-5.4.1@FFTW       = ${fftw3_root}@g" ${app_extract_cache}vasp.6.1.0/makefile.include
sed -i "s@MPI_INC    = /opt/gfortran/openmpi-1.10.2/install/ompi-1.10.2-GFORTRAN-5.4.1/include@MPI_INC    = ${mpi_root}include@g" ${app_extract_cache}vasp.6.1.0/makefile.include
echo -e "CPP_OPTIONS += -DLAPACK36" >> ${app_extract_cache}vasp.6.1.0/makefile.include
echo -e "! routines replaced in LAPACK >=3.6\n#ifdef LAPACK36\n#define DGEGV DGGEV\n#endif" >> ${app_extract_cache}vasp.6.1.0/src/symbol.inc
cd ${app_extract_cache}vasp.6.1.0 && rm -rf build/*
make std >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_std. Please check the log file and retry. Exit now."
  exit 3
fi
echo -e "[ -INFO- ] vasp_std built."
make gam >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_gam. Please check the log file and retry. Exit now."
  exit 3
fi
echo -e "[ -INFO- ] vasp_gam built."
make ncl >> ${tmp_log} 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ FATAL: ] Failed to build vasp_ncl. Please check the log file and retry. Exit now."
  exit 3
fi
echo -e "[ -INFO- ] vasp_ncl built."
mkdir -p ${app_root}vasp.6.1.0/bin
rm -rf ${app_root}vasp.6.1.0/bin/*
/bin/cp -r build/std/vasp ${app_root}vasp.6.1.0/bin/vasp_std
/bin/cp -r build/ncl/vasp ${app_root}vasp.6.1.0/bin/vasp_ncl
/bin/cp -r build/gam/vasp ${app_root}vasp.6.1.0/bin/vasp_gam
echo -e "#! /bin/bash\nmodule purge" > ${app_root}vasp.6.1.0/vasp6.1.sh
echo -e "module load ${fftw3_env}\nmodule load ${scalapack_env}\nmodule load ${lapack_env}\nmodule load ${mpi_env}" >> ${app_root}vasp.6.1.0/vasp6.1.sh
if [ $systemgcc = 'false' ]; then
  echo -e "module load ${gcc_env}" >> ${app_root}vasp.6.1.0/vasp6.1.sh
fi
echo -e "export PATH=${app_root}vasp.6.1.0/bin:\$PATH" >> ${app_root}vasp.6.1.0/vasp6.1.sh
echo -e "echo -e \"VASP-6.1.0 is ready for running.\"" >> ${app_root}vasp.6.1.0/vasp6.1.sh

if [ $current_user = 'root' ]; then
  grep "alias vasp6.1.env" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias vasp6.1.env='source ${app_root}vasp.6.1.0/vasp6.1.sh'" >> /etc/profile
  fi
  echo -e "< vasp5 >" >> $public_app_registry
else
  grep "alias vasp6.1.env" $HOME/.bashrc >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "alias vasp6.1.env='source ${app_root}vasp.6.1.0/vasp6.1.sh'" >> $HOME/.bashrc
  fi
  echo -e "< vasp5 > < ${current_user} >" >> $private_app_registry
fi
echo -e "[ -DONE- ] VASP-6.1.0 has been successfully installed to your cluster." 