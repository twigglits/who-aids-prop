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
                           hivtransmission_param_a=pyabc.RV("uniform", -2.18, 0.2),
                           formation_hazard_agegapry_gap_agescale_man=pyabc.RV("uniform", 0.2, 0.1),
                           formation_hazard_agegapry_gap_agescale_woman=pyabc.RV("uniform", 0.2, 0.15),
                           person_agegap_man_dist_normal_mu=pyabc.RV("uniform", -3, 3),
                           person_agegap_man_dist_normal_sigma=pyabc.RV("uniform", 3, 1.5),
                           person_agegap_woman_dist_normal_mu=pyabc.RV("uniform", -2, 2),
                           person_agegap_woman_dist_normal_sigma=pyabc.RV("uniform", 2, 2),
                           person_eagerness_man_dist_gamma_a=pyabc.RV("uniform", 0.65, 0.3),
                           person_eagerness_woman_dist_gamma_a=pyabc.RV("uniform", 0.2, 0.3),
                           person_eagerness_man_dist_gamma_b=pyabc.RV("uniform", 40, 20),
                           person_eagerness_woman_dist_gamma_b=pyabc.RV("uniform", 40, 20),
                           formation_hazard_agegapry_gap_factor_man_woman_exp=pyabc.RV("uniform", -1.8, 1),
                           formation_hazard_agegapry_baseline=pyabc.RV("uniform", 4, 2),
                           formation_hazard_agegapry_numrel_man=pyabc.RV("uniform", -0.7, 0.6),
                           formation_hazard_agegapry_numrel_woman=pyabc.RV("uniform", -0.8, 0.7),
                           dissolution_alpha_0=pyabc.RV("uniform", -2.55, 0.8),
                           conception_alpha_base=pyabc.RV("uniform", -3.2, 0.2),
                           conception_alpha_base_1=pyabc.RV("uniform", 0.4, 0.2),
                           conception_alpha_base_2=pyabc.RV("uniform", 0.2, 0.2),
                           diagnosis_baseline_t0=pyabc.RV("uniform", -1.85, 0.2),
                           diagnosis_baseline_t1=pyabc.RV("uniform", 0.2, 0.15),
                           diagnosis_baseline_t2=pyabc.RV("uniform", 0.15, 0.1),
                           diagnosis_baseline_t2_2=pyabc.RV("uniform", 0, 0.1),
                           diagnosis_baseline_t3=pyabc.RV("uniform", 0.2, 0.15),
                           diagnosis_baseline_t4=pyabc.RV("uniform", 0.1, 0.2),
                           diagnosis_baseline_t5=pyabc.RV("uniform", 0.4, 0.4), 
                           diagnosis_eagernessfactor=pyabc.RV("uniform", 1.012, 0.025)
                           )
# Adaptive distance
scale_log_file = tempfile.mkstemp(suffix=".json")[1]

distance_adaptive = pyabc.AdaptivePNormDistance(
    p=2,#1 #manhattan
    scale_function=pyabc.distance.mean, #pyabc.distance.mean, #pyabc.distance.pcmad # method by which to scale
    scale_log_file=scale_log_file,
)


# create ABC instance
abc_continued = pyabc.ABCSMC(models=calibration_wrapper_function, 
                   parameter_priors=prior, 
                   distance_function=distance_adaptive,#smape, weighted_distance,pyabc.PNormDistance(p=2),  
                   sampler=MulticoreEvalParallelSampler(n_procs=128),
                   population_size=30) 


# path to save output
db_path = os.path.join("Calibration/data", "simpact_calibration_output.db")

# Load the history from the previous run
abc_continued.load("sqlite:///" + db_path)

# Resume the calibration from the last state
history = abc_continued.run(minimum_epsilon=0.05, max_nr_populations=10)

posterior_params, weights = history.get_distribution()

posterior_params.reset_index(drop=False).to_csv('Calibration/data/posterior_distributions.csv', index=None)
