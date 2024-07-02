def calibration_wrapper_function(parameters = None):
    
    import numpy as np
    import math
    import os
    import pickle
    import pandas as pd
    import shutil
    import random
    # import logging

    import pysimpactcyan
    import psimapacthelper as psh

    simpact = pysimpactcyan.PySimpactCyan()

    # age_distr = psh.agedistr_creator(shape = 5, scale = 65)

    # Creating cfg list -------------------------------------------------------
    cfg = {
        "population.agedistfile": "/home/jupyter/who-aids-prop/build/python/eswatini_2023.csv",
        "population.eyecap.fraction": 0.2,
        "population.simtime": 66,  # start 1985 and end in 2051
        "population.nummen": 1000,
        "population.numwomen": 1000,
        "population.msm": "no",
        "hivseed.time": 10,
        "hivseed.type": "amount",
        "hivseed.amount": 100,  # 30,
        #"hivseed.fraction" : 0.1,
        "hivseed.age.min": 15,
        "hivseed.age.max": 50,
        "hivtransmission.param.a": -1,
        "hivtransmission.param.b": -12.0220,
        "hivtransmission.param.c": 0.5,
        "hivtransmission.param.f1": math.log(2),
        "hivtransmission.param.f2": math.log(math.log(math.sqrt(2)) / math.log(2)) / 5,
        "person.eagerness.man.dist.type":"gamma",
        "person.eagerness.woman.dist.type":"gamma",
        "formation.hazard.type":"simple",
        "formation.hazard.simple.alpha_0":0.2,
        # "formation.hazard.type":"agegapry",
        # "formation.hazard.agegapry.baseline":4,
        # "formation.hazard.agegapry.maxageref.diff":1,
        # "formation.hazard.agegapry.gap_factor_man_age": -0.01,
        # "formation.hazard.agegapry.gap_factor_woman_age": -0.01,
        # "formation.hazard.agegapry.gap_factor_man_const": 0,
        # "formation.hazard.agegapry.gap_factor_woman_const": 0,
        # "formation.hazard.agegapry.gap_factor_man_exp": -1,
        # "formation.hazard.agegapry.gap_factor_woman_exp": -1,
        # "formation.hazard.agegapry.gap_agescale_man": 0.25,
        # "formation.hazard.agegapry.gap_agescale_woman": 0.25,
        # "formation.hazard.agegapry.meanage": -0.025, #younger people more likely to form relationship. Hazard decreases with the average age
        "dissolution.alpha_0": 0.1,
        "dissolution.alpha_4": -0.05,
        "debut.debutage": 15,
        "conception.alpha_base": -2.7,
        "dropout.interval.dist.type": "exponential",
        "syncrefyear.interval": 1,
        "syncpopstats.interval": 5,
        # vmmc
        "EventVMMC.enabled": "false",
        "EventVMMC.threshold": 0.5,
        "EventVMMC.m_vmmcprobDist.dist.type": "uniform",
        "EventVMMC.m_vmmcprobDist.dist.uniform.max": 1,
        "EventVMMC.m_vmmcprobDist.dist.uniform.min": 0,
        "EventVMMC.m_vmmcscheduleDist.dist.type": "discrete.csv.twocol",
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_0.csv",
        # condom programming
        "EventCondom.enabled" : "false",
        "EventCondom.m_condomprobDist.dist.type": "uniform",
        "EventCondom.m_condomprobDist.dist.uniform.min": 0,
        "EventCondom.m_condomprobDist.dist.uniform.max": 1,
        "EventCondom.threshold": 0.99, # threshold for condom preference for an individual 
        "hivtransmission.threshold": 0.5, # threshold for condom use in formation
        "hivtransmission.m_condomformationdist.dist.type":"discrete.csv.twocol",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file":  "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor": 'yes', #to force distirbution to return exact values and not any value within the bin
        "hivtransmission.threshold": 0.3, # threshold for condom use in formation
        # PrEP
        "EventPrep.enabled": "false",
        "EventPrep.threshold": 0.5
    }
    

    # Initial values
    mu_cd4 = 800
    var_cd4 = 200 ** 2
    mu_cd4_end = 20
    var_cd4_end = 5

    # Lognormal distribution parameters for CD4 start
    cfg["person.cd4.start.dist.type"] = "lognormal"
    cfg["person.cd4.start.dist.lognormal.zeta"] = math.log(mu_cd4 / math.sqrt(1 + var_cd4 / mu_cd4 ** 2))
    cfg["person.cd4.start.dist.lognormal.sigma"] = math.sqrt(math.log(1 + var_cd4 / mu_cd4 ** 2))

    # Lognormal distribution parameters for CD4 end
    cfg["person.cd4.end.dist.type"] = "lognormal"
    cfg["person.cd4.end.dist.lognormal.zeta"] = math.log(mu_cd4_end / math.sqrt(1 + var_cd4_end / mu_cd4_end ** 2))
    cfg["person.cd4.end.dist.lognormal.sigma"] = math.sqrt(math.log(1 + var_cd4_end / mu_cd4_end ** 2))

    # Additional configuration settings
    #cfg["formation.hazard.agegapry.baseline"] = 2
    cfg["mortality.aids.survtime.C"] = 65 
    cfg["mortality.aids.survtime.k"] = -0.2
    cfg["monitoring.fraction.log_viralload"] = 0.3
    # cfg["mortality.normal.weibull.scale"] = 1000 #no natural deaths, only HIV
    
    # introduce some randomness to survival time for HIV infected people. This is the 'x' parameter in the formula
    # cfg["person.survtime.logoffset.dist.type"] = "normal"
    # cfg["person.survtime.logoffset.dist.normal.mu"] = 0
    # cfg["person.survtime.logoffset.dist.normal.sigma"] = 0.25
    cfg["person.survtime.logoffset.dist.type"] = "uniform"
    cfg["person.survtime.logoffset.dist.uniform.min"] = 0
    cfg["person.survtime.logoffset.dist.uniform.max"] = 0.01

    cfg["person.agegap.man.dist.type"] = "normal"
    cfg["person.agegap.woman.dist.type"] = "normal"

    cfg["monitoring.cd4.threshold"] = 1
    cfg["person.art.accept.threshold.dist.fixed.value"] = 1
    cfg["diagnosis.baseline"] = -99999
    cfg["periodiclogging.interval"] = 0.25
    cfg["dropout.interval.dist.exponential.lambda"] = 0.1

    # Assuming cfg["population.simtime"] and cfg["population.nummen"] are defined elsewhere
    cfg["population.maxevents"] = float(cfg["population.simtime"]) * float(cfg["population.nummen"]) * 6

    cfg["person.vsp.toacute.x"] = 5  # See Bellan PLoS Medicine

    # seedid = 433474235
    seed_generator = psh.UniqueSeedGenerator()
    seedid = seed_generator.generate_seed()

    #TODO: write a fuction such that if you don't pass a calibration parameter value, it uses the default eg the values above
    # calibration parameters
    
    cfg["hivtransmission.param.f1"] = round(math.log(parameters['hivtransmission_param_f1']),6)
    cfg["hivtransmission.param.f2"] = round(math.log(math.log(math.sqrt(parameters['hivtransmission_param_f1'])) / math.log(parameters['hivtransmission_param_f1'])) / 5,6)
    #cfg["hivtransmission.param.f2"] = -0.000000013862943611198902
    cfg["hivtransmission.param.a"] = round(parameters['hivtransmission_param_a'],6)
    # cfg["hivtransmission.param.b"] = round(parameters['hivtransmission_param_b'],6)
    cfg["hivtransmission.param.c"] = round(parameters['hivtransmission_param_c'],6)
    #cfg["formation.hazard.agegapry.gap_agescale_man"] = round(parameters['formation_hazard_agegapry_gap_agescale_man_woman'],6)
    #cfg["formation.hazard.agegapry.gap_agescale_woman"] = round(parameters['formation_hazard_agegapry_gap_agescale_man_woman'],6)
    cfg["person.agegap.man.dist.normal.mu"] = round(parameters['person_agegap_man_dist_normal_mu'],6)
    cfg["person.agegap.woman.dist.normal.mu"] = round(parameters['person_agegap_woman_dist_normal_mu'],6)
    cfg["person.agegap.man.dist.normal.sigma"] = round(parameters['person_agegap_man_dist_normal_sigma'],6)
    cfg["person.agegap.woman.dist.normal.sigma"] = round(parameters['person_agegap_woman_dist_normal_sigma'],6)
    cfg["person.eagerness.man.dist.gamma.a"] = round(parameters['person_eagerness_man_dist_gamma_a'],6)
    cfg["person.eagerness.woman.dist.gamma.a"] = round(parameters['person_eagerness_woman_dist_gamma_a'],6)
    cfg["person.eagerness.man.dist.gamma.b"] = round(parameters['person_eagerness_man_dist_gamma_b'],6)
    cfg["person.eagerness.woman.dist.gamma.b"] = round(parameters['person_eagerness_woman_dist_gamma_b'],6)
    #cfg["formation.hazard.agegapry.gap_factor_man_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],4)
    #cfg["formation.hazard.agegapry.gap_factor_woman_exp"] = round(parameters['formation_hazard_agegapry_gap_factor_man_woman_exp'],4)
    #cfg["formation.hazard.agegapry.baseline"] = round(parameters['formation_hazard_agegapry_baseline'],4)
    #cfg["formation.hazard.agegapry.numrel_man"] = round(parameters['formation_hazard_agegapry_numrel_man_woman'],4)
    #cfg["formation.hazard.agegapry.numrel_woman"] = round(parameters['formation_hazard_agegapry_numrel_man_woman'],4)
    cfg["conception.alpha_base"] = round(parameters['conception_alpha_base'],4)
    cfg["dissolution.alpha_0"] = round(parameters['dissolution_alpha_0'],4)
    cfg["formation.hazard.simple.alpha_0"] = parameters['formation_hazard_simple_alpha_0']

    # cfg["formation.hazard.agegapry.gap_agescale_man"] = 0.4
    # cfg["formation.hazard.agegapry.gap_agescale_woman"] = 0.4
    # cfg["person.agegap.man.dist.normal.mu"] = 1
    # cfg["person.agegap.woman.dist.normal.mu"] = 1
    # cfg["person.agegap.man.dist.normal.sigma"] = 3.5
    # cfg["person.agegap.woman.dist.normal.sigma"] = 3.5
    # cfg["person.eagerness.man.dist.gamma.a"] = 0.8
    # cfg["person.eagerness.woman.dist.gamma.a"] = 0.3
    # cfg["person.eagerness.man.dist.gamma.b"] = 45
    # cfg["person.eagerness.woman.dist.gamma.b"] = 30
    # cfg["formation.hazard.agegapry.gap_factor_man_exp"] = -1
    # cfg["formation.hazard.agegapry.gap_factor_woman_exp"] = -1
    # cfg["formation.hazard.agegapry.baseline"] = 3.8
    # cfg["formation.hazard.agegapry.numrel_man"] = -0.5
    # cfg["formation.hazard.agegapry.numrel_woman"] = -0.5
    # cfg["conception.alpha_base"] = -2.7
    # cfg["dissolution.alpha_0"] = -0.05

    # ART introduction configurations
    art_intro = {
        "time": 18, #around 2003
        "diagnosis.baseline": -1.5,#parameters['diagnosis_baseline_t0'],#-2,
        "monitoring.cd4.threshold": 100,
        "person.survtime.logoffset.dist.uniform.max":0.5
        #"formation.hazard.agegapry.baseline": cfg['formation.hazard.agegapry.baseline'] - 0.5
    }

    art_intro1 = {
        "time": 22,
        "diagnosis.baseline": -1.5+0.6, #parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'],#-1.8,
        "monitoring.cd4.threshold": 150,
        "hivtransmission.param.a": parameters['hivtransmission_param_a'] + 2,
        "formation.hazard.simple.alpha_0":parameters['formation_hazard_simple_alpha_0']+0.1
    }

    art_intro2 = {
        "time": 23,
        "diagnosis.baseline": -1.5+0.6+0.6,#parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'],#-1.5,
        "monitoring.cd4.threshold": 200,
        #"formation.hazard.agegapry.baseline": cfg['formation.hazard.agegapry.baseline'] - 1
    }

    art_intro3 = {
        "time": 30,
        "diagnosis.baseline":  -1.5+0.6+0.6+0.8,#parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t3'],#-1,
        "monitoring.cd4.threshold": 350
    }

    art_intro4 = {
        "time": 33.5,
        "diagnosis.baseline": -1.5+0.6+0.6+0.8+0.8,#parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t3'] + parameters['diagnosis_baseline_t4'],#-0.5,
        "monitoring.cd4.threshold": 500
    }

    art_intro5 = {
        "time": 36.75,
        "monitoring.cd4.threshold": 6000
    }
    
    
    #vmmc
    
    vmmc_intro1 = {
        "time":22.5, #around 2007
        "EventVMMC.enabled": "true",
        "EventVMMC.threshold": 0.25,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv",
    }
    
    vmmc_intro2 = {
        "time":32.5, 
        "EventVMMC.threshold": 0.35,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_2.csv",
    }
    
    vmmc_intro3 = {
        "time":37.5,
        "EventVMMC.threshold": 0.5,
        "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_3.csv",
    }
    
    #condom use
    condom_intro1 = { 
        "time": 15, #around 2000
        "EventCondom.enabled": "true"
    }
    
    
    condom_intro2 = { 
        "time": 25, 
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv" }
    
    condom_intro3 = { 
        "time": 31, 
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv" }
    
    prep_intro1 = {
            "time":32, #around 2017
            "EventPrep.enabled": "true"
        }

    

   

#     eswatini_factual = [condom_intro1, art_intro, art_intro1, art_intro2,vmmc_intro1,
#                             condom_intro2, art_intro3, condom_intro3, prep_intro1, art_intro4, 
#                             art_intro5,vmmc_intro2,vmmc_intro3]
    eswatini_factual = [ art_intro, art_intro1, art_intro2,vmmc_intro1,
                            art_intro3,  prep_intro1, art_intro4, condom_intro1,
                            art_intro5,vmmc_intro2,vmmc_intro3]

    

    # running the simulation --------------------------------------------------
    identifier = str(seedid)
    rootDir = "Calibration/data" 
    # rootDir <- "/tmp"
    destDir = os.path.join(rootDir, identifier)

    results = simpact.run(
        config=cfg,
        destDir=destDir,
        #agedist=age_distr,
        interventionConfig=eswatini_factual,
        seed=seedid,
        identifierFormat=f'seed {identifier}',
        quiet=False
    )
    
    datalist = psh.readthedata(results)
    
    # Specify the file path to save the dictionary object
    file_path = f'Calibration/final_data/datalist_seed{identifier}.pkl'

    # Save dictionary to a single file using pickle
    with open(file_path, 'wb') as f:
        pickle.dump(datalist, f)

    outputdict = {}

    # Post processing results -------------------------------------------------
    if len(results) == 0:
        outputdict = {"growthrate_35": np.nan,
               'prev_f_25_15_49': np.nan,
               'prev_m_25_15_49': np.nan,
               'prev_f_39_15_49': np.nan,
               'prev_m_39_15_49': np.nan,
               'inc_f_25_15_49': np.nan,
               'inc_m_25_15_49': np.nan,
               'inc_f_39_15_49': np.nan,
               'inc_m_39_15_49': np.nan,
               'art_cov_t_25.5': np.nan,
               'art_cov_t_30.5': np.nan,
               'art_cov_t_35.5': np.nan,
               'art_cov_t_38.5': np.nan,
               'vl_suppr_39': np.nan,
               'vmmc_15_49': np.nan,
               'vmmc_15_24': np.nan 
              }
    else:
        datalist_EAAA = psh.readthedata(results)
        agegroup = [15,50]
#         # if float(results["eventsexecuted"]) >= (float(cfg["population.maxevents"]) - 1):
#         #     outputdict = {"growthrate_35": np.nan,
#         #        'prev_f_25_15_49': np.nan,
#         #        'prev_m_25_15_49': np.nan,
#         #        'prev_f_39_15_49': np.nan,
#         #        'prev_m_39_15_49': np.nan,
#         #        'inc_f_25_15_49': np.nan,
#         #        'inc_m_25_15_49': np.nan,
#         #        'inc_f_39_15_49': np.nan,
#         #        'inc_m_39_15_49': np.nan,
#         #        'art_cov_t_25.5': np.nan,
#         #        'art_cov_t_30.5': np.nan,
#         #        'art_cov_t_35.5': np.nan,
#         #        'art_cov_t_38.5': np.nan,
#         #        'vl_suppr_39': np.nan,
#         #        'vmmc_15_49': np.nan,
#         #        'vmmc_15_24': np.nan 
#         #       }
#         # else:


#         ######## Population growth rate
#         growthrate_19 = psh.pop_growth_calculator(datalist=datalist_EAAA, timewindow=[18, 19])  # Between 20004 and 2005
#         outputdict['growthrate_19'] = math.exp(growthrate_19)
        
#         growthrate_35 = psh.pop_growth_calculator(datalist=datalist_EAAA, timewindow=[34, 35])  # Between 2019 and 2020
#         outputdict['growthrate_35'] = math.exp(growthrate_35)


#         ######## HIV prevalence. To be compared to SHIMS I estimates (point estimate at March 2011 ~ t = 31.25)

# #             # Initialize empty lists to store results
# #             # prev_list = []
# #             # prev_dict = {}

#         # Iterate over age groups and calculate prevalences
#         for agegroup in [(15, 50)]:
# #             for agegroup in [(15, 20), (20, 25), (25, 30),
# #                             (30, 35), (35, 40), (40, 45), 
# #                             (45, 50), (45, 50)]:

            # Calculate prevalence for males and females separately
        prev_m_15 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=15.5).loc[0, 'pointprevalence']
        prev_f_15 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=15.5).loc[1, 'pointprevalence']
        prev_m_25 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=25.5).loc[0, 'pointprevalence']
        prev_f_25 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=25.5).loc[1, 'pointprevalence']
        prev_m_39 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=38.5).loc[0, 'pointprevalence']
        prev_f_39 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=38.5).loc[1, 'pointprevalence']


        start_age, end_age = agegroup 
        end_age = end_age-1
        outputdict[f"prev_f_15_{start_age}_{end_age}"] = prev_f_15 if not pd.isnull(prev_f_15) else 0
        outputdict[f"prev_m_15_{start_age}_{end_age}"] = prev_m_15 if not pd.isnull(prev_m_15) else 0
        outputdict[f"prev_f_25_{start_age}_{end_age}"] = prev_f_25 if not pd.isnull(prev_f_25) else 0
        outputdict[f"prev_m_25_{start_age}_{end_age}"] = prev_m_25 if not pd.isnull(prev_m_25) else 0
        outputdict[f"prev_f_39_{start_age}_{end_age}"] = prev_f_39 if not pd.isnull(prev_f_39) else 0
        outputdict[f"prev_m_39_{start_age}_{end_age}"] = prev_m_39 if not pd.isnull(prev_m_39) else 0


    # Append results to respective lists
    # prev_list.append(float(prev_f) if not pd.isnull(prev_f) else 0)
    # prev_list.append(float(prev_m) if not pd.isnull(prev_m) else 0)

    ######### HIV incidence. Average follow-up period March 2011 until mid Sept 2011 (0.55 years)

    # Iterate over age groups and calculate incidence
