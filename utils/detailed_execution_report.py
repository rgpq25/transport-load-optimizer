"""
Ejecución detallada de SA-GA o GRASP para los documentos técnicos de la tesis.

Ejecuta el algoritmo elegido sobre un dataset reducido con una semilla fija:
  1. una corrida sin --trace, para obtener el tiempo real de ejecución;
  2. una corrida con --trace, para obtener el detalle iteración por iteración.
Con ambas salidas genera un Excel con las hojas de datos del documento técnico
(Ejecución, Datos, Iteraciones, Flujo de datos, Despachos).

Uso: python detailed_execution_report.py <SA-GA|GRASP>
Resultado: output/detailed_execution/<algoritmo>/detailed_execution_<algoritmo>.xlsx
"""
import os
import sys
import shutil
import subprocess
from datetime import datetime

import pandas as pd
from openpyxl import Workbook
from openpyxl.chart import LineChart, Reference
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

from config import (algorithm_executable_path, input_folder_path, output_folder_path,
                    output_dispatches_path, output_metadata_path, repository_path)

# ============================================================ Configuración de la ejecución
INPUT_PATH = os.path.join(input_folder_path, "input_detailed_execution.txt")
SEED = 42

# Parámetros reducidos (previos a la calibración) para que la traza sea legible
ALGORITHM_PARAMS = {
    "SA-GA": {"population": 6, "mutation_rate": 0.2, "t_init": 50, "t_min": 1, "alpha": 0.7},
    "GRASP": {"iterations": 10, "k_percent": 30, "alphas": "0.1,0.5,0.9"},
}
PARAM_LABELS = {
    "population": "Tamaño de población",
    "mutation_rate": "Tasa de mutación",
    "t_init": "Temperatura inicial (T_init)",
    "t_min": "Temperatura mínima (T_min)",
    "alpha": "Factor de enfriamiento (alpha)",
    "iterations": "Número de iteraciones por UT (se ejecutan el doble: construcción y mejora local alternadas)",
    "k_percent": "Porcentaje de bloques removidos en la mejora local (K %)",
    "alphas": "Conjunto de valores de alpha para la RCL",
}

# Valores de main.cpp usados para describir el subproblema
SHIFT_WINDOWS = {"morning": (480, 720), "afternoon": (840, 1080)}
UNLOADING_TIME = 20

# ============================================================ Estilos (iguales al borrador del documento)
BLUE = PatternFill("solid", fgColor="FFD9E1F2")
YELLOW = PatternFill("solid", fgColor="FFFFF2CC")
THIN = Side(style="thin")
BORDER = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)
BOLD = Font(name="Calibri", size=11, bold=True)
NORMAL = Font(name="Calibri", size=11)
TITLE = Font(name="Calibri", size=13, bold=True)
NOTE = Font(name="Calibri", size=10, italic=True, color="FF595959")
WRAP = Alignment(wrap_text=True, vertical="top")
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=True)

SHIFT_ES = {"morning": "Mañana", "afternoon": "Tarde", "null": "Sin turno"}
SAGA_DECISIONS = {
    "child_improves": "Hijo mejora al padre 1: se acepta",
    "child_accepted_by_sa": "Hijo peor: aceptado por el criterio del recocido",
    "child_equal": "Hijo con igual fitness (Δ = 0): se acepta con probabilidad 1",
    "parent1_kept": "Hijo peor: se mantiene el padre 1",
}
GRASP_PHASES = {"construction": "Construcción", "local_search": "Mejora local", "reinsertion": "Reinserción (mejora local)"}
GRASP_EVENTS = {
    "rcl_layer": "Capa elegida al azar de la RCL",
    "best_fit_layer": "Capa de mayor volumen (mejor ajuste)",
    "no_candidates_stop": "Ninguna capa cabe en el espacio: fin de la construcción",
    "empty_rcl_stop": "RCL vacía: fin de la construcción",
}
INFEASIBLE = -9999999


