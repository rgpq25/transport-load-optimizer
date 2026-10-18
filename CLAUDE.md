# CLAUDE.md

## ⚠️ Prerrequisito de cada sesión: sincronizar `documents/` desde Google Drive

- El **repositorio oficial de documentos** (tesis, anexos, cartas de validación) es una carpeta **privada de Google Drive**, porque contiene información sensible. **No** se versiona en Git (este repositorio de GitHub es público).
- La carpeta local `documents/` es solo una **copia de trabajo** de esa carpeta de Drive.
- **Al inicio de cada sesión**, antes de trabajar en cualquier anexo:
  1. Recordar al usuario que descargue la última versión de la carpeta de Drive (tesis + anexos existentes) a `documents/`.
  2. No empezar a trabajar en anexos hasta que el usuario confirme que `documents/` está actualizada.
  3. Verificar que `documents/` exista y contenga al menos la tesis (`20201491_RenzoPinto_RonyCueva_ManuelTupia_EF.docx`).
- Al terminar un anexo, recordar al usuario que lo suba a Google Drive (la copia local no es la fuente oficial).

## Contexto del proyecto

Repositorio de la tesis de pregrado (PUCP, Facultad de Ciencias e Ingeniería):

> **Desarrollo de un algoritmo híbrido basado en técnicas genéticas y de recocido simulado para la optimización de la carga de entregas dentro de las unidades de transporte para una empresa de cerámicos**

- **Autor:** Renzo Gabriel Pinto Quiroz (código 20201491)
- **Asesores:** Mag. Rony Cueva Moscoso, Dr. Manuel Tupia Anticona
- **Algoritmo propuesto:** SA-GA (híbrido de recocido simulado + algoritmo genético).
- **Algoritmo de comparación:** GRASP.

## Situación actual (octubre 2026)

El proyecto se desarrolló hace aproximadamente un año y se está retomando para **finalizar la tesis**.
Los **anexos** de la tesis fueron borrados en una instancia anterior y **se están reconstruyendo** uno por uno.

- El repositorio principal de documentos vive en **Google Drive** (privado); aquí solo se tiene una copia local en `documents/` (ver prerrequisito arriba).
- `documents/20201491_RenzoPinto_RonyCueva_ManuelTupia_EF.docx`: documento final de la tesis (contiene el índice de anexos A–Y y las secciones que referencian a cada anexo).
- `documents/Anexo_T - Pruebas unitarias - Algoritmo SA-GA.docx`: anexo ya reconstruido, usado como **plantilla de formato** para los demás documentos de pruebas unitarias.

## Reglas de trabajo

- **No modificar el código** del repositorio (`main_algorithms/`, `utils/`, `pseudocode/`, `input/`). Solo se trabaja dentro de `documents/` (y este CLAUDE.md).
- **No ejecutar** código ni el programa salvo que se pida explícitamente.
- Si se detectan bugs en el código, se reportan, pero no se corrigen.
- Los anexos se redactan en **español**, con el mismo estilo formal de la tesis.

## Formato de los documentos de pruebas unitarias (basado en el Anexo T)

1. **Portada:** PONTIFICIA UNIVERSIDAD CATÓLICA DEL PERÚ / FACULTAD DE CIENCIAS E INGENIERÍA, título del documento, "Elaborado como parte del proyecto de tesis: …", autor, código, asesores, lugar y fecha.
2. **Tabla de contenido** e **Índice de tablas**.
3. **1. Información general** — Tabla 1: Entregable, Autor, Proyecto, Tipo de prueba ("Pruebas unitarias automatizadas").
4. **2. Ambiente de pruebas** — Tabla 2: Sistema (Windows 11 64 bits), Procesador (Intel Core Ultra 5 225F 3.3GHz), Lenguaje de programación, Tipo de prueba.
5. **3. Casos de prueba** — texto introductorio con la definición de los tipos de caso (Existencia, Conformidad, Múltiples entradas, Cardinalidad) y Tabla 3 con columnas: ID (P001, P002, …), Unidad a probar, Caso de prueba.
6. **4. Detalle de las pruebas realizadas** — Tabla 4: una ficha por caso con filas: ID del caso de prueba, Detalle del escenario, Descripción de la prueba, Datos de entrada, Datos de salida, Estado de la prueba ("Aprobado").

Las secciones de portada, información general, ambiente y definiciones de tipos de prueba se mantienen iguales entre anexos; solo cambian el título/entregable, el lenguaje y las pruebas en sí.

## Estado de los anexos (A–Y)

Se reconstruyen de arriba hacia abajo. `documents/` contiene los anexos ya disponibles (nombrados `Anexo_<letra> - <título>`). En la tesis, cada anexo (salvo A y C) es solo un enlace a Drive/Docs/Sheets/GitHub.

