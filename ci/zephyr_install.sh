#!/bin/bash -x

PWD=$(pwd)

ZEPHYR_SDK_RELEASE="0.9.5"
ZEPHYR_SDK="zephyr-sdk-${ZEPHYR_SDK_RELEASE}-setup.run"
ZEPHYR_SDK_URL="https://github.com/zephyrproject-rtos/meta-zephyr-sdk/releases/download/${ZEPHYR_SDK_RELEASE}/${ZEPHYR_SDK}"
ZEPHYR_SDK_DIR="${HOME}/zephyr-sdk"

export ZEPHYR_BASE="$HOME/zephyr"
ZEPHYR_NFFS_DIR="${ZEPHYR_BASE}/ext/fs/nffs"

export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

git clone --depth=1 https://github.com/zephyrproject-rtos/zephyr.git $ZEPHYR_BASE
source ${ZEPHYR_BASE}/zephyr-env.sh

download_dir=$HOME/zephyr_sdk_download
mkdir -p ${download_dir}

# TODO: cache zephyr-sdk
wget -O ${download_dir}/${ZEPHYR_SDK} -c ${ZEPHYR_SDK_URL}
[[ $? -ne 0 ]] && exit 1

chmod +x ${download_dir}/${ZEPHYR_SDK}
${download_dir}/${ZEPHYR_SDK} --quiet -- -d "${ZEPHYR_SDK_DIR}"
[[ $? -ne 0 ]] && exit 1

pip install PyYAML pyelftools

# Overwrite bundled Zephyr NFFS with current dev version
cp src/* ${ZEPHYR_NFFS_DIR}/src
cp include/nffs/* ${ZEPHYR_NFFS_DIR}/include/nffs
