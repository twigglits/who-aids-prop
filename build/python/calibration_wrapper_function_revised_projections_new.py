def calibration_wrapper_function(parameters = None):
    
    import numpy as np
    import math
    import os
    import pickle
    import pandas as pd
    import shutil
    import random

    import pysimpactcyan
    import psimapacthelper as psh

    simpact = pysimpactcyan.PySimpactCyan()

    # Creating cfg list -------------------------------------------------------
    cfg_list = psh.input_params_creator(
    population_eyecap_fraction=0.2,
    population_simtime=71,  # Until 1 January 2051
    population_nummen=1000,
    population_numwomen=1070, #ratio m:f is 1:1.07
    population_msm="no",
    hivseed_time=8.5,
    hivseed_type="amount",
    hivseed_amount=15,  # 30,
    hivseed_age_min=20,
    hivseed_age_max=50,
    hivtransmission_param_a=-1.3,
    hivtransmission_param_b=-90,
    hivtransmission_param_c=0.5,
    hivtransmission_param_f1=np.log(2.27),
    hivtransmission_param_f2=np.log(np.log(np.sqrt(2.27)) / np.log(2.27)) / 5,
    formation_hazard_agegapry_gap_factor_man_age=-0.02,
    formation_hazard_agegapry_gap_factor_woman_age=-0.02,
    formation_hazard_agegapry_meanage=-0.03, #-0.025
    formation_hazard_agegapry_gap_factor_man_const=0,
    formation_hazard_agegapry_gap_factor_woman_const=0,
    formation_hazard_agegapry_gap_factor_man_exp=-1,
    formation_hazard_agegapry_gap_factor_woman_exp=-1,
    formation_hazard_agegapry_gap_agescale_man=0.44,#0.25,
    formation_hazard_agegapry_gap_agescale_woman=0.44,#0.25,
    dissolution_alpha_0=-2.50494307,#-0.05,
    debut_debutage=15,
    conception_alpha_base=-2.9,#-1.868525049,#
    dropout_interval_dist_type='fixed',#"exponential",
    person_eagerness_man_dist_gamma_a=0.679322802,
    person_eagerness_woman_dist_gamma_a=0.679322802,
    person_eagerness_man_dist_gamma_b=47.03264158,
    person_eagerness_woman_dist_gamma_b=47.03264158,
    formation_hazard_agegapry_numrel_man= -0.649752699,
    formation_hazard_agegapry_numrel_woman= -0.61526928)
    
    cfg_list["hivtransmission.param.k"] = -1.203
    cfg_list["hivtransmission.param.p"] = -1.6094 #prep effectivenss in preventing HIV (80%, remaining risk ln(0.20))
            
    cfg_list["population.agedistfile"] = "/home/jupyter/who-aids-prop/build/python/eswatini_1980.csv"
    cfg_list['diagnosis.eagernessfactor'] = np.log(1.025)
    cfg_list['diagnosis.pregnancyfactor'] = 0
    cfg_list['EventAGYW.enabled']='false'
    cfg_list['diagnosis.AGYWfactor'] = 0
    cfg_list["mortality.aids.survtime.art_e.dist.type"] = "uniform"
    cfg_list["mortality.aids.survtime.art_e.dist.uniform.min"] =  0#5
    cfg_list["mortality.aids.survtime.art_e.dist.uniform.max"] = 10# 20 #24
    
    # vmmc
    cfg_list["EventVMMC.enabled"] = "false"
    cfg_list["EventVMMC.threshold"] = 0.3#0.5
    cfg_list["EventVMMC.m_vmmcprobDist.dist.type"] = "uniform"
    cfg_list["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1
    cfg_list["EventVMMC.m_vmmcprobDist.dist.uniform.min"] = 0
    cfg_list["EventVMMC.m_vmmcscheduleDist.dist.type"] = "discrete.csv.twocol"
    # cfg_list["EventVMMC.m_vmmcscheduleDist.dist.fixed.value"]=0
    cfg_list["EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_0_0.csv"

    # condom
    cfg_list["EventCondom.enabled"] = "false"
    cfg_list["EventCondom.m_condomprobDist.dist.type"] = "uniform"
    cfg_list["EventCondom.m_condomprobDist.dist.uniform.min"] = 0
    cfg_list["EventCondom.m_condomprobDist.dist.uniform.max"] = 1
    cfg_list["EventCondom.threshold"] = 9999 # 0.8 #nobody uses condoms at the beginning, so threshold set very high
    cfg_list["EventCondom.AGYWthreshold"] = 0
    cfg_list["hivtransmission.m_condomformationdist.dist.type"] = "discrete.csv.twocol"
    cfg_list["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv"
    #cfg_list["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor"] = "yes"
    cfg_list["hivtransmission.threshold"] = 9999 #0.3 #no relationship uses condoms at the beginning, so threshold set very high

    # prep
    cfg_list["EventPrep.enabled"] = "false" #current code can't switch prep off
    cfg_list["EventPrep.threshold"] = 1 # threshold for willingness to start prep. Nobody starts
    cfg_list["EventPrep.AGYWthreshold"] = 0 # threshold for willingness to start prep. Nobody starts
    cfg_list['EventPrep.m_prepprobDist.dist.type'] ='uniform' # willingness to start prep
    cfg_list['EventPrep.m_prepprobDist.dist.uniform.min'] = 0
    cfg_list['EventPrep.m_prepprobDist.dist.uniform.max'] = 1
    cfg_list['EventPrepDrop.threshold'] = 1#0.2 # threshold for dropping
    cfg_list['EventPrepDrop.interval.dist.type'] ='uniform' # distribution to choose probablity of dropping which will be used alongside number of relationships or hiv status to decide if dropping out
    cfg_list['EventPrepDrop.interval.dist.uniform.min'] = 0#0.25
    cfg_list['EventPrepDrop.interval.dist.uniform.max'] = 1#10.0
    
    # prep dvr
    cfg_list['EventDVR.enabled'] = "false"
    cfg_list["EventDVR.m_DVRprobDist.dist.type"] = "uniform"
    cfg_list["EventDVR.m_DVRprobDist.dist.uniform.max"] = 1
    cfg_list["EventDVR.m_DVRprobDist.dist.uniform.min"] = 0
    cfg_list['EventDVR.threshold'] = 1 # threshold for willingness to start dvr. Nobody starts
    cfg_list["EventDVRDROP.enabled"] = "false"
    cfg_list["EventDVRDROP.threshold"] = 1 # threshold for dropping, nobody drops
    cfg_list["EventDVRDROP.m_DVRDROPprobDist.dist.type"] = 'uniform'
    cfg_list["EventDVRDROP.m_DVRDROPprobDist.dist.uniform.min"] = 0
    cfg_list["EventDVRDROP.m_DVRDROPprobDist.dist.uniform.max"] = 1
    # cfg_list["EventDVRDROP.schedulemin"] = 1 #people can drop out of prep after 1 month
    # cfg_list["EventDVRDROP.schedulemax"] = 12 #people can drop out of prep upto 6 months after starting
    cfg_list["hivtransmission.param.p1"] = -0.6931 #dvr effectivenss in preventing HIV (50%, remaining risk ln(0.5))
    
    # prep cab
    cfg_list['EventCAB.enabled'] = "false"
    cfg_list["EventCAB.m_CABprobDist.dist.type"] = "uniform"
    cfg_list["EventCAB.m_CABprobDist.dist.uniform.max"] = 1
    cfg_list["EventCAB.m_CABprobDist.dist.uniform.min"] = 0
    cfg_list['EventCAB.threshold'] = 1 # threshold for willingness to start cab. Nobody starts
    cfg_list["EventCABDROP.enabled"] = "false"
    cfg_list["EventCABDROP.threshold"] = 1 # threshold for dropping, nobody drops
    cfg_list["EventCABDROP.m_CABDROPprobDist.dist.type"] = 'uniform'
    cfg_list["EventCABDROP.m_CABDROPprobDist.dist.uniform.min"] = 0
    cfg_list["EventCABDROP.m_CABDROPprobDist.dist.uniform.max"] = 1
    cfg_list["hivtransmission.param.p2"] = -2.9957
    
    # Initial values
    mu_cd4 = 800
    var_cd4 = 200 ** 2
    mu_cd4_end = 20
    var_cd4_end = 5

    # Lognormal distribution parameters for CD4 start
    cfg_list["person.cd4.start.dist.type"] = "lognormal"
    cfg_list["person.cd4.start.dist.lognormal.zeta"] = math.log(mu_cd4 / math.sqrt(1 + var_cd4 / mu_cd4 ** 2))
    cfg_list["person.cd4.start.dist.lognormal.sigma"] = math.sqrt(math.log(1 + var_cd4 / mu_cd4 ** 2))

    # Lognormal distribution parameters for CD4 end
    cfg_list["person.cd4.end.dist.type"] = "lognormal"
    cfg_list["person.cd4.end.dist.lognormal.zeta"] = math.log(mu_cd4_end / math.sqrt(1 + var_cd4_end / mu_cd4_end ** 2))
    cfg_list["person.cd4.end.dist.lognormal.sigma"] = math.sqrt(math.log(1 + var_cd4_end / mu_cd4_end ** 2))
    
    # Binormalsymmetric distribution parameters for SPVL
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.mean"] = 4
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.sigma"] = 1
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.rho"] = 0.33
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.min"] = 2
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.max"] = 6

    # Additional configuration settings
    cfg_list["formation.hazard.agegapry.baseline"] = 3.325157223
    cfg_list["mortality.aids.survtime.C"] = 65
    cfg_list["mortality.aids.survtime.k"] = -0.2
    # cfg_list["monitoring.fraction.log_viralload"] = 0.35#0.5
    cfg_list["monitoring.m_artDist.dist.type"] = "normal"
    cfg_list["monitoring.m_artDist.dist.normal.min"] = 0.15
    cfg_list["monitoring.m_artDist.dist.normal.max"] = 0.55#0.75
    cfg_list["monitoring.m_artDist.dist.normal.mu"] = 0.35#0.45
    cfg_list["monitoring.m_artDist.dist.normal.sigma"] = 0.2

    cfg_list["person.survtime.logoffset.dist.type"] = "normal"
    cfg_list["person.survtime.logoffset.dist.normal.mu"] = 0
    cfg_list["person.survtime.logoffset.dist.normal.sigma"] = 0.2

    cfg_list["person.agegap.man.dist.type"] = "normal"
    cfg_list["person.agegap.woman.dist.type"] = "normal"

    cfg_list["monitoring.cd4.threshold"] = 1
    cfg_list["person.art.accept.threshold.dist.fixed.value"] = 0.3 #0.45
    cfg_list["diagnosis.baseline"] = -99999
    cfg_list["periodiclogging.interval"] = 0.25
    cfg_list["dropout.interval.dist.fixed.value"] = 500 #cfg_list["dropout.interval.dist.exponential.lambda"] = 0.1


    # # Assuming cfg_list["population.simtime"] and cfg_list["population.nummen"] are defined elsewhere
    # cfg_list["population.maxevents"] = float(cfg_list["population.simtime"]) * float(cfg_list["population.nummen"]) * 100

    cfg_list["person.vsp.toacute.x"] = 5  # See Bellan PLoS Medicine

    # seedid = random.randint(0,1000000000)
    # seed_generator = psh.UniqueSeedGenerator()
    # seedid = seed_generator.generate_seed()
    seedid = int(parameters['seed'])
    modelid = int(parameters['model_id'])

    # calibration parameters
    
    cfg_list["hivtransmission.param.f1"] = round(parameters['hivtransmission_param_f1'],8)
    cfg_list["hivtransmission.param.f2"] = round(math.log(math.log(math.sqrt(parameters['hivtransmission_param_f1'])) / math.log(parameters['hivtransmission_param_f1'])) / 5,8)
    cfg_list["hivtransmission.param.a"] = round(parameters['hivtransmission_param_a'],8) + 0.2 #+ 0.23 
    cfg_list["formation.hazard.agegapry.gap_agescale_man"] = round(parameters['formation_hazard_agegapry_gap_agescale_man'],8)
    cfg_list["formation.hazard.agegapry.gap_agescale_woman"] = round(parameters['formation_hazard_agegapry_gap_agescale_woman'],8)
    cfg_list["person.agegap.man.dist.normal.mu"] = round(parameters['person_agegap_man_dist_normal_mu'],8)
    cfg_list["person.agegap.woman.dist.normal.mu"] = round(parameters['person_agegap_woman_dist_normal_mu'],8)
    cfg_list["person.agegap.man.dist.normal.sigma"] = round(parameters['person_agegap_man_dist_normal_sigma'],8)
    cfg_list["person.agegap.woman.dist.normal.sigma"] = round(parameters['person_agegap_woman_dist_normal_sigma'],8)
    cfg_list["person.eagerness.man.dist.gamma.a"] = round(parameters['person_eagerness_man_dist_gamma_a'],8)
    cfg_list["person.eagerness.woman.dist.gamma.a"] = round(parameters['person_eagerness_woman_dist_gamma_a'],8)
    cfg_list["person.eagerness.man.dist.gamma.b"] = round(parameters['person_eagerness_man_dist_gamma_b'],8)
    cfg_list["person.eagerness.woman.dist.gamma.b"] = round(parameters['person_eagerness_woman_dist_gamma_b'],8)
    cfg_list["formation.hazard.agegapry.gap_factor_man_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],8)
    cfg_list["formation.hazard.agegapry.gap_factor_woman_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],8)
    cfg_list["formation.hazard.agegapry.baseline"] = round(parameters['formation_hazard_agegapry_baseline'],8)
    cfg_list["formation.hazard.agegapry.numrel_man"] = round(parameters['formation_hazard_agegapry_numrel_man'],8)
    cfg_list["formation.hazard.agegapry.numrel_woman"] = round(parameters['formation_hazard_agegapry_numrel_woman'],8)
    cfg_list["conception.alpha_base"] = round(parameters['conception_alpha_base'],8)# + 0.17
    cfg_list["dissolution.alpha_0"] = round(parameters['dissolution_alpha_0'],8)# + 0.1
    cfg_list['diagnosis.eagernessfactor'] = round(math.log(parameters['diagnosis_eagernessfactor']),8)

        # hiv testing configurations
    hiv_testing = {
    "time": 12, #around 1992 
    "diagnosis.baseline":-4
    }
    
    # birth rate reduction
    conception_1 = {
    "time": 10,#12.5, #aroung 1992
    "conception.alpha_base": parameters['conception_alpha_base'] - parameters['conception_alpha_base_1'] #+0.17+ 0.03
    }
    
    # conception_2 = {
    # "time": 17,
    # "conception.alpha_base": parameters['conception_alpha_base'] -  parameters['conception_alpha_base_1'] - parameters['conception_alpha_base_2']
    # }
    
    # ART introduction configurations
    art_intro = {
        "time": 20, #around 2000
        "diagnosis.pregnancyfactor":0.2,
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'], 
        "monitoring.cd4.threshold": 100
    }

    art_intro1 = {
        "time": 22, #around 2002
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'], 
        "monitoring.cd4.threshold": 150
    }

    art_intro2 = {
        "time": 23, #around 2003
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'], #-0.6,
        "monitoring.cd4.threshold": 200
    }

    art_intro2_2 = {
        "time": 26, #around 2006
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'], #+ 0.1, #-0.4,
        "person.art.accept.threshold.dist.fixed.value": 0.35, #0.7,
        "mortality.aids.survtime.art_e.dist.uniform.min":0,#15,
        "mortality.aids.survtime.art_e.dist.uniform.max":20,#30
        # "mortality.aids.survtime.art_e.dist.uniform.min":20,
        # "mortality.aids.survtime.art_e.dist.uniform.max":30
    }

    art_intro3 = {
        "time": 30, # 2010
        "diagnosis.pregnancyfactor":0.7,#0.5
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3'],#  + 0.1 + 0.15,#-0.2,
        "monitoring.cd4.threshold": 350,
        "person.art.accept.threshold.dist.fixed.value": 0.25,#0.7,
        # "monitoring.m_artDist.dist.normal.mu": 0.35,
        # "monitoring.m_artDist.dist.normal.min": 0.15,
        # "monitoring.m_artDist.dist.normal.max":0.55,
        "mortality.aids.survtime.art_e.dist.uniform.min":10,#25,
        "mortality.aids.survtime.art_e.dist.uniform.max":20,#35
        # "mortality.aids.survtime.art_e.dist.uniform.min":30,#25,
        # "mortality.aids.survtime.art_e.dist.uniform.max":40#45
    }

    art_intro4 = {
        "time": 33.5, #around 2013
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'],# + 0.1 +0.15+0.5, #-0.1,
        "monitoring.cd4.threshold": 500,
        "person.art.accept.threshold.dist.fixed.value": 0.5,
        # "conception.alpha_base":  parameters['conception_alpha_base']- parameters['conception_alpha_base_1'] - parameters['conception_alpha_base_2'],
        "EventAGYW.enabled": 'true',
        "diagnosis.AGYWfactor":0.1,#0.5,#1.1
        "diagnosis.pregnancyfactor":0.9
    }

    art_intro5 = {
        "time": 36.75, #around oct 2016
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'] + parameters['diagnosis_baseline_t5'],#+ 0.1 +0.15+0.5+0.2,
        "monitoring.cd4.threshold":100000, 
        # "monitoring.m_artDist.dist.normal.mu": 0.3,
        # "monitoring.m_artDist.dist.normal.min": 0.1,
        # "monitoring.m_artDist.dist.normal.max":0.5,
        # "monitoring.m_artDist.dist.normal.mu": 0.35,
        # "monitoring.m_artDist.dist.normal.min": 0.15,
        # "monitoring.m_artDist.dist.normal.max":0.55,
        "diagnosis.AGYWfactor":0.1
    }

        #condom use
    condom_intro1 = { 
            "time": 18, #around 1998
            "EventCondom.enabled": "true",
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv", 
            "EventCondom.threshold": 0.7,#0.2, #0.8,
            "hivtransmission.threshold": 0.8 #0 # 0.3 #30% relationships using condoms consistently
        }

    condom_intro2 = { 
            "time": 25, 
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv",
            "EventCondom.threshold": 0.75, #0.2,#0.4#0.85
            "hivtransmission.threshold": 0.75 #0 # 0.4 #40%relationships using condoms consistently
    }

    condom_intro3 = { 
            "time": 37.11, 
            # "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv",
            "EventCondom.threshold": 0.9,#0.8,
            "hivtransmission.threshold": 0.85, #0 # 0.3 # 20% relationships using condoms consistently
            "EventCondom.AGYWthreshold": 0.1
    }
    
    #vmmc

    vmmc_intro1 = {
        "time":26.1, #around 2007
        "EventVMMC.enabled": "true",
        "EventVMMC.threshold": 0.28
    }
    
    vmmc_intro2 = {
        "time":36.5, #around 2016
        "EventVMMC.threshold": 0.28,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_0_1.csv"
    }
    
    # prep
    prep_intro1 = {
            "time":37, #around 2017
            "EventPrep.enabled": "true",
            "EventPrep.threshold":0.95, #0.87, # threshold for willingness to start prep. coverage is 13%
            'EventPrepDrop.threshold': 0.001#0.8
        }
    
    prep_intro2 = {
            "time":40.5, #around mid 2020
            "EventPrep.threshold":0.90, #0.965, #0.87, # threshold for willingness to start prep. coverage is 13%
            'EventPrepDrop.threshold': 0.001#0.8
        }
    
    prep_intro3 = {
            "time":43, #around  2023
            "EventPrep.threshold":0.89, #0.965, #0.87, # threshold for willingness to start prep. coverage is 13%
            'EventPrepDrop.threshold': 0.001#0.8
        }
    
    ########## future scenarios config
    diagnosis_intro = {
        "time":44,
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'] + parameters['diagnosis_baseline_t5']+ 0.1 +0.15+0.5+0.2 + 1,
    }
    
    # prep cab-la
    cab_intro = {
            "time":44.1, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.95, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.95, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3
    }
    
    cab_intro_1 = {
            "time":47.1, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.90, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.978, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3
    }
    
    cab_intro_opt = {
            "time":44.11, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.93, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.93, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3
    }
    
    cab_intro_opt_1 = {
            "time":47.11, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.88, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.98, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3
    }
    
    
        
    # prep+cab+dvr
    dvr_intro = {
            "time":44.12, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.96, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.95, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3,
            "EventDVR.enabled":"true",
            "EventDVR.threshold": 0.975,
            "EventDVRDROP.enabled":"true",
            "EventDVRDROP.threshold": 0.1
        
    }
    
    dvr_intro_1 = {
            "time":47.12, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.91, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.978, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3,
            "EventDVR.enabled":"true",
            "EventDVR.threshold": 0.96,
            "EventDVRDROP.enabled":"true",
            "EventDVRDROP.threshold": 0.1
    }
    
    dvr_intro_opt = {
            "time":44.13, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.93, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.85, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3,
            "EventDVR.enabled":"true",
            "EventDVR.threshold": 0.96,
            "EventDVRDROP.enabled":"true",
            "EventDVRDROP.threshold": 0.1
        
    }
    
    dvr_intro_opt_1 = {
            "time":47.13, #around 2024
            "EventCAB.enabled":"true",
            "EventCAB.threshold": 0.90, #optimistic with 4400 users by 2030
            "EventPrep.threshold":0.97, #scale down oral prep 
            "EventCABDROP.enabled":"true",
            "EventCABDROP.threshold": 0.3,
            "EventDVR.enabled":"true",
            "EventDVR.threshold": 0.95,
            "EventDVRDROP.enabled":"true",
            "EventDVRDROP.threshold": 0.1
    }

    
    # vmmc
    vmmc_intro3 = {
        "time":44.14, #around 2024
        "EventVMMC.threshold": 0.1,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_0_2.csv"
    }

    # prep
    prep_intro_halfway_2030 = {
            "time":44.15, #around 2024
            "EventPrep.enabled": "true",
            "EventPrep.threshold":0.85, 
            'EventPrepDrop.threshold': 0.001#0.4
        }
    
    # prep_intro_halfway_2030_1 = {
    #         "time":47.15, #around 2027
    #         "EventPrep.enabled": "true",
    #         "EventPrep.threshold":0.70, 
    #         'EventPrepDrop.threshold': 0.001#0.4
    #     }
    
    prep_intro_optimistic_2030 = {
        "time":44.16, #around 2024
        "EventPrep.enabled": "true",
        "EventPrep.threshold":0.8, #0.87, 
        'EventPrepDrop.threshold': 0.001
    }
    
    # prep_intro_optimistic_2030_1 = {
    #     "time":47.16, #around 2024
    #     "EventPrep.enabled": "true",
    #     "EventPrep.threshold":0.65, #0.87, 
    #     'EventPrepDrop.threshold': 0.001
    # }
    
    # condom use
    condom_intro_4 = { 
            "time": 47.17, # 2024
            "EventCondom.threshold": 0.8, #0.99,
            "hivtransmission.threshold": 0.1# 0.1
    }
    
    # AGYW
    # A. diagnosis
    agyw_diagosis_halfway_2030 = {
        "time":44.18, #around 2024
        "diagnosis.AGYWfactor":0.9
    }
    
    agyw_diagosis_optimistic_2030 = {
        "time":44.19, #around 2024
        "diagnosis.AGYWfactor":1.7 
    }
    
    # B. prep
    agyw_prep_intro = {
            "time":44.2, #around 2024
            "EventPrep.AGYWthreshold":0.5
        }
    # C. condom
    agyw_condom_intro = {
            "time":44.21, #around 2024
            "EventCondom.AGYWthreshold":0.5
        }

    ART_factual = [hiv_testing, conception_1,#conception_2,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3]
    
    ART_counterfactual_VMMC = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2, vmmc_intro3,
                   prep_intro1,prep_intro2, prep_intro3]
    
    ART_counterfactual_diagnosis = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, diagnosis_intro]
    
    ART_counterfactual_PrEP_halfway = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1,prep_intro2, prep_intro3, prep_intro_halfway_2030]

    ART_counterfactual_PrEP_optimistic = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, prep_intro_optimistic_2030]
    
    ART_counterfactual_PrEP_cab_halfway = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1,prep_intro2, prep_intro3, cab_intro,cab_intro_1]
    
    ART_counterfactual_PrEP_cab_optimistic = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1,prep_intro2, prep_intro3, cab_intro_opt,cab_intro_opt_1]
    
    ART_counterfactual_PrEP_cab_dvr_halfway = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, dvr_intro, dvr_intro_1]
    
    ART_counterfactual_PrEP_cab_dvr_optimistic = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, dvr_intro_opt, dvr_intro_opt_1]
    
