#!/bin/bash -x

export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR=$HOME/zephyr-sdk
export ZEPHYR_BASE=$HOME/zephyr

mkdir zephyr-build && pushd zephyr-build

cmake -DBOARD=qemu_x86 $ZEPHYR_BASE/tests/subsys/fs/nffs_fs_api/basic
[[ $? -ne 0 ]] && exit 1

make
[[ $? -ne 0 ]] && exit 1

# Run qemu in background, capture output and kill when test is finished
output="${HOME}/zephyr-results.txt"
(make run | tee ${output}) &

# qemu output strings in case of failure
failed1="ASSERTION FAIL"
failed2="CPU Page Fault"
succeded="PROJECT EXECUTION SUCCESSFUL"
MAX_LOOPS=10
count=1
while true; do
    [[ $count == $MAX_LOOPS ]] && break
    grep -q -e "${failed1}" -e "${failed2}" -e "${succeded}" ${output}
    [[ $? -eq 0 ]] && break

    sleep 3
    count=$((count + 1))
done

[[ $count == $MAX_LOOPS ]] && exit 1

kill $(pgrep qemu)
sleep 5  # give some time for qemu to close

cat ${output}

grep -q -e "${failed1}" -e "${failed2}" ${output}
[[ $? -eq 0 ]] && exit 1

# If the grep above fails, the test succeeded, so force success exit!
exit 0
