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

prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 2, 1),
                           hivtransmission_param_a=pyabc.RV("uniform", -2.2, 0.25),
                           formation_hazard_agegapry_gap_agescale_man=pyabc.RV("uniform", 0.2, 0.08),
                           formation_hazard_agegapry_gap_agescale_woman=pyabc.RV("uniform", 0.25, 0.1),
                           person_agegap_man_dist_normal_mu=pyabc.RV("uniform", -2.5, 1.5),
                           person_agegap_man_dist_normal_sigma=pyabc.RV("uniform", 3, 1.5),
                           person_agegap_woman_dist_normal_mu=pyabc.RV("uniform", -1.8, 1.5),
                           person_agegap_woman_dist_normal_sigma=pyabc.RV("uniform", 3, 1.5),
                           person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.7, 0.15),
                           person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.2, 0.15),
                           person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 38, 14),
                           person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 42, 14),
                           formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.5, 1),
                           formation_hazard_agegapry_baseline=pyabc.RV("uniform", 4, 1.5),
                           formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -0.7, 0.5),
                           formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -0.7, 0.6),
                           dissolution_alpha_0=pyabc.RV("uniform", -2.3, 0.3),
                           conception_alpha_base=pyabc.RV("uniform", -3.1, 0.2),
                           conception_alpha_base_1=pyabc.RV("uniform", 0.4, 0.2),
                           #conception_alpha_base_2=pyabc.RV("uniform", 0.15, 0.2),
                           diagnosis_baseline_t0=pyabc.RV("uniform", -1.85, 0.1),
                           diagnosis_baseline_t1=pyabc.RV("uniform", 0.2, 0.1),
                           diagnosis_baseline_t2=pyabc.RV("uniform", 0.1, 0.1),
                           diagnosis_baseline_t2_2=pyabc.RV("uniform", 0.1, 0.1),
                           diagnosis_baseline_t3=pyabc.RV("uniform", 0.3, 0.15),
                           diagnosis_baseline_t4=pyabc.RV("uniform", 0.5, 0.3),
                           diagnosis_baseline_t5=pyabc.RV("uniform", 0.4, 0.15), 
                           diagnosis_eagernessfactor=pyabc.RV("uniform", 1.01, 0.025)
                           )

# prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 2, 1),
#                            hivtransmission_param_a=pyabc.RV("uniform", -2.25, 0.2),
#                            formation_hazard_agegapry_gap_agescale_man=pyabc.RV("uniform", 0.21, 0.07),
#                            formation_hazard_agegapry_gap_agescale_woman=pyabc.RV("uniform", 0.25, 0.1),
#                            person_agegap_man_dist_normal_mu=pyabc.RV("uniform", -2.5, 1.5),
#                            person_agegap_man_dist_normal_sigma=pyabc.RV("uniform", 3, 1.5),
#                            person_agegap_woman_dist_normal_mu=pyabc.RV("uniform", -2, 1.5),
#                            person_agegap_woman_dist_normal_sigma=pyabc.RV("uniform", 3, 1.5),
#                            person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.7, 0.15),
#                            person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.2, 0.15),
#                            person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 40, 15),
#                            person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 45, 10),
#                            formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.5, 1),
#                            formation_hazard_agegapry_baseline=pyabc.RV("uniform", 4, 1),
#                            formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -0.6, 0.55),
#                            formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -0.65, 0.6),
#                            dissolution_alpha_0=pyabc.RV("uniform", -2.3, 0.5),
#                            conception_alpha_base=pyabc.RV("uniform", -3.1, 0.2),
#                            conception_alpha_base_1=pyabc.RV("uniform", 0.45, 0.1),
#                            conception_alpha_base_2=pyabc.RV("uniform", 0.25, 0.1),
#                            diagnosis_baseline_t0=pyabc.RV("uniform", -1.8, 0.1),
#                            diagnosis_baseline_t1=pyabc.RV("uniform", 0.25, 0.1),
#                            diagnosis_baseline_t2=pyabc.RV("uniform", 0.15, 0.1),
#                            diagnosis_baseline_t2_2=pyabc.RV("uniform", 0, 0.1),
#                            diagnosis_baseline_t3=pyabc.RV("uniform", 0.2, 0.1),
#                            diagnosis_baseline_t4=pyabc.RV("uniform", 0.2, 0.1),
#                            diagnosis_baseline_t5=pyabc.RV("uniform", 0.5, 0.2), 
#                            diagnosis_eagernessfactor=pyabc.RV("uniform", 1.012, 0.025)
#                            )