# ============================================================ Ejecución del algoritmo
def run_algorithm(algo, trace_dir=None):
    args = [algorithm_executable_path, "--input", INPUT_PATH, "--algo", algo]
    for key, value in ALGORITHM_PARAMS[algo].items():
        args += [f"--{key}", str(value)]
    args += ["--seed", str(SEED)]
    if trace_dir:
        args += ["--trace", trace_dir]
    print(f"[Ejecución] - {' '.join(args)}")
    subprocess.run(args, capture_output=True, text=True, check=True,
                   cwd=os.path.join(repository_path, "main_algorithms"))
    return args


def execute(algo, work_dir):
    """Ejecuta sin y con traza. Respalda y restaura los archivos de output/ que usan las interfaces."""
    trace_dir = os.path.join(work_dir, "trace")
    real_dir = os.path.join(work_dir, "real_time_run")
    shutil.rmtree(work_dir, ignore_errors=True)
    os.makedirs(trace_dir)
    os.makedirs(real_dir)

    backup = {}
    for path in (output_dispatches_path, output_metadata_path):
        if os.path.exists(path):
            backup[path] = path + ".bak_detailed_execution"
            shutil.copy2(path, backup[path])
    try:
        command_real = run_algorithm(algo)
        shutil.copy2(output_dispatches_path, real_dir)
        shutil.copy2(output_metadata_path, real_dir)
        command_trace = run_algorithm(algo, trace_dir)
    finally:
        for path, copy in backup.items():
            shutil.move(copy, path)
    return command_real, command_trace, trace_dir, real_dir


# ============================================================ Lectura de datos
def parse_input(path):
    """Lee el formato de entrada de main_algorithms/input.cpp."""
    lines = [l.strip() for l in open(path, encoding="utf-8")]
    data = {"clients": {}, "blocks": {}, "units": {}, "routes": {}, "deliveries": []}
    sec, i = None, 0
    while i < len(lines):
        l = lines[i]
        if not l:
            i += 1
            continue
        if l.startswith("@"):
            sec = l.split()[0]
            i += 1
            continue
        p = l.split()
        if sec == "@clients":
            data["clients"][int(p[0])] = l.split(maxsplit=1)[1]
        elif sec == "@blocks":
            data["blocks"][int(p[0])] = dict(id=int(p[0]), h=float(p[1]), w=float(p[2]), l=float(p[3]),
                                             weight=float(p[4]), fragility=float(p[5]))
        elif sec == "@transport_units":
            data["units"][int(p[0])] = dict(id=int(p[0]), h=float(p[1]), w=float(p[2]), l=float(p[3]),
                                            maxW=float(p[4]), minW=float(p[5]))
        elif sec == "@routes":
            n = int(p[1])
            data["routes"][int(p[0])] = [tuple(map(int, lines[i + 1 + k].split()[:2])) for k in range(n)]
            i += n + 1
            continue
        elif sec == "@orders":
            oid, cid, prio, nd = map(int, p)
            i += 1
            for _ in range(nd):
                q = lines[i].split()
                i += 1
                nb = int(q[3])
                blocks = [int(lines[i + k].split()[0]) for k in range(nb)]
                i += nb
                data["deliveries"].append(dict(id=int(q[0]), order=oid, client=cid, priority=prio,
                                               shift=q[1], date=q[2], blocks=blocks))
            continue
        i += 1
    return data


def read_csv(folder, name):
    path = os.path.join(folder, name)
    return pd.read_csv(path, keep_default_na=False) if os.path.exists(path) else pd.DataFrame()


def run_label(run_id):
    """'2024-09-28|R1|morning|08:00-09:55' -> 'Franja 08:00–09:55'"""
    return "Franja " + run_id.split("|")[-1].replace("-", "–")


def blank(value):
    return "" if value is None or (isinstance(value, float) and pd.isna(value)) else value


# ============================================================ Escritura en Excel
def sheet_header(ws, title, description, last_col):
    ws.sheet_view.showGridLines = False
    ws.merge_cells(f"A1:{last_col}1")
    ws["A1"].value, ws["A1"].font = title, TITLE
    ws.merge_cells(f"A2:{last_col}2")
    ws["A2"].value, ws["A2"].font, ws["A2"].alignment = description, NORMAL, WRAP
    ws.row_dimensions[2].height = 45
    return 4


