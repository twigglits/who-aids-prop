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
        "population.simtime": 40
      }

# cfg["vmmc.enabled"] = "yes"
cfg["EventVMMC.m_vmmcprobDist.dist.type"] = "uniform"
cfg["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1
cfg["EventVMMC.m_vmmcprobDist.dist.uniform.min"] = 0
# cfg["EventVMMC.m_vmmcprobDist.dist.type"] = "normal"
# cfg["EventVMMC.m_vmmcprobDist.dist.normal.mu"] = 100
# cfg["EventVMMC.m_vmmcprobDist.dist.normal.sigma"] = 1

res = simpact.run(cfg, "output",seed=1, dataFiles=data)

with open('output.txt', 'w') as f:
    with redirect_stdout(f):
        simpact.showConfiguration(cfg)