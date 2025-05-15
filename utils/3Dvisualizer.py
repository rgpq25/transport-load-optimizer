import pandas as pd
import plotly.graph_objects as go
import random

def visualize_dispatch(dispatch_df, dispatch_id):
    df = dispatch_df[dispatch_df["dispatch_id"] == dispatch_id]

    if df.empty:
        print(f"No data for dispatch {dispatch_id}")
        return

    # Get truck dimensions from the first row
    truck_l = df["truck_l"].iloc[0]
    truck_w = df["truck_w"].iloc[0]
    truck_h = df["truck_h"].iloc[0]

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
    for idx, row in df.iterrows():
        x, y, z = row['x'], row['y'], row['z']
        lx, ly, lz = row['lx'], row['ly'], row['lz']
        block_id = row['block_id']

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
            hovertext=f"Block {block_id}<br>Pos: ({x}, {y}, {z})<br>Size: ({lx}, {ly}, {lz})",
            hoverinfo='text'
        ))

    fig.update_layout(
        title=f"Dispatch {dispatch_id} – 3D Load Plan",
        scene=dict(
            xaxis=dict(title='Length', range=[0, truck_l]),
            yaxis=dict(title='Width', range=[0, truck_w]),
            zaxis=dict(title='Height', range=[0, truck_h]),
        ),
        margin=dict(l=0, r=0, b=0, t=40),
        showlegend=False
    )

    fig.show()

# === MAIN ===

csv_path = "../output/output_dispatches.csv"
dispatches = pd.read_csv(csv_path)

print("Dispatch IDs:", sorted(dispatches['dispatch_id'].unique()))
selected_id = int(input("Enter dispatch_id to visualize: "))
visualize_dispatch(dispatches, selected_id)