def section(ws, row, text):
    cell = ws.cell(row=row, column=1, value=text)
    cell.font, cell.fill = BOLD, BLUE
    return row + 1


def note(ws, row, text, last_col):
    ws.merge_cells(start_row=row, start_column=1, end_row=row, end_column=last_col)
    cell = ws.cell(row=row, column=1, value=text)
    cell.font, cell.alignment = NOTE, WRAP
    ws.row_dimensions[row].height = 30
    return row + 1


def table(ws, row, headers, rows, number_formats=None):
    """Tabla con encabezado amarillo y bordes. Devuelve (fila siguiente, fila del encabezado)."""
    for j, h in enumerate(headers, start=1):
        c = ws.cell(row=row, column=j, value=h)
        c.font, c.fill, c.alignment, c.border = BOLD, YELLOW, CENTER, BORDER
    for i, r in enumerate(rows, start=1):
        for j, v in enumerate(r, start=1):
            c = ws.cell(row=row + i, column=j, value=blank(v))
            c.font, c.alignment, c.border = NORMAL, CENTER, BORDER
            if number_formats and j in number_formats and isinstance(v, (int, float)):
                c.number_format = number_formats[j]
    return row + len(rows) + 2, row


def key_values(ws, row, pairs):
    for key, value in pairs:
        k = ws.cell(row=row, column=1, value=key)
        k.font, k.fill, k.alignment, k.border = BOLD, YELLOW, WRAP, BORDER
        v = ws.cell(row=row, column=2, value=blank(value))
        v.font, v.alignment, v.border = NORMAL, WRAP, BORDER
        row += 1
    return row + 1


def widths(ws, values):
    for idx, w in enumerate(values, start=1):
        ws.column_dimensions[get_column_letter(idx)].width = w


def line_chart(ws, anchor, title, x_title, header_row, first_row, last_row, x_col, y_cols):
    chart = LineChart()
    chart.title, chart.x_axis.title, chart.y_axis.title = title, x_title, "Fitness"
    chart.height, chart.width = 9, 18
    for col in y_cols:
        chart.add_data(Reference(ws, min_col=col, min_row=header_row, max_row=last_row), titles_from_data=True)
    chart.set_categories(Reference(ws, min_col=x_col, min_row=first_row, max_row=last_row))
    ws.add_chart(chart, anchor)


# ============================================================ Hojas comunes
def build_execution_sheet(wb, algo, command_real, command_trace, real_dir, trace_dir):
    ws = wb.active
    ws.title = "Ejecución"
    widths(ws, [34, 110])
    row = sheet_header(ws, "Configuración de la ejecución",
                       "Parámetros y comandos con los que se generaron los datos de este documento. "
                       "Con el mismo ejecutable, la misma entrada, los mismos parámetros y la misma semilla, "
                       "la ejecución se reproduce exactamente (por ejemplo, para las capturas 3D desde la interfaz).", "B")
    real_meta = pd.read_csv(os.path.join(real_dir, "output_result_metadata.csv")).iloc[0]
    trace_meta = pd.read_csv(os.path.join(trace_dir, "output_result_metadata.csv")).iloc[0]
    with open(os.path.join(real_dir, "output_dispatches.csv"), "rb") as a, \
         open(os.path.join(trace_dir, "output_dispatches.csv"), "rb") as b:
        same_result = a.read() == b.read() and real_meta["fitness"] == trace_meta["fitness"]

    row = section(ws, row, "Ejecución")
    row = key_values(ws, row, [
        ("Algoritmo", algo),
        ("Fecha de generación", datetime.now().strftime("%d/%m/%Y %H:%M")),
        ("Ejecutable", os.path.relpath(algorithm_executable_path, repository_path).replace("\\", "/")),
        ("Archivo de entrada", os.path.relpath(INPUT_PATH, repository_path).replace("\\", "/")),
        ("Semilla (--seed)", SEED),
    ])
    row = section(ws, row, "Parámetros del algoritmo")
    row = key_values(ws, row, [(f"{PARAM_LABELS[k]} (--{k})", v) for k, v in ALGORITHM_PARAMS[algo].items()])
    row = section(ws, row, "Resultado")
    row = key_values(ws, row, [
        ("Fitness de la solución", real_meta["fitness"]),
        ("Tiempo real de ejecución (s)", real_meta["duration"]),
        ("Mismo resultado con y sin traza", "Sí" if same_result else "NO: revisar"),
    ])
    row = section(ws, row, "Comandos")
    row = key_values(ws, row, [
        ("Corrida sin traza (tiempo real)", " ".join(os.path.basename(a) if i == 0 else a for i, a in enumerate(command_real))),
        ("Corrida con traza (datos de las hojas)", " ".join(os.path.basename(a) if i == 0 else a for i, a in enumerate(command_trace))),
    ])
    note(ws, row, "El tiempo real proviene de la corrida sin traza: la corrida con traza tarda más porque registra cada "
                  "iteración, por lo que sus tiempos no se reportan.", 2)
    return same_result


