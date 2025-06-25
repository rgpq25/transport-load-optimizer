import tkinter as tk
from tkinter import filedialog, ttk, messagebox
import subprocess
import pandas as pd
import plotly.graph_objects as go
import random

def visualize_dispatch(dispatch):
    # Get truck dimensions from the first row
    truck_l = dispatch["truck_l"]
    truck_w = dispatch["truck_w"]
    truck_h = dispatch["truck_h"]

    fig = go.Figure()

    # Draw container (transparent bounding box)
    fig.add_trace(go.Mesh3d(
        x=[0, truck_l, truck_l, 0, 0, truck_l, truck_l, 0],
        y=[0, 0, truck_w, truck_w, 0, 0, truck_w, truck_w],
        z=[0, 0, 0, 0, truck_h, truck_h, truck_h, truck_h],
        i=[0, 0, 0, 4, 5, 6, 7, 7, 3, 1, 2, 6],
        j=[1, 2, 3, 5, 6, 7, 4, 6, 0, 5, 3, 2],
        k=[2, 3, 0, 6, 7, 4, 5, 5, 1, 4, 2, 3],
        opacity=0.1,
        color='gray',
        name='Truck',
        showscale=False
    ))

    # Add blocks
    colors = {}
    for idx, block in enumerate(dispatch['blocks']):
        x, y, z = block['x'], block['y'], block['z']
        lx, ly, lz = block['lx'], block['ly'], block['lz']
        block_id = block['block_id']

        if block_id not in colors:
            colors[block_id] = f'rgb({random.randint(50,200)}, {random.randint(50,200)}, {random.randint(50,200)})'

        vertices = [
            [x,     y,     z],
            [x+lx,  y,     z],
            [x+lx,  y+ly,  z],
            [x,     y+ly,  z],
            [x,     y,     z+lz],
            [x+lx,  y,     z+lz],
            [x+lx,  y+ly,  z+lz],
            [x,     y+ly,  z+lz],
        ]

        faces = [
            [0, 1, 2], [0, 2, 3],  # bottom
            [4, 5, 6], [4, 6, 7],  # top
            [0, 1, 5], [0, 5, 4],  # front
            [2, 3, 7], [2, 7, 6],  # back
            [1, 2, 6], [1, 6, 5],  # right
            [0, 3, 7], [0, 7, 4],  # left
        ]

        x_vals, y_vals, z_vals = zip(*vertices)
        i, j, k = zip(*faces)

        fig.add_trace(go.Mesh3d(
            x=x_vals, y=y_vals, z=z_vals,
            i=i, j=j, k=k,
            opacity=0.5,
            color=colors[block_id],
            name=f'Block {block_id}',
            hovertext=f"Block {block_id}<br>Pos: ({x}, {y}, {z})<br>Size: ({lx}, {ly}, {lz})<br>Fragility: {block['fragility']}",
            hoverinfo='text'
        ))

    fig.update_layout(
        title=f"Dispatch {dispatch['dispatch_id']} – 3D Load Plan",
        scene=dict(
            xaxis=dict(title='Length', range=[0, truck_l]),
            yaxis=dict(title='Width', range=[0, truck_w]),
            zaxis=dict(title='Height', range=[0, truck_h]),
        ),
        margin=dict(l=0, r=0, b=0, t=40),
        showlegend=False
    )

    fig.show()


saga_args = [
    {
        "nombreParametro": "Población",
        "nombreVariable": "population",
        "defaultValue": "50",
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
        "defaultValue": "0.95",
    },
]