for agegroup in [(15, 50)]:
# for agegroup in [(15, 20), (20, 25), (25, 30),
#                     (30, 35), (35, 40), (40, 45), 
#                     (45, 50), (45, 50)]:

        # Calculate incidence per 10 per period for males and females separately

        inc_m_25 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[25,26]).loc[0, 'incidence']
        inc_f_25 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[25,26]).loc[1, 'incidence']

        inc_m_39 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[38,39]).loc[0, 'incidence']
        inc_f_39 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[38,39]).loc[1, 'incidence']

        inc_m_25 = inc_m_25*10
        inc_f_25 = inc_f_25*10
        inc_m_39 = inc_m_39*10
        inc_f_39 = inc_f_39*10

        start_age, end_age = agegroup 
        end_age = end_age-1

        outputdict[f"inc_f_25_{start_age}_{end_age}"] = round(inc_f_25,5) if not pd.isnull(inc_f_25) else 0
        outputdict[f"inc_m_25_{start_age}_{end_age}"] = round(inc_m_25,5) if not pd.isnull(inc_m_25) else 0
        outputdict[f"inc_f_39_{start_age}_{end_age}"] = round(inc_f_39,5) if not pd.isnull(inc_f_39) else 0
        outputdict[f"inc_m_39_{start_age}_{end_age}"] = round(inc_m_39,5) if not pd.isnull(inc_m_39) else 0


