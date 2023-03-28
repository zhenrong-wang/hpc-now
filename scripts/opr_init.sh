#!/bin/bash
# Shanghai HPC-NOW Technologies Co., Ltd
# All rights reserved, Year 2023
# https://www.hpc-now.com
# mailto: info@hpc-now.com 
# This script is used by 'now-installer'.

AWS_PROVIDER_PATH=/home/hpc-now/.terraform.d/plugins/registry.terraform.io/hashicorp/aws/4.53.0/linux_amd64/
ALI_PROVIDER_PATH=/home/hpc-now/.terraform.d/plugins/registry.terraform.io/aliyun/alicloud/1.193.0/linux_amd64/
TENCENT_PROVIDER_PATH=/home/hpc-now/.terraform.d/plugins/registry.terraform.io/tencentcloudstack/tencentcloud/1.79.7/linux_amd64/
URL_TERRAFORM=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/terraform/
URL_SSHPASS=https://hpc-now-1308065454.cos.ap-guangzhou.myqcloud.com/utils/sshpass-1.08.tar.gz
AWS_PROVIDER_NAME=terraform-provider-aws_4.53.0_linux_amd64.zip
ALI_PROVIDER_NAME=terraform-provider-alicloud_1.193.0_linux_amd64.zip
TENCENT_PROVIDER_NAME=terraform-provider-tencentcloud_1.79.7_linux_amd64.zip
LOGFILE=/tmp/now-installer.log

# Check the package manager, try yum first
which yum >> /dev/null
if [ $? -eq 0 ]; then
  yum install epel-release -y >> ${LOGFILE} 2>&1
  if [ $? -ne 0 ]; then
    #try amazon linux installation
    amazon-linux-extras install epel -y
  fi
  yum install wget unzip sshpass -y >> ${LOGFILE} 2>&1
else # If not yum, try apt then
  which apt-get >> /dev/null
  if [ $? -eq 0 ]; then
    apt-get install wget unzip sshpass -y >> ${LOGFILE} 2>&1
  else
    exit 1 # If no yum or apt, exit with error
  fi
fi
# Download terraform binary to the /usr/bin folder
if [ ! -f /usr/bin/terraform ]; then 
  wget ${URL_TERRAFORM}terraform -O /usr/bin/terraform -q >> /tmp/now-installer.log 2>&1 
  chmod +x /usr/bin/terraform
fi

# create a file to generate the global crypto seed
if [ ! -f /etc/.now_crypto_seed.lock ]; then
  date +%s%N | md5sum | head -c 10 > /etc/.now_crypto_seed.lock
  chattr +i /etc/.now_crypto_seed.lock
fi

# Configure local provider folder to avoid installing from the github. 
echo -e "privider_installation {\n" > /home/hpc-now/.terraformrc
echo -e "  filesystem_mirror {" >> /home/hpc-now/.terraformrc
echo -e "    path    = \"/home/hpc-now/.terraform.d/plugins\"" >> /home/hpc-now/.terraformrc
echo -e "    include = [\"registry.terraform.io/*/*\"]" >> /home/hpc-now/.terraformrc
echo -e "  }\n}" >> /home/hpc-now/.terraformrc
chown -R hpc-now:hpc-now /home/hpc-now/.terraformrc
mkdir -p $AWS_PROVIDER_PATH
mkdir -p $ALI_PROVIDER_PATH
mkdir -p $TENCENT_PROVIDER_PATH
mkdir -p /opt/packs/
if [ ! -f /opt/packs/${AWS_PROVIDER_NAME} ]; then
  wget ${URL_TERRAFORM}${AWS_PROVIDER_NAME} -O /opt/packs/${AWS_PROVIDER_NAME} -q
fi
unzip -q -o /opt/packs/${AWS_PROVIDER_NAME} -d ${AWS_PROVIDER_PATH}

if [ ! -f /opt/packs/${ALI_PROVIDER_NAME} ]; then
  wget ${URL_TERRAFORM}${ALI_PROVIDER_NAME} -O /opt/packs/${ALI_PROVIDER_NAME} -q
fi
unzip -q -o /opt/packs/${ALI_PROVIDER_NAME} -d ${ALI_PROVIDER_PATH}

if [ ! -f /opt/packs/${TENCENT_PROVIDER_NAME} ]; then
  wget ${URL_TERRAFORM}${TENCENT_PROVIDER_NAME} -O /opt/packs/${TENCENT_PROVIDER_NAME} -q
fi
unzip -q -o /opt/packs/${TENCENT_PROVIDER_NAME} -d ${TENCENT_PROVIDER_PATH}
chown -R hpc-now:hpc-now /home/hpc-now/.terraform.d