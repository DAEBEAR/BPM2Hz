# TempoConvert — plugin VST3 / AU

Plugin di utilità (audio in pass-through, non modifica il suono) che legge il **tempo dalla DAW** o accetta un **BPM manuale**, e mostra una tabella con i tempi di delay in **ms** e le frequenze **Hz** per ogni divisione ritmica (dritta, puntata, terzina), da 1/1 a 1/32.

## Cosa ti serve (una volta sola)

1. **Xcode** dal Mac App Store (serve il toolchain di compilazione Apple, anche se non useremo l'IDE Xcode direttamente).
2. Apri Xcode almeno una volta e accetta la licenza, poi da Terminale:
   ```bash
   xcode-select --install
   ```
3. **CMake** — se hai [Homebrew](https://brew.sh):
   ```bash
   brew install cmake
   ```

Non serve installare JUCE a parte: il progetto lo scarica da solo al primo build (richiede connessione internet la prima volta).

## Opzione A — farlo compilare da GitHub, senza Xcode/Terminale

Il progetto include già `.github/workflows/build.yml`: un file che dice a GitHub di compilare il plugin su un vero Mac nei suoi server, gratis.

1. Crea un repository su [github.com](https://github.com) (gratuito) e carica dentro tutta la cartella `TempoConvert` (con l'interfaccia web di GitHub: "Add file" → "Upload files", oppure con `git push` se lo conosci).
2. Vai nella tab **Actions** del repository → dovresti vedere il workflow "Build VST3 + AU" partito da solo (o clicca **Run workflow** per lanciarlo a mano).
3. Aspetta 5-10 minuti che finisca (pallino verde ✅).
4. Apri il job completato, in fondo alla pagina trovi **Artifacts** → scarica `TempoConvert-plugins.zip`.
5. Dentro trovi le cartelle `VST3/` e `AU/`: sposta `TempoConvert.vst3` in `~/Library/Audio/Plug-Ins/VST3/` e `TempoConvert.component` in `~/Library/Audio/Plug-Ins/Components/`.
6. Rescan dei plugin nella tua DAW.

Nota: il plugin **non è firmato/notarizzato** da Apple in questo modo — macOS potrebbe chiederti conferma la prima volta che lo carichi (tasto destro sul file → Apri, oppure Impostazioni di Sicurezza). Per uso personale sul tuo Mac va benissimo così.

## Opzione B — compilarlo tu sul tuo Mac con Xcode

1. Estrai la cartella `TempoConvert` in un punto qualsiasi del tuo Mac (es. `~/Developer/TempoConvert`).
2. Apri il Terminale e vai nella cartella:
   ```bash
   cd ~/Developer/TempoConvert
   ```
3. Configura il progetto (la prima volta scarica JUCE, ci vuole qualche minuto):
   ```bash
   cmake -B build -G Xcode
   ```
4. Compila in modalità Release:
   ```bash
   cmake --build build --config Release
   ```

Alla fine del build, `COPY_PLUGIN_AFTER_BUILD` copia automaticamente i plugin nelle cartelle di sistema:

- **VST3** → `~/Library/Audio/Plug-Ins/VST3/TempoConvert.vst3`
- **AU** → `~/Library/Audio/Plug-Ins/Components/TempoConvert.component`
- **Standalone** (app che gira da sola, utile per testare) → `build/TempoConvert_artefacts/Release/Standalone/TempoConvert.app`

## Usarlo nella DAW

1. Apri la tua DAW (Logic, Ableton, Reaper, ecc.) e fai fare una **rescan dei plugin** se non compare subito.
2. Inserisci **TempoConvert** su una traccia audio come un normale plugin effetto (non altera il suono, serve solo come pannello di calcolo).
3. Di default è su "**Sync al tempo host**": legge automaticamente il BPM del progetto.
4. Disattiva il toggle per inserire un BPM manuale con lo slider.
5. Clicca su un valore ms/Hz nella tabella per copiarlo negli appunti e incollarlo nel tuo delay/LFO preferito.

## Se AU non compare in Logic Pro

Logic è più rigido nel validare i plugin AU. Da Terminale:
```bash
auval -v aufx Tcvt Msdo
```
(`Tcvt` e `Msdo` sono i codici impostati in `CMakeLists.txt` — se li cambi, aggiorna anche qui). Se dà errori, controlla l'output: di solito indica cosa non torna.

## Prima di distribuirlo ad altri

Cambia questi valori in `CMakeLists.txt`, sono provvisori:
```cmake
COMPANY_NAME "MioStudio"
PLUGIN_MANUFACTURER_CODE Msdo
PLUGIN_CODE Tcvt
```
`PLUGIN_MANUFACTURER_CODE` deve essere univoco per te come sviluppatore (4 caratteri, il primo maiuscolo). Se pubblichi il plugin, valuta anche la firma/notarizzazione Apple (`codesign` + `notarytool`), necessaria perché altri Mac lo aprano senza avvisi di sicurezza.

## Struttura del progetto

```
TempoConvert/
├── CMakeLists.txt          # configurazione build, formati (AU/VST3/Standalone)
└── Source/
    ├── PluginProcessor.h/.cpp   # logica: legge il tempo, calcola ms/Hz
    └── PluginEditor.h/.cpp      # interfaccia grafica
```
