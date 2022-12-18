#!/bin/bash

. ./bootstrap/utils

CWD="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
LIB_DIR=$CWD/lib

BOOTSTRAP_DIR=$CWD/bootstrap

mkdir -p "$LIB_DIR"
cd "$LIB_DIR" || error_exit

export LIB_DIR
export BOOTSTRAP_DIR

for f in "$BOOTSTRAP_DIR"/*.get; do
    echo "========================================================================="
    echo "Executing $f..."
    bash "$f" -H || break
    echo "Success"
done

echo "========================================================================="
echo "Done."