# Adaptive distance
scale_log_file = tempfile.mkstemp(suffix=".json")[1]

distance_adaptive = pyabc.AdaptivePNormDistance(
    p=2,#1 #manhattan
    scale_function=pyabc.distance.mean, #pyabc.distance.mean, #pyabc.distance.pcmad # method by which to scale
    scale_log_file=scale_log_file,
)

# SMAPE
# def smape(actual_dict, predicted_dict):
#     """
#     Calculate SMAPE (Symmetric Mean Absolute Percentage Error) for named dictionaries
#     of actual and predicted values.
        
#     Returns:
#         float: Average SMAPE value across all keys.
#     """
#     import numpy as np

#     # Ensure both dictionaries have the same keys
#     keys = actual_dict.keys()

#     # Initialize list to store SMAPE values for each key
#     smape_values = []
    
#     for key in keys:
#         actual = actual_dict[key]
#         predicted = predicted_dict[key]
        
#         # Calculate absolute difference and sum of absolute values
#         absolute_diff = np.abs(actual - predicted)
#         sum_absolute = np.abs(actual) + np.abs(predicted)
        
#         # Avoid division by zero
#         mask = sum_absolute != 0
        
#         # Handle case where all elements are zero to avoid division by zero
#         if np.all(~mask):
#             smape_values.append(float('inf'))
#         else:
#             smape = np.mean(100 * (absolute_diff[mask] / sum_absolute[mask]))
#             smape_values.append(smape)
    
#     # Calculate the average SMAPE across all keys
#     average_smape = np.mean(smape_values) if smape_values else float('nan')
    
#     return average_smape

# create ABC instance
abc = pyabc.ABCSMC(models=calibration_wrapper_function, 
                   parameter_priors=prior, 
                   distance_function=distance_adaptive,#smape, weighted_distance,pyabc.PNormDistance(p=2),  
                   sampler=MulticoreEvalParallelSampler(n_procs=80),
                   population_size=30) 


# target_stats = {
#      'growthrate_m_20':1.917557313,
#      'growthrate_m_30':2.042989392,
#      'growthrate_m_37':2.184401446,
#      'growthrate_f_20':1.85882833,
#      'growthrate_f_30':2.074018657,
#      'growthrate_f_37':2.230833113,
#      'prev_10_15_49': 0.013,
#      'prev_15_15_49': 0.15,
#      'prev_f_20_15_49': 0.26,
#      'prev_m_20_15_49': 0.22,
#      'prev_f_32_15_49': 0.36, 
#      'prev_m_32_15_49': 0.223, 
#      'prev_f_36_15_49': 0.36, 
#      'prev_m_36_15_49': 0.229, 
#      'prev_f_41_15_49': 0.32, 
#      'prev_m_41_15_49': 0.207, 
#      'prev_f_43_15_49': 0.303, #unaids
#      'prev_m_43_15_49': 0.199, #unaids
#      'inc_10_15_49': 0.0783,
#      'inc_12_15_49': 0.2555,
#      'inc_15_15_49': 0.5225,
#      'inc_17_15_49': 0.478,
#      'inc_20_15_49': 0.3913,
#     'inc_f_32_15_49': 0.386,#my estimate based on unaids, to replace shims 0.314, #shims1
#     'inc_m_32_15_49': 0.203,#my estimate based on unaids, to replace shims 0.165, #shims1
#     'inc_f_36_15_49': 0.244,#my estimate based on unaids, to replace shims 0.199, #shims2
#     'inc_m_36_15_49': 0.122,#my estimate based on unaids, to replace shims 0.099, #shims2
#     'inc_f_41_15_49': 0.1664,#my estimate based on unaids, to replace shims 0.145, #shims3
#     'inc_m_41_15_49': 0.0256,#my estimate based on unaids, to replace shims 0.02, #shims3
#     'inc_f_43_15_49': 0.1333,#unaids
#     'inc_m_43_15_49': 0.0205,#unaids
#     'inc_f_20_15_24': 0.538, #my estimate based on unaids
#     'inc_f_32_15_24': 0.3014, #my estimate based on unaids
#     'inc_f_36_15_24': 0.2481, #my estimate based on unaids, to replace #shims 0.187
#     'inc_f_41_15_24': 0.1814, #my estimate based on unaids, to replace #shims 0.163
#     'inc_m_20_15_24': 0.28, #my estimate based on unaids
#     'inc_m_32_15_24': 0.1586, #my estimate based on unaids
#     'inc_m_36_15_24': 0.0919, #my estimate based on unaids, to replace #shims 
#     'inc_m_41_15_24': 0.0189, #my estimate based on unaids, to replace #shims
#     'art_cov_t_25': 0.06,
#     'art_cov_t_30': 0.3,
#     'art_cov_t_35': 0.64,
#     'art_cov_t_38': 0.75,
#     'art_cov_t_43': 0.93,
#     'prop_diag_25':0.45,
#     'prop_diag_30':0.7,
#     'prop_diag_36':0.86, #shims2
#     'prop_diag_41':0.93, #shims3
#     #'prop_diag_43':0.99
# }

