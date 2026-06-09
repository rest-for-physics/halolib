The Halolib library was created to centralize data structures and early analysis
procedures for the haloscope RADES.
It assumes RF type readouts of power spectra and includes processes to prepare
the data for advanced axion signal search analysis outside the framework.



# Low-Level Data Processing with TRestHalolib: Complete Tutorial

This tutorial demonstrates a complete data processing workflow for haloscope (frequency spectrum) data using the REST framework's halolib library. The `processor.py` script serves as a reference implementation for converting raw spectrum data into analyzable TRestHaloEvent objects with advanced processing capabilities.

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Architecture](#architecture)
4. [Detailed Workflow](#detailed-workflow)
5. [Running the Script](#running-the-script)
6. [Understanding Each Step](#understanding-each-step)
7. [Customization Guide](#customization-guide)
8. [Advanced Topics](#advanced-topics)
9. [Troubleshooting](#troubleshooting)

---

## Overview

### What This Script Does

The processor script implements a complete data pipeline:

```
Input TGraphs (Raw Data)
    ↓
[Convert] → TRestHaloEvent objects
    ↓
[Enhance] → Add metadata (frequency info, experiment details)
    ↓
[Process] → Trim spectra to uniform bin count
    ↓
[Output] → ROOT file with trimmed events
    ↓
[Combine] → Chi2-weighted average of first two spectra
    ↓
[Output] → Separate ROOT file with combined spectrum
```

### Key Capabilities

- **Format Conversion**: TGraph → TRestHaloEvent
- **Unit Conversion**: Volts (V_RMS) ↔ Watts (power)
- **Spectrum Trimming**: Chop data to the physics region you are interested in
- **Metadata Tracking**: Experiment conditions, frequency parameters
- **Spectral Combination**: Chi2-weighted averaging with per-bin uncertainties
- **Batch Processing**: Efficient handling of thousands of spectra

---

## Prerequisites

### Software Requirements

- **Python 3.6+** with PyROOT bindings (this processing script only, not necessary for halolib itself)
- **ROOT Framework** 6.0+ 
- **REST Framework** 2.4.3+ with halolib library

### Installation Verification

```bash
# Check ROOT installation
root --version

# Verify libhalolib is installed
ls -l /path/to/rest-install/lib/libhalolib.so

# Test Python/ROOT integration
python3 -c "from ROOT import TFile; print('ROOT available')"
```

### Data Requirements

Input data is a ROOT file format containing:
- **TGraph objects** with names starting with `SpectrumSumFile_`
- **X-axis values**: Frequency points (Hz)
- **Y-axis values**: Amplitude values (typically raw is V_RMS)
Note: ROOT file not necessary to initialize REST objects, only arrays for assigning X and Y values

---

## Architecture

### Core REST Classes

#### 1. **TRestHaloEvent**
The fundamental container for spectrum data.

```
TRestHaloEvent
├── fFrequency[]           - Frequency points (Hz)
├── fValues[]              - Spectrum values (V_RMS or Watts)
├── fUncertainties[]       - Per-bin uncertainties, defaults as sampling standard deviation
├── fMetadata              - Experiment information, often manually declared 
├── fStartFrequency        - First frequency point
├── fStopFrequency         - Last frequency point
└── fResolution            - Bin size (Hz per bin)
```

**Key Methods:**
- `SetSpectrum(freqs, values, unit)` - Initialize with uncertainties computed by sampling standard deviation
- `SetSpectrum(freqs, values, uncertainties, unit)` - Initialize with measured uncertainties (given array)
- `GetFrequencies()` - Retrieve frequency array
- `GetValues()` - Retrieve spectrum values
- `GetUncertainties()` - Retrieve uncertainty array
- `GetValueAtFrequency(f, unit)` - Interpolate value at specific frequency

#### 2. **TRestHaloMetadata**
Stores experimental and statistical information about spectrum.

```
TRestHaloMetadata
├── fValueUnit             - Unit type (V_RMS=0, W=1)
├── fNumFreqPoints         - Number of frequency bins
├── fResolutionBandwidth   - Bin size in Hz
├── fCenterFrequency       - Central frequency of spectrum
├── fExperimentName        - Experiment identifier
├── fNotes                 - Description/comments
├── fLastCal               - Most recent calibration file
├── fStdDev                - Standard deviation/average uncertainty
└── fUncertaintiesProvided - Flag: uncertainties computed vs. provided
```

**Key Methods:**
- `SetValueUnit(unit)` - Set unit (0 for V_RMS, 1 for Watts)
- `SetNumFreqPoints(n)` - Store bin count
- `SetResolutionBandwidth(res)` - Store frequency resolution
- `SetCenterFrequency(f)` - Store center frequency
- `PrintMetadata()` - Display all metadata

#### 3. **TRestHaloTrimProcess**
Resamples spectra to uniform bin count.

```cpp
TRestHaloTrimProcess
├── SetTrimBins(n)         - Target bin count
└── ProcessEvent(event)    - Perform resampling → returns new TRestHaloEvent
```

**Algorithm**: Linearly interpolates spectrum values from original bins to new uniform grid.

#### 4. **TRestHaloCombine**
Combines multiple spectra with chi2 weighting.

```cpp
TRestHaloCombine
├── AddEvent(event)        - Queue event for combination
├── Combine()              - Perform sequential pairwise combination
├── GetCombinedEvent()     - Retrieve result
└── Clear()                - Reset for new combination
```

**Algorithm**: 
- Combines events sequentially: E₀ + E₁ → R₁, then R₁ + E₂ → R₂, etc.
- For overlapping frequency bins: Chi2-weighted average
- For non-overlapping bins: Values copied from single source
- Per-bin uncertainties preserved independently

---

## Detailed Workflow

### Phase 1: Data Loading

```python
fin = TFile.Open(infile)
```

Opens ROOT file containing raw TGraph objects. The file pointer remains open during processing.

**What's in `fin`:**
```
20241115-161219_12kA_Weekends_Datataking_0.root
├── SpectrumSumFile_0     (TGraph)
├── SpectrumSumFile_1     (TGraph)
├── SpectrumSumFile_2     (TGraph)
...
└── SpectrumSumFile_1999  (TGraph)
```

### Phase 2: Output Preparation

```python
fout = TFile.Open(outfile, 'RECREATE')
```

Creates new ROOT file for trimmed events. The `RECREATE` mode overwrites any existing file.

### Phase 3: Per-Spectrum Processing

For each TGraph matching `SpectrumSumFile_*` pattern:

#### Step 3a: Data Extraction
```python
npts = obj.GetN()                              # Number of points
xs = obj.GetX()                                # Frequency array
ys = obj.GetY()                                # Amplitude array
freqs = [xs[i] for i in range(npts)]          # Convert to Python list
vals_v = [ys[i] for i in range(npts)]         # Values in V_RMS
```

#### Step 3b: Unit Conversion (V_RMS → Watts)
```python
vals_w = [TRestHaloEvent.VoltsToWatts(v) for v in vals_v]
```

**Conversion formula:**
$$P = \frac{V^2}{50 \, \Omega}$$

where 50Ω is standard impedance for RF systems.

**Why convert?**
- Watts represent absolute power levels (load-independent)
- V_RMS depends on measurement system impedance
- Standard for haloscope physics analysis

#### Step 3c: Event Creation
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, vals_w, 1)  # 1 = Watts unit
```

Creates TRestHaloEvent with:
- Input frequency array
- Input power values (Watts)
- Unit specified as Watts
- Automatic uncertainty computation: standard deviation of spectrum values

#### Step 3d: Metadata Population
```python
meta = ev.GetMetadata()
meta.SetValueUnit(1)                          # Watts
meta.SetNumFreqPoints(npts)                   # Bin count
meta.SetResolutionBandwidth(freqs[1] - freqs[0])  # Hz per bin
meta.SetCenterFrequency((freqs[0] + freqs[-1])/2)  # Central frequency
meta.SetExperimentName('ConvertedFromTGraph')
meta.SetNotes('Converted with processor.py')
```

**Metadata Persistence**: These values are serialized with the event in ROOT file, preserving experimental context.

#### Step 3e: Spectrum Trimming
```python
proc = TRestHaloTrimProcess()
proc.SetTrimBins(14000)              # Target: 14000 bins
out_ev = proc.ProcessEvent(ev)       # Returns resampled event
```

**Why trim?**
- Input spectra have varying bin counts (data-dependent)
- ML analysis requires uniform input dimensions
- 14000 bins provides reasonable frequency resolution:
  - If spectrum spans 1-5 GHz (4 GHz): resolution = 286 kHz/bin
  - Captures relevant haloscope features while reducing data size

**Resampling Algorithm:**
1. Creates uniform frequency grid: 14000 points from min to max input frequency
2. Linearly interpolates spectrum values onto new grid
3. Preserves spectral shape while standardizing bin count

#### Step 3f: Output to ROOT
```python
fout.cd()                           # Select output file
fout.WriteObject(write_ev, name)    # Save with original name
```

Object stored in ROOT file with ROOT's serialization format (.so binary encoding).

### Phase 4: Event Collection for Combination

```python
if processed < 2:
    first_two_events.append(write_ev)
```

Holds references to first two trimmed events for later combination. These remain in memory after main loop.

### Phase 5: Combined Spectrum Generation

```python
if len(first_two_events) >= 2:
    combiner = TRestHaloCombine()
    combiner.AddEvent(first_two_events[0])
    combiner.AddEvent(first_two_events[1])
    combined_event = combiner.Combine()
```

**Combination Algorithm** (chi2-weighted average):

For overlapping frequency bins:
$$C_i = \frac{A_i/\sigma_{A,i}^2 + B_i/\sigma_{B,i}^2}{1/\sigma_{A,i}^2 + 1/\sigma_{B,i}^2}$$

For non-overlapping bins:
$$C_i = \begin{cases} A_i & \text{if only in spectrum A} \\ B_i & \text{if only in spectrum B} \end{cases}$$

**Result characteristics:**
- Spans full frequency range (union of both spectra)
- Overlapping regions have reduced uncertainty: $\sigma_C = \sqrt{\frac{1}{1/\sigma_A^2 + 1/\sigma_B^2}}$
- Non-overlapping regions preserve original values/uncertainties
- Per-bin uncertainties maintained independently

### Phase 6: Combined Output

```python
fout_comb = TFile.Open('test_combination.root', 'RECREATE')
fout_comb.WriteObject(combined_event, 'CombinedSpectrum_0_1')
fout_comb.Close()
```

Saves combined spectrum to separate file for analysis.

---

## Running the Script

### Basic Usage

```bash
cd /path/to/rest-halolib/RADES/sample_scripts

python3 processor.py input.root output.root [libpath]
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `input.root` | Input file with TGraph objects | `sample_data/data.root` |
| `output.root` | Output file for trimmed events | `processed_output.root` |
| `libpath` | (Optional) Path to libhalolib.so | `/usr/local/lib/libhalolib.so` |

### Example Commands

**With automatic library discovery:**
```bash
python3 processor.py sample_data/20241115-161219_12kA.root processed.root
```

**With explicit library path:**
```bash
python3 processor.py data.root output.root /home/sophia/rest-install/lib/libhalolib.so
```

### Expected Output

```
Converting: SpectrumSumFile_0
Converting: SpectrumSumFile_10
Converting: SpectrumSumFile_20
...
Converting: SpectrumSumFile_1990
Finished. Written 2000 TRestHaloEvent objects to processed_output.root

Combining first two events...
Combined spectrum written to test_combination.root
```

---

## Understanding Each Step

### Step 1: Why Convert TGraph to TRestHaloEvent?

**TGraph advantages:**
- ROOT's native 2D data structure
- Simple point-by-point storage
- Suitable for visualization

**TRestHaloEvent advantages:**
- Structured spectrum container with physical meaning
- Integrated metadata tracking
- Built-in uncertainty handling per bin
- Type-safe for physics operations
- Serializable with full context preservation
- Integration with REST framework analysis tools

**Conversion bridges data acquisition → physics analysis**

### Step 2: Why Unit Conversion?

**V_RMS (Volts root-mean-square):**
- Measured directly from RF receiver
- Depends on receiver impedance (typically 50Ω)
- System-specific (different receivers → different V_RMS for same signal)

**Watts (Power):**
- Absolute physical quantity
- Independent of measurement system
- Standard unit for physics analysis: $P = \frac{V_{RMS}^2}{Z}$

**Example:** Two identical signals measured with different impedances would have different V_RMS but same Watts. Always use Watts for consistent physics.

### Step 3: Why Trim Spectra?

**Problem:** Raw spectra have varying bin counts
```
SpectrumSumFile_0: 8234 bins
SpectrumSumFile_1: 8241 bins
SpectrumSumFile_2: 8229 bins
...
```

**Solution:** Resample to uniform 14000 bins

**Benefits:**
- Machine learning requires uniform input dimensions
- Batch processing efficiency
- Database storage uniformity
- Preserves spectral information through interpolation

**Trade-offs:**
- 14000 bins provides ~300 kHz resolution (typical haloscope needs)
- Fewer bins → faster processing but less detail
- More bins → finer detail but slower processing

### Step 4: Why Store Metadata?

**Metadata in file enables:**
- Experimental reproducibility: who ran, when, where
- Data lineage: tracing back to original conditions
- Quality assessment: resolution, frequency range, calibration
- Automated analysis: scripts can read metadata without manual configuration

**Example metadata retrieval:**
```python
from ROOT import TFile
f = TFile.Open('processed_output.root')
event = f.Get('SpectrumSumFile_0')
meta = event.GetMetadata()
print(f"Experiment: {meta.GetExperimentName()}")
print(f"Frequency range: {meta.GetCenterFrequency()} ± {meta.GetResolutionBandwidth()*100/2:.0f} Hz")
print(f"Bins: {meta.GetNumFreqPoints()}")
```

### Step 5: Why Combine Spectra?

**Averaging measurements improves signal quality:**

**Single measurement:**
- One noise realization
- Large statistical uncertainty
- May miss weak features

**Combined measurements (chi2-weighted):**
- Average over independent noise
- Reduced uncertainty: $\sigma_{combined} = \sqrt{\frac{1}{1/\sigma_1^2 + 1/\sigma_2^2}}$
- Better sensitivity to weak signals
- Proper weighting accounts for individual measurement quality

**Example sensitivity improvement:**
```
Two measurements with σ = 0.1:
Combined uncertainty = √(1/(100+100)) = 0.0707  (-29% uncertainty)

Two measurements with σ₁ = 0.05, σ₂ = 0.1:
Combined uncertainty = √(1/(400+100)) = 0.0447  (-automatic weighting favors better measurement)
```

---

## Customization Guide

### Modifying Trim Bin Count

```python
# Line ~103
proc.SetTrimBins(14000)  # Change this value
```

**Recommended values:**
- 8000 bins: Fast, coarse resolution (~500 kHz for 4 GHz spectrum)
- 14000 bins: Default, good balance (~300 kHz for 4 GHz spectrum)
- 20000 bins: Fine detail, slower (~200 kHz for 4 GHz spectrum)

### Processing All Events (Not Just First Two)

```python
# Original: stores only first two
if processed < 2:
    first_two_events.append(write_ev)
```

**To process all events:**
```python
# Modified: store all events
all_events.append(write_ev)

# After main loop
if len(all_events) >= 2:
    combiner = TRestHaloCombine()
    for event in all_events:
        combiner.AddEvent(event)
    combined_event = combiner.Combine()
    # ... write to file
```

### Custom Metadata Fields

Add your own metadata after line ~88:

```python
meta.SetExperimentName('ConvertedFromTGraph')
meta.SetNotes('Converted with processor.py')

# Add custom information
meta.SetNotes(f'Converted with processor.py | Source: {name}')
# Or use available setters for standardized fields
```

**Available metadata setters:**
```python
meta.SetValueUnit(int)              # 0 = V_RMS, 1 = Watts
meta.SetNumFreqPoints(int)
meta.SetResolutionBandwidth(float)
meta.SetCenterFrequency(float)
meta.SetExperimentName(string)
meta.SetNotes(string)
meta.SetLastCal(string)             # Calibration filename
meta.SetStdDev(float)               # Standard deviation
meta.SetUncertaintiesProvided(bool) # Uncertainties computed vs provided
```

### Conditional Processing

**Process only spectra above minimum quality threshold:**

```python
# After creating TRestHaloEvent
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, vals_w, 1)

# Add quality check
max_value = max(vals_w)
if max_value < 1e-15:  # Threshold: 1 femtowatt
    print(f'Skipping {name}: signal too weak')
    continue

# Proceed with trimming and output
proc = TRestHaloTrimProcess()
...
```

### Selective Trimming

**Only trim if bin count differs significantly:**

```python
target_bins = 14000
if abs(len(freqs) - target_bins) > 100:  # Only trim if difference > 100 bins
    proc = TRestHaloTrimProcess()
    proc.SetTrimBins(target_bins)
    out_ev = proc.ProcessEvent(ev)
else:
    out_ev = ev  # Use original if already close to target
```

### Multiple Output Formats

**Save both Watts and V_RMS versions:**

```python
# Process Watts (current)
ev_watts = TRestHaloEvent()
ev_watts.SetSpectrum(freqs, vals_w, 1)  # 1 = Watts

# Also create V_RMS version
ev_vrms = TRestHaloEvent()
ev_vrms.SetSpectrum(freqs, vals_v, 0)   # 0 = V_RMS

# Trim both
proc = TRestHaloTrimProcess()
proc.SetTrimBins(14000)
out_ev_watts = proc.ProcessEvent(ev_watts)
out_ev_vrms = proc.ProcessEvent(ev_vrms)

# Output both with different names
fout.cd()
fout.WriteObject(out_ev_watts, name + '_Watts')
fout.WriteObject(out_ev_vrms, name + '_VRms')
```

---

## Advanced Topics

### Understanding Chi2 Weighting

**Scenario:** Two measurements of same frequency bin
- Measurement A: value = 100 pW, uncertainty = 5 pW
- Measurement B: value = 110 pW, uncertainty = 20 pW

**Chi2 combination:**
```
Weight_A = 1/(5²) = 0.04
Weight_B = 1/(20²) = 0.0025

Combined = (100×0.04 + 110×0.0025) / (0.04 + 0.0025)
         = (4 + 0.275) / 0.0425
         = 100.6 pW

New uncertainty = √(1/(0.04 + 0.0025)) = 4.85 pW
```

**Key insight:** Better measurement (smaller uncertainty) dominates result. This is optimal for independent Gaussian measurements.

### Frequency Compatibility

**Criterion for combining events:** All events must have compatible bin sizes

```
Bin size A = (fmax - fmin) / nbins_A
Bin size B = (fmax - fmin) / nbins_B

Compatible if: |bin_size_A - bin_size_B| / bin_size_A < 1e-6
```

**Example:**
```
Event 1: 1-5 GHz, 8000 bins → 500 kHz/bin
Event 2: 1-5 GHz, 8001 bins → 499.9 kHz/bin
Compatible? 0.0001/500 = 2e-7 < 1e-6 ✓

Event 1: 1-5 GHz, 8000 bins → 500 kHz/bin
Event 2: 2-5 GHz, 8000 bins → 375 kHz/bin
Compatible? Different frequency ranges → align on union, check within overlap ✓
```

### Uncertainty Propagation

**Types of uncertainty in combined spectrum:**

1. **Measurement uncertainty** (per-bin): From individual spectrum quality
   - Computed automatically if not provided
   - Tracked through combination process

2. **Combination uncertainty**: Reduced when combining
   - Formula: $\sigma_{combined} = \sqrt{\frac{1}{\sum_i 1/\sigma_i^2}}$
   - Always ≤ minimum input uncertainty

3. **Resampling uncertainty**: From interpolation during trimming
   - Generally negligible compared to measurement uncertainty
   - Affects smooth regions more than sharp features

**Best practice:** Propagate uncertainties through entire pipeline

### Batch Processing Large Datasets

**For datasets with thousands of spectra:**

```python
import sys
from datetime import datetime

processed = 0
error_count = 0
skipped_count = 0

print(f'[{datetime.now()}] Starting batch processing')

for key in fin.GetListOfKeys():
    try:
        obj = key.ReadObj()
        if not obj: continue
        
        # Process event
        ...
        
        processed += 1
        if processed % 100 == 0:
            print(f'[{datetime.now()}] Processed {processed} events ({error_count} errors)')
            
    except Exception as e:
        error_count += 1
        print(f'Error processing {key.GetName()}: {e}')
        continue

print(f'Completed: {processed} processed, {error_count} errors, {skipped_count} skipped')
```

### Memory Management

**For very large event counts, cache selectively:**

```python
# Original: stores all events in memory
first_two_events = []

# For large datasets: store only essential info
trimmed_events_info = []

if processed < 2:
    # Store only filename reference, reload later
    trimmed_events_info.append({
        'filename': outfile,
        'object_name': name,
        'index': processed
    })

# Later: reload from disk instead of memory
if len(trimmed_events_info) >= 2:
    events = []
    for info in trimmed_events_info:
        f = TFile.Open(info['filename'])
        ev = f.Get(info['object_name'])
        events.append(ev)
    # Combine events...
```

### Parallel Processing

**For multi-file processing:**

```python
#!/usr/bin/env python3
import sys
import os
from multiprocessing import Pool

def process_single_file(args):
    infile, outfile, libpath = args
    # Import inside function for subprocess isolation
    from ROOT import gSystem
    gSystem.Load(libpath if libpath else '')
    
    from ROOT import TFile, TRestHaloEvent, TRestHaloTrimProcess
    # ... run single file processing ...
    return outfile

if __name__ == '__main__':
    files = [
        ('data1.root', 'out1.root', libpath),
        ('data2.root', 'out2.root', libpath),
        ('data3.root', 'out3.root', libpath),
    ]
    
    with Pool(4) as p:  # 4 parallel processes
        results = p.map(process_single_file, files)
    
    print(f'Processed {len(results)} files')
```

---

## Troubleshooting

### Issue: "Failed to import REST classes from ROOT"

**Cause:** libhalolib.so not found or not loaded

**Solutions:**
```bash
# Check library exists
ls -l /path/to/rest-install/lib/libhalolib.so

# Try explicit path
python3 processor.py input.root output.root /home/sophia/rest-install/lib/libhalolib.so

# Add to LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/sophia/rest-install/lib:$LD_LIBRARY_PATH
python3 processor.py input.root output.root
```

### Issue: "no dictionary for class TRestHaloEvent"

**Cause:** ROOT dictionary (.pcm files) not found

**Solutions:**
```bash
# Ensure PCM files installed
ls /home/sophia/rest-install/lib/CINT_TRestHalo*.pcm

# Add to ROOT search path
export LD_LIBRARY_PATH=/home/sophia/rest-install/lib:$LD_LIBRARY_PATH

# Rebuild library
cd /path/to/rest-halolib/framework/build
cmake ..
make install
```

### Issue: "Trimming failed for [spectrum]"

**Cause:** Invalid spectrum data (NaN, inf, empty frequency array)

**Solutions:**
```python
# Add validation before trimming
if not freqs or not vals_w:
    print(f'Skipping {name}: empty spectrum')
    continue

if any(v != v for v in vals_w):  # Check for NaN
    print(f'Skipping {name}: contains NaN')
    continue

if any(float('inf') == abs(v) for v in vals_w):  # Check for inf
    print(f'Skipping {name}: contains infinity')
    continue
```

### Issue: Output file becomes corrupted during processing

**Cause:** File not properly closed on error

**Solutions:**
```python
# Use try-finally to ensure cleanup
try:
    fout = TFile.Open(outfile, 'RECREATE')
    # ... processing ...
finally:
    if fout:
        fout.Close()
    if fin:
        fin.Close()
```

### Issue: Combination produces empty result

**Causes:**
- Less than 2 events processed
- Events have incompatible frequency bins
- No frequency overlap between events

**Debug:**
```python
if len(first_two_events) < 2:
    print(f'Error: Need 2+ events, have {len(first_two_events)}')

# Check frequency compatibility
e1_freqs = first_two_events[0].GetFrequencies()
e2_freqs = first_two_events[1].GetFrequencies()
print(f'Event 1: {e1_freqs[0]:.3f} - {e1_freqs[-1]:.3f} Hz')
print(f'Event 2: {e2_freqs[0]:.3f} - {e2_freqs[-1]:.3f} Hz')
print(f'Overlap: {max(e1_freqs[0], e2_freqs[0]):.3f} - {min(e1_freqs[-1], e2_freqs[-1]):.3f} Hz')
```

### Issue: Performance degradation with large files

**Cause:** Holding too many events in memory

**Solutions:**
```python
# Process in batches
batch_size = 100
for i, key in enumerate(fin.GetListOfKeys()):
    if i > 0 and i % batch_size == 0:
        # Periodically flush to disk or garbage collect
        import gc
        gc.collect()
```



