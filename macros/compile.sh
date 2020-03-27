#!/bin/bash

# -------------------------------------------------#
# Bash script for compiling the processor of the
# analysis.
# Be sure a modern software is sourced! (E.g. sourcing load_env.sh)
# -------------------------------------------------#

# -------------------------------------------------#
# Check for input arguments to script

# Default parameters
REBUILD=false

# Help message
USAGE="Usage: ./compile.sh [-h/--help/-H] [--rebuild]"

for i in "$@"
do
case $i in
  --rebuild)
    REBUILD=true
    shift
  ;;
  -h|--help|-H)
    echo ""
    echo "Macro to compile the PrEWRun source code"
    echo "${USAGE}"
    echo ""
    echo "Arguments:"
    echo "  -h/--help/-H  : Help information (this)." 
    echo "  --rebuild     : Delete previous build folder for clean rebuild." 
    echo ""
    exit
  ;;
  *)
    # unknown option
    echo ""
    echo "Unknown argument: ${1}"
    echo "${USAGE}"
    echo ""
    exit
  ;;
esac
done

# -------------------------------------------------#
# Check if necessary directories are in place

dir="$( cd "$( dirname "${BASH_SOURCE[0]}"  )" && pwd  )"
home_folder=${dir}/..

if [ -d "${home_folder}"/build  ] ; then
	echo "Already have  --build-- directory to compile"
  # If requested remove build folder content
  if [ "${REBUILD}" = true ]; then 
    echo "Rebuild requested => Recreating build directory"
    rm -r "${home_folder}"/build
    mkdir "${home_folder}"/build
  fi
else
	echo "no build directory => Recreating build directory"
	mkdir "${home_folder}"/build
fi

if [ -d "${home_folder}"/output  ] ; then
	echo "Already have output directory"
else 
  echo "Creating output directory"
  mkdir "${home_folder}"/output
fi

# -------------------------------------------------#
# Move to build directory and to build framework
# (cmake + make)

cd "${home_folder}"/build

echo "begin to config"
echo
cmake .. 
wait 
echo
echo "begin to make" 
make
wait
echo
echo "begin to make install" 
make install
wait
echo 
echo "done!"
# -------------------------------------------------#