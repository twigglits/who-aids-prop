def calibration_wrapper_function(parameters = None):
    
    import numpy as np
    import math
    import os
    import pandas as pd
    import shutil
    import random

    import pysimpactcyan
    import psimapacthelper as psh

    simpact = pysimpactcyan.PySimpactCyan()

    # Creating cfg list -------------------------------------------------------
    cfg_list = psh.input_params_creator(
    population_eyecap_fraction=0.2,
    population_simtime=21,  # Until 1 January 2001
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
    dropout_interval_dist_type="exponential",
    person_eagerness_man_dist_gamma_a=0.679322802,
    person_eagerness_woman_dist_gamma_a=0.679322802,
    person_eagerness_man_dist_gamma_b=47.03264158,
    person_eagerness_woman_dist_gamma_b=47.03264158,
    formation_hazard_agegapry_numrel_man= -0.649752699,
    formation_hazard_agegapry_numrel_woman= -0.61526928)
            
    cfg_list["population.agedistfile"] = "/home/jupyter/who-aids-prop/build/python/agedist.csv"
    cfg_list['diagnosis.eagernessfactor'] = np.log(1.025)
    cfg_list["mortality.aids.survtime.art_e"] = 10
    
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
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.mean"] = 3.95
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.sigma"] = 1
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.rho"] = 0.33
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.min"] = 2
    cfg_list["person.vsp.model.logdist2d.dist2d.binormalsymm.max"] = 6

    # Additional configuration settings
    cfg_list["formation.hazard.agegapry.baseline"] = 3.325157223
    cfg_list["mortality.aids.survtime.C"] = 65
    cfg_list["mortality.aids.survtime.k"] = -0.2
    cfg_list["monitoring.fraction.log_viralload"] = 0.35#0.5

    cfg_list["person.survtime.logoffset.dist.type"] = "normal"
    cfg_list["person.survtime.logoffset.dist.normal.mu"] = 0
    cfg_list["person.survtime.logoffset.dist.normal.sigma"] = 0.2

    cfg_list["person.agegap.man.dist.type"] = "normal"
    cfg_list["person.agegap.woman.dist.type"] = "normal"

    cfg_list["monitoring.cd4.threshold"] = 1
    cfg_list["person.art.accept.threshold.dist.fixed.value"] = 0.85
    cfg_list["diagnosis.baseline"] = -99999
    cfg_list["periodiclogging.interval"] = 0.25
    cfg_list["dropout.interval.dist.exponential.lambda"] = 0.01

    # # Assuming cfg_list["population.simtime"] and cfg_list["population.nummen"] are defined elsewhere
    # cfg_list["population.maxevents"] = float(cfg_list["population.simtime"]) * float(cfg_list["population.nummen"]) * 100

    cfg_list["person.vsp.toacute.x"] = 5  # See Bellan PLoS Medicine

    # seedid = random.randint(0,1000000000)
    seed_generator = psh.UniqueSeedGenerator()
    seedid = seed_generator.generate_seed()

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
        "monitoring.fraction.log_viralload": 0.3#0.45
    }

    art_intro4 = {
        "time": 33.5, #around 2013
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'], #-0.1,
        "monitoring.cd4.threshold": 500,
        "monitoring.fraction.log_viralload": 0.3,#0.4, #0.35
        "person.art.accept.threshold.dist.fixed.value": 0.95
    }

    art_intro5 = {
        "time": 36.75, #around oct 2016
        "diagnosis.baseline": parameters['diagnosis_baseline_t0'] + parameters['diagnosis_baseline_t1'] + parameters['diagnosis_baseline_t2'] + parameters['diagnosis_baseline_t2_2'] + parameters['diagnosis_baseline_t3']+ parameters['diagnosis_baseline_t4'] + parameters['diagnosis_baseline_t5'],#-0.01,
        "monitoring.cd4.threshold":100000, 
        "monitoring.fraction.log_viralload": 0.25,#0.3, #0.4, 0.2
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


    # ART_factual = [hiv_testing, art_intro, art_intro1, art_intro2, art_intro2_2, art_intro3, art_intro4, art_intro5,
    #               vmmc_intro1, vmmc_intro2, vmmc_intro3, prep_intro1]

    ART_factual = [hiv_testing, art_intro, art_intro1, art_intro2, art_intro3, art_intro4, art_intro5,
                  condom_intro1, condom_intro2, condom_intro3, vmmc_intro1, vmmc_intro2, vmmc_intro3,
                  prep_intro1]

    # running the simulation --------------------------------------------------
    identifier = str(seedid)
    #rootDir = "/Users/emdominic/Documents/Wimmy/who_hiv_inc_modelling/Calibration/data" 
    rootDir = "Calibration/data" 
 
    destDir = os.path.join(rootDir, identifier)

    results = simpact.run(
        config=cfg_list,
        destDir=destDir,
        #agedist=age_distr,
        interventionConfig=ART_factual,
        seed=seedid,
        identifierFormat=f'seed {identifier}',
        quiet=True
    )

    outputdict = {}

    # Post processing results -------------------------------------------------
    if len(results) == 0:
        outputdict = {
            'growthrate_20': np.nan,
            'prev_10_15_49': np.nan,
            'prev_11_15_49': np.nan,
            'prev_12_15_49': np.nan,
            'prev_13_15_49': np.nan,
            'prev_14_15_49': np.nan,
            'prev_15_15_49': np.nan,
            'prev_16_15_49': np.nan,
            'prev_17_15_49': np.nan,
            'prev_18_15_49': np.nan,
            'prev_19_15_49': np.nan,
            'prev_20_15_49': np.nan,
            'inc_10_15_49': np.nan,
            'inc_11_15_49': np.nan,
            'inc_12_15_49': np.nan,
            'inc_13_15_49': np.nan,
            'inc_14_15_49': np.nan,
            'inc_15_15_49': np.nan,
            'inc_16_15_49': np.nan,
            'inc_17_15_49': np.nan,
            'inc_18_15_49': np.nan,
            'inc_19_15_49': np.nan,
            'inc_20_15_49': np.nan
        }
    else:
        
        datalist_EAAA = psh.readthedata(results)

        ######## Population growth rate
        growthrate_20 = psh.pop_growth_calculator(datalist=datalist_EAAA, timewindow=[10, 20])  # Between 1990 and 2000
        outputdict['growthrate_20'] = round(np.exp(growthrate_20),5)


        ######## HIV prevalence. To be compared to SHIMS I estimates (point estimate at March 2011 ~ t = 31.25)

        # Iterate over age groups and calculate prevalences
        for agegroup in [(15, 50)]:

            # Calculate prevalence for males and females separately

            prev_10_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=10).loc[2, 'pointprevalence']
            prev_11_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=11).loc[2, 'pointprevalence']
            prev_12_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=12).loc[2, 'pointprevalence']
            prev_13_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=13).loc[2, 'pointprevalence']
            prev_14_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=14).loc[2, 'pointprevalence']
            prev_15_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=15).loc[2, 'pointprevalence']
            prev_16_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=16).loc[2, 'pointprevalence']
            prev_17_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=17).loc[2, 'pointprevalence']
            prev_18_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=18).loc[2, 'pointprevalence']
            prev_19_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=19).loc[2, 'pointprevalence']
            prev_20_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=20).loc[2, 'pointprevalence']
            
            prev_m_20_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=20).loc[0, 'pointprevalence']
            prev_f_20_15_49 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timepoint=20).loc[1, 'pointprevalence']


            outputdict['prev_10_15_49'] = prev_10_15_49 if not pd.isnull(prev_10_15_49) else 0
            outputdict['prev_11_15_49'] = prev_11_15_49 if not pd.isnull(prev_11_15_49) else 0
            outputdict['prev_12_15_49'] = prev_12_15_49 if not pd.isnull(prev_12_15_49) else 0
            outputdict['prev_13_15_49'] = prev_13_15_49 if not pd.isnull(prev_13_15_49) else 0
            outputdict['prev_14_15_49'] = prev_14_15_49 if not pd.isnull(prev_14_15_49) else 0
            outputdict['prev_15_15_49'] = prev_15_15_49 if not pd.isnull(prev_15_15_49) else 0
            outputdict['prev_16_15_49'] = prev_16_15_49 if not pd.isnull(prev_16_15_49) else 0
            outputdict['prev_17_15_49'] = prev_17_15_49 if not pd.isnull(prev_17_15_49) else 0
            outputdict['prev_18_15_49'] = prev_18_15_49 if not pd.isnull(prev_18_15_49) else 0
            outputdict['prev_19_15_49'] = prev_19_15_49 if not pd.isnull(prev_19_15_49) else 0
            outputdict['prev_20_15_49'] = prev_20_15_49 if not pd.isnull(prev_20_15_49) else 0
            outputdict['prev_m_20_15_49'] = prev_m_20_15_49 if not pd.isnull(prev_m_20_15_49) else 0
            outputdict['prev_f_20_15_49'] = prev_f_20_15_49 if not pd.isnull(prev_f_20_15_49) else 0


        ######### HIV incidence. Average follow-up period March 2011 until mid Sept 2011 (0.55 years)

        # Iterate over age groups and calculate incidence
        for agegroup in [(15, 50)]:

            # Calculate incidence per 10 per period for males and females separately

            inc_10_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[9,10]).loc[2, 'incidence']
            inc_11_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[10,11]).loc[2, 'incidence']
            inc_12_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[11,12]).loc[2, 'incidence']
            inc_13_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[12,13]).loc[2, 'incidence']
            inc_14_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[13,14]).loc[2, 'incidence']
            inc_15_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[14,15]).loc[2, 'incidence']
            inc_16_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[15,16]).loc[2, 'incidence']
            inc_17_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[16,17]).loc[2, 'incidence']
            inc_18_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[17,18]).loc[2, 'incidence']
            inc_19_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[18,19]).loc[2, 'incidence']
            inc_20_15_49 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=agegroup, timewindow=[19,20]).loc[2, 'incidence']
    

            inc_10_15_49 = inc_10_15_49*10
            inc_11_15_49 = inc_11_15_49*10
            inc_12_15_49 = inc_12_15_49*10
            inc_13_15_49 = inc_13_15_49*10
            inc_14_15_49 = inc_14_15_49*10
            inc_15_15_49 = inc_15_15_49*10
            inc_16_15_49 = inc_16_15_49*10
            inc_17_15_49 = inc_17_15_49*10
            inc_18_15_49 = inc_18_15_49*10
            inc_19_15_49 = inc_19_15_49*10
            inc_20_15_49 = inc_20_15_49*10

            outputdict['inc_10_15_49'] = round(inc_10_15_49,5) if not pd.isnull(inc_10_15_49) else 0
            outputdict['inc_11_15_49'] = round(inc_11_15_49,5) if not pd.isnull(inc_11_15_49) else 0
            outputdict['inc_12_15_49'] = round(inc_12_15_49,5) if not pd.isnull(inc_12_15_49) else 0
            outputdict['inc_13_15_49'] = round(inc_13_15_49,5) if not pd.isnull(inc_13_15_49) else 0
            outputdict['inc_14_15_49'] = round(inc_14_15_49,5) if not pd.isnull(inc_14_15_49) else 0
            outputdict['inc_15_15_49'] = round(inc_15_15_49,5) if not pd.isnull(inc_15_15_49) else 0
            outputdict['inc_16_15_49'] = round(inc_16_15_49,5) if not pd.isnull(inc_16_15_49) else 0
            outputdict['inc_17_15_49'] = round(inc_17_15_49,5) if not pd.isnull(inc_17_15_49) else 0
            outputdict['inc_18_15_49'] = round(inc_18_15_49,5) if not pd.isnull(inc_18_15_49) else 0
            outputdict['inc_19_15_49'] = round(inc_19_15_49,5) if not pd.isnull(inc_19_15_49) else 0
            outputdict['inc_20_15_49'] = round(inc_20_15_49,5) if not pd.isnull(inc_20_15_49) else 0

            
    shutil.rmtree(destDir) #deletes the folder with output files

    return(outputdict)
