# Housings

The housings of both consoles: Fusion source, print-ready meshes, and the Bambu Studio
projects they were printed from.

| Path | Contents |
|---|---|
| `cad/LittleGameConsole.f3z` | The Fusion source of the LittleGameConsole, as an archive containing the whole assembly |
| `cad/TintinRocketShooter.f3d` | The Fusion source of the TintinRocketShooter |
| `stl/LittleGameConsole.stl` | Print-ready mesh of the LittleGameConsole: 7 bodies in one file, 93 x 127 x 23 mm overall |
| `stl/TintinRocketShooter.stl` | Print-ready mesh of the TintinRocketShooter: 13 bodies in one file, 150 x 45 x 244 mm overall |
| `print-profiles/LittleGameConsole/` | Bambu Studio projects, one per part |
| `print-profiles/TintinRocketShooter/` | Bambu Studio projects, one per part or group of parts, some in two materials |

The TintinRocketShooter's housing is an original design based on Tintin's moon rocket from
Hergé's comics; the red and white checks are printed as separate tiles.

## The parts

Taken from the projects in `print-profiles/`. Every project was sliced for one Bambu Lab
printer and nozzle, named in the table.

### LittleGameConsole

| Project | Part | Printer, nozzle | Layer | Walls | Infill | Material |
|---|---|---|---|---|---|---|
| `LittleGameConsole_Top0.2.3mf` | top shell | H2D, 0.2 mm | 0.10 mm | 4 | 15% | PLA |
| `LittleGameConsole_Bottom_90Deg.3mf` | bottom shell, standing on edge (90°) | H2D, 0.4 mm | 0.12 mm | 4 | 15% | PETG-CF |
| `LittleGameConsole_Bottom_30Deg.3mf` | bottom shell, tilted (30°) | H2D, 0.4 mm | 0.16 mm | 2 | 15% | PETG-CF |
| `Buttons.3mf` | action and control buttons | H2D, 0.4 mm | 0.12 mm | 4 | 50% | ABS |

The two bottom-shell projects are the same part in two print orientations; print one of them.

### TintinRocketShooter

| Project | Part | Printer, nozzle | Layer | Walls | Infill | Material |
|---|---|---|---|---|---|---|
| `TinTinRocket-Lower-H2D-ABS-GF.3mf` | lower body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | ABS-GF |
| `TinTinRocket-Lower-H2D-PLA-GF.3mf` | lower body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | ABS-GF (see below) |
| `TinTinRocket-Upper-H2D-ABS-GF ABS-Support.3mf` | upper body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | ABS-GF |
| `TinTinRocket-Upper-H2D-PLA-GF Support.3mf` | upper body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | PLA-GF |
| `TinTinRocket-Top-H2D-ABS-GF ABS-Support.3mf` | top | H2D, 0.4 mm | 0.08 mm | 6 | 15% | ABS-GF |
| `TinTinRocket-Top-H2D-PLA-GF Support.3mf` | top | H2D, 0.4 mm | 0.12 mm | 6 | 15% | PLA-GF |
| `RocketShooterLower.3mf` | lower body | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `RocketShooterUpper.3mf` | upper body | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `RocketTop.3mf` | top | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `Tiles.3mf` | the checker tiles, rows 1 to 5 | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `SelectStartBtn.3mf` | Select and Start buttons | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `JoyTopCover.3mf` | joystick cap | X1 Carbon, 0.4 mm | 0.2 mm | 2 | 15% | TPU |
| `Battery.3mf` | battery end pieces, plus and minus | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `Stand.3mf` | stand | X1 Carbon, 0.4 mm | 0.12 mm | 3 | 15% | PLA |

The body - lower part, upper part and top - comes in two sets of projects: `TinTinRocket-*`
for an H2D with a 0.4 mm nozzle, in glass-filled ABS or PLA, and `RocketShooter*` /
`RocketTop` for an X1 Carbon with a 0.2 mm nozzle, in PLA. Print one set.

## Opening the CAD

`LittleGameConsole.f3z` is a Fusion archive rather than a single `.f3d` part - open it with
*File → Open* in Fusion, which unpacks the design and its components.
`TintinRocketShooter.f3d` opens directly.

There is no STEP export here. If you want to modify a housing without a Fusion licence, a
STEP export is the thing to ask for; the STLs are fine for printing but carry no editable
geometry.

## About `print-profiles/`

These are Bambu Studio projects: a full copy of the mesh plus the slicer state, so each is
specific to one printer and one filament, and the larger ones run to tens of megabytes. They
are here as a reference for anyone with the same machine. On any other printer, the STLs
plus the tables above carry the same information in a form your own slicer can use.
