# TRestHaloMetadata Guide

## Overview

`TRestHaloMetadata` is a metadata container that stores information about haloscope spectrum measurements. It captures both **hardware/acquisition parameters** and **optional information** about the experiment, calibration, and data analysis.

Metadata is automatically attached to every `TRestHaloEvent` and provides context for:
- Understanding acquisition conditions
- Cross-referencing calibration files
- Documenting analysis notes
Note: All values here are examples to show formatting types, not actual experimental values!
---

## Metadata Fields

### Acquisition Parameters (Hardware/Processing)

These fields describe the experimental acquisition conditions:

| Field | Type | Units | Description |
|-------|------|-------|-------------|
| `StartTime` | double | Unix timestamp | When acquisition began |
| `IntegrationTime` | double | seconds | Integration time per spectrum |
| `FramesPerSpectrum` | int | count | Number of FFT frames averaged per spectrum |
| `ResolutionBandwidth` | double | Hz | Frequency resolution (Hz per bin) |
| `ReferencePower` | double | dBm | Reference power level at input |
| `PreamplifierEnabled` | bool | — | Whether preamplifier was active |
| `CenterFrequency` | double | Hz | Center frequency of measurement |
| `NumSpectra` | int | count | Total spectra in dataset |
| `NumFreqPoints` | int | count | Number of frequency bins per spectrum |

### Optional Metadata (Experiment/Analysis Info)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `ExperimentName` | string | "" | Experiment identifier (e.g., "RADES", "SUPAX") |
| `RunNumber` | int | -1 | Run identifier |
| `Attenuation` | double | 0 | Attenuation applied (dB) |
| `Notes` | string | "" | Descriptive notes about measurement |
| `LastCal` | string | "" | Calibration file name/path |
| `SGParams` | string | "" | Savitzky-Golay filter parameters from high level analysis |
| `StdDev` | double | 0 | Standard deviation of spectrum or average uncertainty |
| `UncertaintiesProvided` | bool | false | True if per-bin uncertainties were provided |

### Value Units

Data can be stored in two formats:

```python
meta.SetValueUnit(0)  # V_RMS - voltage RMS (default of hardware)
meta.SetValueUnit(1)  # W - Watts (power)
```

Use the enum for clarity:
```python
from ROOT import TRestHaloMetadata
meta.SetValueUnit(TRestHaloMetadata.W)      # Watts
meta.SetValueUnit(TRestHaloMetadata.V_RMS)  # Voltage
```
---

## Quick Start

### Minimal Setup

```python
from ROOT import TRestHaloEvent, TRestHaloMetadata

# Create an event with metadata
event = TRestHaloEvent()
meta = TRestHaloMetadata()

# Set essential fields
meta.SetExperimentName("IAXO")
meta.SetRunNumber(12345)
meta.SetNumFreqPoints(len(frequencies))
meta.SetResolutionBandwidth(1.0)  # Hz
meta.SetCenterFrequency(1.4e9)     # Hz

# Attach to event
event.SetMetadata(meta)
```

### Full Setup with All Metadata

```python
meta = TRestHaloMetadata()

# Acquisition parameters
meta.SetStartTime(1622505600)           # Unix timestamp
meta.SetIntegrationTime(3600)           # seconds
meta.SetFramesPerSpectrum(100)          # averaging count
meta.SetResolutionBandwidth(1.0)        # Hz/bin
meta.SetReferencePower(-20)             # dBm
meta.SetPreamplifierEnabled(True)       # bool
meta.SetCenterFrequency(1.4e9)          # Hz
meta.SetNumSpectra(1000)                # spectra in dataset
meta.SetNumFreqPoints(8000)             # bins per spectrum

# Optional information
meta.SetExperimentName("RADES")
meta.SetRunNumber(12345)
meta.SetAttenuation(10)                 # dB
meta.SetNotes("Background measurement during calibration")
meta.SetLastCal("cal_2026-06-05.root")
meta.SetSGParams("window=51, order=3")
meta.SetStdDev(0.05)                    # V_RMS or Watts
meta.SetUncertaintiesProvided(True)     # uncertainties in event

# Data format
meta.SetValueUnit(1)                    # 0=V_RMS, 1=Watts
```

---

## Reading Metadata

### Access Individual Fields

```python
event = TRestHaloEvent()
meta = event.GetMetadata()

exp_name = meta.GetExperimentName()
run_num = meta.GetRunNumber()
center_freq = meta.GetCenterFrequency()
num_points = meta.GetNumFreqPoints()
unit = meta.GetValueUnit()
unit_str = meta.GetValueUnitString()  # "V_RMS" or "W"
```

### Print All Metadata

```python
meta.PrintMetadata()
```

Output includes:
```
=== Halo Metadata ===
 Experiment        : RADES
 Run number        : 12345
 Start time        : 1622505600
 Integration time  : 3600 s
 Frames/spectrum   : 100
 RBW               : 1 Hz
 Reference power   : -20 dBm
 Preamp enabled    : 1
 Center freq       : 1.4e+09 Hz
 Num spectra       : 1000
 Frequency points  : 8000
 Attenuation       : 10 dB
 Notes             : Background measurement
 Calibration file  : cal_2026-06-05.root
 SG Parameters     : window=51, order=3
 Standard Deviation: 0.05
 Uncertainties     : Provided
 Stored value unit : W
```

---

## Extending Metadata

For custom needs beyond standard fields, use the `Notes` field:

```python
meta.SetNotes("""
    Custom parameters:
    - Temperature: 25°C
    - Humidity: 45%
    - Magnetic field: 0.5 Tesla
""")
```

For permanent extensions, modify `TRestHaloMetadata` class directly (requires code changes and recompilation).

---

## See Also

- [processor.py](processor.py) - Example Python processing script with metadata
- TRestHaloMetadata.h/cxx - Source code for metadata class
