from calibration_wrapper_function_revised_new import calibration_wrapper_function
import pyabc
from pyabc.sampler import MulticoreEvalParallelSampler, SingleCoreSampler
import os
import numpy as np
import tempfile
import pyabc.inference_util.inference_util as inference_util

        
# Define our custom create_weight_function
def custom_create_weight_function(prior_pdf, transition_pdf):
    def custom_weight_function(m_ss, theta_ss, acceptance_weight: float):
        """Calculate total weight, from sampling and acceptance weight."""
        # prior and transition density (can be equal)
        prior_pd = prior_pdf(m_ss, theta_ss)
        transition_pd = transition_pdf(m_ss, theta_ss)
        # Add small epsilon to avoid division by zero
        epsilon = 1e-10
        weight = acceptance_weight * prior_pd / (transition_pd + epsilon)
        return weight
    return custom_weight_function

# Check if we can identify the original function
original_create_weight_function = getattr(inference_util, 'create_weight_function', None)

if original_create_weight_function:
    print("Original create_weight_function found, applying monkey patch.")
    # Monkey-patch the original function with our custom function
    inference_util.create_weight_function = custom_create_weight_function
else:
    print("Original create_weight_function not found, please check the function name or module.")

# the parameters need to be named and not just passed as a vector # loc is lower bound, scale = upper - lower

# prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 1.5, 2),
#                            hivtransmission_param_a=pyabc.RV("uniform", -2.5, 1),
#                            formation_hazard_agegapry_gap_agescale_man_woman=pyabc.RV("uniform", 0.30, 0.12),
#                            person_agegap_man_woman_dist_normal_mu=pyabc.RV("uniform", 0, 3),
#                            person_agegap_man_woman_dist_normal_sigma=pyabc.RV("uniform", 2, 4),
#                            person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.1, 1.4),
#                            person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.1, 1.4),
#                            person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 50, 15),
#                            person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 40, 15),
#                            formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.5, 1),
#                            formation_hazard_agegapry_baseline=pyabc.RV("uniform", 4, 2),
#                            formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -1, 1),
#                            formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -1, 1),
#                            dissolution_alpha_0=pyabc.RV("uniform", -2, 1),
#                            conception_alpha_base=pyabc.RV("uniform", -3, 0.4),
#                            conception_alpha_base_1=pyabc.RV("uniform", 0, 0.5),
#                            conception_alpha_base_2=pyabc.RV("uniform", 0, 0.5),
#                            diagnosis_baseline_t0=pyabc.RV("uniform", -2, 1.5),
#                            diagnosis_baseline_t1=pyabc.RV("uniform", 0.25, 1),
#                            diagnosis_baseline_t2=pyabc.RV("uniform", 0.2, 1),
#                            diagnosis_baseline_t2_2=pyabc.RV("uniform", 0.1, 1),
#                            diagnosis_baseline_t3=pyabc.RV("uniform", 0.1, 1),
#                            diagnosis_baseline_t4=pyabc.RV("uniform", 0.2, 1),
#                            diagnosis_baseline_t5=pyabc.RV("uniform", 0.2, 1), 
#                            diagnosis_eagernessfactor=pyabc.RV("uniform", 1.025, 0.03)
#                            )

prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 1.5, 1.5),
                           hivtransmission_param_a=pyabc.RV("uniform", -2.4, 0.6),
                           formation_hazard_agegapry_gap_agescale_man_woman=pyabc.RV("uniform", 0.32, 0.06),
                           person_agegap_man_woman_dist_normal_mu=pyabc.RV("uniform", 2, 1),
                           person_agegap_man_woman_dist_normal_sigma=pyabc.RV("uniform", 4, 1),
                           person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.75, 0.1),
                           person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.3, 0.1),
                           person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 50, 15),
                           person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 40, 15),
                           formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.2, 0.3),
                           formation_hazard_agegapry_baseline=pyabc.RV("uniform", 4, 1),
                           formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -0.8, 0.4),
                           formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -0.5, 0.4),
                           dissolution_alpha_0=pyabc.RV("uniform", -2.5, 0.6),
                           conception_alpha_base=pyabc.RV("uniform", -3.1, 0.5),
                           conception_alpha_base_1=pyabc.RV("uniform", 0.1, 0.4),
                           conception_alpha_base_2=pyabc.RV("uniform", 0.1, 0.4),
                           diagnosis_baseline_t0=pyabc.RV("uniform", -1.8, 0.3),
                           diagnosis_baseline_t1=pyabc.RV("uniform", 0.2, 0.2),
                           diagnosis_baseline_t2=pyabc.RV("uniform", 0.15, 0.2),
                           diagnosis_baseline_t2_2=pyabc.RV("uniform", 0, 0.1),
                           diagnosis_baseline_t3=pyabc.RV("uniform", 0.2, 0.2),
                           diagnosis_baseline_t4=pyabc.RV("uniform", 0.15, 0.2),
                           diagnosis_baseline_t5=pyabc.RV("uniform", 0.4, 0.4), 
                           diagnosis_eagernessfactor=pyabc.RV("uniform", 1.01, 0.03)
                           )

# Adaptive distance
scale_log_file = tempfile.mkstemp(suffix=".json")[1]

distance_adaptive = pyabc.AdaptivePNormDistance(
    p=2,
    scale_function=pyabc.distance.mean,  # method by which to scale
    scale_log_file=scale_log_file,
)

