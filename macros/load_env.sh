#!/bin/bash

# Script tries to use the setup used by PrEW 
# => Tries to find it on my NAF, else see if a local version exists

PrEW_NAF_dir="/afs/desy.de/group/flc/pool/beyerjac/TGCAnalysis/PrEW"
PrEW_local_dir="../../PrEW"

PrEW_dir=""

if [ -d "${PrEW_NAF_dir}" ] ; then
  echo "Found PrEW on NAF."
  PrEW_dir="${PrEW_NAF_dir}"
elif [ -d "${PrEW_local_dir}" ] ; then
  echo "Found PrEW on local computer."
  PrEW_dir="${PrEW_local_dir}"
else 
  echo "ERROR: Did not find PrEW!"
  exit
fi

source ${PrEW_dir}/macros/load_env.sh
