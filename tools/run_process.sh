#!/bin/bash
#usage: run_process.sh num_process
# where num_process is 1, 2 or 3
# Change the current working directory to the location of the present file
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

../template_cpp/run.sh --id $1 --hosts ../example/hosts --output ../example/output/$1.output ../example/configs/fifo-broadcast.config