def build_data_sheet(wb, data):
    ws = wb.create_sheet("Datos")
    widths(ws, [14, 14, 30, 24, 36, 16, 16, 14])
    row = sheet_header(ws, "Conjunto de datos",
                       "Datos de entrada del subproblema analizado. Los mismos datos se emplean en los documentos "
                       "técnicos de SA-GA y de GRASP.", "H")
    route_id, points = next(iter(data["routes"].items()))
    shift = data["deliveries"][0]["shift"]
    travel = sum(t for _, t in points)
    visited = sum(1 for c, _ in points if c != 1)  # timeSlotUtils.cpp: el ID 1 es el centro de distribución
    start, end = SHIFT_WINDOWS[shift]

    row = section(ws, row, "Subproblema analizado")
    row, _ = table(ws, row, ["Fecha", "Turno", "Ventana del turno", "ID ruta", "Duración del recorrido (min)"],
                   [[data["deliveries"][0]["date"], SHIFT_ES[shift], f"{start // 60:02d}:00–{end // 60:02d}:00", route_id,
                     f"{travel + UNLOADING_TIME * visited} ({travel} de viaje + {visited} × {UNLOADING_TIME} de descarga)"]])
    row = section(ws, row, "Ruta")
    row, _ = table(ws, row, ["Orden", "ID cliente", "Cliente", "Tiempo de viaje desde el punto anterior (min)"],
                   [[i, c, data["clients"][c] + (" (centro de distribución)" if c == 1 else ""), t]
                    for i, (c, t) in enumerate(points, start=1)])
    row = section(ws, row, "Entregas")
    row, _ = table(ws, row, ["ID entrega", "ID pedido", "Cliente", "Prioridad", "Turno", "Fecha de entrega", "N.° de bloques", "Bloques"],
                   [[d["id"], d["order"], data["clients"][d["client"]], d["priority"], SHIFT_ES[d["shift"]], d["date"],
                     len(d["blocks"]), ", ".join(map(str, d["blocks"]))] for d in data["deliveries"]])
    row = section(ws, row, "Bloques")
    block_delivery = {b: d["id"] for d in data["deliveries"] for b in d["blocks"]}
    rows = [[b["id"], block_delivery.get(b["id"], ""), b["h"], b["w"], b["l"], round(b["h"] * b["w"] * b["l"], 3),
             b["weight"], b["fragility"]] for b in data["blocks"].values()]
    rows.append(["Total", "", "", "", "", round(sum(r[5] for r in rows), 3), round(sum(r[6] for r in rows), 1), ""])
    row, _ = table(ws, row, ["ID bloque", "ID entrega", "Alto (m)", "Ancho (m)", "Largo (m)", "Volumen (m³)", "Peso (kg)", "Fragilidad"],
                   rows, {3: "0.00", 4: "0.00", 5: "0.00", 6: "0.000", 7: "0.0", 8: "0.0"})
    row = section(ws, row, "Unidades de transporte disponibles")
    table(ws, row, ["ID UT", "Alto (m)", "Ancho (m)", "Largo (m)", "Volumen (m³)", "Peso máximo (kg)", "Peso mínimo (kg)"],
          [[u["id"], u["h"], u["w"], u["l"], round(u["h"] * u["w"] * u["l"], 3), u["maxW"], u["minW"]] for u in data["units"].values()],
          {2: "0.00", 3: "0.00", 4: "0.00", 5: "0.000"})


