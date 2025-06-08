SubProceso seleccionarPadre(population)
		torneo <- 3
		mejor <- individuoAleatorio(population)
		
		Para i desde 2 hasta torneo hacer
			retador <- individuoAleatorio(population)
			Si fitness de retador > fitness de mejor Entonces
				mejor <- retador
			FinSi
		FinPara
		
		Retornar mejor
FinSubProceso
