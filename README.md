The Halolib library was created to centralize data structures and early analysis
procedures for the haloscope RADES.
It assumes RF type readouts of power spectra and includes processes to prepare
the data for advanced axion signal search analysis outside the framework.

## Building with REST

With this repository checked out at `source/libraries/halo` in the REST
framework, enable it with `-DRESTLIB_HALO=ON` (or `-DREST_ALL_LIBS=ON`).
The normal framework build and installation produce `libRestHalolib`, install
the `TRestHalo*.h` headers and ROOT dictionaries, and register the library with
`rest-config`. After sourcing the installed `thisREST.sh`, `restRoot` loads it
through the normal REST library discovery.

Generated halolib ROOT dictionaries are also copied beside the build-directory
library, so it can be loaded without running the installation step.

Use `-DTEST=ON` to build the three bundled tests. Their assertions remain enabled
in Release builds. Run them with `ctest --test-dir <build> -R '^TRestHalo'`.

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

