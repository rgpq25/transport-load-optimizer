# CLAUDE.md

Este archivo es público (el repositorio de GitHub es público): contiene solo instrucciones de trabajo. El estado del proyecto, los pendientes y las inconsistencias detectadas viven en `documents/ESTADO_ACTUAL.md`, que es privado y no se versiona en Git.

## ⚠️ Prerrequisitos de cada sesión

El **repositorio oficial de documentos** (tesis, anexos, cartas de validación y `ESTADO_ACTUAL.md`) es una carpeta **privada de Google Drive**. La carpeta local `documents/` es solo una **copia de trabajo** de esa carpeta y está ignorada por Git.

Al inicio de cada sesión, antes de cualquier otro trabajo:

1. Pedir al usuario que descargue la última versión de la carpeta de Drive (tesis, anexos y `ESTADO_ACTUAL.md`) a `documents/`.
2. Esperar a que el usuario confirme **explícitamente** que `documents/` (incluido `ESTADO_ACTUAL.md`) es la versión más reciente de Drive. **No iniciar ningún trabajo** hasta tener esa confirmación.
3. Verificar que `documents/` contenga al menos:
   - `ESTADO_ACTUAL.md`
   - la tesis (`20201491_RenzoPinto_RonyCueva_ManuelTupia_EF.docx`)
4. Recién entonces, leer `documents/ESTADO_ACTUAL.md` completo para conocer el estado de los anexos, los pendientes y las inconsistencias registradas.

El `ESTADO_ACTUAL.md` que haya en `documents/` al abrir la sesión **nunca se considera verídico** por sí solo: puede ser una copia antigua. No usar su contenido (ni para responder, ni para planificar, ni para trabajar en anexos) hasta que el usuario confirme que es el más reciente.

Si falta `ESTADO_ACTUAL.md`, no continuar: pedirlo al usuario.

## Al terminar la sesión

1. Actualizar `documents/ESTADO_ACTUAL.md` con lo avanzado: estado de los anexos, nuevas notas e inconsistencias detectadas, y la fecha de última actualización.
2. Recordar al usuario que suba a Google Drive los anexos nuevos o modificados y `ESTADO_ACTUAL.md` (la copia local no es la fuente oficial).

## Dónde va cada cosa

- **Instrucciones** (cómo trabajar, formatos, reglas): en este `CLAUDE.md`.
- **Datos** (estado de los anexos, pendientes, notas por anexo, inconsistencias tesis–código, observaciones sobre el código): en `documents/ESTADO_ACTUAL.md`. Nunca en este archivo ni en otro archivo versionado.

## Contexto del proyecto

Repositorio de la tesis de pregrado (PUCP, Facultad de Ciencias e Ingeniería):

> **Desarrollo de un algoritmo híbrido basado en técnicas genéticas y de recocido simulado para la optimización de la carga de entregas dentro de las unidades de transporte para una empresa de cerámicos**

- **Autor:** Renzo Gabriel Pinto Quiroz (código 20201491)
- **Asesores:** Mag. Rony Cueva Moscoso, Dr. Manuel Tupia Anticona
- **Algoritmo propuesto:** SA-GA (híbrido de recocido simulado + algoritmo genético).
- **Algoritmo de comparación:** GRASP.

## Reglas de trabajo

- **No modificar el código** del repositorio (`main_algorithms/`, `utils/`, `pseudocode/`, `input/`). Solo se trabaja dentro de `documents/` (y este CLAUDE.md).
- **No ejecutar** código ni el programa salvo que se pida explícitamente.
- Si se detectan bugs en el código, o diferencias entre la tesis y el código, se registran en `ESTADO_ACTUAL.md`, pero no se corrigen. Cada diferencia se resuelve, si corresponde, ajustando la redacción de la tesis.
- Los anexos se redactan en **español**, con el mismo estilo formal de la tesis.

## Formato de los documentos de pruebas unitarias (basado en el Anexo T)

1. **Portada:** PONTIFICIA UNIVERSIDAD CATÓLICA DEL PERÚ / FACULTAD DE CIENCIAS E INGENIERÍA, título del documento, "Elaborado como parte del proyecto de tesis: …", autor, código, asesores, lugar y fecha.
2. **Tabla de contenido** e **Índice de tablas**.
3. **1. Información general** — Tabla 1: Entregable, Autor, Proyecto, Tipo de prueba ("Pruebas unitarias automatizadas").
4. **2. Ambiente de pruebas** — Tabla 2: Sistema (Windows 11 64 bits), Procesador (Intel Core Ultra 5 225F 3.3GHz), Lenguaje de programación, Tipo de prueba.
5. **3. Casos de prueba** — texto introductorio con la definición de los tipos de caso (Existencia, Conformidad, Múltiples entradas, Cardinalidad) y Tabla 3 con columnas: ID (P001, P002, …), Unidad a probar, Caso de prueba.
6. **4. Detalle de las pruebas realizadas** — Tabla 4: una ficha por caso con filas: ID del caso de prueba, Detalle del escenario, Descripción de la prueba, Datos de entrada, Datos de salida, Estado de la prueba ("Aprobado").

Las secciones de portada, información general, ambiente y definiciones de tipos de prueba se mantienen iguales entre anexos; solo cambian el título/entregable, el lenguaje y las pruebas en sí.

## Estructura del repositorio

- `main_algorithms/`: implementación en C++ de SA-GA y GRASP (proyecto NetBeans/MinGW, `main.cpp` como planificador principal).
- `pseudocode/`: pseudocódigo en PseInt de ambos algoritmos.
- `utils/`: scripts de Python (interfaz, generación de datos, calibración, experimentación numérica, visualización 3D, notebooks de gráficos).
- `input/`: archivos de entrada de prueba, calibración y experimentación numérica.
- `documents/`: copia de trabajo local de la carpeta privada de Drive (ignorada por Git).
