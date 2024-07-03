import numpy as np
import scipy.stats as ss
import elfi
import math
from elfi_simpact_calibration_model import simpact_model

# The multiprocessing client allows you to easily use the cores available in your computer. 
elfi.set_client('multiprocessing')

# Observed summary statistics
observation = np.array([[1.4,
                         0.16,0.338, 
                         0.186, 0.408, 0.028, 0.083,
                         0.77, 0.98,
                         0.983,
                         0.48, 0.7
                        ]])

# Define the ELFI model
m = elfi.new_model()
hivtransmission_param_f1 = elfi.Prior(ss.uniform, 2, 4, model=m) 
hivtransmission_param_a = elfi.Prior(ss.uniform, -2, 2, model=m) 
formation_hazard_agegapry_gap_agescale_man_woman = elfi.Prior(ss.uniform, 0, 0.5, model=m) 
person_agegap_man_woman_dist_normal_mu = elfi.Prior(ss.uniform,-3, 3, model=m) 
person_agegap_man_woman_dist_normal_sigma = elfi.Prior(ss.uniform, 1, 5, model=m) 
person_eagerness_man_woman_dist_gamma_a = elfi.Prior(ss.uniform, 0, 1.5, model=m) 
person_eagerness_man_woman_dist_gamma_b = elfi.Prior(ss.uniform, 30, 60, model=m) 
formation_hazard_agegapry_gap_factor_man_woman_exp = elfi.Prior(ss.uniform, -1, 0, model=m) 
formation_hazard_agegapry_baseline = elfi.Prior(ss.uniform, 4.5, 6, model=m)
formation_hazard_agegapry_numrel_man_woman = elfi.Prior(ss.uniform, -1, 1, model=m) 
conception_alpha_base = elfi.Prior(ss.uniform, -2, 1, model=m) 
dissolution_alpha_0 = elfi.Prior(ss.uniform, -3, 1, model=m) 
diagnosis_baseline_t0 = elfi.Prior(ss.uniform, -7, 1, model=m) 
diagnosis_baseline_t1 = elfi.Prior(ss.uniform, 0, 4, model=m) 
diagnosis_baseline_t2 = elfi.Prior(ss.uniform, 0, 4, model=m) 
diagnosis_baseline_t3 = elfi.Prior(ss.uniform, 0, 4, model=m) 
diagnosis_baseline_t4 = elfi.Prior(ss.uniform, -2, 2, model=m) 

sim = elfi.Simulator(simpact_model, 
                    hivtransmission_param_f1,
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
                    diagnosis_baseline_t0,
                    diagnosis_baseline_t1,
                    diagnosis_baseline_t2,
                    diagnosis_baseline_t3,
                    diagnosis_baseline_t4, 
                    observed=observation)

# Define the distance function
d = elfi.Distance('euclidean', sim)

#arraypool = elfi.ArrayPool(['simtime', 'd'])

# Perform SMC ABC
smc = elfi.SMC(d, batch_size=4, seed=42)
schedule = [1, 0.7, 0.5]
result_smc = smc.sample(10, schedule)

print(result_smc)