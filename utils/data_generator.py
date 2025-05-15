import random
from datetime import datetime, timedelta

random.seed(42)

NUM_CLIENTS = 20
NUM_BLOCKS = 300
NUM_VEHICLES = 8
NUM_ORDERS = 100
MAX_DELIVERIES_PER_ORDER = 3
MAX_BLOCKS_PER_DELIVERY = 5
NUM_ROUTES = 6
TODAY = datetime(2024, 9, 21)

def generate_random_date(start_date, end_date):
    delta = end_date - start_date
    random_days = random.randint(0, delta.days)
    return (start_date + timedelta(days=random_days)).strftime("%Y-%m-%d")



def generate_clients():
    return [(i + 1, f"Client {i + 1}") for i in range(NUM_CLIENTS)]


def generate_blocks():
    blocks = []
    for i in range(1, NUM_BLOCKS + 1):
        h = round(random.uniform(0.5, 1.2), 2)
        w = round(random.uniform(0.5, 1.2), 2)
        l = round(random.uniform(0.5, 1.2), 2)
        weight = round(random.uniform(30, 60), 1)
        fragility = round(random.uniform(1, 10), 1)
        blocks.append((i, h, w, l, weight, fragility))
    return blocks


def generate_vehicles():
    vehicles = []
    for i in range(1, NUM_VEHICLES + 1):
        h = round(random.uniform(2.0, 3.0), 2)
        w = round(random.uniform(2.0, 2.5), 2)
        l = round(random.uniform(5.0, 7.0), 2)
        maxW = random.randint(1800, 4000)
        vehicles.append((i, h, w, l, maxW, 0))
    return vehicles


def generate_routes(clients, orders):
    routes = []
    client_ids_with_orders = set(oid[1] for oid in orders)
    client_ids = [c[0] for c in clients if c[0] != 1]

    # Inicialmente incluir explícitamente cada cliente con orden en alguna ruta
    shuffled_clients = list(client_ids_with_orders)
    random.shuffle(shuffled_clients)

    clients_per_route = max(3, len(shuffled_clients) // NUM_ROUTES + 1)
    route_id = 1

    while shuffled_clients:
        current_clients = shuffled_clients[:clients_per_route]
        shuffled_clients = shuffled_clients[clients_per_route:]

        route_points = [(cid, random.randint(5, 20)) for cid in current_clients]
        route_points.append((1, random.randint(5, 20)))  # Return to dispatch center
        routes.append((route_id, route_points))
        route_id += 1

    # Rellenar hasta NUM_ROUTES con rutas aleatorias
    while len(routes) < NUM_ROUTES:
        random_clients = random.sample(client_ids, k=random.randint(3, 6))
        route_points = [(cid, random.randint(5, 20)) for cid in random_clients]
        route_points.append((1, random.randint(5, 20)))
        routes.append((route_id, route_points))
        route_id += 1

    return routes



def generate_orders(clients, available_blocks):
    orders = []
    used_blocks = set()
    delivery_id_counter = 1
    block_idx = 0
    for order_id in range(1, NUM_ORDERS + 1):
        client = random.choice(clients[1:])  # avoid dispatch center
        client_id = client[0]
        priority = random.randint(1, 10)
        num_deliveries = random.randint(1, MAX_DELIVERIES_PER_ORDER)
        deliveries = []

        for _ in range(num_deliveries):
            shift = random.choice(["morning", "afternoon", "null"])

            start = datetime(2024, 9, 21)
            end = datetime(2024, 10, 5)
            due_date = generate_random_date(start, end)

            num_blocks = random.randint(1, MAX_BLOCKS_PER_DELIVERY)
            blocks = []

            for _ in range(num_blocks):
                if block_idx >= len(available_blocks):
                    break
                blocks.append(available_blocks[block_idx])
                used_blocks.add(available_blocks[block_idx])
                block_idx += 1

            if blocks:
                deliveries.append((delivery_id_counter, shift, due_date, blocks))
                delivery_id_counter += 1

        if deliveries:
            orders.append((order_id, client_id, priority, deliveries))
    return orders


def save_dataset(clients, blocks, vehicles, routes, orders, filepath):
    with open(filepath, "w") as f:
        f.write(f"@clients {len(clients)}\n")
        for cid, name in clients:
            f.write(f"{cid} {name}\n")

        f.write(f"@blocks {len(blocks)}\n")
        for b in blocks:
            f.write(f"{b[0]} {b[1]} {b[2]} {b[3]} {b[4]} {b[5]}\n")

        f.write(f"@transport_units {len(vehicles)}\n")
        for v in vehicles:
            f.write(f"{v[0]} {v[1]} {v[2]} {v[3]} {v[4]} {v[5]}\n")

        f.write(f"@routes {len(routes)}\n")
        for rid, points in routes:
            f.write(f"{rid} {len(points)}\n")
            for cid, t in points:
                f.write(f"{cid} {t}\n")

        f.write(f"@orders {len(orders)}\n")
        for oid, cid, prio, dels in orders:
            f.write(f"{oid} {cid} {prio} {len(dels)}\n")
            for did, shift, ddate, blist in dels:
                f.write(f"{did} {shift} {ddate} {len(blist)}\n")
                for b in blist:
                    f.write(f"{b}\n")
    print(f"Dataset saved to {filepath}")


# GENERATE
clients = generate_clients()
blocks = generate_blocks()
vehicles = generate_vehicles()
available_block_ids = [b[0] for b in blocks]
orders = generate_orders(clients, available_block_ids)
routes = generate_routes(clients, orders)  # Ahora depende de los pedidos

# SAVE
save_dataset(clients, blocks, vehicles, routes, orders, "../input/input_large.txt")
