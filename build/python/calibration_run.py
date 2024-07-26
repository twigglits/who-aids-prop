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

prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 1, 4),
                           hivtransmission_param_a=pyabc.RV("uniform", -3, 2),
                           formation_hazard_agegapry_gap_agescale_man_woman=pyabc.RV("uniform", 0.25, 0.2),
                           person_agegap_man_woman_dist_normal_mu=pyabc.RV("uniform", 0, 3),
                           person_agegap_man_woman_dist_normal_sigma=pyabc.RV("uniform", 2, 4),
                           person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.1, 1.4),
                           person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.1, 1.4),
                           person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 50, 20),
                           person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 40, 20),
                           formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.5, 0.7),
                           formation_hazard_agegapry_baseline=pyabc.RV("uniform", 3, 3),
                           formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -1, 1),
                           formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -1, 1),
                           dissolution_alpha_0=pyabc.RV("uniform", -3, 2),
                           conception_alpha_base=pyabc.RV("uniform", -3.3, 0.5),
                           diagnosis_baseline_t0=pyabc.RV("uniform", -3, 5),
                           diagnosis_baseline_t1=pyabc.RV("uniform", -2, 5),
                           diagnosis_baseline_t2=pyabc.RV("uniform", -2, 5),
                           diagnosis_baseline_t2_2=pyabc.RV("uniform", -2, 5),
                           diagnosis_baseline_t3=pyabc.RV("uniform", -2, 5),
                           diagnosis_baseline_t4=pyabc.RV("uniform", -2, 5),
                           diagnosis_baseline_t5=pyabc.RV("uniform", -2, 5), 
                           diagnosis_eagernessfactor=pyabc.RV("uniform", 1, 0.05),
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
    'growthrate_43': 5,
    'prev_f_20_15_49': 5,
    'prev_m_20_15_49': 2,
    'prev_f_25_15_49': 5,
    'prev_m_25_15_49': 2,
    'prev_f_36_15_49': 5,
    'prev_m_36_15_49': 2,
    'prev_f_41_15_49': 5,
    'prev_m_41_15_49': 2,
    'inc_f_32_15_49': 5,
    'inc_m_32_15_49': 2,
    'inc_f_36_15_49': 5,
    'inc_m_36_15_49': 2,
    'inc_f_41_15_49': 5,
    'inc_m_41_15_49': 2,
    'art_cov_t_30.5': 2,
    'art_cov_t_35.5': 2,
    'art_cov_t_38.5': 3,
    'art_cov_t_43.5': 3,
    'vl_suppr': 1}
        
    distance = 0
    for key in x.keys():
        # #print(f"Key: {key}")
        # if key not in weights:
        #     print(f"Key {key} not found in weights")
        #     continue
        # if key not in x_0:
        #     print(f"Key {key} not found in x_0")
        #     continue
        # #print(f"Weight: {weights[key]}, x[key]: {x[key]}, x_0[key]: {x_0[key]}")
        distance += weights[key] * (x[key] - x_0[key]) ** 2
    
    return np.sqrt(distance)

# create ABC instance
abc = pyabc.ABCSMC(models=calibration_wrapper_function, 
                   parameter_priors=prior, 
                   distance_function=distance_adaptive, #weighted_distance
                   sampler=MulticoreEvalParallelSampler(n_procs=95),
                   population_size=100) 


# target stats
# target_stats = {
#     'growthrate_43': 1.01,
#     'prev_f_20_15_49': 0.26,
#     'prev_m_20_15_49': 0.22,
#     'prev_f_25_15_49': 0.30,
#     'prev_m_25_15_49': 0.22,
#     'prev_f_36_15_49': 0.34,
#     'prev_m_36_15_49': 0.19,
#     'prev_f_41_15_49': 0.32,
#     'prev_m_41_15_49': 0.16,
#     'inc_f_32_15_49': 0.314,
#     'inc_m_32_15_49': 0.165,
#     'inc_f_36_15_49': 0.199,
#     'inc_m_36_15_49': 0.099,
#     'inc_f_41_15_49': 0.145,
#     'inc_m_41_15_49': 0.02,
#     'art_cov_t_30.5': 0.5,
#     'art_cov_t_35.5': 0.8,
#     'art_cov_t_38.5': 0.87,
#     'art_cov_t_43.5': 0.95,
#     'vl_suppr': 0.9
#  }

target_stats = {
     'growthrate_20': 1.01,
     'prev_10_15_49': 0.013,
     'prev_11_15_49': 0.027,
     'prev_12_15_49': 0.05,
     'prev_13_15_49': 0.08,
     'prev_14_15_49': 0.114,
     'prev_15_15_49': 0.15,
     'prev_16_15_49': 0.18,
     'prev_17_15_49': 0.204,
     'prev_18_15_49': 0.222,
     'prev_19_15_49': 0.236,
     'prev_20_15_49': 0.246,
     'prev_30_15_49': 0.279,
     'prev_35_15_49': 0.299,
     'prev_40_15_49': 0.276,
     'prev_f_20_15_49': 0.26,
     'prev_m_20_15_49': 0.22,
     'inc_10_15_49': 0.0783,
     'inc_11_15_49': 0.1515,
     'inc_12_15_49': 0.2555,
     'inc_13_15_49': 0.3743,
     'inc_14_15_49': 0.4733,
     'inc_15_15_49': 0.5225,
     'inc_16_15_49': 0.5016,
     'inc_17_15_49': 0.478,
     'inc_18_15_49': 0.437,
     'inc_19_15_49': 0.4174,
     'inc_20_15_49': 0.3913,
     'inc_30_15_49': 0.286,
     'inc_35_15_49': 0.239,
     'inc_40_15_49': 0.098
    
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