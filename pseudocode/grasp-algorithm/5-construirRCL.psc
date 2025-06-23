SubProceso construirRCL(layers, alpha)
    Si layers est� vac�o Entonces
        Retornar lista vac�a
    FinSi
	
    sorted <- copiar layers
    Ordenar sorted por volumen en orden descendente
	
    rclSize <- redondearArriba(alpha * tama�o de sorted)
    rclSize <- m�ximo entre 1 y rclSize
    rclSize <- m�nimo entre rclSize y tama�o de sorted
	
    rcl <- primeros rclSize elementos de sorted
    Retornar rcl
FinSubProceso
