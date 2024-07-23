import pysimpactcyan
import sys
from contextlib import redirect_stdout
import logging

# Continue with the rest of your PySimpactCyan code
simpact = pysimpactcyan.PySimpactCyan()

myMatrix = [[1, 3], [2, 4]]
data = {}
data["csvMatrix"] = myMatrix

cfg = { 
    "population.nummen": 500,
    "population.numwomen": 500,
    "population.simtime": 50,
    "population.agedistfile": "/home/jupyter/who-aids-prop/build/python/agedist.csv",
    "periodiclogging.interval": 1,
    "periodiclogging.starttime": 0,
    "mortality.aids.survtime.art_e": 10,
    # vmmc
    "EventVMMC.enabled": "false",
    "EventVMMC.threshold":0.5,
    "EventVMMC.m_vmmcprobDist.dist.type": "uniform",
    "EventVMMC.m_vmmcprobDist.dist.uniform.max": 1,
    "EventVMMC.m_vmmcprobDist.dist.uniform.min": 0,
    "EventVMMC.m_vmmcscheduleDist.dist.type": "discrete.csv.twocol",
    "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv",
    # condom programming
    "EventCondom.enabled" : "false",
    "EventCondom.m_condomprobDist.dist.type": "uniform",
    "EventCondom.m_condomprobDist.dist.uniform.min": 0,
    "EventCondom.m_condomprobDist.dist.uniform.max": 1,
    "EventCondom.threshold": 0.01, # threshold for condom preference for an individual 
    "hivtransmission.threshold": 0.5, # threshold for condom use in formation
    "hivtransmission.m_condomformationdist.dist.type":"discrete.csv.twocol",
    "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv",
    "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor": 'yes', # to force distribution to return exact values and not any value within the bin
    "hivtransmission.threshold": 0.5, # threshold for condom use in formation
    # prep
    "EventPrep.enabled": "false"
}

# no testing before time 20
cfg["monitoring.cd4.threshold"] = 1
cfg["person.art.accept.threshold.dist.fixed.value"] = 0.75
cfg["diagnosis.baseline"] = -99999
cfg["periodiclogging.interval"] = 0.25
#cfg["dropout.interval.dist.exponential.lambda"] = 0.1

cfg["mortality.aids.survtime.C"] = 65
cfg["mortality.aids.survtime.k"] = -0.2
cfg["monitoring.fraction.log_viralload"] = 0.3

# iv1 = { }
# iv1["time"] = 20 #start intervention 20years after simulation has started
# iv1["EventPrep.enabled"] = "true"

art_intro = {
        "time": 20, #around 2000
        "diagnosis.baseline": -0.8,
        "monitoring.cd4.threshold": 100}

# prep_intro = {"time": 30.1, "EventPrep.enabled": "true"}

# vmmc_intro = { "time": 20, "EventVMMC.enabled": "true" }

# condom_intro1 = { "time": 20.2, "EventCondom.enabled": "true" }

# condom_intro2 = { "time": 25, "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv" }

# condom_intro3 = { "time": 30, "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv" }

# condom_intro4 = { "time": 35, "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_4.csv" }

# all_int = [art_intro, prep_intro, vmmc_intro, condom_intro1, condom_intro2, condom_intro3, condom_intro4]

# Ensure the interventions are sorted by time
# all_int.sort(key=lambda x: x["time"])

# logging.info(f{"all_int})

res = simpact.run(cfg, "/home/jupyter/who-aids-prop/build/python/data",
                  seed=42, 
                  interventionConfig=[art_intro], # pass all_int directly, not [all_int]
                  dataFiles=data)

with open('output.txt', 'w') as f:
    with redirect_stdout(f):
        simpact.showConfiguration(cfg)
