#!/bin/bash

# set your home directory here
$home_directory_name = 'jeannaude'   #set your home directory here

mkdir -p /home/$home_directory_name/simpactcyan/build
cd /home/jeannaude/simpactcyan/build

cmake ..
make -j 4
./simpact-cyan-release

# Open R session and execute commands
# NOT YET WORKING, COMMENTED OUT FOR NOW: 
Rscript -e 'Sys.setenv(PATH=paste("/home/jeannaude/simpactcyan/build", Sys.getenv("PATH"), sep=":"));
            Sys.setenv(PYTHONPATH="/home/jeannaude/simpactcyan/python");
            Sys.setenv(SIMPACT_DATA_DIR="/home/jeannaude/simpactcyan/data/")'
            # library(RSimpactCyan)'