#!/bin/bash

# Change the current working directory to the location of the present file
cd "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

python validate_perfect_links.py --hosts ../example/output/hosts --config ../example/output/config --out_dir ../example/output