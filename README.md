The Halolib library was created to centralize data structures and early analysis
procedures for the haloscope RADES.
It assumes RF type readouts of power spectra and includes processes to prepare
the data for advanced axion signal search analysis outside the framework.

## Example Processing Script

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
- **Optional Uncertainties**: Add per-bin uncertainties at creation or later with `SetUncertainties()`
- **Spectral Combination**: Chi2-weighted averaging with per-bin uncertainties
- **Batch Processing**: Efficient handling of thousands of spectra

---

## Using the Unified SetSpectrum API

With the unified `SetSpectrum()` method, you have three usage patterns:

### Pattern 1: Basic Spectrum (No Uncertainties)
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, values, unit)
```
Use this when you only have spectrum values and no uncertainty information.

### Pattern 2: Spectrum with Uncertainties at Creation
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, values, unit, uncertainties)
```
Use this when you have per-bin uncertainty estimates available at creation time.

### Pattern 3: Add Uncertainties Later
```python
ev = TRestHaloEvent()
ev.SetSpectrum(freqs, values, unit)  # Create without uncertainties
# ... later ...
ev.SetUncertainties(uncertainties)  # Add them when available
```
Use this when uncertainties are computed or become available after initial event creation.

All three patterns are fully supported and work seamlessly with `TRestHaloCombine` for spectral combination.



