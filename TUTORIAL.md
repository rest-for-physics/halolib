# Low-Level Data Processing with TRestHalolib: Complete Tutorial

This tutorial demonstrates a complete data processing workflow for haloscope (frequency spectrum) data using the REST framework's halolib library. The `processor.py` script serves as a reference implementation for converting raw spectrum data into analyzable TRestHaloEvent objects with advanced processing capabilities.

---

## Prerequisites

### Software Requirements

- **ROOT Framework** 6.0+ 
- **REST Framework** 2.4.3+ with halolib library
- Process script: **Python 3.6+** with PyROOT bindings 

### Installation Verification

```bash
# Check ROOT installation
root --version

# Verify libhalolib is installed
ls -l /path/to/rest-install/lib/libhalolib.so

# Test Python/ROOT integration
python3 -c "from ROOT import TFile; print('ROOT available')"
```

### Practice Data

Input data in the example processing script is a ROOT file format containing:
- **TGraph objects** with names starting with `SpectrumSumFile_`
- **X-axis values**: Frequency points (Hz)
- **Y-axis values**: Amplitude values (V_RMS)
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
├── fUncertainties[]       - Per-bin uncertainties
├── fMetadata              - Experiment information, manually declared 
├── fStartFrequency        - First frequency point
├── fStopFrequency         - Last frequency point
└── fResolution            - Bin size (Hz per bin)
```

**Key Methods:**
- `SetSpectrum(freqs, values, unit, uncertainties=optional)` - Initialize data arrays with optional uncertainties
- `SetUncertainties(uncertainties)` - Add or update uncertainties for existing spectrum
- `GetFrequencies()` - Retrieve frequency array
- `GetValues()` - Retrieve (power) spectrum values
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
└── fUncertaintiesProvided - Flag: uncertainties provided or not 
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

**Crops either side of central requency (center bin) by 'n'

#### 4. **TRestHaloCombine**
Combines multiple spectra with chi2 weighting.

```cpp
TRestHaloCombine
├── AddEvent(event)        - Queue event for combination
├── Combine()              - Perform sequential pairwise combination
├── GetCombinedEvent()     - Retrieve result
└── Clear()                - Reset for new combination
```

- Combines events sequentially: E₀ + E₁ → R₁, then R₁ + E₂ → R₂, etc.
- For overlapping frequency bins: Chi2-weighted average
- For non-overlapping bins: Values carried over unchanged

---
## Example Processing Script

### What This Script Does

The processor script implements a pre-processing pipeline:

```
Input TGraphs (V_RMS Data)
    ↓
[Convert] → TRestHaloEvent objects (Power in Watts)
    ↓
[Enhance] → Tag metadata (frequency info, experiment details)
    ↓
[Process] → Trim spectra tails
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
- **Metadata Tracking**: Experiment conditions, frequency parameters
- **Spectrum Trimming**: Chop data to the physics region you are interested in
- **Optional Uncertainties**: Add per-bin uncertainties at creation or later with `SetUncertainties()`
- **Spectral Combination**: Chi2-weighted averaging with per-bin uncertainties
- **Batch Processing**: Efficient handling of thousands of spectra

---

## processor.py Workflow

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
$$P = V_{\text{p-p}}^2 = \left(2\sqrt{2}V_\text{RMS}\right)^2$$

Note: Hardware gives data in Volts-RMS, convert to Watts early and use for the rest of the analysis

#### Step 3c: Event Creation
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, vals_w, 1)  # 1 = Watts unit, no uncertainties provided
```

Creates TRestHaloEvent with:
- Input frequency array
- Input power values (Watts)
- Unit specified as Watts
- No uncertainties (can be added later with `SetUncertainties()` if needed)

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

**Metadata Persistence**: These values are tagged to each TRestHaloEvent.

#### Step 3e: Spectrum Trimming
```python
proc = TRestHaloTrimProcess()
proc.SetTrimBins(14000)              # Target: 14000 bins
out_ev = proc.ProcessEvent(ev)       # Returns resampled event
```

**Why trim?**
- Preserve disk space by only saving the data of the physics region of interest
- Makes fitting the central peak easier if there are no tails


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
python3 processor.py data.root output.root /home/user/rest-install/lib/libhalolib.so
```

### Expected Output

```
Converting: SpectrumSumFile_0
Converting: SpectrumSumFile_10
Converting: SpectrumSumFile_20
...
Converting: SpectrumSumFile_1990
Finished. Written 2000 TRestHaloEvent objects to processed_output.root

```

---

### Adding Uncertainties to Data

**Pattern 1: If you have uncertainties to add:**
```python
ev = TRestHaloEvent()
uncertainties = compute_uncertainties(spectrum)  # Your uncertainty calculation
ev.SetSpectrum(freqs, vals_w, 1, uncertainties)  # Add at creation
```

**Pattern 2: If uncertainties become available later:**
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, vals_w, 1)  # Create first
# ... process event, trimming, etc ...
ev.SetUncertainties(uncertainties)  # Add when ready
```

**Pattern 3: For combination, uncertainties are essential:**
```python
ev1 = TRestHaloEvent()
ev1.SetSpectrum(freqs1, vals1, 1, unc1)  # Must have uncertainties
ev2 = TRestHaloEvent()
ev2.SetSpectrum(freqs2, vals2, 1, unc2)  # Must have uncertainties

combiner = TRestHaloCombine()
combiner.AddEvent(ev1)
combiner.AddEvent(ev2)
result = combiner.Combine()  # Produces chi2-weighted average
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
python3 processor.py input.root output.root /home/user/rest-install/lib/libhalolib.so

# Add to LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/user/rest-install/lib:$LD_LIBRARY_PATH
python3 processor.py input.root output.root
```

### Issue: "no dictionary for class TRestHaloEvent"

**Cause:** ROOT dictionary (.pcm files) not found

**Solutions:**
```bash
# Ensure PCM files installed
ls /home/user/rest-install/lib/CINT_TRestHalo*.pcm

# Add to ROOT search path
export LD_LIBRARY_PATH=/home/user/rest-install/lib:$LD_LIBRARY_PATH

# Rebuild library
cd /path/to/rest-halolib/framework/build
cmake ..
make install
```



