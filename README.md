# AEGIS DOMINION – Foundation

> Dieses Repository entwickelt den spielbaren SFML-Prototyp schrittweise zu einer datengetriebenen Architektur weiter. Karten, Editorzustand und Rendering werden getrennt, damit MAP FORGE und ein späterer 3D-Renderer dieselben Inhalte verwenden können.

## V3 milestone already included
- zentrale UTF-8-/Font-Infrastruktur für deutsche Umlaute und ß
- renderer-independent `MapDocument`
- versioned `.aegismap` save/load format
- built-in maps moved into data files
- `GameMap` reads `.aegismap` files with legacy fallback
- `MapEditorModel` with path/spawn/goal/zones/decorations and undo/redo foundation
- automatisierte Tests für Map-Serialisierung, Fehlerfälle, Versionierung, Editor-Historie und UTF-8
- `AGENTS.md` and Codex work instructions
- architecture designed so a later 3D renderer can reuse map/game data

See `docs/ARCHITECTURE_V3.md` and `docs/CODEX_START.md`.

---

# AEGIS DOMINION

Ein vollständigerer 2D-Tower-Defense-Prototyp in **C++17 + SFML**. Die Deluxe-Fassung benutzt echte PNG-Grafiken, mehrere Karten, Soundeffekte, ein Hauptmenü, Kartenwahl, eine sechsteilige In-Game-Anleitung und ein OOP-System für Türme und Gegner.

## Schnellstart auf Fedora

```bash
sudo dnf install gcc-c++ cmake SFML-devel
cd aegis_dominion
./build.sh
./run.sh
```

Alternativ:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
cd build
./aegis_dominion
```

> Wichtig: Starte über `./run.sh` oder aus dem `build`-Ordner. CMake kopiert beim Build den kompletten `assets`-Ordner neben die ausführbare Datei.

## Was diese Version enthält

- 3 grafisch unterschiedliche Karten: **Grüne Grenze**, **Frostpass**, **Aschefeld**
- echte PNG-Hintergründe statt nur farbiger Rechtecke
- eigene PNG-Sprites für alle Türme
- getrennte Turmbasis und drehbarer Turmkopf
- 7 Gegnertypen mit eigenen Sprites
- Lebens- und Schildbalken
- Partikel, Tracer, Explosionen und Reichweitenanzeigen
- 6 unterschiedliche Turmklassen
- 4 Turmstufen
- ab Stufe 2 zwei dauerhafte Spezialisierungswege
- 20 Angriffswellen
- Boss alle 5 Wellen
- Splitter-Gegner, die beim Tod zwei schnelle Einheiten erzeugen
- Rüstung, Schild, Regeneration, Slow-Resistenz
- Zielprioritäten: Vorne, Stärkster, Nächster, Hinten
- Verkaufssystem
- 1×/2× Geschwindigkeit
- Pause
- Score und Abschusszähler
- Hauptmenü und Kartenwahl
- sechsteilige **Anleitung direkt im Spiel**
- F1-Kurzanleitung während eines laufenden Spiels
- generierte WAV-Soundeffekte; keine zusätzlichen Downloads nötig

## Steuerung

| Taste / Maus | Funktion |
|---|---|
| `1`–`6` | Turm auswählen |
| Linksklick | Turm bauen / gebauten Turm auswählen |
| Rechtsklick | Baumodus oder Auswahl abbrechen |
| `Leertaste` | Nächste Welle starten |
| `U` | Turm upgraden |
| `T` | Zielpriorität wechseln |
| `S` | Turm verkaufen |
| `F` | 1× / 2× Geschwindigkeit |
| `P` oder `Esc` | Pause |
| `F1` | Kurzanleitung öffnen |

## Die sechs Türme

### 1. Pulsar — 90 C
Schneller Allround-Turm. Gut für frühe Wellen und einzelne leichte Gegner.

- **Overclock:** deutlich höhere Feuerrate
- **Panzerbrecher:** mehr Schaden und Reichweite

### 2. Mörser — 145 C
Langsamer Turm mit großem Flächenschaden. Besonders stark gegen dichte Gruppen.

- **Splitterladung:** größerer Explosionsradius
- **Schweres Kaliber:** sehr hoher Schaden pro Treffer

### 3. Kryo — 120 C
Verlangsamt Gegner und hält sie länger in den Feuerzonen deiner anderen Türme.

- **Tiefkühlung:** stärkerer und längerer Slow
- **Kristallsplitter:** mehr Direktschaden

### 4. Railgun — 175 C
Extrem hohe Reichweite und sehr hoher Einzelschaden. Ideal gegen Tanks und Bosse.

- **Beschleuniger:** schnellere Schussfolge
- **Exekutor:** massiver Schaden pro Schuss

### 5. Tesla — 165 C
Ein Blitz springt automatisch auf mehrere nahe Gegner über.

- **Relaisnetz:** mehr Kettenziele
- **Überladung:** mehr Schaden pro Sprung

### 6. Raketen — 210 C
Teurer Flächenschaden-Turm mit großer Reichweite und starken Explosionen.

- **Schwarm:** schnellere Raketenfolge
- **Sprengkopf:** deutlich größere und stärkere Explosionen

## Gegner

- **Plünderer:** Standardgegner
- **Jäger:** schnell, aber fragil
- **Brecher:** langsam, viel Leben und Rüstung
- **Schildträger:** besitzt einen zusätzlichen Energieschild
- **Regenerator:** heilt sich permanent
- **Splitter:** erzeugt beim Tod zwei Jäger
- **Titan:** Boss mit sehr viel Leben, Schild, Regeneration und Slow-Resistenz

## Grundtaktik

Kryo-Türme sind besonders stark vor Mörsern, Tesla oder Raketen, weil verlangsamte Gegner länger im Wirkungsbereich bleiben. Railguns gehören an Positionen mit großer Sicht auf mehrere lange Straßenstücke. Flächenschaden lohnt sich besonders an engen Kurven, an denen sich Gegnergruppen sammeln.

Du musst nicht jede Welle sofort starten. Zwischen zwei Wellen kannst du in Ruhe bauen, verkaufen und verbessern.

## Projektstruktur

```text
assets/
  maps/       fertige Kartenhintergründe
  towers/     Basis- und Turret-Sprites
  enemies/    Gegner-Sprites
  ui/         Menü, Logo und HUD-Icons
  sfx/        WAV-Soundeffekte
