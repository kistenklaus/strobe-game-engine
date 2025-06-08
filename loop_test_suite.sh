#!/bin/bash
# Save this as e.g. run_until_crash.sh

while true; do
    make -Cbuild test || break
done
