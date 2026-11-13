# Calibración medida — 13 noviembre 2026

Capturas del Phoenix II instalado mediante AudioSuite de Pro Tools 12.5.0. Señal sintética propia, mono PCM24, 48 kHz, Input 0 dB, Process 100, Output -6 dB; se compensa exactamente +6 dB durante el análisis. Los renders contienen 96000 muestras de margen inicial; el render Process 0 contiene los bytes PCM originales en ese desplazamiento. Esto valida esa ruta de captura de 24 bits, sin demostrar identidad float32 del original.

Se ajustaron 15 combinaciones (cinco modos por tres colores), sin recorte duro y con hashes distintos. Los hashes y condiciones están en reference-metadata.json. No se incluyen binarios AAX ni metadatos BWF personales.

Ajuste por mínimos cuadrados de siete parámetros: ganancia, drive, exponente, frecuencia/ganancia de pre-shelf y frecuencia/ganancia de post-shelf. Ventanas de 50 ms tras 250 ms de asentamiento, ponderadas por amplitud de entrada, sobre tonos de 1 kHz a siete niveles, 250 Hz, 8 kHz y bitono 19/20 kHz. Se reservaron 62.5 Hz, 4 kHz, bitono 60/7000 Hz, ráfaga y barrido para comparación posterior. Esas señales ya se observaron durante la exploración de modelos: no son una validación ciega independiente.

El renderizador C++ coincide con el modelo numérico con diferencia máxima inferior a 1e-5. comparison.csv compara Gold con el prototipo inicial; color-comparison.csv compara los colores nuevos con el modelo Gold calibrado que todavía tenía los antiguos filtros de brillo. Un error en dB significa 20*log10(RMS(error)/RMS(referencia)); no es un porcentaje de parecido sonoro.

| Modo | Gold: mejora mínima frente al prototipo, 5 pruebas | Opal: mejora media frente al brillo anterior | Sapphire: mejora media frente al brillo anterior |
|---|---:|---:|---:|
| Luminescent | 15.88 dB | 1.64 dB | 7.79 dB |
| Iridescent | 17.12 dB | 2.73 dB | 7.30 dB |
| Radiant | 15.97 dB | 3.99 dB | 6.96 dB |
| Luster | 13.89 dB | 2.51 dB | 13.94 dB |
| DarkEssence | 16.68 dB | 4.02 dB | 9.57 dB |

Los resultados de la tabla anterior corresponden a la primera calibración. Ver la revisión siguiente para el estado actual. La mejora de los colores es media: algunas señales concretas empeoran. Persisten diferencias claras: en los 30 tests de referencia completos el error RMS relativo abarca aproximadamente 0.46–25.93%; el peor corresponde a la ráfaga Luster/Opal. No se afirma un clon 1:1 ni un null húmedo exacto.

Las primeras dos capturas Gold/Process 50 de Luminescent y Dark Essence demostraron desviación respecto a mezcla lineal de los extremos. En Dark Essence, usando los propios extremos originales, la mezcla 50/50 da 3.82% de error RMS relativo; incluso la mejor mezcla escalar (~0.455) deja 3.12%. La primera versión conservó mezcla lineal; la revisión siguiente incorpora una corrección limitada tras ampliar las capturas.

Pendiente: completar Process 25/50/75 en las 15 combinaciones, trims, capturas nativas a 44.1/96/192 kHz, señales musicales independientes y comportamiento estéreo. Los tests de estabilidad/null a esas frecuencias prueban funcionamiento de DIAMOND II, no equivalencia al original.

reference-fixtures.bin.gz contiene dos señales propias y sus 15 salidas capturadas normalizadas, sin cabeceras BWF. Formato gzip little-endian: PHX2, uint32 frecuencia (48000), uint32 número de pruebas (2); por prueba uint32 muestras, float32 entrada[muestras], 15 arrays float32 salida[muestras] en orden Opal/Gold/Sapphire y Luminescent/Iridescent/Radiant/Luster/Dark Essence. Los límites de regresión están fijados en Tests/DspVerificationTest.cpp y dejan 10% relativo de margen más 0.001. El null bit-accurate de DIAMOND II es únicamente su ruta seca a ganancia unitaria.


