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
    population_numwomen=1000,
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
    formation_hazard_agegapry_gap_factor_man_age=-0.01,
    formation_hazard_agegapry_gap_factor_woman_age=-0.01,
    formation_hazard_agegapry_meanage=-0.025,
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
            
    cfg_list["population.agedistfile"] = "/home/jupyter/who-aids-prop/build/python/agedist.csv"
    cfg_list['diagnosis.eagernessfactor'] = np.log(1.025)
    cfg_list["mortality.aids.survtime.art_e"] = 10 #art survival time effect
    
    # vmmc
    cfg_list["EventVMMC.enabled"] = "false"
    cfg_list["EventVMMC.threshold"] = 0.5
    cfg_list["EventVMMC.m_vmmcprobDist.dist.type"] = "uniform"
    cfg_list["EventVMMC.m_vmmcprobDist.dist.uniform.max"] = 1
    cfg_list["EventVMMC.m_vmmcprobDist.dist.uniform.min"] = 0
    cfg_list["EventVMMC.m_vmmcscheduleDist.dist.type"] = "discrete.csv.twocol"
    cfg_list["EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_0.csv"

    # condom
    cfg_list["EventCondom.enabled"] = "false"
    cfg_list["EventCondom.m_condomprobDist.dist.type"] = "uniform"
    cfg_list["EventCondom.m_condomprobDist.dist.uniform.min"] = 0
    cfg_list["EventCondom.m_condomprobDist.dist.uniform.max"] = 1
    cfg_list["EventCondom.threshold"] = 0.8
    cfg_list["hivtransmission.m_condomformationdist.dist.type"] = "discrete.csv.twocol"
    cfg_list["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file"] = "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv"
    cfg_list["hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor"] = "yes"
    cfg_list["hivtransmission.threshold"] = 0.3

    # prep
    cfg_list["EventPrep.enabled"] = "false" #current code can't switch prep off
    cfg_list["EventPrep.threshold"] = 1 # threshold for willingness to start prep. Nobody starts
    cfg_list['EventPrep.m_prepprobDist.dist.type'] ='uniform' # willingness to start prep
    cfg_list['EventPrep.m_prepprobDist.dist.uniform.min'] = 0
    cfg_list['EventPrep.m_prepprobDist.dist.uniform.max'] = 1
    # cfg_list['EventPrep.m_prepscheduleDist.dist.type'] = 'fixed' #legacy options
    # cfg_list['EventPrep.m_prepscheduleDist.dist.fixed.value'] = 0.246575
    cfg_list['EventPrepDrop.threshold'] = 0.2 # threshold for dropping
    cfg_list['EventPrepDrop.interval.dist.type'] ='uniform' # distribution to choose probablity of dropping which will be used alongside number of relationships or hiv status to decide if dropping out
    cfg_list['EventPrepDrop.interval.dist.uniform.min'] = 0.25
    cfg_list['EventPrepDrop.interval.dist.uniform.max'] = 10.0

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
    cfg_list["monitoring.m_artDist.dist.normal.max"] = 0.75
    cfg_list["monitoring.m_artDist.dist.normal.mu"] = 0.45
    cfg_list["monitoring.m_artDist.dist.normal.sigma"] = 0.2

    cfg_list["person.survtime.logoffset.dist.type"] = "normal"
    cfg_list["person.survtime.logoffset.dist.normal.mu"] = 0
    cfg_list["person.survtime.logoffset.dist.normal.sigma"] = 0.2

    cfg_list["person.agegap.man.dist.type"] = "normal"
    cfg_list["person.agegap.woman.dist.type"] = "normal"

    cfg_list["monitoring.cd4.threshold"] = 1
    cfg_list["person.art.accept.threshold.dist.fixed.value"] = 0.85
    cfg_list["diagnosis.baseline"] = -99999
    cfg_list["periodiclogging.interval"] = 0.25
    cfg_list["dropout.interval.dist.fixed.value"] = 500 #cfg_list["dropout.interval.dist.exponential.lambda"] = 0.1


    # # Assuming cfg_list["population.simtime"] and cfg_list["population.nummen"] are defined elsewhere
    # cfg_list["population.maxevents"] = float(cfg_list["population.simtime"]) * float(cfg_list["population.nummen"]) * 100

    cfg_list["person.vsp.toacute.x"] = 5  # See Bellan PLoS Medicine

    # seedid = random.randint(0,1000000000)
    # seed_generator = psh.UniqueSeedGenerator()
    # seedid = seed_generator.generate_seed()
    seedid = parameters['seed']

    #TODO: write a fuction such that if you don't pass a calibration parameter value, it uses the default eg the values above
    # calibration parameters
    
    cfg_list["hivtransmission.param.f1"] = round(parameters['hivtransmission_param_f1'],8)
    cfg_list["hivtransmission.param.f2"] = round(math.log(math.log(math.sqrt(parameters['hivtransmission_param_f1'])) / math.log(parameters['hivtransmission_param_f1'])) / 5,8)
    cfg_list["hivtransmission.param.a"] = round(parameters['hivtransmission_param_a'],8)
    cfg_list["formation.hazard.agegapry.gap_agescale_man"] = round(parameters['formation_hazard_agegapry_gap_agescale_man_woman'],8)
    cfg_list["formation.hazard.agegapry.gap_agescale_woman"] = round(parameters['formation_hazard_agegapry_gap_agescale_man_woman'],8)
    cfg_list["person.agegap.man.dist.normal.mu"] = round(parameters['person_agegap_man_woman_dist_normal_mu'],8)
    cfg_list["person.agegap.woman.dist.normal.mu"] = round(parameters['person_agegap_man_woman_dist_normal_mu'],8)
    cfg_list["person.agegap.man.dist.normal.sigma"] = round(parameters['person_agegap_man_woman_dist_normal_sigma'],8)
    cfg_list["person.agegap.woman.dist.normal.sigma"] = round(parameters['person_agegap_man_woman_dist_normal_sigma'],8)
    cfg_list["person.eagerness.man.dist.gamma.a"] = round(parameters['person_eagerness_man_dist_gamma_a'],8)
    cfg_list["person.eagerness.woman.dist.gamma.a"] = round(parameters['person_eagerness_woman_dist_gamma_a'],8)
    cfg_list["person.eagerness.man.dist.gamma.b"] = round(parameters['person_eagerness_man_dist_gamma_b'],8)
    cfg_list["person.eagerness.woman.dist.gamma.b"] = round(parameters['person_eagerness_woman_dist_gamma_b'],8)
    cfg_list["formation.hazard.agegapry.gap_factor_man_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],8)
    cfg_list["formation.hazard.agegapry.gap_factor_woman_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],8)
    cfg_list["formation.hazard.agegapry.baseline"] = round(parameters['formation_hazard_agegapry_baseline'],8)
    cfg_list["formation.hazard.agegapry.numrel_man"] = round(parameters['formation_hazard_agegapry_numrel_man'],8)
    cfg_list["formation.hazard.agegapry.numrel_woman"] = round(parameters['formation_hazard_agegapry_numrel_woman'],8)
    cfg_list["conception.alpha_base"] = round(parameters['conception_alpha_base'],8)
    cfg_list["dissolution.alpha_0"] = round(parameters['dissolution_alpha_0'],8)
    cfg_list['diagnosis.eagernessfactor'] = round(math.log(parameters['diagnosis_eagernessfactor']),8)

    # ART introduction configurations
    # ART introduction configurations
    hiv_testing = {
    "time": 12, #around 1992 
    "diagnosis.baseline":-4
    }
    # ART introduction configurations
    art_intro = {
        "time": 20, #around 2000
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'], #-1,
        "monitoring.cd4.threshold": 100,
        "conception.alpha_base": round(parameters['conception_alpha_base'],8) - 0.1,
        "formation.hazard.agegapry.baseline": cfg_list["formation.hazard.agegapry.baseline"] + 0.5
    }

    art_intro1 = {
        "time": 22, #around 2002
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'], #-0.8,
        "monitoring.cd4.threshold": 150
    }

    art_intro2 = {
        "time": 23, #around 2003
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'], #-0.6,
        "monitoring.cd4.threshold": 200,
        "conception.alpha_base": round(parameters['conception_alpha_base'],8) - 0.15,
        "formation.hazard.agegapry.baseline": cfg_list["formation.hazard.agegapry.baseline"] + 0.7
    }

    art_intro2_2 = {
        "time": 26, #around 2006
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'], #-0.4,
        "person.art.accept.threshold.dist.fixed.value": 0.9,
    }

    art_intro3 = {
        "time": 30, # 2010
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3'],#-0.2,
        "monitoring.cd4.threshold": 350,
        "monitoring.m_artDist.dist.normal.mu": 0.35,
        "monitoring.m_artDist.dist.normal.min": 0.15,
        "monitoring.m_artDist.dist.normal.max":0.55
    }

    art_intro4 = {
        "time": 33.5, #around 2013
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'], #-0.1,
        "monitoring.cd4.threshold": 500,
        "person.art.accept.threshold.dist.fixed.value": 0.95
    }

    art_intro5 = {
        "time": 36.75, #around oct 2016
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'] + parameters['diagnosis_baseline_t5'],#-0.01,
        "monitoring.cd4.threshold":100000, 
        "monitoring.m_artDist.dist.normal.mu": 0.3,
        "monitoring.m_artDist.dist.normal.min": 0.1,
        "monitoring.m_artDist.dist.normal.max":0.5,
        "mortality.aids.survtime.art_e": 15,
        "conception.alpha_base": round(parameters['conception_alpha_base'],8) - 0.2,
    }


        #condom use
    condom_intro1 = { 
            "time": 18, #before 2000
            "EventCondom.enabled": "true",
            "EventCondom.threshold": 0.8,
            "hivtransmission.threshold": 0.2 #relationships using condoms consistently
        }

    condom_intro2 = { 
            "time": 25, 
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv", 
            "EventCondom.threshold": 0.85,
            "hivtransmission.threshold": 0.15}

    condom_intro3 = { 
            "time": 31, 
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv",
            "EventCondom.threshold": 0.9,
            "hivtransmission.threshold": 0.1}

    #vmmc

    vmmc_intro1 = {
        "time":27, #around 2007
        "EventVMMC.enabled": "true",
        "EventVMMC.threshold": 0.25,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv",
    }

    vmmc_intro2 = {
        "time":32.5, 
        "EventVMMC.threshold": 0.25
    }

    vmmc_intro3 = {
        "time":37.5,
        "EventVMMC.threshold": 0.25
    }

    # prep
    prep_intro1 = {
            "time":37, #around 2017
            "EventPrep.enabled": "true",
            "EventPrep.threshold":0.87 # threshold for willingness to start prep. coverage is 13%
        
        }


    ART_factual = [hiv_testing, art_intro, art_intro1, art_intro2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1, vmmc_intro2, vmmc_intro3,
                  prep_intro1]

    # running the simulation --------------------------------------------------
    identifier = str(seedid)
    #rootDir = "/Users/emdominic/Documents/Wimmy/who_hiv_inc_modelling/Calibration/data" 
    rootDir = "Calibration/data" 
 
    destDir = os.path.join(rootDir, identifier)
    
    # Print log message
    print(f'========== Now running for parameter set with seed {seedid} ===========')

    results = simpact.run(
        config=cfg_list,
        destDir=destDir,
        interventionConfig=ART_factual,
        seed=seedid,
        identifierFormat=f'seed {identifier}',
        quiet=True
    )

    datalist = psh.readthedata(results)

    # Specify the file path to save the dictionary object
    file_path = f'Calibration/final_data/datalist_seed{identifier}.pkl'

    # Save dictionary to a single file using pickle
    with open(file_path, 'wb') as f:
        pickle.dump(datalist, f)
        
    shutil.rmtree(destDir) #deletes the folder with output files
    
    return None



