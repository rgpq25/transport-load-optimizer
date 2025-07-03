import os

repository_path = "C:/main/university/semesters/CICLO XII/TESIS2/transport-load-optimizer"


input_folder_path = os.path.join(repository_path, "input")
output_folder_path = os.path.join(repository_path, "output")

output_dispatches_path = os.path.join(output_folder_path, "output_dispatches.csv")
output_metadata_path = os.path.join(output_folder_path, "output_result_metadata.csv")

algorithm_executable_path = os.path.join(repository_path, "main_algorithms/dist/Debug/MinGW-Windows/main_algorithms.exe")

saga_args = [
    {
        "nombreParametro": "Población",
        "nombreVariable": "population",
        "defaultValue": "100",
    },
    {
        "nombreParametro": "Tasa de mutación",
        "nombreVariable": "mutation_rate",
        "defaultValue": "0.2",
    },
    {
        "nombreParametro": "Temperatura inicial",
        "nombreVariable": "t_init",
        "defaultValue": "1000",
    },
    {
        "nombreParametro": "Temperatura mínima",
        "nombreVariable": "t_min",
        "defaultValue": "1",
    },
    {
        "nombreParametro": "Factor enfriamiento",
        "nombreVariable": "alpha",
        "defaultValue": "0.9",
    },
]

grasp_args = [
    {
        "nombreParametro": "Numero de iteraciones",
        "nombreVariable": "iterations",
        "defaultValue": "700",
    },
    {
        "nombreParametro": "% de bloques por remover (K%)",
        "nombreVariable": "k_percent",
        "defaultValue": "30",
    },
    {
        "nombreParametro": "Alfas (coma separados)",
        "nombreVariable": "alphas",
        "defaultValue": "0.1,0.5,0.9",
    },
]