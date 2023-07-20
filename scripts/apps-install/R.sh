#!/bin/bash

# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'hpcmgr' command to build *R & RStudio* to HPC-NOW cluster.

# THIS Script *ONLY* works and validated for CentOS Stream 9
# It is quite difficult for CentOS 7 to run R environment, considering so many dependencies are needed. 
# Therefore, CentOS Stream or Fedora would be a good choice to aviod as many dependency problems as possible.

current_user=`whoami`
url_root=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/
url_pkgs=${url_root}packages/
num_processors=`cat /proc/cpuinfo | grep "processor" | wc -l`
if [ $current_user != 'root' ]; then
  echo -e "[ FATAL: ] ONLY root user or user1 with sudo can $1 this app."
  echo -e "           Please contact the administrator. Exit now."
  exit 1
fi
public_app_registry="/usr/hpc-now/.public_apps.reg"
app_root="/hpc_apps/"
app_cache="/hpc_apps/.cache/"
app_extract_cache="/root/.app_extract_cache/"
tmp_log="/tmp/hpcmgr_install_R_${current_user}.log"
centos_v=`cat /etc/redhat-release | awk '{print $4}' | awk -F"." '{print $1}'`
if [ $centos_v -eq 7 ]; then
  echo -e "[ FATAL: ] R & RStudio can not be installed to CentOS 7.x. Exit now."
  exit 3
fi
mkdir -p ${app_cache}
mkdir -p ${app_extract_cache}

if [ $1 = 'remove' ]; then
  echo -e "[ -INFO- ] Removing RStudio ..."
  rpm -e rstudio
  echo -e "[ -INFO- ] Removing R environment ..."
  rm -rf ${app_root}R-4.2.1
  echo -e "[ -INFO- ] Removing openssl-1.1.1q ..."
  rm -rf ${app_root}openssl-1.1.1q
  echo -e "[ -INFO- ] Removing gdal-3.5.2 ..."
  rm -rf ${app_extract_cache}gdal-3.5.2
  echo -e "[ -INFO- ] Removing proj & geos-devel ..."
  yum -y install proj-devel geos-devel >> $tmp_log
  echo -e "[ -INFO- ] Removing envrionment varables ..."
  sed -i '/R-4.2.1/d' /etc/profile
  sed -i '/openssl-1.1.1q/d' /etc/profile
  echo -e "[ -INFO- ] Removing desktop shortcuts ..."
  rm -rf $HOME/Desktop/rstudio.desktop
  find /home -name "Desktop" > /tmp/desktop_dirs.txt
  while read rows
  do 
    rm -rf $HOME/Desktop/rstudio.desktop
  done < /tmp/desktop_dirs.txt
  rm -rf /tmp/desktop_dirs.txt
  sed -i '/< R >/d' ${public_app_registry}
  sed -i '/< r_env >/d' ${public_app_registry}
  sed -i '/< openssl-1.1.1q >/d' ${public_app_registry}
  echo -e "[ -INFO- ] R & RStudio removed. JAVA packages not removed."
  echo -e "[ -INFO- ] You can remove manually by 'yum -y remove java java-devel'."
  exit 0
fi

yum list installed -q | grep gnome-desktop >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo -e "[ -INFO- ] This app needs desktop environment. Installing now ..."
  hpcmgr install desktop >> ${tmp_log}
  if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] Desktop environment installation failed. Please check the log file for details. Exit now."
    exit 5
  fi
fi

source /etc/profile
echo -e "[ STEP 1 ] Installing OpenSSL-1.1.1 ... "
# RStudio needs OpenSSL-1.1.1
yum -y install perl-devel perl-App-cpanminus 
cpanm FindBin
if [ ! -f ${app_cache}openssl-1.1.1q.tar.gz ]; then
  wget ${url_pkgs}openssl-1.1.1q.tar.gz -O ${app_cache}openssl-1.1.1q.tar.gz
fi
grep "< openssl-1.1.1q >" $public_app_registry >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  tar zvxf ${app_cache}openssl-1.1.1q.tar.gz -C ${app_extract_cache} >> $tmp_log
  cd ${app_extract_cache}openssl-1.1.1q
  ./config --prefix=${app_root}openssl-1.1.1q >> $tmp_log
  make -j$num_processors >> $tmp_log
  make install >> $tmp_log
  grep "openssl-1.1.1q/lib" /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export LD_LIBRARY_PATH=${app_root}openssl-1.1.1q/lib:\$LD_LIBRARY_PATH\nexport PATH=${app_root}openssl-1.1.1q/bin:\$PATH\n" >> /etc/profile
  fi
  source /etc/profile
  echo -e "< openssl-1.1.1q >" >> $public_app_registry
fi

