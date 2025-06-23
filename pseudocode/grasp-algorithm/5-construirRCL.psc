SubProceso construirRCL(layers, alpha)
    Si layers está vacío Entonces
        Retornar lista vacía
    FinSi
	
    sorted <- copiar layers
    Ordenar sorted por volumen en orden descendente
	
    rclSize <- redondearArriba(alpha * tamaño de sorted)
    rclSize <- máximo entre 1 y rclSize
    rclSize <- mínimo entre rclSize y tamaño de sorted
	
    rcl <- primeros rclSize elementos de sorted
    Retornar rcl
FinSubProceso
