SubProceso intentarColocarBloque(bloque, orientacion)
    Si orientacion = 0 Entonces
        tama�o <- [largo, ancho, alto] del bloque
    Sino
        tama�o <- [ancho, largo, alto] del bloque
    FinSi
	
    Para cada espacioDisponible en EMS hacer
        posicion <- origen del espacioDisponible
        tama�oEMS <- dimensiones del espacioDisponible
		
        Si tama�o no cabe dentro de tama�oEMS Entonces
            Continuar
        FinSi
		
        Si tama�o no cabe dentro del contenedor en posicion Entonces
            Continuar
        FinSi
		
        Si colisionaConOtrosBloques(posicion, tama�o) Entonces
            Continuar
        FinSi
		
        Si no tieneSoporteValido(posicion, tama�o, fragilidad del bloque) Entonces
            Continuar
        FinSi
		
        nuevoBloque <- crearBloqueColocado(bloque, posicion, tama�o, orientacion)
        a�adir nuevoBloque a bloquesColocados
		
        dividirEMS(posicion, tama�o)
        podarEMS()
		
        Retornar Verdadero
    FinPara
	
    Retornar Falso
FinSubProceso
