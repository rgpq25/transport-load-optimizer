SubProceso intentarColocarBloque(bloque, orientacion)
    Si orientacion = 0 Entonces
        tamaño <- [largo, ancho, alto] del bloque
    Sino
        tamaño <- [ancho, largo, alto] del bloque
    FinSi
	
    Para cada espacioDisponible en EMS hacer
        posicion <- origen del espacioDisponible
        tamañoEMS <- dimensiones del espacioDisponible
		
        Si tamaño no cabe dentro de tamañoEMS Entonces
            Continuar
        FinSi
		
        Si tamaño no cabe dentro del contenedor en posicion Entonces
            Continuar
        FinSi
		
        Si colisionaConOtrosBloques(posicion, tamaño) Entonces
            Continuar
        FinSi
		
        Si no tieneSoporteValido(posicion, tamaño, fragilidad del bloque) Entonces
            Continuar
        FinSi
		
        nuevoBloque <- crearBloqueColocado(bloque, posicion, tamaño, orientacion)
        añadir nuevoBloque a bloquesColocados
		
        dividirEMS(posicion, tamaño)
        podarEMS()
		
        Retornar Verdadero
    FinPara
	
    Retornar Falso
FinSubProceso
