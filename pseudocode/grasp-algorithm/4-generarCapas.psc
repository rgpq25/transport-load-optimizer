SubProceso  generarCapas(e, d)
    Definir blocks <- bloques de d
    Definir candidates como lista vac�a
	
    Si blocks est� vac�o Entonces
        Retornar candidates
    FinSi
	
    // Opci�n 1: orientaci�n original (0)
    sumLen <- suma de longitudes de blocks
    maxWid <- m�ximo de anchos de blocks
    maxHei <- m�ximo de alturas de blocks
	
    Si sumLen <= e.length Y maxWid <= e.width Entonces
        orients <- lista de ceros del tama�o de blocks
        Agregar nueva capa (d, blocks, orients, sumLen, maxWid, maxHei) a candidates
    FinSi
	
    // Opci�n 2: orientaci�n rotada (1)
    sumLen1 <- suma de anchos de blocks
    maxWid1 <- m�ximo de longitudes de blocks
	
    Si sumLen1 <= e.length Y maxWid1 <= e.width Entonces
        orients <- lista de unos del tama�o de blocks
        Agregar nueva capa (d, blocks, orients, sumLen1, maxWid1, maxHei) a candidates
    FinSi
	
    Retornar candidates
FinSubProceso

