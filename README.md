# HADES by GERMO

A six-module "cohesion strip" plugin (VST3 / AU / standalone) — tape, heat, drift, glue, grain and space, plus a soft EQ — built to make every sound in a mix feel like it was recorded in the same room. Works on drums, bass, synths, guitars, vocals and whole buses.

## Install (Mac)
1. Open the **Releases** page of this repo and download the newest `HADES-mac.zip`.
2. Unzip it, right-click **Install HADES.command** → **Open**.
3. In Ableton: **Settings → Plug-Ins → Rescan**. HADES shows up under GERMO.

Every push to `main` builds a new universal (Apple Silicon + Intel) release automatically and runs the test suite on all 100 presets first.

## Signal flow
```
input → [low protect split] → SOFT EQ (pre) → HEAT → TAPE → DRIFT → GLUE → GRAIN → SPACE → SOFT EQ (post)
      → + protected lows → + DUST layer → tone tilt → LU match → output
```
Routing **Parallel** feeds GRAIN and SPACE side by side from GLUE instead of in series.

## Modules
| Module | What it does | Modes |
|---|---|---|
| **HEAT** | Distortion on a focus band only; adds back just the difference it makes, with auto-gain and a tone tilt | Tape Sat, Tube, Transformer, Fold, Crush, Rectify, Fuzz |
| **TAPE** | Pre-emphasis → saturation → de-emphasis, head bump, HF loss, wear, random dropouts, volume drift, plus the DUST noise layer (hiss / vinyl / room / hum) that breathes with the input | 7.5 / 15 / 30 ips |
| **DRIFT** | Modulated delay with cubic interpolation | Wow/Flutter, Juno, Dimension, Ensemble, Vibrato |
| **GLUE** | Stereo-linked compressor with knee, sidechain HPF, auto release and parallel mix | VCA, Opto, FET, Pump |
| **GRAIN** | Granular delay (4 s buffer, up to 48 grains): size, density, spray, pitch ±12, reverse, feedback, freeze, ducking, tempo sync | 1/4, 1/8 D, 1/8, 1/16, Free |
| **SPACE** | 8-line feedback-delay-network reverb with diffusion, modulation, damping, width, ducking and a focus filter | Room, Plate, Spring, Hall, Haze |

## Global
- **AGE** – one macro that scales every module, the dust and the dropouts.
- **FLUX** – each module has its own Flux depth; global Rate and Chaos. It's a smoothed random walk (not an LFO), 85 % linked left/right, with occasional "events" when Chaos is up.
- **Source** (Drums / Bass / Keys / Gtr / Vox / Bus) re-voices the modules: e.g. Bass keeps everything under 90 Hz clean and mono and tames pitch wobble; Vox keeps HEAT off the low end; Bus makes everything subtler.
- **LU MATCH** – K-weighted loudness match (input vs. processed), slow and gated, so A/B and bypass compare tone, not volume. The Δ readout shows how much louder/quieter the effect would be without it.
- **Low Protect**, **Transient Keep**, **Oversampling** (1× / 2× / 4× on HEAT + TAPE), **Tone tilt**, in/out gain, **A/B**, **Randomize**.
- **Soft EQ**: HP, low shelf, mid bell, high shelf — drag the dots, double-click to reset, mouse-wheel on the mid dot for Q. Pre or post the chain.
- **100 presets** in 9 categories (Drums, Bass, Keys, Guitar, Vocals, Mix Bus, Lo-fi, Ambient, FX). Hosts also see them as programs.

## Build it yourself
```bash
git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git JUCE
cmake -B build -DCMAKE_BUILD_TYPE=Release -DHD_TESTS=ON
cmake --build build --config Release --target Hades_VST3 HadesTest
./build/HadesTest_artefacts/Release/HadesTest
```
Presets live in `tools/make_presets.py` — edit there, then run it to regenerate `Source/Presets.inc`.
