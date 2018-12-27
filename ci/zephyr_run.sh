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

succeded="PROJECT EXECUTION SUCCESSFUL"

# There are multiple possible output messages, depending on which test was
# run, where it failed, or when some low-level fault in qemu happens.
declare -a finished_msgs=(
    "ASSERTION FAIL"
    "CPU Page Fault"
    "PROJECT EXECUTION FAILED"
    "${succeded}"
)

MAX_LOOPS=10
count=1
while [[ $count -lt $MAX_LOOPS ]]; do
    for msg in "${finished_msgs[@]}"; do
        grep -q -e "${msg}" ${output}
        rc=$?
        # break inner loop
        [[ ${rc} -eq 0 ]] && break
    done

    [[ ${rc} -eq 0 ]] && break
    sleep 3
    count=$((count + 1))
done

[[ $count == $MAX_LOOPS ]] && exit 1

kill $(pgrep qemu)
sleep 5  # give some time for qemu to close

cat ${output}

grep -q -e "${succeded}" ${output}
