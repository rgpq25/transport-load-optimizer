SubProceso generarPoblacionInicial(deliveries, bloques, vehiculos, ruta, slot, N) 
		Definir population como lista vacía
		numDeliveries <- tamaño de deliveries
		numBlocks <- tamaño de bloques
		numVehicles <- tamaño de vehiculos
		
		Para i desde 1 hasta N hacer
			// Asignación aleatoria de repartos a vehículos
			Definir deliveryAssignments como lista vacía
			Para d desde 1 hasta numDeliveries hacer
				vehiculoAsignado <- valorAleatorioEntre(-1, numVehicles - 1)
				Agregar vehiculoAsignado a deliveryAssignments
			FinPara
			
			// Asignación aleatoria de orientaciones de bloques
			Definir blockOrientations como lista vacía
			Para b desde 1 hasta numBlocks hacer
				orientacion <- valorAleatorioEntre(0, 1)
				Agregar orientacion a blockOrientations
			FinPara
			
			cromosoma <- nuevo Cromosoma(deliveryAssignments, blockOrientations)
			Agregar cromosoma a population
		FinPara
		Retornar population
FinSubProceso
