#!/bin/bash

#  Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

current_dir=$(pwd)
cd $(dirname $0)

export REL=08_06_00_00

if [ `arch` == "aarch64" ]; then
    install_dir="/opt/"
else
    install_dir="../../"
fi
while getopts ":i:" flag; do
    case "${flag}" in
        i)
            if [ -z $OPTARG ] || [ ! -d $OPTARG ]; then
                echo "Invalid installation directory "
                cd $current_dir
                exit 1
            fi
            install_dir="$OPTARG"
            ;;
        *)
            if [ $OPTARG == i ]; then
                echo "Installation directory not provided"
                cd $current_dir
                exit 1
            fi
            ;;
    esac
done

# Clone edgeai-tidl-tools under /opt
cd $install_dir
ls | grep "edgeai-tidl-tools"
if [ "$?" -ne "0" ]; then
    git clone --single-branch --branch 08.06.00.01 https://github.com/TexasInstruments/edgeai-tidl-tools.git
    if [ "$?" -ne "0" ]; then
        cd $current_dir
        exit 1
    fi
    cd edgeai-tidl-tools/dockers/dependency_build/qemu
    sed -i 's/wget/wget --proxy off/g' targetfs_load.sh
    cd $install_dir
fi

if [ `arch` == "aarch64" ]; then
    if grep -qi ubuntu /etc/os-release; then
        cd edgeai-tidl-tools/dockers/J721E/ubuntu_20.04
        ./container_setup.sh
    fi
fi

cd $current_dir
