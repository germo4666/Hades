#!/usr/bin/env python3
"""Generates Source/Presets.inc (the 100 factory presets).
Values are in real parameter units; choices are indexes; bools are 0/1.
Anything not listed falls back to the parameter default."""

# choice indexes
TAPE_SAT, TUBE, XFMR, FOLD, CRUSH, RECT, FUZZ = range(7)
WOW, JUNO, DIM, ENS, VIB = range(5)
VCA, OPTO, FET, PUMP = range(4)
Q4, E8D, E8, S16, FREE = range(5)
ROOM, PLATE, SPRING, HALL, HAZE = range(5)
HISS, VINYL, ROOMN, HUM = range(4)
DRUMS, BASS, KEYS, GTR, VOX, BUS = range(6)

def P(cat, name, **v): return (cat, name, v)

presets = [
# ---------------- Drums (15) ----------------
P("Drums","Dusty Breaks", source=DRUMS, transKeep=1, heatAmt=.35, tapeAmt=.62, tapeWear=.35, tapeDrops=.15, tapeDrift=.2, dustLevel=.3, dustType=VINYL, driftAmt=.15, glueAmt=.45, spaceAmt=.2, spaceDecay=.9),
P("Drums","Room Smash", source=DRUMS, heatMode=TUBE, heatAmt=.55, tapeAmt=.3, glueMode=FET, glueAmt=.7, glueMix=.6, spaceAmt=.35, spaceSize=.3, spaceDecay=.7, driftOn=0),
P("Drums","Lo-Hat Shimmer", source=DRUMS, heatAmt=.2, grainOn=1, grainAmt=.25, grainPitch=12, grainSize=60, grainDensity=.6, grainLo=2000, spaceMode=PLATE, spaceAmt=.25, driftOn=0),
P("Drums","Boom Bap Glue", source=DRUMS, heatMode=TUBE, heatAmt=.3, tapeAmt=.55, tapeSpeed=0, glueMode=OPTO, glueAmt=.5, dustLevel=.2, dustType=VINYL, lowGain=2, highGain=-2, driftAmt=.1),
P("Drums","Tape Kit", source=DRUMS, transKeep=1, tapeAmt=.7, tapeBump=.7, tapeWear=.25, glueAmt=.35, heatAmt=.15, driftOn=0, spaceAmt=.12),
P("Drums","SP Crunch", source=DRUMS, heatMode=CRUSH, heatAmt=.45, heatHi=12000, tapeAmt=.3, glueAmt=.4, driftOn=0, spaceAmt=.1),
P("Drums","Pump Bus", source=DRUMS, glueMode=PUMP, glueAmt=.55, glueRelease=180, tapeAmt=.3, driftOn=0, spaceAmt=.15),
P("Drums","Haze Snare", source=DRUMS, heatAmt=.25, spaceMode=HAZE, spaceAmt=.3, spaceDecay=4, spaceDuck=.5, spaceLo=300, driftOn=0),
P("Drums","Spring Kit", source=DRUMS, heatAmt=.3, spaceMode=SPRING, spaceAmt=.3, spaceDecay=1.6, driftOn=0),
P("Drums","Fold Perc", source=DRUMS, heatMode=FOLD, heatAmt=.4, heatLo=400, driftAmt=.25, spaceAmt=.2),
P("Drums","Cassette Loop", source=DRUMS, tapeAmt=.6, tapeSpeed=0, tapeWear=.55, tapeDrops=.3, tapeDrift=.35, dustLevel=.35, dustType=HISS, driftAmt=.3, glueAmt=.3),
P("Drums","Parallel Smash", source=DRUMS, heatMode=XFMR, heatAmt=.4, glueMode=FET, glueAmt=.85, glueMix=.45, driftOn=0, spaceAmt=.15),
P("Drums","Grain Groove", source=DRUMS, grainOn=1, grainAmt=.3, grainTime=S16, grainSize=40, grainDensity=.7, grainSpray=.2, grainReverse=.1, grainFeedback=.2, tapeAmt=.35, driftOn=0),
P("Drums","Velvet Kit", source=DRUMS, glueMode=OPTO, glueAmt=.35, tapeAmt=.35, highGain=-2, spaceAmt=.15, heatAmt=.15, driftOn=0),
P("Drums","Overdriven Break", source=DRUMS, transKeep=1, heatMode=FUZZ, heatAmt=.45, heatMix=.5, tapeAmt=.4, glueAmt=.5, driftAmt=.1),
# ---------------- Bass (10) ----------------
P("Bass","Warm Sub Glue", source=BASS, heatAmt=.2, tapeAmt=.45, tapeBump=.7, glueMode=OPTO, glueAmt=.4, spaceOn=0, driftOn=0),
P("Bass","Fuzz Top", source=BASS, lowProt=1, lowProtFreq=100, heatMode=FUZZ, heatAmt=.5, heatLo=300, tapeAmt=.3, spaceOn=0, driftOn=0),
P("Bass","Tape Bass", source=BASS, tapeAmt=.65, tapeSpeed=0, tapeBump=.8, glueAmt=.3, heatAmt=.15, spaceOn=0, driftOn=0),
P("Bass","Tube Bass", source=BASS, heatMode=TUBE, heatAmt=.45, glueAmt=.4, tapeAmt=.3, spaceOn=0, driftOn=0),
P("Bass","Wobble Bass", source=BASS, driftAmt=.4, driftRate=.3, tapeAmt=.4, spaceOn=0),
P("Bass","Crunch Reese", source=BASS, heatMode=CRUSH, heatAmt=.3, driftMode=JUNO, driftAmt=.4, driftMix=.6, glueAmt=.4, spaceOn=0),
P("Bass","Dub Bass", source=BASS, tapeAmt=.5, tapeSpeed=0, highGain=-4, highFreq=3000, spaceMode=SPRING, spaceAmt=.12, driftOn=0),
P("Bass","Fold Bass", source=BASS, heatMode=FOLD, heatAmt=.35, heatLo=200, glueAmt=.4, spaceOn=0, driftOn=0),
P("Bass","Pumping Bass", source=BASS, glueMode=PUMP, glueAmt=.6, glueRelease=220, tapeAmt=.3, spaceOn=0, driftOn=0),
P("Bass","Clean Glue Bass", source=BASS, heatAmt=.1, tapeAmt=.25, glueMode=OPTO, glueAmt=.45, spaceOn=0, driftOn=0),
# ---------------- Synths / Keys (15) ----------------
P("Keys","Juno Haze", source=KEYS, driftMode=JUNO, driftAmt=.5, driftMix=.8, spaceMode=HALL, spaceAmt=.25, spaceDecay=3, tapeAmt=.35),
P("Keys","Cassette Rhodes", source=KEYS, tapeAmt=.6, tapeSpeed=0, tapeWear=.4, tapeDrops=.2, driftAmt=.35, dustLevel=.25, dustType=HISS, spaceAmt=.18),
P("Keys","Dimension Pad", source=KEYS, driftMode=DIM, driftAmt=.5, driftMix=.8, spaceMode=HALL, spaceAmt=.3, spaceDecay=4),
P("Keys","Ensemble Strings", source=KEYS, driftMode=ENS, driftAmt=.55, driftMix=.9, spaceMode=PLATE, spaceAmt=.25, tapeAmt=.3),
P("Keys","Warped Keys", source=KEYS, driftAmt=.7, driftRate=.2, driftFlux=.6, tapeAmt=.5, spaceAmt=.2),
P("Keys","Vibrato Organ", source=KEYS, driftMode=VIB, driftAmt=.45, driftRate=.6, heatMode=TUBE, heatAmt=.35, spaceAmt=.2),
P("Keys","Lo-fi Piano", source=KEYS, tapeAmt=.65, tapeSpeed=0, tapeWear=.5, dustLevel=.35, dustType=VINYL, highGain=-4, driftAmt=.25, spaceAmt=.18),
P("Keys","Glass Grains", source=KEYS, grainOn=1, grainAmt=.35, grainPitch=12, grainSize=120, grainDensity=.5, grainSpray=.5, grainFeedback=.4, spaceMode=HAZE, spaceAmt=.3, driftOn=0),
P("Keys","Frozen Chord", source=KEYS, grainOn=1, grainAmt=.45, grainSize=250, grainDensity=.8, grainSpray=.8, grainFeedback=.6, grainDuck=.2, spaceMode=HAZE, spaceAmt=.35, spaceDecay=8),
P("Keys","Digital Dream", source=KEYS, heatMode=CRUSH, heatAmt=.35, driftMode=JUNO, driftAmt=.4, spaceMode=PLATE, spaceAmt=.3),
P("Keys","Plate Lead", source=KEYS, spaceMode=PLATE, spaceAmt=.3, spaceDecay=2, driftMode=ENS, driftAmt=.2, driftMix=.4, heatAmt=.25),
P("Keys","Arp Spring", source=KEYS, spaceMode=SPRING, spaceAmt=.3, grainOn=1, grainAmt=.2, grainTime=E8, grainFeedback=.45, grainSpray=.1, driftOn=0),
P("Keys","Synth Bus Glue", source=KEYS, glueAmt=.35, tapeAmt=.35, heatAmt=.2, driftAmt=.1, spaceAmt=.12),
P("Keys","Sunburn Pad", source=KEYS, heatMode=TUBE, heatAmt=.3, tapeAmt=.5, driftAmt=.3, spaceMode=HALL, spaceAmt=.3, tilt=-1.5),
P("Keys","Broken Synth", source=KEYS, tapeAmt=.55, tapeDrops=.5, tapeDrift=.5, driftAmt=.5, driftRate=.7, driftFlux=.8, dustLevel=.2, dustType=HUM, fluxChaos=.6),
# ---------------- Guitars (10) ----------------
P("Guitar","Bedroom Amp", source=GTR, heatMode=TUBE, heatAmt=.5, heatHi=7000, spaceAmt=.2, driftOn=0),
P("Guitar","Spring Drip", source=GTR, spaceMode=SPRING, spaceAmt=.4, spaceDecay=2.2, heatAmt=.3, driftAmt=.2),
P("Guitar","Chorus Clean", source=GTR, driftMode=JUNO, driftAmt=.45, driftMix=.7, spaceMode=PLATE, spaceAmt=.2, heatAmt=.15),
P("Guitar","Tape Echo Gtr", source=GTR, grainOn=1, grainAmt=.3, grainTime=E8D, grainSize=300, grainDensity=.3, grainSpray=.05, grainReverse=0, grainFeedback=.5, grainHi=5000, tapeAmt=.4, driftAmt=.15),
P("Guitar","Fuzz Wall", source=GTR, heatMode=FUZZ, heatAmt=.6, spaceMode=HALL, spaceAmt=.25, glueAmt=.4, driftOn=0),
P("Guitar","Shoegaze Haze", source=GTR, driftMode=ENS, driftAmt=.5, spaceMode=HAZE, spaceAmt=.45, spaceDecay=8, heatMode=FOLD, heatAmt=.3),
P("Guitar","Acoustic Air", source=GTR, highGain=3, glueMode=OPTO, glueAmt=.3, spaceAmt=.15, heatAmt=.15, driftOn=0),
P("Guitar","Surf Spring", source=GTR, spaceMode=SPRING, spaceAmt=.5, spaceDecay=3, driftMode=VIB, driftAmt=.3),
P("Guitar","Lo-fi Strum", source=GTR, tapeAmt=.6, tapeSpeed=0, tapeWear=.4, dustLevel=.3, dustType=HISS, driftAmt=.3, spaceAmt=.15),
P("Guitar","Reverse Swells", source=GTR, grainOn=1, grainAmt=.4, grainReverse=.9, grainSize=350, grainDensity=.5, grainFeedback=.3, spaceMode=HALL, spaceAmt=.3, driftOn=0),
# ---------------- Vocals (12) ----------------
P("Vocals","Rap Glue", source=VOX, hpFreq=80, highGain=2, glueMode=FET, glueAmt=.45, glueMix=.7, heatMode=XFMR, heatAmt=.3, heatLo=200, spaceAmt=.12, driftOn=0),
P("Vocals","Ghost Double", source=VOX, driftMode=DIM, driftAmt=.45, driftMix=.7, spaceMode=PLATE, spaceAmt=.15, glueAmt=.3),
P("Vocals","Phone Verse", source=VOX, hpFreq=400, highGain=-10, highFreq=3500, midGain=6, midFreq=1500, heatMode=XFMR, heatAmt=.5, spaceOn=0, driftOn=0),
P("Vocals","Tape Vox", source=VOX, tapeAmt=.5, glueMode=OPTO, glueAmt=.4, heatAmt=.2, spaceAmt=.12, driftAmt=.1),
P("Vocals","Dark Plate Vox", source=VOX, spaceMode=PLATE, spaceAmt=.3, spaceDecay=1.8, spaceHi=5000, spaceDuck=.5, glueAmt=.35, driftOn=0),
P("Vocals","Hook Haze", source=VOX, spaceMode=HAZE, spaceAmt=.3, spaceDuck=.6, driftMode=ENS, driftAmt=.25, driftMix=.4, glueAmt=.35),
P("Vocals","Radio Ad-lib", source=VOX, hpFreq=300, heatMode=TUBE, heatAmt=.5, heatLo=500, heatHi=4000, spaceAmt=.15, driftOn=0),
P("Vocals","Grain Choir", source=VOX, grainOn=1, grainAmt=.3, grainPitch=12, grainSize=200, grainDensity=.7, grainSpray=.6, grainDuck=.7, spaceMode=HALL, spaceAmt=.3, driftOn=0),
P("Vocals","Lo-fi Vox", source=VOX, tapeAmt=.6, tapeSpeed=0, dustLevel=.25, dustType=VINYL, highGain=-3, driftAmt=.15),
P("Vocals","Slap Room", source=VOX, grainOn=1, grainAmt=.25, grainTime=FREE, grainFree=110, grainSize=110, grainDensity=.3, grainSpray=0, grainReverse=0, grainFeedback=.1, grainDuck=.2, spaceAmt=.15, driftOn=0),
P("Vocals","Airy Lead Vox", source=VOX, highGain=3, highFreq=10000, glueMode=OPTO, glueAmt=.35, spaceMode=PLATE, spaceAmt=.2, spaceDuck=.5, driftOn=0),
P("Vocals","Demon Low", source=VOX, grainOn=1, grainAmt=.35, grainPitch=-12, grainSize=120, grainDensity=.6, grainSpray=.2, grainDuck=.3, heatMode=TUBE, heatAmt=.3, driftOn=0),
# ---------------- Mix / Bus (10) ----------------
P("Mix Bus","Same Room", source=BUS, heatAmt=.15, tapeAmt=.3, glueAttack=30, glueRelease=300, glueAmt=.3, spaceAmt=.12, spaceSize=.3, driftOn=0),
P("Mix Bus","Tape Master", source=BUS, tapeAmt=.55, tapeSpeed=2, glueAmt=.25, heatOn=0, driftOn=0, spaceOn=0),
P("Mix Bus","Warm Bus", source=BUS, heatMode=TUBE, heatAmt=.2, tapeAmt=.4, lowGain=1, highGain=-1, driftOn=0, spaceOn=0),
P("Mix Bus","Punch Bus", source=BUS, transKeep=1, glueAmt=.45, glueAttack=30, tapeAmt=.3, driftOn=0, spaceOn=0),
P("Mix Bus","Vintage Glue", source=BUS, tapeAmt=.5, tapeWear=.2, glueMode=OPTO, glueAmt=.35, dustLevel=.15, dustType=HISS, driftAmt=.1, spaceOn=0),
P("Mix Bus","Wide & Warm", source=BUS, driftMode=DIM, driftAmt=.3, driftMix=.4, tapeAmt=.35, spaceOn=0),
P("Mix Bus","Night Drive", source=BUS, tilt=-2, tapeAmt=.4, spaceMode=HALL, spaceAmt=.15, driftOn=0),
P("Mix Bus","Loud & Clear", source=BUS, glueAmt=.5, glueAttack=20, heatMode=XFMR, heatAmt=.25, tilt=1, driftOn=0, spaceOn=0),
P("Mix Bus","Glue Only", source=BUS, heatOn=0, tapeOn=0, driftOn=0, spaceOn=0, glueAmt=.35),
P("Mix Bus","Old Record", source=BUS, tapeAmt=.6, tapeSpeed=0, dustLevel=.4, dustType=VINYL, hpFreq=60, highGain=-5, highFreq=6000, driftAmt=.25),
# ---------------- Lo-fi / Character (10) ----------------
P("Lo-fi","Thrift Tape", tapeAmt=.6, tapeSpeed=0, tapeWear=.5, tapeDrops=.25, driftAmt=.3, dustLevel=.3, dustType=HISS, heatAmt=.2),
P("Lo-fi","Sunburn", heatMode=TUBE, heatAmt=.35, tapeAmt=.5, spaceMode=HALL, spaceAmt=.2, tilt=-1.5, driftAmt=.2),
P("Lo-fi","Walkman", tapeAmt=.7, tapeSpeed=0, tapeWear=.7, tapeDrops=.35, tapeDrift=.4, driftAmt=.55, driftRate=.7, hpFreq=120, highGain=-6, highFreq=5000, dustLevel=.35, dustType=HISS),
P("Lo-fi","VHS Memory", driftAmt=.5, driftRate=.25, tapeAmt=.55, heatMode=CRUSH, heatAmt=.15, highGain=-4, dustLevel=.3, dustType=HUM),
P("Lo-fi","8-Bit Dust", heatMode=CRUSH, heatAmt=.65, dustLevel=.25, dustType=VINYL, spaceAmt=.1, driftOn=0),
P("Lo-fi","Basement Session", spaceAmt=.3, spaceSize=.2, spaceDecay=.6, spaceDamp=.7, heatMode=TUBE, heatAmt=.3, dustLevel=.25, dustType=ROOMN),
P("Lo-fi","Radio Ghost", hpFreq=500, highGain=-8, highFreq=3000, heatMode=XFMR, heatAmt=.4, dustLevel=.3, dustType=HUM, spaceMode=PLATE, spaceAmt=.2),
P("Lo-fi","Dusty Shelf", tapeAmt=.5, tapeWear=.4, dustLevel=.45, dustType=VINYL, highGain=-3, driftAmt=.15),
P("Lo-fi","Warped Vinyl", dustLevel=.4, dustType=VINYL, driftAmt=.6, driftRate=.35, tapeAmt=.4),
P("Lo-fi","Sample Lab", heatMode=CRUSH, heatAmt=.4, heatHi=15000, tapeAmt=.45, tapeSpeed=0, glueMode=OPTO, glueAmt=.3, driftOn=0),
# ---------------- Ambient / Space (10) ----------------
P("Ambient","Haze Wash", spaceMode=HAZE, spaceAmt=.5, spaceDecay=10, spaceSize=.8, driftMode=ENS, driftAmt=.3, driftMix=.5),
P("Ambient","Frozen Grain", grainOn=1, grainAmt=.5, grainSize=300, grainDensity=.9, grainSpray=.9, grainFeedback=.7, grainDuck=.1, spaceMode=HAZE, spaceAmt=.45, spaceDecay=12),
P("Ambient","Cathedral", spaceMode=HALL, spaceAmt=.5, spaceDecay=9, spaceSize=1, spaceDamp=.4, driftOn=0),
P("Ambient","Endless Haze", spaceMode=HAZE, spaceAmt=.6, spaceDecay=20, spaceSize=1, spaceMod=.8, spaceDuck=.3),
P("Ambient","Shimmer Up", grainOn=1, grainAmt=.4, grainPitch=12, grainSize=200, grainDensity=.7, grainSpray=.5, grainFeedback=.6, spaceMode=HALL, spaceAmt=.45, spaceDecay=7),
P("Ambient","Under Water", highGain=-10, highFreq=1200, driftAmt=.5, driftRate=.2, spaceMode=HAZE, spaceAmt=.35),
P("Ambient","Dream Plate", spaceMode=PLATE, spaceAmt=.45, spaceDecay=5, driftMode=JUNO, driftAmt=.4, driftMix=.6),
P("Ambient","Grain Cloud", grainOn=1, grainAmt=.5, grainSize=120, grainDensity=1, grainSpray=1, grainFeedback=.5, grainReverse=.4, spaceMode=HALL, spaceAmt=.3),
P("Ambient","Distant Room", hpFreq=200, highGain=-3, spaceAmt=.5, spaceSize=.6, spaceDecay=1.5, spaceDuck=0, driftOn=0),
P("Ambient","Infinite Spring", spaceMode=SPRING, spaceAmt=.55, spaceDecay=8, driftAmt=.2),
# ---------------- FX / Destroy (8) ----------------
P("FX","Melt", driftAmt=1, driftRate=.15, driftFlux=1, tapeAmt=.7, tapeDrops=.4, spaceMode=HAZE, spaceAmt=.3),
P("FX","Broken Walkman", tapeAmt=.85, tapeSpeed=0, tapeWear=.9, tapeDrops=.8, tapeDrift=.8, driftAmt=.8, driftRate=.8, driftFlux=1, dustLevel=.5, dustType=HISS, fluxChaos=.8),
P("FX","Inferno", heatMode=FUZZ, heatAmt=.85, glueMode=FET, glueAmt=.7, spaceMode=HALL, spaceAmt=.3, spaceDecay=3),
P("FX","Underworld", grainOn=1, grainAmt=.5, grainPitch=-12, grainSize=250, grainDensity=.8, grainSpray=.6, grainFeedback=.6, spaceMode=HAZE, spaceAmt=.5, spaceDecay=12, highGain=-6),
P("FX","Glitch Grains", grainOn=1, grainAmt=.6, grainTime=S16, grainSize=30, grainDensity=1, grainSpray=.3, grainReverse=.5, grainFeedback=.4, grainFlux=1, fluxChaos=.9, driftOn=0),
P("FX","Total Fold", heatMode=FOLD, heatAmt=.9, heatMix=.8, glueMode=PUMP, glueAmt=.5),
P("FX","Bitcrush Hell", heatMode=CRUSH, heatAmt=.9, glueMode=FET, glueAmt=.6, spaceAmt=.2, driftOn=0),
P("FX","Styx", driftMode=VIB, driftAmt=.6, driftRate=.3, grainOn=1, grainAmt=.4, grainReverse=.8, grainFeedback=.5, spaceMode=HAZE, spaceAmt=.45, spaceDecay=10),
]

assert len(presets) == 100, len(presets)
names = [p[1] for p in presets]
assert len(set(names)) == 100, "duplicate names"

# validate ids against Params.h
import re, pathlib
root = pathlib.Path(__file__).resolve().parents[1]
ids = set(re.findall(r"X \((\w+)\s*,", (root / "Source/Params.h").read_text()))
for _, n, v in presets:
    for k in v:
        assert k in ids, f"{n}: unknown param {k}"

out = ["// Generated by tools/make_presets.py - edit that file, not this one.",
       "const std::vector<Preset>& factoryPresets()", "{", "    static const std::vector<Preset> p {"]
for cat, name, v in presets:
    vals = ", ".join(f'{{ "{k}", {float(x)}f }}' for k, x in v.items())
    out.append(f'        {{ "{cat}", "{name}", {{ {vals} }} }},')
out += ["    };", "    return p;", "}", ""]
(root / "Source/Presets.inc").write_text("\n".join(out))
print("wrote", len(presets), "presets")
