Proceso GRASPOptimization()
    Definir vehiclePatterns como lista de VehiclePattern
    Definir LPveh <- copia de LP
    Definir Vveh <- copia de V
	
    Mientras LPveh no est� vac�o Y Vveh no est� vac�o hacer
        targetVolume <- sumar vol�menes de todos los bloques en LPveh
        ve <- veh�culo de Vveh cuyo volumen m�s se aproxime a targetVolume
		
        Definir BestVehiclePattern con ve
        localsearch <- FALSO
		
        E <- generarEspaciosIniciales(ve)
		
        Para iterador desde 0 hasta maxIter - 1 hacer
            Si localsearch = FALSO Entonces
                E <- generarEspaciosIniciales(ve)
            FinSi
			
            LPcopy <- copia de LPveh
            Definir CurrentPattern con ve
			
            Mientras LPcopy no est� vac�o Y E no est� vac�o hacer
                e <- seleccionarEspacioAleatorio(E)
                Remover e de E
				
                layers <- []
                Para cada d en LPcopy hacer
                    capas <- generarCapas(d, e)
                    Agregar capas a layers
                FinPara
				
                Si layers est� vac�o Entonces salir del bucle
					
				Mezclar aleatoriamente layers
					
				Si localsearch = FALSO Entonces
					alpha <- alphaTuner.seleccionar()
					rcl <- construirRCL(layers, alpha)
					rcl <- filtrarCapasPorPeso(rcl, CurrentPattern.currentWeight, ve)
						
					Si rcl est� vac�o Entonces salir del bucle
						sp <- seleccionarAleatorio(rcl)
				Sino
					layers <- filtrarCapasPorPeso(layers, CurrentPattern.currentWeight, ve)
					Si layers est� vac�o Entonces salir del bucle
						sp <- seleccionarMejorAjuste(layers)
				FinSi
							
				spWeight <- pesoTotal(sp.blocks)
				CurrentPattern.currentWeight <- CurrentPattern.currentWeight + spWeight
				Agregar sp a CurrentPattern.layers
				
				newSpaces <- []
				ubicarCapa(sp, e, newSpaces)
				Agregar newSpaces a E
				
				Remover sp.delivery de LPcopy
			FinMientras
						
			Si localsearch = VERDADERO Y CurrentPattern tiene capas Entonces
				removed <- removerPorcentajeK(CurrentPattern)
				Para cada layer en removed hacer
					Agregar layer.delivery a LPcopy
				FinPara
				E <- generarEspaciosIniciales(ve)
			FinSi
						
			Si evaluarFitness({CurrentPattern}) > evaluarFitness({BestVehiclePattern}) Entonces
				BestVehiclePattern <- CurrentPattern
			FinSi
			
			localsearch <- NO localsearch
		FinPara
					
		Agregar BestVehiclePattern a vehiclePatterns
		Para cada layer en BestVehiclePattern.layers hacer
			Remover layer.delivery de LPveh
		FinPara
		Remover ve de Vveh
	FinMientras
				
	Retornar vehiclePatterns
FinProceso
