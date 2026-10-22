SubProceso mutar(cromosoma, mutationRate)
    deliveries <- obtenerDeliveryAssignments(cromosoma)
    orientations <- obtenerBoxOrientations(cromosoma)
	
    Para i desde 1 hasta tamaño de deliveries hacer
        Si valorAleatorioEntre0y1() < mutationRate Entonces
            nuevoVehiculo <- valorAleatorioEntre(-1, tamaño de vehicles - 1)
            deliveries[i] <- nuevoVehiculo
        FinSi
    FinPara
	
    Para i desde 1 hasta tamaño de orientations hacer
        Si valorAleatorioEntre0y1() < mutationRate Entonces
            orientations[i] <- 1 - orientations[i]
        FinSi
    FinPara
FinSubProceso