######### ART coverage among adults 15+ years old from spectrum data (2005 - 2023 estimates)

# Define evaluation timepoints
ART_cov_eval_timepoints = np.array([25.5, 30.5, 35.5, 38.5])  # Using np.arange to generate timepoints

# Initialize ART coverage vector
# ART_cov_vector = [0] * len(ART_cov_eval_timepoints)

# Iterate over each timepoint
for idx, art_cov_timepoint in enumerate(ART_cov_eval_timepoints):
# Calculate ART coverage using ART_coverage_calculator (assuming it returns a dictionary or object with 'sum.onART' and 'sum.cases')
    result = psh.ART_coverage_calculator(datalist=datalist_EAAA,
                                        agegroup=[15, 150],
                                        timepoint=art_cov_timepoint)
    try:
        art_cov = float(result['ART_coverage'][2])
    except (IndexError, ValueError, KeyError) as e:
        art_cov = 0
    # ART_cov_vector[idx] = art_cov
    outputdict[f"art_cov_t_{art_cov_timepoint}"] = art_cov

########## VL suppression fraction (all ages in 2017 ~ >= 15 yo) 0.74
vl = psh.VL_suppression_calculator(datalist=datalist_EAAA,
                                    agegroup=[15, 300], 
                                    timepoint=38.5, 
                                    vl_cutoff=1000, 
                                    site="All")
