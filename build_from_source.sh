#!/bin/bash

# set your home directory here
$home_directory_name = $USER   #set your home directory here

mkdir -p /home/$home_directory_name/simpactcyan/build
cd /home/$USER/simpactcyan/build

cmake ..
make -j 4
./simpact-cyan-release

# Open R session and execute commands
# NOT YET WORKING, COMMENTED OUT FOR NOW: 
Rscript -e 'Sys.setenv(PATH=paste("/home/$USER/simpactcyan/build", Sys.getenv("PATH"), sep=":"));
            Sys.setenv(PYTHONPATH="/home/$USER/simpactcyan/python");
            Sys.setenv(SIMPACT_DATA_DIR="/home/$USER/simpactcyan/data/")'
            library(RSimpactCyan)'