| Anexo | Documento | Estado |
|-------|-----------|--------|
| A | Cronograma de trabajo (PFC1) | Incluido dentro de la tesis |
| B | Formulario de extracción | Disponible (.xlsx) |
| C | Plan de proyecto | Incluido dentro de la tesis |
| D | Lista de variables y restricciones de los estudios analizados | **Pendiente**: en espera del Excel original con la matriz (no reconstruir hasta tenerlo) |
| E–M | Validaciones (R1–R5) y muestra de datos (I) | Disponibles |
| N | Pruebas unitarias de la interfaz de ejecución (Python) | Reconstruido (fecha 17/06/2025) |
| O | Validación de la interfaz | Disponible |
| P | Datos de entrada para comparación de rendimiento (.txt) | Reconstruido: copia de `input/used_tests/calibration_test.txt` (dataset de calibración, sección 7.2.2 / Tabla 7.1) |
| Q | Calibración de parámetros SA-GA (hoja de cálculo) | **En progreso**: plantilla `documents/Anexo_Q - ...xlsx` con el formato de salida de `utils/parameter_calibration.py` (36 permutaciones × 10 ejecuciones, fitness por completar; ganadora = permutación 24, Tabla 7.4) |
| R | Calibración de parámetros GRASP (hoja de cálculo) | **Falta** |
| S | Validación de la calibración | Disponible |
| T | Pruebas unitarias SA-GA (C++) | Reconstruido |
| U | Pruebas unitarias GRASP (C++) | Disponible |
| V | Datos de entrada de la experimentación numérica (.zip con 20 archivos) | Reconstruido: `.zip` con `input1.txt`–`input20.txt` de `input/used_tests/numeric_experimentation/` (coinciden con la Tabla 7.8). Cada archivo se ejecutó 30 veces por algoritmo; esos resultados son el Anexo W |
| W | Resultados de ejecuciones de la experimentación numérica (hoja de cálculo) | **Falta** |
| X | Validación de la experimentación numérica | Disponible |
| Y | Repositorio principal (enlace a GitHub) | Solo enlace |
| J (complemento) | Documento técnico: ejecución detallada de SA-GA de inicio a fin (población inicial → despachos), citado en la sección 5.2.3.2 | **Pendiente**: reconstruir. Material: `utils/plots_saga.ipynb`, log por generación de `SAGAOptimizer.cpp` (modo debug). Idea: script que arme la tabla de iteraciones y guarde el detalle en Excel |
| M (complemento) | Documento técnico: ejecución detallada de GRASP con resultados intermedios y evolución del fitness, citado en la sección 6.2.3.2 | **Pendiente**: reconstruir. Material: `utils/plots_grasp.ipynb`, log por iteración de `GRASPOptimizer.cpp` (modo debug). Idea: script que arme la tabla de iteraciones y guarde el detalle en Excel |

Nota: los Anexos J y M actuales son solo cartas de validación; la tesis dice que contienen (o respaldan) estos documentos técnicos, que no están en el índice de anexos.

### Anexo N — Interfaz de ejecución

- Código: `utils/interface.py` (Tkinter + Plotly + pandas), configuración en `utils/config.py` (rutas al ejecutable/CSV de salida y parámetros `saga_args` / `grasp_args`).
- Pantallas: (1) selección de archivo de entrada, algoritmo y parámetros, botón "Ejecutar" que invoca el ejecutable C++ vía `subprocess`; (2) resultados: configuración usada, fitness, duración y tarjetas por despacho leídas de `output_dispatches.csv` y `output_result_metadata.csv`, botón "Volver"; (3) visualización 3D de bloques por despacho (botón "Ver bloques", Plotly).
- Según la sección 7.2.1.2 de la tesis, las pruebas cubren: carga de archivos y rutas de entrada; selección dinámica de algoritmos y despliegue de parámetros; ejecución de los algoritmos y lectura del archivo de resultados; despliegue de las tarjetas de despacho; botón "Ver bloques" y generación del entorno 3D.
- Metodología: igual que el Anexo T, los resultados se deducen leyendo el código (no hay pruebas automatizadas), asumiendo que el flujo completo funciona de inicio a fin (ejecutable presente y generando sus CSV). No se reportan defectos: todas las pruebas quedan "Aprobado".
- Escenario base: entrada `input/used_tests/documented_test.txt`; despacho 1 de ejemplo = unidad 1 (5.29 × 2.48 × 2.68 m), ruta 4, 2024-09-28 08:00–10:00, entregas 1 (bloques 1, 2) y 16 (bloques 43, 44); metadata fitness 5432.18 / duración 12.47 s (valores ilustrativos).
- Observaciones del código (no corregir): `interface.py:244` usa `==` en lugar de `=` (`paramLabel == "[ERROR]"`); `config.py` apunta a la ruta antigua del repositorio.

## Estructura del repositorio

- `main_algorithms/`: implementación en C++ de SA-GA y GRASP (proyecto NetBeans/MinGW, `main.cpp` como planificador principal).
- `pseudocode/`: pseudocódigo en PseInt de ambos algoritmos.
- `utils/`: scripts de Python (interfaz, generación de datos, calibración, experimentación numérica, visualización 3D, notebooks de gráficos).
- `input/`: archivos de entrada de prueba, calibración y experimentación numérica.
- `documents/`: tesis y anexos en reconstrucción.