# def scale_targets_to_one(target_stats):
#     """
#     Scales all target values to 1 and calculates the scaling factors.
    
#     Parameters:
#     - target_stats (dict): A dictionary of target values.
    
#     Returns:
#     - scaled_targets (dict): A dictionary of scaled targets (all will be 1).
#     - scaling_factors (dict): A dictionary of scaling factors used for scaling.
#     """
#     scaling_factors = {}
#     scaled_targets = {}
    
#     # Calculate scaling factors and scale targets
#     for key, value in target_stats.items():
#         scaling_factors[key] = 1 / value  # Compute the scaling factor
#         scaled_targets[key] = value * scaling_factors[key]  # Scale target to 1
    
#     return scaled_targets, scaling_factors

scaled_targets = {'growthrate_m_20': 0.9999999999999999,
     'growthrate_m_30': 1.0,
     'growthrate_m_37': 1.0,
     'growthrate_f_20': 1.0,
     'growthrate_f_30': 1.0,
     'growthrate_f_37': 1.0,
     'prev_10_15_49': 0.9999999999999999,
     'prev_15_15_49': 1.0,
     'prev_f_20_15_49': 1.0,
     'prev_m_20_15_49': 1.0,
     'prev_f_32_15_49': 0.9999999999999999,
     'prev_m_32_15_49': 0.9999999999999999,
     'prev_f_36_15_49': 0.9999999999999999,
     'prev_m_36_15_49': 0.9999999999999999,
     'prev_f_41_15_49': 1.0,
     'prev_m_41_15_49': 1.0,
     'prev_f_43_15_49': 1.0,
     'prev_m_43_15_49': 1.0,
     'inc_10_15_49': 0.9999999999999999,
     'inc_12_15_49': 1.0,
     'inc_15_15_49': 1.0,
     'inc_17_15_49': 0.9999999999999999,
     'inc_20_15_49': 1.0,
     'inc_f_32_15_49': 1.0,
     'inc_m_32_15_49': 0.9999999999999999,
     'inc_f_36_15_49': 0.9999999999999999,
     'inc_m_36_15_49': 0.9999999999999999,
     'inc_f_41_15_49': 1.0,
     'inc_m_41_15_49': 1.0,
     'inc_f_43_15_49': 1.0,
     'inc_m_43_15_49': 1.0,
     'inc_f_20_15_24': 1.0,
     'inc_f_32_15_24': 1.0,
     'inc_f_36_15_24': 1.0,
     'inc_f_41_15_24': 1.0,
     'inc_m_20_15_24': 1.0,
     'inc_m_32_15_24': 1.0,
     'inc_m_36_15_24': 1.0,
     'inc_m_41_15_24': 1.0,
     'art_cov_t_25': 1.0,
     'art_cov_t_30': 1.0,
     'art_cov_t_35': 1.0,
     'art_cov_t_38': 1.0,
     'art_cov_t_43': 1.0,
     'prop_diag_25': 1.0,
     'prop_diag_30': 1.0,
     'prop_diag_36': 1.0,
     'prop_diag_41': 1.0}


# path to save output
db_path = os.path.join("Calibration/data", "simpact_calibration_output.db")

if os.path.exists(db_path):
    # Remove the file
    os.remove(db_path)

abc.new("sqlite:///" + db_path, scaled_targets)

history = abc.run(minimum_epsilon=0.05,max_nr_populations= 15) 

posterior_params, weights = history.get_distribution()

posterior_params.reset_index(drop=False).to_csv('Calibration/data/posterior_distributions.csv', index=None)