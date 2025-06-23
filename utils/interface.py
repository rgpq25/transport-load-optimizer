import tkinter as tk
from tkinter import filedialog, ttk, messagebox
import plotly.graph_objects as go
import numpy as np

def generate_mock_blocks(delivery_id):
    np.random.seed(delivery_id)
    blocks = []
    for i in range(np.random.randint(3, 10)):
        length, width, height = np.random.randint(1, 5, 3)
        x, y, z = np.random.randint(0, 10, 3)
        blocks.append({
            'id': f'B{i}',
            'x': x, 'y': y, 'z': z,
            'dx': length, 'dy': width, 'dz': height
        })
    return blocks

def plot_blocks(blocks):
    fig = go.Figure()
    for block in blocks:
        fig.add_trace(go.Mesh3d(
            x=[block['x'], block['x']+block['dx'], block['x']+block['dx'], block['x'], block['x'], block['x']+block['dx'], block['x']+block['dx'], block['x']],
            y=[block['y'], block['y'], block['y']+block['dy'], block['y']+block['dy'], block['y'], block['y'], block['y']+block['dy'], block['y']+block['dy']],
            z=[block['z'], block['z'], block['z'], block['z'], block['z']+block['dz'], block['z']+block['dz'], block['z']+block['dz'], block['z']+block['dz']],
            i=[0, 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 6],
            j=[1, 2, 3, 5, 6, 3, 7, 0, 5, 6, 7, 0],
            k=[2, 3, 0, 6, 7, 7, 0, 1, 6, 7, 4, 5],
            opacity=0.5,
            color=np.random.choice(['red', 'green', 'blue', 'orange', 'purple'])
        ))
    fig.update_layout(scene=dict(
        xaxis_title='X',
        yaxis_title='Y',
        zaxis_title='Z'
    ))
    fig.show()

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("Interfaz de Optimización de Carga")
        self.root.geometry("600x400")

        self.file_path = None
        self.selected_algorithm = tk.StringVar(value="SA-GA")
        self.deliveries = [1, 2, 3, 4, 5]

        self.setup_widgets()

    def setup_widgets(self):
        tk.Label(self.root, text="Seleccione archivo de datos:").pack(pady=5)
        tk.Button(self.root, text="Cargar archivo", command=self.load_file).pack(pady=5)

        tk.Label(self.root, text="Algoritmo:").pack()
        ttk.Combobox(self.root, textvariable=self.selected_algorithm, values=["SA-GA", "GRASP"]).pack(pady=5)

        tk.Button(self.root, text="Iniciar Optimización", command=self.run_algorithm).pack(pady=15)

        tk.Label(self.root, text="Ver Entrega ID:").pack()
        self.delivery_combo = ttk.Combobox(self.root, values=self.deliveries)
        self.delivery_combo.pack(pady=5)

        tk.Button(self.root, text="Visualizar Entrega", command=self.view_delivery).pack(pady=10)

    def load_file(self):
        self.file_path = filedialog.askopenfilename(title="Seleccionar archivo")
        if self.file_path:
            messagebox.showinfo("Archivo cargado", f"Archivo cargado:\n{self.file_path}")

    def run_algorithm(self):
        algo = self.selected_algorithm.get()
        messagebox.showinfo("Ejecutando", f"Ejecutando algoritmo: {algo}\n(Implementación aún en progreso)")

    def view_delivery(self):
        delivery_id = self.delivery_combo.get()
        if not delivery_id:
            messagebox.showwarning("Sin selección", "Seleccione una entrega para visualizar.")
            return
        blocks = generate_mock_blocks(int(delivery_id))
        plot_blocks(blocks)

root = tk.Tk()
app = App(root)
root.mainloop()
