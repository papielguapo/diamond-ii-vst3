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

La mejora de los colores es media: algunas señales concretas empeoran. Persisten diferencias claras: en los 30 tests de referencia completos el error RMS relativo abarca aproximadamente 0.46–25.93%; el peor corresponde a la ráfaga Luster/Opal. No se afirma un clon 1:1 ni un null húmedo exacto.

Dos capturas adicionales Gold/Process 50 de Luminescent y Dark Essence demuestran desviación respecto a mezcla lineal de los extremos. En Dark Essence, usando los propios extremos originales, la mezcla 50/50 da 3.82% de error RMS relativo; incluso la mejor mezcla escalar (~0.455) deja 3.12%. No se modifica la ley de Process a partir de solo dos puntos: faltan curvas por modo, color y nivel.

Pendiente: Process 25/50/75 en las 15 combinaciones, trims, capturas nativas a 44.1/96/192 kHz, señales musicales independientes y comportamiento estéreo. Los tests de estabilidad/null a esas frecuencias prueban funcionamiento de DIAMOND II, no equivalencia al original.

reference-fixtures.bin.gz contiene dos señales propias y sus 15 salidas capturadas normalizadas, sin cabeceras BWF. Formato gzip little-endian: PHX2, uint32 frecuencia (48000), uint32 número de pruebas (2); por prueba uint32 muestras, float32 entrada[muestras], 15 arrays float32 salida[muestras] en orden Opal/Gold/Sapphire y Luminescent/Iridescent/Radiant/Luster/Dark Essence. Los límites de regresión están fijados en Tests/DspVerificationTest.cpp y dejan 10% relativo de margen más 0.001. El null bit-accurate de DIAMOND II es únicamente su ruta seca a ganancia unitaria.
