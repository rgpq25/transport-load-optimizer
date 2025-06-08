SubProceso crossover(p1, p2) 
	d1 <- obtenerDeliveryAssignments(p1)
	d2 <- obtenerDeliveryAssignments(p2)
	o1 <- obtenerBlockOrientations(p1)
	o2 <- obtenerBlockOrientations(p2)

	Definir childDeliveries como lista vacía
	Definir childOrientations como lista vacía

	Para i desde 1 hasta tamaño de d1 hacer
		gene <- si valorAleatorioEntre(0, 1) = 0 entonces d1[i] sino d2[i]
		Agregar gene a childDeliveries
	FinPara

	Para i desde 1 hasta tamaño de o1 hacer
		gene <- si valorAleatorioEntre(0, 1) = 0 entonces o1[i] sino o2[i]
		Agregar gene a childOrientations
	FinPara

	Retornar nuevo Cromosoma(childDeliveries, childOrientations)
FinSubProceso