#     ART_counterfactual_condom = [hiv_testing, conception_1,
#                    art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
#                   condom_intro1, condom_intro2, condom_intro3, condom_intro_4, vmmc_intro1,vmmc_intro2,
#                    prep_intro1, prep_intro2, prep_intro3]
    
    AGYW_diagnosis_halfway = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, agyw_diagosis_halfway_2030]
    
    AGYW_diagnosis_optimistic = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, agyw_diagosis_optimistic_2030]
    
    AGYW_condom = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, agyw_condom_intro]
    
    AGYW_diagnosis_condom = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, agyw_diagosis_halfway_2030, agyw_condom_intro]
    
    # AGYW_prep = [hiv_testing, conception_1,
    #                art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
    #               condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
    #                prep_intro1, agyw_prep_intro]
    
    
    
    All_combined = [hiv_testing, conception_1,#conception_2,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1,vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3,
                   vmmc_intro3, dvr_intro, dvr_intro_1, agyw_diagosis_halfway_2030, agyw_condom_intro]
    
    All_combined_optimistic = [hiv_testing, conception_1,
                   art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, condom_intro_4, vmmc_intro1, vmmc_intro2,
                   prep_intro1, prep_intro2, prep_intro3, vmmc_intro3, diagnosis_intro,
                     dvr_intro_opt, dvr_intro_opt_1, agyw_diagosis_optimistic_2030]
    
    # running the simulation --------------------------------------------------
    identifier = f'model_{modelid}_seed_{seedid}'
    #rootDir = "/Users/emdominic/Documents/Wimmy/who_hiv_inc_modelling/Calibration/data" 
    rootDir = "Calibration/data" 
 
    destDir = os.path.join(rootDir, identifier)
    
    # Print log message
    print(f'========== Now running for model {modelid} seed {seedid} ===========')

    results = simpact.run(
        config=cfg_list,
        destDir=destDir,
        interventionConfig=ART_factual,
        seed=seedid,
        #identifierFormat=f'seed {identifier}',
        identifierFormat = identifier,
        quiet=True
    )

    datalist = psh.readthedata(results) 

    # Specify the file path to save the dictionary object
    file_path = f'Calibration/final_data/datalist_baseline_{identifier}.pkl'

    # Save dictionary to a single file using pickle
    with open(file_path, 'wb') as f:
        pickle.dump(datalist, f)
        
    shutil.rmtree(destDir) #deletes the folder with output files
    
    return None