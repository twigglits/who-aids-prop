import pandas as pd
import numpy as np
from multiprocessing import Pool, cpu_count
from calibration_wrapper_function_revised_projections import calibration_wrapper_function

def duplicate_rows(df, times):
    return pd.concat([df] * times, ignore_index=True)

def simpact_parallel(model=calibration_wrapper_function,
                     input_csv='input.csv',
                     # seed_count=0,
                     n_cluster=None,
                     repeat=2):
    
    # Read the actual input matrix from a CSV file
    actual_input_matrix = pd.read_csv(input_csv)
    # Add model number column
    actual_input_matrix['model_id'] = [i + 1 for i in range(len(actual_input_matrix))]
    
    # Duplicate each row 'repeat' times
    actual_input_matrix = duplicate_rows(actual_input_matrix, repeat)
    
    # Use the model number to group the seeds:
    actual_input_matrix['seed'] = (actual_input_matrix.groupby('model_id').cumcount() % repeat + 1).astype(int)
    
    if n_cluster is None:
        n_cluster = min(cpu_count(), 8)
        
    nb_simul = len(actual_input_matrix)
    # nb_simul = 1
    
    list_param = []
    for i in range(nb_simul):
        param = actual_input_matrix.iloc[i, :].to_dict()
        # param['seed'] = seed_count + i + 1
        list_param.append(param)
    
    with Pool(n_cluster) as pool:
        pool.map(model, list_param)
    
    return f'Finished running {nb_simul} simulations'

# Example usage:
if __name__ == "__main__":
    result = simpact_parallel(input_csv='Calibration/data/posterior_distributions-Copy1.csv', 
                              n_cluster=128, 
                              repeat=20)
    print(result)