src/
  Assets.*
  Enemy.*
  Tower.*
  Projectile.*
  Effects.*
  Map.*
  WaveManager.*
  Game.*
  main.cpp
tools/
  generate_assets.py
```

## OOP-Aufbau

`Enemy` und `Tower` sind Basisklassen. Konkrete Einheiten wie `TankEnemy`, `BossEnemy`, `PulseTower`, `TeslaTower` und `MissileTower` werden als eigene Klassen erzeugt. Angriffe laufen polymorph über `Tower::fire()`, während Gegner ihre eigenen Werte und Spezialfähigkeiten besitzen.

## Grafiken neu erzeugen

Die bereits fertigen PNGs liegen im ZIP. Wenn du sie selbst neu erzeugen möchtest:

```bash
python3 -m pip install pillow
python3 tools/generate_assets.py
```

## Fehlerbehebung

### `Could not find SFML`

```bash
sudo dnf install SFML-devel
```

Danach den alten Build löschen:

```bash
rm -rf build
./build.sh
```

### Fenster startet, aber Bilder fehlen

Nicht nur die Binary einzeln verschieben. Starte über `./run.sh`. Der `assets`-Ordner muss neben der Binary vorhanden sein.

### Kein Text sichtbar

Die zentrale Textinfrastruktur sucht zuerst unter `assets/fonts/` nach `NotoSans-Regular.ttf` oder `DejaVuSans.ttf` und danach nach einer installierten Schrift mit vollständiger deutscher Zeichenabdeckung. Für portable Builds sollte eine entsprechend lizenzierte Fontdatei unter `assets/fonts/` mitgeliefert werden.
