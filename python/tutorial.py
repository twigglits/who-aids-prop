import pysimpactcyan
import sys

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
cfg["person.geo.dist2d.type"] = "discrete"
cfg["person.geo.dist2d.discrete.densfile"] = "data:csvMatrix"

cfg["EventVMMC.m_vmmcprobDist.dist.type"] = "discrete.csv.twocol"
cfg["EventVMMC.m_vmmcprobDist.dist.discrete.csv.twocol.file"] = "who-aids-prop/build/birth_twocol.csv"
# cfg["EventVMMC.m_vmmcscheduleDist.dist.type"] = "fixed"
# cfg["EventVMMC.m_vmmcscheduleDist.dist.fixed.value"]="0.66575"


# cfg["EventVMMC.EventVMMC_schedule_dist.dist.fixed.value"] = '0.66575'
# cfg["EventVMMC.m_vmmcscheduleDist.dist.type"] = "uniform"
# cfg["EventVMMC.m_vmmcscheduleDist.dist.uniform.max"] = 1
# cfg["EventVMMC.m_vmmcscheduleDist.dist.uniform.min"] = 0
# cfg["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1

# cfg["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1
# cfg["EventVMMC.m_vmmcprobDist.dist.uniform.min"] = 0

# Run PySimpactCyan
simpact.run(cfg, "output",seed=42, dataFiles=data)