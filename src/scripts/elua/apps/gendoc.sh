#!/bin/sh
# a parallel doc generation script

# exit on failure
set -e

gendoc() {
    elua gendoc.lua --pass $@
}

gendoc rm $@
gendoc ref $@

# limit jobs otherwise stuff starts complaining in eldbus etc
MAXJ=192

I=0
for cl in $(gendoc clist $@); do
    gendoc "$cl" $@ &
    I=$(($I + 1))
    if [ $I -gt $MAXJ ]; then
        I=0
        # wait for the batch to finish
        wait
    fi
done

gendoc types $@ &
gendoc vars $@ &

# wait for all remaining stuff to finish
wait

# final results
gendoc stats $@

exit 0