# weighted distances
def weighted_distance(x, x_0):
    
    weights = {
                'popsize_20':2,
                'popsize_30':2,
                'popsize_40':2,
                 'prev_10_15_49': 2,
                 'prev_15_15_49':2,
                 'prev_f_20_15_49': 2,
                 'prev_m_20_15_49': 2,
                'prev_f_32_15_49': 5,
                'prev_m_32_15_49': 5,
                 'prev_f_36_15_49': 5, #shims2
                 'prev_m_36_15_49': 5, #shims2
                'prev_f_41_15_49': 5,
                'prev_m_41_15_49': 5,
                'inc_10_15_49': 2,
                'inc_12_15_49': 5,
                'inc_15_15_49': 5,
                'inc_17_15_49': 5,
                'inc_20_15_49': 5,
                'inc_f_32_15_49': 5,
                'inc_m_32_15_49': 5,
                'inc_f_36_15_49': 5,
                'inc_m_36_15_49': 5,
                'inc_f_41_15_49': 10,
                'inc_m_41_15_49': 10,
                'inc_f_43_15_49': 10,
                'inc_m_43_15_49': 10,
                'inc_f_20_15_24': 10,
                'inc_f_32_15_24': 5,
                'inc_f_36_15_24': 5,
                'inc_f_41_15_24': 5,
                'inc_m_20_15_24': 5,
                'inc_m_32_15_24': 5,
                'inc_m_36_15_24': 2,
                'inc_m_41_15_24': 2,
                'art_cov_t_30.5': 2,
                'art_cov_t_35.5': 2,
                'art_cov_t_38.5': 2,
                'art_cov_t_43.5': 2,
                'prop_diag_25':2,
                'prop_diag_30':2,
                'prop_diag_36':2,
                'prop_diag_41':2,
                'prop_diag_43':2
                }
        
    distance = 0
    for key in x.keys():
        distance += weights[key] * (x[key] - x_0[key]) ** 2
    
    return np.sqrt(distance)

# create ABC instance
abc = pyabc.ABCSMC(models=calibration_wrapper_function, 
                   parameter_priors=prior, 
                   distance_function= distance_adaptive, #weighted_distance,pyabc.PNormDistance(p=2),  
                   sampler=MulticoreEvalParallelSampler(n_procs=95),
                   population_size=30) 


target_stats = {
     #'growthrate_40': 1.007,
     'popsize_20':1.73,
     'popsize_30':1.84,
     'popsize_40':1.98,
     'prev_10_15_49': 0.013,
     'prev_15_15_49': 0.15,
     # 'prev_20_15_49': 0.246,
     'prev_f_20_15_49': 0.26,
     'prev_m_20_15_49': 0.22,
     'prev_f_32_15_49': 0.36, 
     'prev_m_32_15_49': 0.223, 
     'prev_f_36_15_49': 0.36, 
     'prev_m_36_15_49': 0.229, 
     'prev_f_41_15_49': 0.32, 
     'prev_m_41_15_49': 0.207, 
     # 'prev_f_43_15_49': 0.303, #unaids
     # 'prev_m_43_15_49': 0.199, #unaids
     'inc_10_15_49': 0.0783,
     # 'inc_11_15_49': 0.1515,
     'inc_12_15_49': 0.2555,
     # 'inc_13_15_49': 0.3743,
     # 'inc_14_15_49': 0.4733,
     'inc_15_15_49': 0.5225,
     # 'inc_16_15_49': 0.5016,
     'inc_17_15_49': 0.478,
     # 'inc_18_15_49': 0.437,
     # 'inc_19_15_49': 0.4174,
     'inc_20_15_49': 0.3913,
    'inc_f_32_15_49': 0.386,#my estimate based on unaids, to replace shims 0.314, #shims1
    'inc_m_32_15_49': 0.203,#my estimate based on unaids, to replace shims 0.165, #shims1
    'inc_f_36_15_49': 0.244,#my estimate based on unaids, to replace shims 0.199, #shims2
    'inc_m_36_15_49': 0.122,#my estimate based on unaids, to replace shims 0.099, #shims2
    'inc_f_41_15_49': 0.1664,#my estimate based on unaids, to replace shims 0.145, #shims3
    'inc_m_41_15_49': 0.0256,#my estimate based on unaids, to replace shims 0.02, #shims3
    'inc_f_43_15_49': 0.1333,#unaids
    'inc_m_43_15_49': 0.0205,#unaids
    'inc_20_15_24': 0.41,   #unaids
    'inc_f_32_15_24': 0.3014, #my estimate based on unaids
    'inc_f_36_15_24': 0.2481, #my estimate based on unaids, to replace #shims 0.187
    'inc_f_41_15_24': 0.1814, #my estimate based on unaids, to replace #shims 0.163
    'inc_m_32_15_24': 0.1586, #my estimate based on unaids
    'inc_m_36_15_24': 0.0919, #my estimate based on unaids, to replace #shims 
    'inc_m_41_15_24': 0.0189, #my estimate based on unaids, to replace #shims 
    'art_cov_t_30.5': 0.5,
    'art_cov_t_35.5': 0.8,
    'art_cov_t_38.5': 0.87,
    'art_cov_t_43.5': 0.95,
    'prop_diag_25':0.45,
    'prop_diag_30':0.7,
    'prop_diag_36':0.86, #shims2
    'prop_diag_41':0.93, #shims3
    'prop_diag_43':0.99
}

# path to save output
db_path = os.path.join("Calibration/data", "simpact_calibration_output.db")

if os.path.exists(db_path):
    # Remove the file
    os.remove(db_path)

abc.new("sqlite:///" + db_path, target_stats)

history = abc.run(minimum_epsilon=0.1,max_nr_populations= 20) 

posterior_params, weights = history.get_distribution()

posterior_params.reset_index(drop=False).to_csv('Calibration/data/posterior_distributions.csv', index=None)