## Revisión: filtros y recorrido de Process

Se capturaron tres referencias nuevas Gold/Process 50 para Iridescent, Radiant y Luster, más Process 25 y 75 de Luster y Dark Essence. Las nueve condiciones intermedias disponibles son capturas originales distintas y sin recorte. Los hashes están en process-reference-metadata.json; no se cambia el binario del original.

Se probaron tres extensiones del modelo: mezcla lineal dentro de la curva, amortiguamiento independiente en los filtros, y memoria no lineal de una muestra. Solo se seleccionó el nuevo amortiguamiento de Luster/Opal: mejora las cinco señales excluidas del ajuste entre 0.78 y 10.34 dB (media 5.15 dB) frente a la versión anterior. Estos datos se usaron para seleccionar el modelo y no son validación ciega. Algunas señales usadas en el propio ajuste, especialmente tonos muy bajos de nivel, empeoran ligeramente: el error relativo de 1 kHz/-48 dBFS pasa de 2.25% a 2.52%.

| Luster/Opal, Process 100 | Error RMS anterior | Error RMS nuevo |
|---|---:|---:|
| 4 kHz, -12 dBFS | 16.10% | 4.89% |
| Ráfaga de 1 kHz | 25.93% | 23.71% |
| Barrido 20 Hz–20 kHz | 7.27% | 3.83% |

El nuevo amortiguamiento d entra en `N=1/(1+d*K+K*K)`, `b0=N*(V+sqrt(V)*d*K+K*K)`, `b2=N*(V-sqrt(V)*d*K+K*K)`, `a2=N*(1-d*K+K*K)`; b1 y a1 no cambian. No aumenta el orden del filtro ni añade latencia. Los nueve parámetros exactos por combinación están en all-colors-48k-fit.json: los siete históricos y preDamping/postDamping. En las otras 14 combinaciones ambos amortiguamientos siguen siendo sqrt(2).

Para Luster y Dark Essence en Gold se añade `4*p*(1-p)*(a*x+b*r)` a `x+p*r`, donde r es la diferencia húmeda y x la entrada tras Input Trim. Los coeficientes se estimaron con ventanas estacionarias de 50 ms en las capturas originales Process 50, usando como extremos el propio original seco y Process 100. No se modifican los extremos 0/100. Process 25 y 75 se reservaron para comprobar la interpolación. No se generaliza la corrección a otros colores o modos sin medirlos.

| Modo Gold | Process | Error RMS del archivo completo anterior | Nuevo |
|---|---:|---:|---:|
| Luster | 25 | 5.46% | 3.35% |
| Luster | 50 | 7.33% | 5.06% |
| Luster | 75 | 6.21% | 5.39% |
| DarkEssence | 25 | 2.66% | 1.82% |
| DarkEssence | 50 | 3.60% | 2.88% |
| DarkEssence | 75 | 4.32% | 4.15% |

La mejora de Process se refiere al error total del archivo: hay pequeñas regresiones en algunos bitonos y en la ráfaga Luster al 75%. La calibración continúa siendo una aproximación, sin identidad 1:1. Los otros tres modos Gold al 50% se miden como controles y conservan su procesamiento.

Se verificó el modelo compilado contra el numérico con diferencia máxima inferior a 1e-5. Los datos detallados están en luster-opal-refinement.json, process-refinement.json y process-compiled-verification.json. Los dos límites de regresión Luster/Opal de la suite anterior se han estrechado. Se añaden 18 comprobaciones de referencias originales de Process (dos señales por nueve condiciones), con límites congelados en Tests/ProcessReferenceLimits.h.

process-reference-fixtures.bin.gz usa formato little-endian PHXP, uint32 frecuencia 48000, uint32 casos 18; por caso uint32 Type, uint32 Brightness, float32 Process en porcentaje, uint32 muestras, float32 entrada[muestras] y float32 referencia[muestras]. No contiene metadatos BWF. El null bit-accurate sigue correspondiendo únicamente a la ruta seca de DIAMOND II.
