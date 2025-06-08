SubProceso mutar(cromosoma)
    deliveries <- obtenerDeliveryAssignments(cromosoma)
    orientations <- obtenerBoxOrientations(cromosoma)
	
    probMutacionEntregas <- 0.1
    probMutacionOrientaciones <- 0.1
	
    Para i desde 1 hasta tamaño de deliveries hacer
        Si valorAleatorioEntre0y1() < probMutacionEntregas Entonces
            nuevoVehiculo <- valorAleatorioEntre(-1, tamaño de vehicles - 1)
            deliveries[i] <- nuevoVehiculo
        FinSi
    FinPara
	
    Para i desde 1 hasta tamaño de orientations hacer
        Si valorAleatorioEntre0y1() < probMutacionOrientaciones Entonces
            orientations[i] <- 1 - orientations[i]
        FinSi
    FinPara
FinSubProceso
