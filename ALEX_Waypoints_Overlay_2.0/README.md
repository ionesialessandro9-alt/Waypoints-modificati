# ALEX Waypoints 2.0 — Minecraft 1.21.8 Fabric + Windows overlay

Questa versione sposta la visualizzazione dei waypoint in una **finestra Windows separata**.
La mod Fabric scrive posizione del giocatore e waypoint più vicino in:
`.minecraft/config/alex-waypoints-live.json`
L'app `ALEX-Overlay.exe` legge quel file e mostra un pannello sempre in primo piano.

## OBS
- In OBS usa **Game Capture → Capture specific window** e seleziona la finestra Minecraft (`javaw.exe`).
- Non usare Display Capture.
- La finestra overlay è un'app Windows separata: Game Capture della sola finestra Minecraft normalmente non la acquisisce.
- Verifica sempre facendo una registrazione di prova. Se usi una sorgente Display Capture o una cattura desktop, l'overlay sarà visibile.
- L'overlay non è un metodo per occultare contenuti ad altri software: è una normale finestra separata.

## Comandi mod
- `/alexwp add base 120 64 -300`
- `/alexwp list`
- `/alexwp remove base`

## Build mod
Requisiti: JDK 21, Gradle 8.x.
Dalla cartella `fabric-mod`: `gradle build`.
Copia `fabric-mod/build/libs/alex-waypoints-2.0.0.jar` in `.minecraft/mods` insieme a Fabric API 1.21.8.

## Build overlay EXE
Requisiti: Visual Studio Build Tools con C++ Desktop workload.
Apri Developer Command Prompt nella cartella `overlay` ed esegui:
`cl /nologo /std:c++17 /EHsc /DUNICODE /D_UNICODE main.cpp /link /SUBSYSTEM:WINDOWS User32.lib Gdi32.lib /OUT:ALEX-Overlay.exe`

Avvia `ALEX-Overlay.exe` mentre Minecraft è aperto. La finestra si può trascinare con il mouse. Premi ESC per chiuderla.
