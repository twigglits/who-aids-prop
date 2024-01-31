#!/bin/bash
user=jeannaude

mkdir -p /home/$user/who-aids-prop/build

cd /home/$user/who-aids-prop/build
cmake ..
make
make -j 4   
/home/$user/who-aids-prop/build/simpact-cyan-release

sudo Rscript -e 'Sys.setenv(PATH=paste("/home/$user/who-aids-prop/build", Sys.getenv("PATH"), sep=":"));
            Sys.setenv(PYTHONPATH="/home/$user/who-aids-prop/python");
            Sys.setenv(SIMPACT_DATA_DIR="/home/$user/who-aids-prop/data/");
            library("RSimpactCyan")'