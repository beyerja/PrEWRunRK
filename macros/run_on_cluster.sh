#!/bin/bash

# -------------------------------------------------#
# Bash script for run the compiled PrEWRunRK 
# executable.
# -------------------------------------------------#

# -------------------------------------------------#
# Check for input arguments to script

# Default parameters
REBUILD=false
N_CPUS=3

# Help message
USAGE="Usage: ./run_on_cluster.sh [-h/--help/-H] [--rebuild] [--cpus=n_cpus]"

for i in "$@"
do
case $i in
  --cpus=*)
    N_CPUS="${i#*=}"
    echo "Using ${N_CPUS} cpus."
    shift # past argument=value
  ;;
  --rebuild)
    REBUILD=true
    echo "Rebuild requested => Will recompile source code."
    shift
  ;;
  
  -h|--help|-H)
    echo ""
    echo "Macro to run the PrEWRunRK executable"
    echo "${USAGE}"
    echo ""
    echo "Arguments:"
    echo "  -h/--help/-H  : Help information (this)." 
    echo "  --rebuild     : Recompile the source code." 
    echo "  --cpus=n_cpus : Set a number of cluster cpus to use." 
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

condor_out_dir="${home_folder}/output/Condor_output"
if ! [ -d "${condor_out_dir}"  ] ; then
	echo "Creating HTCondor output directory."
	mkdir --parents "${condor_out_dir}"
fi

# -------------------------------------------------#
# Check if compilation necessary / requested
executable_path="${home_folder}/bin/PrEWRunRK"
recompile=false

if [ -f "${executable_path}" ]; then
  if [ "${REBUILD}" = true ]; then 
    recompile=true
  fi
else
  echo "Executable doesn't exist, will recompile source code."
  recompile=true
fi

# -------------------------------------------------#
# Determine fill command to run executable on cluster

run_command="cd ${home_folder}/macros \&\& source load_env.sh"

if [ "${recompile}" = true ]; then 
  run_command="${run_command} \&\& ${home_folder}/macros/compile.sh --rebuild"
fi

run_command="${run_command} \&\& cd ${home_folder}/bin \&\& ./PrEWRunRK"

# -------------------------------------------------#
# Determine additional HTCondor steering parameters

condor_pars="RequestCpus=${N_CPUS}"

# -------------------------------------------------#
# Submit run job to HTCondor scheduler

condor_dir="${home_folder}/macros/HTCondor_runs"
submit_script="submit_script.submit"

# Submit job to HTCondor using standard submitting setup
# -> Start job and keep track of job ID to know when it's done
cd ${condor_dir}
condor_job_output=$(condor_submit ${submit_script} ${condor_pars} arguments="${run_command}")
  
# Split output up by spaces and only read last part (which is cluster ID).
# Details at: https://stackoverflow.com/questions/3162385/how-to-split-a-string-in-shell-and-get-the-last-field
condor_job_ID="${condor_job_output##* }"
echo "Submitted job under ID ${condor_job_ID}"
echo "Job output can be found in ${condor_out_dir}"

# -------------------------------------------------#
# Finish up

echo "Waiting for job to finish."
job_log_path=$(ls ${condor_out_dir}/${condor_job_ID}*.log)
wait_output=$(condor_wait ${job_log_path}) # Write into variable to suppress spammy output
echo "Job finished!"

# -------------------------------------------------#