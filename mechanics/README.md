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
| `LittleGameConsole-Top-H2D-PLA-0.2.3mf` | top shell | H2D, 0.2 mm | 0.10 mm | 4 | 15% | PLA |
| `LittleGameConsole-Bottom90-H2D-PETG-CF.3mf` | bottom shell, standing on edge (90°) | H2D, 0.4 mm | 0.12 mm | 4 | 15% | PETG-CF |
| `LittleGameConsole-Bottom30-H2D-PETG-CF.3mf` | bottom shell, tilted (30°) | H2D, 0.4 mm | 0.16 mm | 2 | 15% | PETG-CF |
| `LittleGameConsole-Buttons-H2D-ABS.3mf` | action and control buttons | H2D, 0.4 mm | 0.12 mm | 4 | 50% | ABS |

The two bottom-shell projects are the same part in two print orientations; print one of them.

### TintinRocketShooter

| Project | Part | Printer, nozzle | Layer | Walls | Infill | Material |
|---|---|---|---|---|---|---|
| `TintinRocketShooter-Lower-H2D-ABS-GF.3mf` | lower body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | ABS-GF |
| `TintinRocketShooter-Upper-H2D-ABS-GF-ABS-Support.3mf` | upper body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | ABS-GF, supports in ABS |
| `TintinRocketShooter-Upper-H2D-PLA-GF-Support.3mf` | upper body | H2D, 0.4 mm | 0.12 mm | 6 | 15% | PLA-GF, with supports |
| `TintinRocketShooter-Top-H2D-ABS-GF-ABS-Support.3mf` | top | H2D, 0.4 mm | 0.08 mm | 6 | 15% | ABS-GF, supports in ABS |
| `TintinRocketShooter-Top-H2D-PLA-GF-Support.3mf` | top | H2D, 0.4 mm | 0.12 mm | 6 | 15% | PLA-GF, with supports |
| `TintinRocketShooter-Lower-X1-PLA-0.2.3mf` | lower body | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-Upper-X1-PLA-0.2.3mf` | upper body | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-Top-X1-PLA-0.2.3mf` | top | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-Tiles-X1-PLA-0.2.3mf` | the checker tiles, rows 1 to 5 | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-SelectStartBtn-X1-PLA-0.2.3mf` | Select and Start buttons | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-JoyTopCover-X1-TPU.3mf` | joystick cap | X1 Carbon, 0.4 mm | 0.2 mm | 2 | 15% | TPU |
| `TintinRocketShooter-Battery-X1-PLA-0.2.3mf` | battery end pieces, plus and minus | X1 Carbon, 0.2 mm | 0.06 mm | 6 | 15% | PLA |
| `TintinRocketShooter-Stand-X1-PLA.3mf` | stand | X1 Carbon, 0.4 mm | 0.12 mm | 3 | 15% | PLA |

The body - lower part, upper part and top - comes in two sets of projects: `-H2D-` for an
H2D with a 0.4 mm nozzle, in glass-filled ABS (upper part and top also in glass-filled PLA;
the lower part has no PLA-GF project), and `-X1-` for an X1 Carbon with a 0.2 mm nozzle, in
PLA. Print one set.

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

The file names follow one pattern: console, part, printer, material, `-Support` if the
project prints supports (with their filament in front where it is a separate one, as in
`-ABS-Support`), and `-0.2` for the 0.2 mm nozzle (no suffix: 0.4 mm). For example
`TintinRocketShooter-Top-H2D-ABS-GF-ABS-Support.3mf` is the top for an H2D, in ABS-GF with
supports in ABS, on the 0.4 mm nozzle.
