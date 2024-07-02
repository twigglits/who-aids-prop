def simpact_model(hivtransmission_param_f1,
                  hivtransmission_param_a,
                  formation_hazard_agegapry_gap_agescale_man_woman,
                  person_agegap_man_woman_dist_normal_mu,
                  person_agegap_man_woman_dist_normal_sigma,
                  person_eagerness_man_woman_dist_gamma_a,
                  person_eagerness_man_woman_dist_gamma_b,
                  formation_hazard_agegapry_gap_factor_man_woman_exp,
                  formation_hazard_agegapry_baseline,
                  formation_hazard_agegapry_numrel_man_woman,
                  conception_alpha_base,dissolution_alpha_0,
                  diagnosis_baseline_t0,diagnosis_baseline_t1,
                  diagnosis_baseline_t2,diagnosis_baseline_t3,
                  diagnosis_baseline_t4, 
                  batch_size=1, 
                  random_state=None):
    import pysimpactcyan
    simpact = pysimpactcyan.PySimpactCyan()
    import psimapacthelper as psh
    import shutil
    import numpy as np
    import math
    import os
    import pandas as pd

    outputs = []

    for i in range(batch_size):
        cfg = {
        "population.agedistfile": "/home/jupyter/who-aids-prop/build/python/eswatini_2023.csv",
        "population.eyecap.fraction": 0.2,
        "population.simtime": 66,  # start 1985 and end in 2051
        "population.nummen": 1000,
        "population.numwomen": 1000,
        "population.msm": "no",
        "hivseed.time": 10,
        "hivseed.type": "fraction",
        #"hivseed.amount": 100,  # 30,
        "hivseed.fraction" : 0.1,
        "hivseed.age.min": 20,
        "hivseed.age.max": 50,
        "hivtransmission.param.a": -1,
        "hivtransmission.param.b": -90,
        "hivtransmission.param.c": 0.5,
        "hivtransmission.param.f1": math.log(2),
        "hivtransmission.param.f2": math.log(math.log(math.sqrt(2)) / math.log(2)) / 5,
        "person.eagerness.man.dist.type":"gamma",
        "person.eagerness.woman.dist.type":"gamma",
        "formation.hazard.type":"agegapry",
        "formation.hazard.agegapry.baseline":3.8,
        "formation.hazard.agegapry.maxageref.diff":1,
        "formation.hazard.agegapry.gap_factor_man_age": -0.01,
        "formation.hazard.agegapry.gap_factor_woman_age": -0.01,
        "formation.hazard.agegapry.gap_factor_man_const": 0,
        "formation.hazard.agegapry.gap_factor_woman_const": 0,
        "formation.hazard.agegapry.gap_factor_man_exp": -1,
        "formation.hazard.agegapry.gap_factor_woman_exp": -1,
        "formation.hazard.agegapry.gap_agescale_man": 0.25,
        "formation.hazard.agegapry.gap_agescale_woman": 0.25,
        "formation.hazard.agegapry.meanage": -0.025, #younger people more likely to form relationship. Hazard decreases with the average age
        "dissolution.alpha_0": 0.1,
        "dissolution.alpha_4": -0.05,
        "debut.debutage": 15,
        "conception.alpha_base": -2.7,
        "dropout.interval.dist.type": "exponential",
        "syncrefyear.interval": 1,
        # vmmc
        "EventVMMC.enabled": "true",
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
        "EventCondom.threshold": 0.5, # threshold for condom preference for an individual 
        "hivtransmission.threshold": 0.5, # threshold for condom use in formation
        "hivtransmission.m_condomformationdist.dist.type":"discrete.csv.twocol",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file":  "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_1.csv",
        "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.floor": 'yes', #to force distirbution to return exact values and not any value within the bin
        "hivtransmission.threshold": 0.5, # threshold for condom use in formation
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
        cfg["formation.hazard.agegapry.baseline"] = 2
        cfg["mortality.aids.survtime.C"] = 65
        cfg["mortality.aids.survtime.k"] = -0.2
        cfg["monitoring.fraction.log_viralload"] = 0.3

        cfg["person.survtime.logoffset.dist.type"] = "normal"
        cfg["person.survtime.logoffset.dist.normal.mu"] = 0
        cfg["person.survtime.logoffset.dist.normal.sigma"] = 0.1

        cfg["person.agegap.man.dist.type"] = "normal"
        cfg["person.agegap.woman.dist.type"] = "normal"

        cfg["monitoring.cd4.threshold"] = 1
        cfg["person.art.accept.threshold.dist.fixed.value"] = 0.65
        cfg["diagnosis.baseline"] = -99999
        cfg["periodiclogging.interval"] = 0.25
        cfg["dropout.interval.dist.exponential.lambda"] = 0.1

        # Assuming cfg["population.simtime"] and cfg["population.nummen"] are defined elsewhere
        cfg["population.maxevents"] = float(cfg["population.simtime"]) * float(cfg["population.nummen"]) * 6

        cfg["person.vsp.toacute.x"] = 5  # See Bellan PLoS Medicine

        # seedid = random.randint(0,1000000000)
        seed_generator = psh.UniqueSeedGenerator()
        seedid = seed_generator.generate_seed()
        
            #TODO: write a fuction such that if you don't pass a calibration parameter value, it uses the default eg the values above
        # calibration parameters
    
        cfg["hivtransmission.param.f1"] = round(hivtransmission_param_f1[i],4)
        cfg["hivtransmission.param.f2"] = round(math.log(math.log(math.sqrt(hivtransmission_param_f1[i])) / math.log(hivtransmission_param_f1[i])) / 5,4)
        cfg['hivtransmission.param.a'] = round(hivtransmission_param_a[i],4)
        cfg["formation.hazard.agegapry.gap_agescale_man"] = round(formation_hazard_agegapry_gap_agescale_man_woman[i],4)
        cfg["formation.hazard.agegapry.gap_agescale_woman"] = round(formation_hazard_agegapry_gap_agescale_man_woman[i],4)
        cfg["person.agegap.man.dist.normal.mu"] = round(person_agegap_man_woman_dist_normal_mu[i],4)
        cfg["person.agegap.woman.dist.normal.mu"] = round(person_agegap_man_woman_dist_normal_mu[i],4)
        cfg["person.agegap.man.dist.normal.sigma"] = round(person_agegap_man_woman_dist_normal_sigma[i],4)
        cfg["person.agegap.woman.dist.normal.sigma"] = round(person_agegap_man_woman_dist_normal_sigma[i],4)
        cfg["person.eagerness.man.dist.gamma.a"] = round(person_eagerness_man_woman_dist_gamma_a[i],4)
        cfg["person.eagerness.woman.dist.gamma.a"] = round(person_eagerness_man_woman_dist_gamma_a[i],4)
        cfg["person.eagerness.man.dist.gamma.b"] = round(person_eagerness_man_woman_dist_gamma_b[i],4)
        cfg["person.eagerness.woman.dist.gamma.b"] = round(person_eagerness_man_woman_dist_gamma_b[i],4)
        cfg["formation.hazard.agegapry.gap_factor_man_exp"] = round(formation_hazard_agegapry_gap_factor_man_woman_exp[i],4)
        cfg["formation.hazard.agegapry.gap_factor_woman_exp"] = round(formation_hazard_agegapry_gap_factor_man_woman_exp[i],4)
        cfg["formation.hazard.agegapry.baseline"] = round(formation_hazard_agegapry_baseline[i],4)
        cfg["formation.hazard.agegapry.numrel_man"] = round(formation_hazard_agegapry_numrel_man_woman[i],4)
        cfg["formation.hazard.agegapry.numrel_woman"] = round(formation_hazard_agegapry_numrel_man_woman[i],4)
        cfg["conception.alpha_base"] = round(conception_alpha_base[i],4)
        cfg["dissolution.alpha_0"] = round(dissolution_alpha_0[i],4)

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

        # Intervention introduction configurations
        
        #condom use
        condom_intro1 = { 
            "time": 15, #around 2000
            "EventCondom.enabled": "true" }
        
        art_intro = {
            "time": 20, #around 2005
            "diagnosis.baseline": diagnosis_baseline_t0[i],#-2,
            "monitoring.cd4.threshold": 100,
            "formation.hazard.agegapry.baseline": cfg["formation.hazard.agegapry.baseline"] - 0.5
        }

        art_intro1 = {
            "time": 22,
            "diagnosis.baseline": diagnosis_baseline_t0[i] + diagnosis_baseline_t1[i],#-1.8,
            "monitoring.cd4.threshold": 150
        }

        art_intro2 = {
            "time": 23,
            "diagnosis.baseline": diagnosis_baseline_t0[i] + diagnosis_baseline_t1[i] + diagnosis_baseline_t2[i],#-1.5,
            "monitoring.cd4.threshold": 200,
            "formation.hazard.agegapry.baseline": cfg["formation.hazard.agegapry.baseline"] - 1
        }
        
        #vmmc

        vmmc_intro1 = {
            "time":24, #around 2009
            "EventVMMC.m_vmmcscheduleDist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/vmmc_schedule_twocol_1.csv",
        }
        
        condom_intro2 = { 
            "time": 25, 
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_2.csv" }

        art_intro3 = {
            "time": 30,
            "diagnosis.baseline": diagnosis_baseline_t0[i] + diagnosis_baseline_t1[i] + diagnosis_baseline_t2[i] + diagnosis_baseline_t3[i],#-1,
            "monitoring.cd4.threshold": 350
        }
        
        condom_intro3 = { 
            "time": 31, 
            "hivtransmission.m_condomformationdist.dist.discrete.csv.twocol.file": "/home/jupyter/who-aids-prop/build/python/relationship_condom_use_3.csv" }
        
        prep_intro1 = {
            "time":32, #around 2017
            "EventPrep.enabled": "true"
        }

        art_intro4 = {
            "time": 33.5,
            "diagnosis.baseline": diagnosis_baseline_t0[i] + diagnosis_baseline_t1[i] + diagnosis_baseline_t2[i] + diagnosis_baseline_t3[i] + diagnosis_baseline_t4[i],#-0.5,
            "monitoring.cd4.threshold": 500
        }

        art_intro5 = {
            "time": 36.75,
            "monitoring.cd4.threshold": 6000
        }


        eswatini_factual = [condom_intro1, art_intro, art_intro1, art_intro2,vmmc_intro1,
                            condom_intro2, art_intro3, condom_intro3, prep_intro1, art_intro4, 
                            art_intro5]

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
        quiet=True
        )


        # Post processing results -------------------------------------------------
        if len(results) == 0:
            growthrate_35 = 0
            prev_m_39 = 0
            prev_f_39 =0
            inc_m_25 = 0
            inc_f_25 = 0
            inc_m_39 = 0
            inc_f_39 = 0
            art_30 = 0 
            art_38 = 0
            VL_suppression_fraction = 0
            vmmc_15_49_fraction = 0 
            vmmc_15_24_fraction = 0
        else:
            if float(results["eventsexecuted"]) >= (float(cfg["population.maxevents"]) - 1):
                growthrate_35 = 0
                prev_m_39 = 0
                prev_f_39 =0
                inc_m_25 = 0
                inc_f_25 = 0
                inc_m_39 = 0
                inc_f_39 = 0
                art_30 = 0 
                art_38 = 0
                VL_suppression_fraction = 0
                vmmc_15_49_fraction = 0 
                vmmc_15_24_fraction = 0
            else:
                datalist_EAAA = psh.readthedata(results)

                ######## Population growth rate
                growthrate_35 = round(math.exp(psh.pop_growth_calculator(datalist=datalist_EAAA, timewindow=[34, 35])),5)  # Between 2019 and 2020
                
                ######### HIV prevalence
                
                # Calculate prevalence for males and females separately

                prev_m_39 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timepoint=38.5).loc[1, 'pointprevalence']
                prev_f_39 = psh.prevalence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timepoint=38.5).loc[2, 'pointprevalence']
                
                prev_m_39 = prev_m_39 if not pd.isnull(prev_m_39) else 0
                prev_f_39 = prev_f_39 if not pd.isnull(prev_f_39) else 0

                ######### HIV incidence. Average follow-up period March 2011 until mid Sept 2011 (0.55 years)
            
                # for agegroup in [(15, 20), (20, 25), (25, 30),
                #                     (30, 35), (35, 40), (40, 45), 
                #                     (45, 50), (45, 50)]:

                # Calculate incidence per 10 per period for males and females separately

                inc_m_25 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timewindow=[25,26]).loc[1, 'incidence']
                inc_f_25 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timewindow=[25,26]).loc[2, 'incidence']

                inc_m_39 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timewindow=[38,39]).loc[1, 'incidence']
                inc_f_39 = psh.incidence_calculator(datalist=datalist_EAAA, agegroup=[15,50], timewindow=[38,39]).loc[2, 'incidence']

                inc_m_25 = round(inc_f_25*10,5) if not pd.isnull(inc_f_25) else 0
                inc_f_25 = round(inc_m_25*10,5) if not pd.isnull(inc_m_25) else 0
                inc_m_39 = round(inc_f_39*10,5) if not pd.isnull(inc_f_39) else 0
                inc_f_39 = round(inc_m_39*10,5) if not pd.isnull(inc_m_39) else 0
                    
                ######### ART coverage among adults 15+ years old from spectrum data (2005 - 2023 estimates)

                art1 = psh.ART_coverage_calculator(datalist=datalist_EAAA,agegroup=[15, 150],timepoint=30.5)
                art2 = psh.ART_coverage_calculator(datalist=datalist_EAAA,agegroup=[15, 150],timepoint=38.5)
                try:
                    art_30 = float(art1['ART_coverage'][2])
                except (IndexError, ValueError, KeyError) as e:
                    art_30 = 0
                try:
                    art_38 = float(art2['ART_coverage'][2])
                except (IndexError, ValueError, KeyError) as e:
                    art_38 = 0


                ########## VL suppression fraction (all ages in 2017 ~ >= 15 yo) 0.74
                vl = psh.VL_suppression_calculator(datalist=datalist_EAAA,
                                                    agegroup=[15, 300], 
                                                    timepoint=35.5, 
                                                    vl_cutoff=1000, 
                                                    site="All")
                try:
                    VL_suppression_fraction = round(float(vl['vl_suppr_frac'][2]),5)
                except (IndexError, ValueError, KeyError) as e:
                    VL_suppression_fraction = 0
                    
                ########### VMMC target
                # 15-49 in 2023 48%,
                vmmc_1 = psh.vmmc_calculator(datalist=datalist_EAAA, agegroup=[15, 50], timepoint=38.5)

                try:
                    vmmc_15_49_fraction = float(vmmc_1['vmmcprevalence'].iloc[0])
                except (IndexError, ValueError, KeyError) as e:
                    vmmc_15_49_fraction = 0


                # 15-24 ~ 70%
                vmmc_2 = psh.vmmc_calculator(datalist=datalist_EAAA, agegroup=[15, 24], timepoint=38.5)

                try:
                    vmmc_15_24_fraction = float(vmmc_2['vmmcprevalence'].iloc[0])
                except (IndexError, ValueError, KeyError) as e:
                    vmmc_15_24_fraction = 0
                
        outputs.append([growthrate_35,
                        prev_m_39, prev_f_39, 
                        inc_m_25,inc_f_25, inc_m_39,inc_f_39,
                        art_30,art_38,
                        VL_suppression_fraction,
                        vmmc_15_49_fraction, vmmc_15_24_fraction])
        
        shutil.rmtree(destDir)

    outputs_array = np.array(outputs)
    return outputs_array