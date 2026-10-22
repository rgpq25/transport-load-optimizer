SubProceso evaluarFitness(cromosoma)
    assignments <- obtenerDeliveryAssignments(cromosoma)
    orientaciones <- obtenerBlockOrientations(cromosoma)
	
    mapaBloquesPorVehiculo <- mapa vacío
    volumenPorVehiculo <- mapa vacío
    pesoPorVehiculo <- mapa vacío
    vehiculosUsados <- conjunto vacío
	
    entregasAtendidas <- 0
    prioridadTotal <- 0
    prioridadAtendida <- 0
	
    Para i desde 1 hasta tamaño de assignments hacer
        prioridad <- obtenerPrioridad(entrega[i])
        prioridadTotal <- prioridadTotal + prioridad
		
        si assignments[i] <> -1 Entonces
            entregasAtendidas <- entregasAtendidas + 1
            prioridadAtendida <- prioridadAtendida + prioridad
			
            bloques <- obtenerBloques(entrega[i])
            Para cada bloque en bloques hacer
                Agregar bloque a mapaBloquesPorVehiculo[assignments[i]]
                volumenPorVehiculo[assignments[i]] <- volumenPorVehiculo[assignments[i]] + volumen(bloque)
                pesoPorVehiculo[assignments[i]] <- pesoPorVehiculo[assignments[i]] + peso(bloque)
            FinPara
			
            Agregar assignments[i] a vehiculosUsados
        FinSi
    FinPara
	
    Para cada v en vehiculosUsados hacer
        bin <- crearContenedor3D(tamaño del vehículo v)
        orientacionesVehiculo <- obtenerOrientacionesParaVehiculo(cromosoma, v)
        bloques <- mapaBloquesPorVehiculo[v]
		
        Para i desde 1 hasta tamaño de bloques hacer
            si bin.intentarColocarBloque(bloques[i], orientacionesVehiculo[i]) == Falso Entonces
                Retornar 0.0  // Solución inviable
            FinSi
        FinPara
    FinPara
	
    utilizaciónTotal <- 0.0
    penalizaciónPeso <- 0.0
	
    Para cada v en vehiculosUsados hacer
        volumenMax <- volumenMaximoDelVehiculo(v)
        pesoMax <- pesoMaximoDelVehiculo(v)
		
        volumenUsado <- volumenPorVehiculo[v]
        pesoUsado <- pesoPorVehiculo[v]
		
        utilización <- volumenUsado / volumenMax
        utilizaciónTotal <- utilizaciónTotal + utilización
		
        si pesoUsado > pesoMax Entonces
            penalizaciónPeso <- penalizaciónPeso + (pesoUsado - pesoMax) * 10.0
        FinSi
    FinPara
	
    promedioUtilización <- utilizaciónTotal / tamaño de vehiculosUsados
    ratioEntregas <- entregasAtendidas / total de entregas
    ratioPrioridad <- prioridadAtendida / prioridadTotal
    scoreVehiculos <- 1 - (tamaño de vehiculosUsados / total de vehículos disponibles)
	
    A <- -5
    B <- 30
    C <- 70
    D <- 5
    E <- -1000
	
    fitness <- A * scoreVehiculos 
			+ B * promedioUtilización 
			+ C * ratioEntregas
			+ D * ratioPrioridad 
			+ E * penalizaciónPeso
	
    Retornar fitness
FinSubProceso