grasp_arg = [
    {
        "nombreParametro": "Numero de iteraciones",
        "nombreVariable": "iterations",
        "defaultValue": "100",
    },
    {
        "nombreParametro": "% de pallets por remover (K%)",
        "nombreVariable": "k_percent",
        "defaultValue": "30",
    },
    {
        "nombreParametro": "Alfas (coma separados)",
        "nombreVariable": "alphas",
        "defaultValue": "0.1,0.5",
    },
]

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("Optimización de Carga")
        self.root.geometry("450x300")
        self.setup_ui()
        self.result_frame = None

    def setup_ui(self):
        # Frame Datos
        datos_frame = tk.LabelFrame(self.root, text="Datos", padx=10, pady=10)
        datos_frame.pack(fill="x", padx=10, pady=5)

        # Input de datos
        tk.Label(datos_frame, text="Input de datos:").grid(row=0, column=0, sticky="w")
        self.data_entry = tk.Entry(datos_frame)
        self.data_entry.grid(row=0, column=1, sticky="we", padx=5)
        tk.Button(datos_frame, text="Seleccionar", command=self.load_file).grid(row=0, column=2, padx=5)
        datos_frame.grid_columnconfigure(1, weight=1)

        # Selección de algoritmo
        tk.Label(datos_frame, text="Algoritmo:").grid(row=1, column=0, sticky="w", pady=(10, 0))
        self.algo_combo = ttk.Combobox(datos_frame, values=["SA-GA", "GRASP"], state="readonly")
        self.algo_combo.grid(row=1, column=1, sticky="we", padx=5, pady=(10, 0))
        self.algo_combo.bind("<<ComboboxSelected>>", self.update_params)

        # Frame Parámetros
        self.params_frame = tk.LabelFrame(self.root, text="Parámetros")
        self.params_frame.pack(fill="both", expand=True, padx=10, pady=5)

        # Botón Ejecutar
        tk.Button(self.root, text="Ejecutar", width=20, command=self.execute).pack(pady=10)

    def clear_params(self):
        for widget in self.params_frame.winfo_children():
            widget.destroy()

    def update_params(self, event=None):
        self.clear_params()
        self.param_entries = {}
        algo = self.algo_combo.get()

        args_config = saga_args if algo == "SA-GA" else grasp_arg

        for i, param in enumerate(args_config):
            label = param["nombreParametro"]
            variable = param["nombreVariable"]
            default = param["defaultValue"]

            tk.Label(self.params_frame, text=label + ":").grid(row=i, column=0, sticky="e", pady=2, padx=5)
            entry = tk.Entry(self.params_frame)
            entry.insert(0, default)
            entry.grid(row=i, column=1, sticky="w", pady=2, padx=5)
            self.param_entries[variable] = entry

    def load_file(self):
        path = filedialog.askopenfilename(title="Seleccionar archivo de datos")
        if path:
            self.data_entry.delete(0, tk.END)
            self.data_entry.insert(0, path)

    def execute(self):
        algo = self.algo_combo.get()
        data = self.data_entry.get()
        if not data or not algo:
            messagebox.showwarning("Faltan datos", "Complete todos los campos antes de ejecutar el algoritmo.")
            return

        exec_path = "../main_algorithms/dist/Debug/MinGW-Windows/main_algorithms.exe"
        params = {label: entry.get() for label, entry in self.param_entries.items()}
        args = [exec_path, "--input", data, "--algo", algo]
        for k, v in params.items():
            args += [f"--{k.lower().replace(' ', '_')}", v]

        try:
            self.root.config(cursor="watch")
            self.root.update()

            # Ejecutar el algoritmo
            result = subprocess.run(args, capture_output=True, text=True, check=True)

            self.root.config(cursor="")
            output = result.stdout.strip()
            print(output)

            self.last_execution_params = {
                "algo": algo,
                **params
            }

            self.show_result_screen()
        except subprocess.CalledProcessError as e:
            self.root.config(cursor="")
            messagebox.showerror("Error de ejecución", f"Error ejecutando el algoritmo:\n{e.stderr}")

        except FileNotFoundError:
            self.root.config(cursor="")
            messagebox.showerror("Archivo no encontrado", f"No se encontró el ejecutable en la ruta: {exec_path}")

    def show_result_screen(self):
        if self.result_frame:
            self.result_frame.destroy()

        self.result_frame = tk.Frame(self.root)
        self.root.geometry("700x600")
        self.result_frame.pack(fill="both", expand=True)

        btn_volver = tk.Button(self.result_frame, text="Volver", command=self.back_to_main)
        btn_volver.place(relx=1.0, x=-10, y=10, anchor="ne")

        try:
            df = pd.read_csv("../output/output_dispatches.csv")
            grouped = df.groupby("dispatch_id")
            self.dispatches = []
            for dispatch_id, group in grouped:
                print(f"Procesando despacho {dispatch_id} con {len(group)} entregas")
                first = group.iloc[0]
                blocks = group[["block_id", "delivery_id", "x", "y", "z", "lx", "ly", "lz", "fragility"]].to_dict(orient="records")

                self.dispatches.append({
                    "dispatch_id": dispatch_id,
                    "truck_id": first["truck_id"],
                    "truck_l": first["truck_l"],
                    "truck_w": first["truck_w"],
                    "truck_h": first["truck_h"],
                    "date": first["date"],
                    "slot_start": first["slot_start"],
                    "slot_end": first["slot_end"],
                    "route_id": first["route_id"],
                    "route_string": first["route_string"],
                    "deliveries": sorted(group["delivery_id"].unique().tolist()),
                    "blocks": blocks
                })

            tk.Label(self.result_frame, text="Configuración", font=("Arial", 12, "bold")).pack(anchor="w", padx=10, pady=(10, 0))
            if hasattr(self, 'last_execution_params'):
                for k, v in self.last_execution_params.items():
                    tk.Label(self.result_frame, text=f"{k}: {v}", anchor="w").pack(anchor="w", padx=20)

            tk.Label(self.result_frame, text="Resultado", font=("Arial", 12, "bold")).pack(anchor="w", padx=10, pady=(10, 0))
            tk.Label(self.result_frame, text="El algoritmo ejecutado obtuvo una solución con fitness estimado (a insertar)", anchor="w").pack(anchor="w", padx=10)

            tk.Label(self.result_frame, text="Entregas", font=("Arial", 12, "bold")).pack(anchor="w", padx=10, pady=(10, 0))

            # Canvas con scrollbar
            scroll_canvas = tk.Canvas(self.result_frame, height=400)
            scrollbar = tk.Scrollbar(self.result_frame, orient="vertical", command=scroll_canvas.yview)
            scroll_canvas.configure(yscrollcommand=scrollbar.set)

            scrollbar.pack(side="right", fill="y")
            scroll_canvas.pack(side="left", fill="both", expand=True)

            scroll_frame = tk.Frame(scroll_canvas)
            scroll_window = scroll_canvas.create_window((0, 0), window=scroll_frame, anchor="nw")

            def resize_scroll_frame(event):
                scroll_canvas.itemconfig(scroll_window, width=event.width)
            scroll_canvas.bind("<Configure>", resize_scroll_frame)

            def configure_scroll_region(event):
                scroll_canvas.configure(scrollregion=scroll_canvas.bbox("all"))
            scroll_frame.bind("<Configure>", configure_scroll_region)

            def on_mousewheel(event):
                scroll_canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")
            scroll_canvas.bind_all("<MouseWheel>", on_mousewheel)

            # Agregar entregas al scroll_frame
            for d in self.dispatches:
                frame = tk.LabelFrame(scroll_frame, text=f"Entrega #{d['dispatch_id']}", padx=5, pady=5)
                frame.pack(fill="x", expand=True, padx=10, pady=8)

                # Contenedor principal para columnas
                content = tk.Frame(frame)
                content.pack(fill="x", expand=True, pady=(5, 0))

                # Botón 'Ver bloques' alineado arriba a la derecha del frame de la entrega
                btn_bloques = tk.Button(
                    frame,
                    text="Ver bloques",
                    width=12,
                    command=lambda dispatch=d: visualize_dispatch(dispatch)
                )
                btn_bloques.place(relx=1.0, x=-10, y=0, anchor="ne")

                # Usar grid para asegurar misma altura y ancho proporcional
                content.columnconfigure(0, weight=1, uniform="col")
                content.columnconfigure(1, weight=1, uniform="col")

                col_left = tk.Frame(content)
                col_left.grid(row=0, column=0, sticky="nsew", padx=(5, 10))

                col_right = tk.Frame(content)
                col_right.grid(row=0, column=1, sticky="nsew", padx=(10, 5))

                # Contenido columna izquierda
                tk.Label(col_left, text=f"ID de unidad de transporte: {d['truck_id']}", anchor="w").pack(anchor="w", pady=2)
                tk.Label(col_left, text=f"Fecha: {d['date']}", anchor="w").pack(anchor="w", pady=2)
                tk.Label(col_left, text=f"Hora salida/llegada: {d['slot_start']} - {d['slot_end']}", anchor="w").pack(anchor="w", pady=2)

                # Contenido columna derecha
                tk.Label(col_right, text=f"ID de ruta: {d['route_id']}", anchor="w").pack(anchor="w", pady=2)
                tk.Label(col_right, text=f"Ruta: {d['route_string']}", anchor="w").pack(anchor="w", pady=2)
                tk.Label(col_right, text=f"IDs de entregas: {', '.join(map(str, d['deliveries']))}", anchor="w").pack(anchor="w", pady=2)

        except Exception as e:
            tk.Label(self.result_frame, text=f"Error cargando resultados: {e}", fg="red").pack(padx=10, pady=10)

        self.result_frame.pack(fill="both", expand=True)
        self.hide_main_screen()


    def back_to_main(self):
        if self.result_frame:
            self.result_frame.destroy()
            self.result_frame = None
        self.root.geometry("450x300")
        self.show_main_screen()

    def hide_main_screen(self):
        for widget in self.root.winfo_children():
            if widget not in [self.result_frame]:
                widget.pack_forget()

    def show_main_screen(self):
        self.setup_ui()

if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()
