import pandas as pd
import numpy as np
from multiprocessing import Pool, cpu_count
from calibration_wrapper_function_revised_projections import calibration_wrapper_function

def duplicate_rows(df, times):
    return pd.concat([df] * times, ignore_index=True)

def simpact_parallel(model=calibration_wrapper_function,
                     input_csv='input.csv',
                     seed_count=0,
                     n_cluster=None,
                     repeat=2):
    
    # Read the actual input matrix from a CSV file
    actual_input_matrix = pd.read_csv(input_csv)
    
    # Duplicate each row 'repeat' times
    actual_input_matrix = duplicate_rows(actual_input_matrix, repeat)
    
    if n_cluster is None:
        n_cluster = min(cpu_count(), 8)
        
    nb_simul = len(actual_input_matrix)
    
    list_param = []
    for i in range(nb_simul):
        param = actual_input_matrix.iloc[i, :].to_dict()
        param['seed'] = seed_count + i + 1
        list_param.append(param)
    
    with Pool(n_cluster) as pool:
        pool.map(model, list_param)
    
    return f'Finished running {nb_simul} simulations'

# Example usage:
if __name__ == "__main__":
    result = simpact_parallel(input_csv='Calibration/data/posterior_distributions.csv', 
                              n_cluster=128, 
                              repeat=1)
    print(result)
