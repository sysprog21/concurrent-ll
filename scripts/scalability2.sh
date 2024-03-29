#!/usr/bin/env bash

cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;

prog1=$1;
shift;
prog2=$1;
shift;
params="$@";

printf "#       %-32s%-32s\n" "$prog1" "$prog2";
echo "#cores  throughput  %linear scalability throughput  %linear scalability";

prog=$prog1;

printf "%-8d" 1;
thr1a=$(./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1a;
printf "%-8.2f" 100.00;
printf "%-12d" 1;

prog=$prog2;

thr1b=$(./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1b;
printf "%-8.2f" 100.00;
printf "%-8d\n" 1;

for c in $cores
do
    if [ $c -eq 1 ]
    then
	continue;
    fi;

    printf "%-8d" $c;

    prog=$prog1;
    thr1=$thr1a;

    thr=$(./$prog $params -n$c | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-12.2f" $scl;

    prog=$prog2;
    thr1=$thr1b;

    thr=$(./$prog $params -n$c | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-8.2f\n" $scl;


done;

source scripts/unlock_exec;
