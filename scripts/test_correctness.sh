#!/usr/bin/env bash

source scripts/lock_exec;
source scripts/config;

for bin in $(ls out/test-*);
do
    echo "Testing: $bin";
    $bin -n$max_cores | grep -i "expected";
    $bin -n$max_cores -i32 -r64 | grep -i "expected";
    $bin -n$max_cores -i16 -r32 -u100 | grep -i "expected";
done;

source scripts/unlock_exec;
