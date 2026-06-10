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