def build_dispatches_sheet(wb, data, trace_dir):
    ws = wb.create_sheet("Despachos")
    widths(ws, [13, 13, 13, 13, 13, 13, 13, 13, 13, 13])
    row = sheet_header(ws, "Despachos construidos",
                       "Despachos de la solución final: unidad de transporte asignada, entregas cargadas y posición de "
                       "cada bloque dentro de la unidad (origen x, y, z y dimensiones según la orientación).", "J")
    df = pd.read_csv(os.path.join(trace_dir, "output_dispatches.csv"))
    meta = pd.read_csv(os.path.join(trace_dir, "output_result_metadata.csv")).iloc[0]

    summary = []
    for dispatch_id, g in df.groupby("dispatch_id"):
        first = g.iloc[0]
        unit = data["units"][int(first["truck_id"])]
        weight = sum(data["blocks"][int(b)]["weight"] for b in g["block_id"])
        volume = sum(data["blocks"][int(b)]["h"] * data["blocks"][int(b)]["w"] * data["blocks"][int(b)]["l"] for b in g["block_id"])
        unit_volume = unit["h"] * unit["w"] * unit["l"]
        summary.append([dispatch_id, f"{first['slot_start']}–{first['slot_end']}", int(first["truck_id"]),
                        ", ".join(map(str, sorted(g["delivery_id"].unique()))), len(g), round(weight, 1), unit["maxW"],
                        round(volume, 3), round(100 * volume / unit_volume, 1)])
    row = section(ws, row, "Resumen")
    row, _ = table(ws, row, ["ID despacho", "Franja horaria", "ID UT", "Entregas", "N.° de bloques", "Peso cargado (kg)",
                             "Peso máximo (kg)", "Volumen cargado (m³)", "Uso de volumen (%)"], summary, {8: "0.000"})
    row = key_values(ws, row, [("Fitness de la solución", meta["fitness"])])

    row = section(ws, row, "Posición de los bloques")
    table(ws, row, ["ID despacho", "ID UT", "ID entrega", "ID bloque", "Orientación", "x (m)", "y (m)", "z (m)",
                    "Largo (m)", "Ancho (m)", "Alto (m)"],
          df[["dispatch_id", "truck_id", "delivery_id", "block_id", "orientation", "x", "y", "z", "lx", "ly", "lz"]].values.tolist())


