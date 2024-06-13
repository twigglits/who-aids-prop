import pysimpactcyan
import sys
from contextlib import redirect_stdout

# Check which directories are on PATH
# echo $PATH
# Assuming that 'usr/local/bin' is on PATH, then you need to add a symbolic link as follows:
# ln -s /home/jupyter/who-aids-prop/build/simpact-cyan-release /usr/local/bin/simpact-cyan-release
# you should then be able to call the package from anywhere in your home directory

# Continue with the rest of your PySimpactCyan code
simpact = pysimpactcyan.PySimpactCyan()

myMatrix = [[1, 3], [2, 4]]
data = {}
data["csvMatrix"] = myMatrix

cfg = {}
cfg = { "population.nummen": 500,
        "population.numwomen": 500,
        "population.simtime": 50,
        "population.agedistfile": "/home/jupyter/who-aids-prop/build/python/eswatini_2023.csv",
        "periodiclogging.interval": 1,
        "periodiclogging.starttime": 0,
        # vmmc
        "EventVMMC.enabled": "false",
        "EventVMMC.m_vmmcprobDist.dist.type": "uniform",
        "EventVMMC.m_vmmcprobDist.dist.uniform.max": 1,
        "EventVMMC.m_vmmcprobDist.dist.uniform.min": 0,
        "EventVMMC.m_vmmcscheduleDist.dist.type": "discrete.csv.twocol",
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv",
        # condom programming
        "EventCondom.enabled" : "true",
        "EventCondom.m_condomprobDist.dist.type": "uniform",
        "EventCondom.m_condomprobDist.dist.uniform.min": 0,
        "EventCondom.m_condomprobDist.dist.uniform.max": 1,
        "EventCondom.threshold": 0.01, # threshold for condom preference for an individual 
        "hivtransmission.threshold": 0.5, # threshold for condom use in formation
        "hivtransmission.m_condomformationdist.dist.type":"discrete.csv.twocol",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file":  "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor": 'yes', #to force distirbution to return exact values and not any value within the bin
        "hivtransmission.threshold": 0.5 # threshold for condom use in formation
      }

# iv1 = { }
# iv1["time"] = 20 #start intervention 20years after simulation has started
# iv1["EventPrep.enabled"] = "true"

iv1 = { }
iv1["time"] = 20 #start intervention 20years after simulation has started
iv1["EventCondom.enabled"] = "true"

iv2 = {}
iv2["time"] = 25 
iv2["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv"

iv3 = {}
iv3["time"] = 30 
iv3["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv"

iv4 = {}
iv4["time"] = 35 
iv4["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_4.csv"

iv = [iv1,iv2,iv3,iv4]
res = simpact.run(cfg, "/home/jupyter/who-aids-prop/build/python/output",
                  seed=42, 
                  interventionConfig=iv,
                  dataFiles=data)

with open('output.txt', 'w') as f:
    with redirect_stdout(f):
        simpact.showConfiguration(cfg)