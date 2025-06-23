SubProceso  generarCapas(e, d)
    Definir blocks <- bloques de d
    Definir candidates como lista vacía
	
    Si blocks está vacío Entonces
        Retornar candidates
    FinSi
	
    // Opción 1: orientación original (0)
    sumLen <- suma de longitudes de blocks
    maxWid <- máximo de anchos de blocks
    maxHei <- máximo de alturas de blocks
	
    Si sumLen <= e.length Y maxWid <= e.width Entonces
        orients <- lista de ceros del tamaño de blocks
        Agregar nueva capa (d, blocks, orients, sumLen, maxWid, maxHei) a candidates
    FinSi
	
    // Opción 2: orientación rotada (1)
    sumLen1 <- suma de anchos de blocks
    maxWid1 <- máximo de longitudes de blocks
	
    Si sumLen1 <= e.length Y maxWid1 <= e.width Entonces
        orients <- lista de unos del tamaño de blocks
        Agregar nueva capa (d, blocks, orients, sumLen1, maxWid1, maxHei) a candidates
    FinSi
	
    Retornar candidates
FinSubProceso

