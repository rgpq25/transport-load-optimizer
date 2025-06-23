SubProceso generarEspaciosIniciales(truck)
    espacioInicial <- MaximalSpace(
		origenX = 0.0,
		origenY = 0.0,
		origenZ = 0.0,
		largo = truck.longitud,
		ancho = truck.ancho,
		alto = truck.altura
    )
    espacios <- lista con espacioInicial
    Retornar espacios
FinSubProceso
