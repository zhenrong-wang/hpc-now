#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *R & RStudio* to HPC-NOW cluster.

# THIS Script *ONLY* works and validated for CentOS Stream 9
# It is quite difficult for CentOS 7 to run R environment, considering so many dependencies are needed. 
# Therefore, CentOS Stream or Fedora would be a good choice to aviod as many dependency problems as possible.

if [ ! -d /hpc_apps ]; then
  echo -e "[ FATAL: ] The root directory /hpc_apps is missing. Installation abort. Exit now."
  exit
fi

CENTOS_V=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ $CENTOS_V -ne 9 ]; then
  echo -e "[ FATAL: ] R & RStudio must be installed to CentOS Stream 9. Your current OS version doesn't satisfy this requirement. Exit now."
  exit
fi

if [ ! -d /opt/packs ]; then
  mkdir -p /opt/packs
fi

URL_ROOT=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
URL_UTILS=${URL_ROOT}utils/
URL_PKGS=${URL_ROOT}packages/
NUM_PROCESSORS=`cat /proc/cpuinfo | grep "processor" | wc -l`
APP_ROOT=/hpc_apps
tmp_log=/tmp/hpcmgr_install.log

yum list installed -q | grep gnome3 >> /dev/null 2>&1

if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] RStudio needs desktop enviroment. Installing now ... "
  hpcmgr install desktop >> $tmp_log
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Failed to install desktop environment. Please check the log file for details. Exit now."
    exit
  fi
fi

echo -e "[ STEP 1 ] Installing OpenSSL-1.1.1 ... "
# RStudio needs OpenSSL-1.1.1
yum -y install perl-devel perl-App-cpanminus >> $tmp_log
cpanm FindBin $tmp_log
wget -q ${URL_PKGS}openssl-1.1.1q.tar.gz -O /opt/packs/openssl-1.1.1q.tar.gz
cd /opt/packs && tar zxf openssl-1.1.1q.tar.gz
cd /opt/openssl-1.1.1q && ./config --prefix=/opt/openssl >> $tmp_log && make -j$NUM_PROCESSORS >> $tmp_log && make install >> $tmp_log
cat /etc/profile | grep "openssl/lib" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "export LD_LIBRARY_PATH=/opt/openssl/lib:/usr/local/lib:\$LD_LIBRARY_PATH\nexport PATH=/opt/openssl/bin:\$PATH\n" >> /etc/profile && source /etc/profile
fi
cat ~/.bashrc | grep "source /etc/profile" >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "source /etc/profile" >> ~/.bashrc
fi
rm -rf /opt/openssl-1*

echo -e "[ STEP 2 ] Installing R ... "
# Install R
yum -y install readline-devel gcc-gfortran gcc-c++ libXt-devel zlib-devel bzip2-devel lzma xz-devel pcre2-devel libcurl-devel >> $tmp_log
wget -q https://cran.r-project.org/src/base/R-4/R-4.2.1.tar.gz -O /opt/packs/R-4.2.1.tar.gz 
tar zxf /opt/packs/R-4.2.1.tar.gz -C /hpc_apps
cd /hpc_apps/R-4.2.1 && ./configure --enable-R-shlib >> $tmp_log && make -j$NUM_PROCESSORS >> $tmp_log && make install >> $tmp_log

echo -e "[ STEP 3 ] Installing RStudio ... "
# Install RStudio
yum -y install postgresql-devel postgresql sqlite >> $tmp_log
wget -q ${URL_PKGS}rstudio-2022.07.1-554-x86_64-centos9.rpm -O /opt/packs/rstudio-2022.07.1-554-x86_64-centos9.rpm
rpm -ivh /opt/rstudio-2022.07.1-554-x86_64-centos9.rpm >> $tmp_log
# RStudio & R should work well. The problems are in installing other packages.

echo -e "[ STEP 4 ] Installing JAVA Support ... "
yum -y install cmake3 icu libicu-devel java java-devel libjpeg-turbo-devel libpng-devel netcdf netcdf-devel python-devel >> $tmp_log
JAVAHOME=`find /usr/lib/jvm -name "*-openjdk-*" | grep java- `
sed -i '/JAVA_HOME/d' /etc/profile
echo -e "export JAVA_HOME=$JAVAHOME" >> /etc/profile
echo -e "export LD_LIBRARY_PATH=\$JAVA_HOME/lib:\$LD_LIBRARY_PATH" >> /etc/profile
echo -e "export C_INCLUDE_PATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$C_INCLUDE_PATH" >> /etc/profile
echo -e "export CPLUS_INCLUDE_PATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$CPLUS_INCLUDE_PATH" >> /etc/profile
echo -e "export PATH=\$JAVA_HOME/bin:\$PATH" >> /etc/profile
echo -e "export CPATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$CPATH" >> /etc/profile
source /etc/profile && R CMD javareconf
# JAVA support should work well now. proj & gdal might be important for some R packages. Therefore we need to build them

echo -e "[ STEP 4 ] Installing gdal ... This step usually takes tens of minutes. "
# To build gdal really costs time. To be optimized later.
yum -y install proj-devel geos-devel
wget -q ${URL_PKGS}gdal-3.5.2.tar.gz -O /opt/packs/gdal-3.5.2.tar.gz
tar zxf /opt/packs/gdal-3.5.2.tar.gz -C /hpc_apps && cd /hpc_apps/gdal-3.5.2 && ./configure && make -j$NUM_PROCESSORS && make install
# NOW, you can use R & RStudio environment for data processing
# NOTE: when install package rgdal, please use 'install.packages("rgdal",configure.args = "--host=x86_64")' to specify configure vars
echo -e "[ -DONE- ] R & RStudio has been installed to the cluster."