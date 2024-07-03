import pandas as pd
import pickle
import glob
import os
from psimapacthelper import *
from calibration_wrapper_function_revised_Copy import calibration_wrapper_function

#best_models = pd.read_csv('Calibration/data/posterior_distributions.csv')
best_models = pd.read_csv('Calibration/posterior_distributions_manual_.csv')

# Run for each parameter combination 10 times

def run_model_for_each_row(df):
    results = []
    total_rows = len(df)
    
    for index, row in df.iterrows():
        row_dict = row.to_dict()
        for run in range(1, 11):
            print(f'========== Now running for parameter set {index + 1}/{total_rows}, run {run}/10 ===========')
            result = calibration_wrapper_function(row_dict)
            results.append(result)
            
    return print('Finished running all models')

# check and remove all files in the folder to store final models
folder_path = "Calibration/final_data"

# Remove files in the specified folder
remove_files_in_folder(folder_path)

run_model_for_each_row(df=best_models)