#!/bin/bash

C=$(pwd)
S="huawei/alice"
D="system/core system/bt bootable/recovery hardware/interfaces frameworks/native"

apply_patches() { cd ${C}/${1}; git apply --ignore-whitespace ${C}/device/${S}/patches/$1/*.patch; cd ${C}; }

for R in $D; do
    apply_patches $R
done
