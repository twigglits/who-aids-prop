from calibration_wrapper_function_revised import calibration_wrapper_function
import pyabc
# from pyabc.sampler import SingleCoreSampler
import os
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

# the parameters need to be named and not just passed as a vector
prior = pyabc.Distribution(hivtransmission_param_f1=pyabc.RV("uniform", 1, 1.2),
                           formation_hazard_agegapry_gap_agescale_man_woman=pyabc.RV("uniform", 0.25, 0.65),
                           person_agegap_man_woman_dist_normal_mu=pyabc.RV("uniform", 0, 2),
                           person_agegap_man_woman_dist_normal_sigma=pyabc.RV("uniform", 2, 5),
                           person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.6, 1.4),
                           person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.1, 0.6),
                           person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 25, 65),
                           person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 15, 55),
                           formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1, 1),
                           formation_hazard_agegapry_baseline=pyabc.RV("uniform", 2.5, 5),
                           formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -1, 0.1),
                           formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -1, 0.1),
                           dissolution_alpha_0=pyabc.RV("uniform", -3, 1),
                           conception_alpha_base=pyabc.RV("uniform", -2, 1),
                           diagnosis_baseline_t0=pyabc.RV("uniform", -7, 1),
                           diagnosis_baseline_t1=pyabc.RV("uniform", 0, 4),
                           diagnosis_baseline_t2=pyabc.RV("uniform", 0, 3.5),
                           diagnosis_baseline_t3=pyabc.RV("uniform", 0, 2),
                           diagnosis_baseline_t4=pyabc.RV("uniform", -1, 1)
                           )



# Adaptive distance
scale_log_file = tempfile.mkstemp(suffix=".json")[1]

distance_adaptive = pyabc.AdaptivePNormDistance(
    p=2,
    scale_function=pyabc.distance.mad,  # method by which to scale
    scale_log_file=scale_log_file,
)

# weighted distances



# def weighted_distance(x, x_0):
    
#     weights = {"growthrate_19": 5,
#                "growthrate_35": 5,
#                'prev_f_15_15_49': 5,
#                'prev_m_15_15_49': 5,
#                'prev_f_25_15_49': 5,
#                'prev_m_25_15_49': 5,
#                'prev_f_39_15_49': 5,
#                'prev_m_39_15_49': 5,
#                'inc_f_25_15_49': 10,
#                'inc_m_25_15_49': 10,
#                'inc_f_39_15_49': 10,
#                'inc_m_39_15_49': 10,
#                'art_cov_t_25.5': 1,
#                'art_cov_t_30.5': 1,
#                'art_cov_t_35.5': 1,
#                'art_cov_t_38.5': 1,
#                'vl_suppr_39': 1,
#                'vmmc_15_49': 1,
#                'vmmc_15_24': 1   
#               }
        
#     distance = 0
#     for key in x.keys():
#         #print(f"Key: {key}")
#         if key not in weights:
#             print(f"Key {key} not found in weights")
#             continue
#         if key not in x_0:
#             print(f"Key {key} not found in x_0")
#             continue
#         #print(f"Weight: {weights[key]}, x[key]: {x[key]}, x_0[key]: {x_0[key]}")
#         distance += weights[key] * (x[key] - x_0[key]) ** 2
    
#     return np.sqrt(distance)


# create ABC instance
abc = pyabc.ABCSMC(models=calibration_wrapper_function, 
                   parameter_priors=prior, 
                   distance_function=distance_adaptive, 
                   #sampler=SingleCoreSampler(),
                   #eps=pyabc.epsilon.MedianEpsilon(0.5),
                   population_size=5) 

# target stats
target_stats = {
    'growthrate_35': 1.015113064615719,
     'prev_f_18_19': 0.143,
     'prev_m_18_19': 0.008,
     'prev_f_20_24': 0.315,
     'prev_m_20_24': 0.066,
     'prev_f_25_29': 0.467,
     'prev_m_25_29': 0.213,
     'prev_f_30_34': 0.538,
     'prev_m_30_34': 0.366,
     'prev_f_35_39': 0.491,
     'prev_m_35_39': 0.47,
     'prev_f_40_44': 0.397,
     'prev_m_40_44': 0.455,
     'prev_f_45_49': 0.316,
     'prev_m_45_49': 0.425,
     'inc_f_18_19': 1.0387312328784977,
     'inc_m_18_19': 1.0080320855042735,
     'inc_f_20_24': 1.0439378948506126,
     'inc_m_20_24': 1.016128685406095,
     'inc_f_25_29': 1.0202013400267558,
     'inc_m_25_29': 1.026340948473442,
     'inc_f_30_34': 1.0273678027634894,
     'inc_m_30_34': 1.0314855038865227,
     'inc_f_35_39': 1.0408107741923882,
     'inc_m_35_39': 1.004008010677342,
     'inc_f_40_44': 1.0212220516375285,
     'inc_m_40_44': 1.0120722888660778,
     'inc_f_45_49': 1.0120722888660778,
     'inc_m_45_49': 1.0,
     'art_cov_t_25.5': 0.37,
     'art_cov_t_30.5': 0.4,
     'art_cov_t_35.5': 0.44,
     'art_cov_t_38.5': 0.49,
     'vl_suppr': 0.74
 }

# path to save output
db_path = os.path.join("Calibration/data", "simpact_calibration_output.db")

if os.path.exists(db_path):
    # Remove the file
    os.remove(db_path)

abc.new("sqlite:///" + db_path, target_stats)

history = abc.run(minimum_epsilon=0.5,max_nr_populations= 50) 

posterior_params, weights = history.get_distribution()

posterior_params.reset_index(drop=False).to_csv('Calibration/data/posterior_distributions.csv', index=None)