# ============================================================ Hojas de SA-GA
def build_saga_sheets(wb, data, trace_dir):
    generations = read_csv(trace_dir, "saga_generations.csv")
    population = read_csv(trace_dir, "saga_population.csv")
    crossovers = read_csv(trace_dir, "saga_crossovers.csv")
    best = read_csv(trace_dir, "saga_best.csv")
    run_ids = list(dict.fromkeys(generations["run_id"]))
    block_order = ", ".join(str(b) for d in data["deliveries"] for b in d["blocks"])

    # --- Iteraciones
    ws = wb.create_sheet("Iteraciones")
    widths(ws, [14, 16, 20, 24, 22])
    row = sheet_header(ws, "Evolución por generación",
                       "Evolución de la población en cada generación, desde la población inicial (generación 0) hasta "
                       "que la temperatura alcanza el valor mínimo.", "E")
    for run_id in run_ids:
        g = generations[generations["run_id"] == run_id]
        if len(run_ids) > 1:
            row = section(ws, row, run_label(run_id))
        rows = [[int(r.generation), "" if r.temperature == "" else float(r.temperature), float(r.avg_fitness),
                 float(r.best_fitness_population), float(r.best_fitness_global)] for r in g.itertuples()]
        next_row, header_row = table(ws, row, ["Generación", "Temperatura", "Fitness promedio", "Mejor fitness población",
                                               "Mejor fitness global"], rows, {2: "0.000", 3: "0.00", 4: "0.00", 5: "0.00"})
        line_chart(ws, f"G{header_row}", "Evolución del fitness", "Generación", header_row, header_row + 1,
                   header_row + len(rows), 1, [4, 5])
        row = max(next_row, header_row + 20)
    note(ws, row, "El fitness promedio puede ser muy negativo porque las soluciones infactibles en 3D reciben "
                  f"{INFEASIBLE}. El gráfico muestra el mejor fitness de la población y el mejor global.", 5)

    # --- Flujo de datos
    ws = wb.create_sheet("Flujo de datos")
    widths(ws, [9, 10, 12, 10, 12, 34, 16, 34, 16, 10, 12, 10, 14, 14, 12, 34, 14])
    row = sheet_header(ws, "Flujo de datos",
                       "Cómo se construye cada generación: por cada cruce se muestran los padres elegidos por torneo, el "
                       "hijo antes y después de la mutación y la decisión del criterio de aceptación del recocido simulado "
                       "(si el hijo mejora al padre 1 se acepta; si no, se acepta con probabilidad e^(−Δ/T)).", "Q")
    row = note(ws, row, "Cromosoma: asignación de cada entrega a una unidad de transporte (E<entrega>:UT<unidad>; \"-\" = "
                        f"sin asignar) y orientación de cada bloque (0/1) en el orden {block_order}.", 17)
    for run_id in run_ids:
        if len(run_ids) > 1:
            row = section(ws, row, run_label(run_id))
        row = section(ws, row, "Población inicial (generación 0)")
        p = population[population["run_id"] == run_id]
        row, _ = table(ws, row, ["Cromosoma", "Asignación", "Orientaciones", "Fitness", "Despachos", "Uso de volumen",
                                 "Entregas atendidas (%)", "Cobertura de prioridad (%)", "Penalización por peso (kg)",
                                 "Factible en 3D", "Peso por UT"],
                       [[int(r.chromosome), r.assignments, str(r.orientations), float(r.fitness), r.dispatches,
                         r.avg_utilization, r.fulfillment_ratio, r.priority_coverage, r.weight_penalty_kg,
                         "Sí" if int(r.feasible_3d) else "No", r.weight_by_vehicle] for r in p.itertuples()],
                       {4: "0.00", 6: "0.00", 7: "0.00", 8: "0.00"})
        c = crossovers[crossovers["run_id"] == run_id]
        for generation, gc in c.groupby("generation", sort=True):
            row = section(ws, row, f"Generación {generation} (T = {float(gc.iloc[0]['temperature']):.3f})")
            rows = []
            for r in gc.itertuples():
                rows.append([int(r.crossover), int(r.parent1_index), float(r.parent1_fitness), int(r.parent2_index),
                             float(r.parent2_fitness),
                             f"{r.child_assignments_before_mutation} | {r.child_orientations_before_mutation}",
                             int(r.mutated_genes), f"{r.child_assignments} | {r.child_orientations}",
                             float(r.child_fitness), "Sí" if int(r.feasible_3d) else "No", float(r.delta),
                             "" if r.acceptance_probability == "" else float(r.acceptance_probability),
                             "" if r.random_value == "" else float(r.random_value),
                             SAGA_DECISIONS["child_equal"] if r.decision == "child_accepted_by_sa" and float(r.delta) == 0
                             else SAGA_DECISIONS.get(r.decision, r.decision), float(r.fitness_in_new_population)])
            row, _ = table(ws, row, ["Cruce", "Padre 1 (índice)", "Fitness padre 1", "Padre 2 (índice)", "Fitness padre 2",
                                     "Hijo antes de mutar", "Genes mutados", "Hijo después de mutar", "Fitness hijo",
                                     "Factible en 3D", "Δ (padre 1 − hijo)", "Probabilidad e^(−Δ/T)", "Valor aleatorio",
                                     "Decisión", "Fitness que ingresa"],
                           rows, {3: "0.00", 5: "0.00", 9: "0.00", 11: "0.00", 12: "0.0000", 13: "0.0000", 15: "0.00"})
        row = section(ws, row, "Mejor solución de la corrida")
        b = best[best["run_id"] == run_id]
        row, _ = table(ws, row, ["Asignación", "Orientaciones", "Fitness", "Despachos", "Uso de volumen",
                                 "Entregas atendidas (%)", "Cobertura de prioridad (%)", "Penalización por peso (kg)", "Peso por UT"],
                       [[r.assignments, str(r.orientations), float(r.fitness), r.dispatches, r.avg_utilization,
                         r.fulfillment_ratio, r.priority_coverage, r.weight_penalty_kg, r.weight_by_vehicle]
                        for r in b.itertuples()], {3: "0.00", 5: "0.00", 6: "0.00", 7: "0.00"})