try:
    VL_suppression_fraction = float(vl['vl_suppr_frac'][2])
except (IndexError, ValueError, KeyError) as e:
    VL_suppression_fraction = 0

outputdict['vl_suppr_39']= VL_suppression_fraction

########### VMMC target
# 15-49 in 2023 48%,
vmmc_1 = psh.vmmc_calculator(datalist=datalist_EAAA, agegroup=[15, 50], timepoint=38.5)

try:
    vmmc_15_49_fraction = float(vmmc_1['vmmcprevalence'].iloc[0])
except (IndexError, ValueError, KeyError) as e:
    vmmc_15_49_fraction = 0

outputdict['vmmc_15_49']= round(vmmc_15_49_fraction,5)

# 15-24 ~ 70%
vmmc_2 = psh.vmmc_calculator(datalist=datalist_EAAA, agegroup=[15, 24], timepoint=38.5)

try:
    vmmc_15_24_fraction = float(vmmc_2['vmmcprevalence'].iloc[0])
except (IndexError, ValueError, KeyError) as e:
    vmmc_15_24_fraction = 0

outputdict['vmmc_15_24']= round(vmmc_15_24_fraction,5)

########### Condom use SHIMS3 62.6 use condoms 2021
condom = psh.condom_users_calculator(datalist=datalist_EAAA, agegroup=[15, 50], timepoint=36)

try:
    condom_fraction = float(condom['condom_users_prevalence'][2])
except (IndexError, ValueError, KeyError) as e:
    condom_fraction = 0

# outputdict['condom_15_49']= round(condom_fraction,5)

#     #shutil.rmtree(destDir) #deletes the folder with output files


    print(outputdict)
    return(results)



# cfg = {}
# cfg = { "population.nummen": 500,
#         "population.numwomen": 500,
#         "population.simtime": 50}

# res = simpact.run(cfg, "data",
#                   seed=42) 

# python terminal
# with open('calibration_wrapper_function.py') as file:
# ...     exec(file.read())
