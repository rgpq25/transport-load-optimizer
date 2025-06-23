SubProceso ubicarCapa(sp, e, espaciosOut)
	ox <- e.originX
	oy <- e.originY
	oz <- e.originZ
	L <- e.length
	W <- e.width
	H <- e.height

	layerL <- sp.footprintLength
	layerW <- sp.footprintWidth
	layerH <- sp.layerHeight

	// Espacio residual a la derecha (eje X)
	remLx <- L - layerL
	Si remLx > 0.000001 Entonces
		Agregar espacio (ox + layerL, oy, oz, remLx, layerW, layerH) a espaciosOut
	FinSi

	// Espacio residual al frente (eje Y)
	remWy <- W - layerW
	Si remWy > 0.000001 Entonces
		Agregar espacio (ox, oy + layerW, oz, L, remWy, layerH) a espaciosOut
	FinSi
FinSubProceso