# ============================================================ Hojas de GRASP
def build_grasp_sheets(wb, data, trace_dir):
    vehicles = read_csv(trace_dir, "grasp_vehicle_selection.csv")
    iterations = read_csv(trace_dir, "grasp_iterations.csv")
    layers = read_csv(trace_dir, "grasp_layers.csv")
    local_search = read_csv(trace_dir, "grasp_local_search.csv")
    run_ids = list(dict.fromkeys(iterations["run_id"]))

    # --- Iteraciones
    ws = wb.create_sheet("Iteraciones")
    widths(ws, [10, 11, 16, 14, 10, 14, 14, 14, 18, 12, 16])
    row = sheet_header(ws, "Evolución por iteración",
                       "Patrón de carga construido en cada iteración para cada unidad de transporte. Las iteraciones "
                       "alternan la fase de construcción (capas elegidas de la RCL) y la de mejora local (remoción del "
                       "K % de los bloques y reinserción por mejor ajuste). Cada franja horaria es una corrida del optimizador.", "K")
    for run_id in run_ids:
        row = section(ws, row, run_label(run_id))
        it = iterations[iterations["run_id"] == run_id]
        rows = [[int(r.vehicle_id), int(r.iteration), GRASP_PHASES.get(r.phase, r.phase), r.deliveries, int(r.layers),
                 float(r.pattern_weight), float(r.max_weight), float(r.fitness), float(r.best_vehicle_fitness_before),
                 "Sí" if int(r.new_best) else "No", float(r.best_fitness_global)] for r in it.itertuples()]
        next_row, header_row = table(ws, row, ["ID UT", "Iteración", "Fase", "Entregas del patrón", "N.° de capas",
                                               "Peso del patrón (kg)", "Peso máximo (kg)", "Fitness del patrón",
                                               "Mejor fitness de la UT (antes)", "¿Nuevo mejor?", "Mejor fitness global"],
                                     rows, {6: "0.0", 8: "0.00", 9: "0.00", 11: "0.00"})
        line_chart(ws, f"M{header_row}", f"Evolución del fitness – {run_label(run_id)}", "Iteración", header_row,
                   header_row + 1, header_row + len(rows), 2, [8, 11])
        row = max(next_row, header_row + 20)

    # --- Flujo de datos
    ws = wb.create_sheet("Flujo de datos")
    widths(ws, [10, 11, 22, 8, 22, 22, 11, 11, 8, 10, 40, 10, 12, 11, 22, 11, 11, 13, 11])
    row = sheet_header(ws, "Flujo de datos",
                       "Detalle de la construcción de cada patrón: selección de la unidad de transporte, capas colocadas en "
                       "cada paso (espacio libre elegido, capas candidatas, RCL y capa elegida) y capas removidas en la "
                       "mejora local.", "S")
    for run_id in run_ids:
        row = section(ws, row, run_label(run_id))
        row = section(ws, row, "Selección de la unidad de transporte (la de volumen más cercano al volumen pendiente)")
        v = vehicles[vehicles["run_id"] == run_id]
        row, _ = table(ws, row, ["Paso", "Entregas pendientes", "Volumen pendiente (m³)", "ID UT", "Volumen UT (m³)",
                                 "Diferencia (m³)", "¿Elegida?"],
                       [[int(r.step), int(r.pending_deliveries), float(r.target_volume), int(r.vehicle_id),
                         float(r.vehicle_volume), float(r.difference), "Sí" if int(r.selected) else "No"]
                        for r in v.itertuples()], {3: "0.000", 5: "0.000", 6: "0.000"})

        row = section(ws, row, "Capas colocadas")
        ly = layers[layers["run_id"] == run_id]
        rows = []
        for r in ly.itertuples():
            space = f"({float(r.space_x):.2f}, {float(r.space_y):.2f}, {float(r.space_z):.2f})"
            dims = f"{float(r.space_length):.2f} × {float(r.space_width):.2f} × {float(r.space_height):.2f}"
            layer_dims = "" if r.layer_length == "" else \
                f"{float(r.layer_length):.2f} × {float(r.layer_width):.2f} × {float(r.layer_height):.2f}"
            rows.append([int(r.vehicle_id), int(r.iteration), GRASP_PHASES.get(r.phase, r.phase), int(r.step), space, dims,
                         int(r.spaces_left), int(r.candidates), "" if r.alpha == "" else float(r.alpha),
                         "" if r.rcl_size == "" else int(r.rcl_size), GRASP_EVENTS.get(r.event, r.event),
                         r.delivery_id, r.blocks, r.orientation, layer_dims,
                         "" if r.layer_volume == "" else float(r.layer_volume),
                         "" if r.layer_weight == "" else float(r.layer_weight), float(r.pattern_weight), r.new_spaces])
        row, _ = table(ws, row, ["ID UT", "Iteración", "Fase", "Paso", "Origen del espacio (x, y, z)",
                                 "Espacio (largo × ancho × alto)", "Espacios restantes", "Capas candidatas", "Alpha",
                                 "Tamaño RCL", "Evento", "ID entrega", "Bloques", "Orientación",
                                 "Capa (largo × ancho × alto)", "Volumen capa (m³)", "Peso capa (kg)",
                                 "Peso del patrón (kg)", "Espacios nuevos"],
                       rows, {16: "0.000", 17: "0.0", 18: "0.0"})

        row = section(ws, row, "Mejora local: capas removidas")
        ls = local_search[local_search["run_id"] == run_id]
        row, _ = table(ws, row, ["ID UT", "Iteración", "Bloques del patrón", "K (%)", "Bloques por remover",
                                 "Entregas removidas", "Bloques removidos"],
                       [[int(r.vehicle_id), int(r.iteration), int(r.total_blocks), float(r.k_percent),
                         int(r.blocks_to_remove), r.removed_deliveries, int(r.removed_blocks)] for r in ls.itertuples()])


# ============================================================ Programa principal
def main():
    if len(sys.argv) != 2 or sys.argv[1] not in ALGORITHM_PARAMS:
        print("Uso: python detailed_execution_report.py <SA-GA|GRASP>")
        sys.exit(1)
    algo = sys.argv[1]
    work_dir = os.path.join(output_folder_path, "detailed_execution", algo)

    command_real, command_trace, trace_dir, real_dir = execute(algo, work_dir)
    data = parse_input(INPUT_PATH)

    wb = Workbook()
    same_result = build_execution_sheet(wb, algo, command_real, command_trace, real_dir, trace_dir)
    build_data_sheet(wb, data)
    if algo == "SA-GA":
        build_saga_sheets(wb, data, trace_dir)
    else:
        build_grasp_sheets(wb, data, trace_dir)
    build_dispatches_sheet(wb, data, trace_dir)

    out_path = os.path.join(work_dir, f"detailed_execution_{algo}.xlsx")
    wb.save(out_path)
    print(f"[Éxito] Archivo generado en: {out_path}")
    if not same_result:
        print("[Advertencia] Los despachos con y sin traza no coinciden. Revisar antes de usar el documento.")


if __name__ == "__main__":
    main()
