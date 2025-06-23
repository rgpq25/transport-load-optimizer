Proceso PlanificadorPrincipalGRASP()
	// Considerando que este fragmento se usa en la misma condición
	// que el presentado para el algoritmo SA-GA (datos filtrados).
	
	optimizer <- GRASPOptimizer(
		finalDeliveries, 
		blocksForThisBatch, 
		availableVehicles, 
		route, 
		ts, 
		dueDate, 
		graspIterations, 
		Kpercent, 
		alphaSet
	)
	vehiclePatterns <- optimizer.run()
	
	tracker.registrarSolucionGRASP(
		vehiclePatterns, 
		finalDeliveries, 
		blocksForThisBatch, 
		availableVehicles, 
		ts, 
		dueDate
	)
	
	dispatches <- construirDespachosDesdeGRASP(
		vehiclePatterns, 
		finalDeliveries, 
		availableVehicles, 
		route, 
		ts, 
		dueDate
	)
	Agregar dispatches a finalDispatches

FinProceso