echo -e "[ STEP 2 ] Installing R ... "
# Install R
yum -y install readline-devel gcc-gfortran gcc-c++ libXt-devel zlib-devel bzip2-devel lzma xz-devel pcre2-devel libcurl-devel java java-devel
JAVAHOME=`find /usr/lib/jvm -name "*-openjdk-*" | grep java- `
sed -i '/JAVA_HOME/d' /etc/profile
echo -e "export JAVA_HOME=$JAVAHOME" >> /etc/profile
echo -e "export LD_LIBRARY_PATH=\$JAVA_HOME/lib:\$LD_LIBRARY_PATH" >> /etc/profile
echo -e "export C_INCLUDE_PATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$C_INCLUDE_PATH" >> /etc/profile
echo -e "export CPLUS_INCLUDE_PATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$CPLUS_INCLUDE_PATH" >> /etc/profile
echo -e "export PATH=\$JAVA_HOME/bin:\$PATH" >> /etc/profile
echo -e "export CPATH=\$JAVA_HOME/include:\$JAVA_HOME/include/linux:\$CPATH" >> /etc/profile
source /etc/profile

grep "< r_env >" $public_app_registry >> /dev/null 2>&1
if [ $? -ne 0 ]; then
  if [ ! -f ${app_cache}R-4.2.1.tar.gz ]; then
    wget ${url_pkgs}R-4.2.1.tar.gz -O ${app_cache}R-4.2.1.tar.gz 
  fi
  tar zvxf ${app_cache}R-4.2.1.tar.gz -C ${app_extract_cache} >> $tmp_log
  cd ${app_extract_cache}R-4.2.1
  ./configure --enable-R-shlib --prefix=${app_root}R-4.2.1 >> $tmp_log
  make -j$num_processors >> $tmp_log
  make install >> $tmp_log
  grep ${app_root}R-4.2.1/bin /etc/profile >> /dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo -e "export PATH=${app_root}R-4.2.1/bin:\$PATH" >> /etc/profile
    echo -e "export LD_LIBRARY_PATH=${app_root}R-4.2.1/lib64:\$LD_LIBRARY_PATH" >> /etc/profile
    source /etc/profile
  fi
  echo -e "< r_env >" >> $public_app_registry
fi

echo -e "[ STEP 3 ] Installing RStudio ... "
# Install RStudio
yum -y install postgresql-devel postgresql sqlite >> $tmp_log
if [ ! -f ${app_cache}rstudio-2022.07.1-554-x86_64-centos9.rpm ]; then
  wget ${url_pkgs}rstudio-2022.07.1-554-x86_64-centos9.rpm -O ${app_cache}rstudio-2022.07.1-554-x86_64-centos9.rpm
fi
rpm -ivh ${app_cache}rstudio-2022.07.1-554-x86_64-centos9.rpm >> $tmp_log
# RStudio & R should work well. The problems are in installing other packages.

echo -e "[ STEP 4 ] Installing JAVA Support ... "
yum -y install cmake3 icu libicu-devel libjpeg-turbo-devel libpng-devel netcdf netcdf-devel python-devel >> $tmp_log
R CMD javareconf
# JAVA support should work well now. proj & gdal might be important for some R packages. Therefore we need to build them

echo -e "[ STEP 4 ] Installing gdal ... This step usually takes tens of minutes. "
# To build gdal really costs time. To be optimized later.
yum -y install proj-devel geos-devel
if [ ! -f ${app_cache}gdal-3.5.2.tar.gz ]; then
  wget -q ${url_pkgs}gdal-3.5.2.tar.gz -O ${app_cache}gdal-3.5.2.tar.gz
fi
tar zvxf ${app_cache}gdal-3.5.2.tar.gz -C ${app_extract_cache} >> $tmp_log
cd ${app_extract_cache}gdal-3.5.2
./configure --prefix=${app_root}gdal-3.5.2 >> $tmp_log
make -j$num_processors >> $tmp_log
make install >> $tmp_log
# NOW, you can use R & RStudio environment for data processing
echo -e "[ -INFO- ] Creating a shortcut on the desktop ..."
echo -e "[Desktop Entry]" > $HOME/Desktop/rstudio.desktop
echo -e "Encoding=UTF-8" >> $HOME/Desktop/rstudio.desktop
echo -e "Version=1.0" >> $HOME/Desktop/rstudio.desktop
echo -e "Name=RStudio" >> $HOME/Desktop/rstudio.desktop
echo -e "Comment=RStudio" >> $HOME/Desktop/rstudio.desktop
echo -e "Exec=/bin/rstudio" >> $HOME/Desktop/rstudio.desktop
echo -e "Icon=/opt/app.png" >> $HOME/Desktop/rstudio.desktop
echo -e "Terminal=false" >> $HOME/Desktop/rstudio.desktop
echo -e "StartupNotify=true" >> $HOME/Desktop/rstudio.desktop
echo -e "Type=Application" >> $HOME/Desktop/rstudio.desktop
echo -e "Categories=Applications;" >> $HOME/Desktop/rstudio.desktop

find /home -name "Desktop" > /tmp/desktop_dirs.txt
while read rows
do 
  user_row=`echo $rows | awk -F"/" '{print $3}'`
  /bin/cp $HOME/Desktop/rstudio.desktop ${rows}
  chown -R ${user_row}:${user_row} ${rows}
done < /tmp/desktop_dirs.txt
rm -rf /tmp/desktop_dirs.txt

echo -e "[ -WARN- ] when install package rgdal, please use 'install.packages(\"rgdal\",configure.args = \"--host=x86_64\")' to specify configure vars."
echo -e "< R >" >> ${public_app_registry}
echo -e "[ -DONE- ] R & RStudio has been installed to the cluster."