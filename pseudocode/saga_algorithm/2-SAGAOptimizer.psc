SubProceso SAGAOptimizer(deliveries, bloques, vehiculos, ruta, slot, T_init, T_min, alpha, N)
		Definir population como lista de Cromosomas
		population <- generarPoblacionInicial(deliveries, bloques, vehiculos, ruta, slot, N)
		Para cada cromosoma en population hacer
			fitness <- evaluarFitness(cromosoma)
			asignarFitness(cromosoma, fitness)
		FinPara
		
		mejor <- population[0]
		Para cada cromosoma en population hacer
			Si fitness de cromosoma > fitness de mejor Entonces
				mejor <- cromosoma
			FinSi
		FinPara
		
		T <- T_init
		Mientras T > T_min hacer
			Definir nuevaPoblacion como lista vacía
			Para i desde 1 hasta N hacer
				padre1 <- seleccionarPadre(population)
				padre2 <- seleccionarPadre(population)
				hijo <- crossover(padre1, padre2)
				mutar(hijo)
				asignarFitness(hijo, evaluarFitness(hijo))
				
				delta <- fitness de padre1 - fitness de hijo
				Si delta < 0 Entonces
					agregar hijo a nuevaPoblacion
				Sino
					probabilidad <- exp(-delta / T)
					aleatorio <- valorAleatorioEntre0y1()
					Si aleatorio < probabilidad Entonces
						agregar hijo a nuevaPoblacion
					Sino
						agregar padre1 a nuevaPoblacion
					FinSi
				FinSi
				
				Si fitness de hijo > fitness de mejor Entonces
					mejor <- hijo
				FinSi
			FinPara
			population <- nuevaPoblacion
			T <- T * alpha
		FinMientras
		Retornar mejor
FinSubProceso
