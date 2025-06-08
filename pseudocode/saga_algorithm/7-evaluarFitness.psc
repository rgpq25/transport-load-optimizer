SubProceso evaluarFitness(cromosoma)
    assignments <- obtenerDeliveryAssignments(cromosoma)
    orientaciones <- obtenerBlockOrientations(cromosoma)
	
    mapaBloquesPorVehiculo <- mapa vac�o
    volumenPorVehiculo <- mapa vac�o
    pesoPorVehiculo <- mapa vac�o
    vehiculosUsados <- conjunto vac�o
	
    entregasAtendidas <- 0
    prioridadTotal <- 0
    prioridadAtendida <- 0
	
    Para i desde 1 hasta tama�o de assignments hacer
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
        bin <- crearContenedor3D(tama�o del veh�culo v)
        orientacionesVehiculo <- obtenerOrientacionesParaVehiculo(cromosoma, v)
        bloques <- mapaBloquesPorVehiculo[v]
		
        Para i desde 1 hasta tama�o de bloques hacer
            si bin.intentarColocarBloque(bloques[i], orientacionesVehiculo[i]) == Falso Entonces
                Retornar 0.0  // Soluci�n inviable
            FinSi
        FinPara
    FinPara
	
    utilizaci�nTotal <- 0.0
    penalizaci�nPeso <- 0.0
	
    Para cada v en vehiculosUsados hacer
        volumenMax <- volumenMaximoDelVehiculo(v)
        pesoMax <- pesoMaximoDelVehiculo(v)
		
        volumenUsado <- volumenPorVehiculo[v]
        pesoUsado <- pesoPorVehiculo[v]
		
        utilizaci�n <- volumenUsado / volumenMax
        utilizaci�nTotal <- utilizaci�nTotal + utilizaci�n
		
        si pesoUsado > pesoMax Entonces
            penalizaci�nPeso <- penalizaci�nPeso + (pesoUsado - pesoMax) * 10.0
        FinSi
    FinPara
	
    promedioUtilizaci�n <- utilizaci�nTotal / tama�o de vehiculosUsados
    ratioEntregas <- entregasAtendidas / total de entregas
    ratioPrioridad <- prioridadAtendida / prioridadTotal
    scoreVehiculos <- 1 - (tama�o de vehiculosUsados / total de veh�culos disponibles)
	
    A <- 0.5  // Minimizar uso de veh�culos
    B <- 0.5  // Maximizar uso de espacio
    C <- 1.0  // Penalizaci�n por sobrepeso
    D <- 1.0  // Bonus por atender entregas
    E <- 1.0  // Bonus por cubrir prioridades
	
    fitness <- A * scoreVehiculos 
			+ B * promedioUtilizaci�n 
			- C * penalizaci�nPeso 
			+ D * ratioEntregas 
			+ E * ratioPrioridad
	
    Retornar fitness
FinSubProceso