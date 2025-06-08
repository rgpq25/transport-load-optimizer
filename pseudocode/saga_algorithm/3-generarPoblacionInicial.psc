SubProceso generarPoblacionInicial(deliveries, bloques, vehiculos, ruta, slot, N) 
		Definir population como lista vac�a
		numDeliveries <- tama�o de deliveries
		numBlocks <- tama�o de bloques
		numVehicles <- tama�o de vehiculos
		
		Para i desde 1 hasta N hacer
			// Asignaci�n aleatoria de repartos a veh�culos
			Definir deliveryAssignments como lista vac�a
			Para d desde 1 hasta numDeliveries hacer
				vehiculoAsignado <- valorAleatorioEntre(-1, numVehicles - 1)
				Agregar vehiculoAsignado a deliveryAssignments
			FinPara
			
			// Asignaci�n aleatoria de orientaciones de bloques
			Definir blockOrientations como lista vac�a
			Para b desde 1 hasta numBlocks hacer
				orientacion <- valorAleatorioEntre(0, 1)
				Agregar orientacion a blockOrientations
			FinPara
			
			cromosoma <- nuevo Cromosoma(deliveryAssignments, blockOrientations)
			Agregar cromosoma a population
		FinPara
		Retornar population
FinSubProceso
