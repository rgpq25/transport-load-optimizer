Proceso PlanificadorPrincipal
    Definir tracker como GlobalExecutionTracker
    Definir finalDispatches como lista de Dispatch
    Definir uniqueDueDates <- obtenerFechasUnicasDeEntrega()
    Definir deliveries <- obtenerTodosLosRepartos()
    Para cada fechaEntrega en uniqueDueDates hacer
        Para cada ruta en listaDeRutas hacer
            repartosRuta <- filtrarRepartosPorFechaYRuta(deliveries, fechaEntrega, ruta)
            Para cada turno en ["morning", "afternoon"] hacer
                repartosTurno <- filtrarRepartosPorTurno(repartosRuta, turno)
                Si repartosTurno está vacío Entonces
                    Continuar
                FinSi
				
                timeSlots <- generarFranjasHorarias(turno, ruta)
                Para cada slot en timeSlots hacer
                    repartosDisponibles <- filtrarRepartosNoAtendidos(repartosTurno, tracker)
                    repartosValidos <- filtrarRepartosConBloquesDisponibles(repartosDisponibles, tracker)
                    Si repartosValidos está vacío Entonces
                        Continuar
                    FinSi
					
                    bloques <- obtenerBloquesDe(repartosValidos)
                    vehiculosDisponibles <- filtrarVehiculosDisponibles(slot, fechaEntrega, tracker)
                    Si vehiculosDisponibles está vacío Entonces
                        Continuar
                    FinSi
					
                    // Ejecutar optimización para este subproblema
                    optimizer <- SAGAOptimizer(repartosValidos, bloques, vehiculosDisponibles, 
					ruta, slot, T_init, T_min, alpha, N)
                    mejorSolucion <- optimizer.run()
					
                    // Registrar resultado en tracker
                    tracker.registrarSolucion(mejorSolucion, repartosValidos, bloques, 
					vehiculosDisponibles, slot, fechaEntrega)
					
                    // Construir despachos a partir de la mejor solución
                    despachos <- construirDespachos(mejorSolucion, repartosValidos, 
					vehiculosDisponibles, ruta, slot, fechaEntrega)
                    Agregar despachos a finalDispatches
                FinPara
            FinPara
        FinPara
    FinPara
FinProceso
