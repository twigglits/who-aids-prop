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
cfg = { "population.nummen": 1000,
        "population.numwomen": 1000,
        "population.simtime": 50,
        "population.agedistfile": "/home/jupyter/who-aids-prop/build/python/eswatini_2023.csv",
        "periodiclogging.interval": 1,
        "periodiclogging.starttime": 0
      }
cfg["EventVMMC.enabled"] = "false"

cfg["EventCondom.enabled"] = "false"

iv1 = { }
iv1["time"] = 20 #start intervention 20years after simulation has started
iv1["EventCondom.enabled"] = "true"

cfg["EventCondom.m_condomprobDist.dist.type"] = "uniform"
cfg["EventCondom.m_condomprobDist.dist.uniform.max"] = 1
cfg["EventCondom.m_condomprobDist.dist.uniform.min"] = 0
cfg["EventCondom.threshold"] = 0.5      # threshold for condom preference for an individual 
cfg["hivtransmission.threshold"] = 0.5  # threshold for condom use in formation
cfg["hivtransmission.m_condomformationdist.dist.type"] = "discrete.csv.twocol"
cfg["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use.csv"
cfg["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor"] = 'yes' #to force distirbution to return exact values and not any value within the bin

# cfg["EventVMMC.m_vmmcprobDist.dist.type"] = "uniform"
# cfg["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1
# cfg["EventVMMC.m_vmmcprobDist.dist.uniform.min"] = 0

# cfg["EventVMMC.m_vmmcscheduleDist.dist.type"] = "discrete.csv.twocol"
# cfg["EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv"

res = simpact.run(cfg, "/home/jupyter/who-aids-prop/build/python/output",
                  seed=42, 
                  interventionConfig=[iv1],
                  dataFiles=data)

with open('output.txt', 'w') as f:
    with redirect_stdout(f):
        simpact.showConfiguration(cfg)