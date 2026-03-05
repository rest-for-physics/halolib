#ifndef RestCore_TRestHaloEvent
#define RestCore_TRestHaloEvent

#include "TRestEvent.h"
#include "TRestHaloMetadata.h"
#include <vector>

// Unified event for haloscope power (or volts) spectra. 
// Stores a single data vector whose units
// are described in the associated TRestHaloMetadata (V_RMS or W).
class TRestHaloEvent : public TRestEvent {
private:
    std::vector<double> fFrequency; // Hz
    std::vector<double> fValues;    // stored values (units in metadata)

    // metadata containing units and acquisition info
    TRestHaloMetadata fMetadata;

    double fStartFrequency = 0; // Hz
    double fStopFrequency = 0;  // Hz
    double fResolution = 0;     // Hz per bin

public:
    TRestHaloEvent();
    ~TRestHaloEvent();

    // Accept input values and their units
    void SetSpectrum(const std::vector<double>& freq,
                     const std::vector<double>& values,
                     TRestHaloMetadata::EValueUnit unit);

    const std::vector<double>& GetFrequencies() const { return fFrequency; }
    const std::vector<double>& GetValues() const { return fValues; }

    const TRestHaloMetadata& GetMetadata() const { return fMetadata; }
    TRestHaloMetadata& GetMetadata() { return fMetadata; }

    // Query stored unit
    TRestHaloMetadata::EValueUnit GetStoredUnit() const {
        return static_cast<TRestHaloMetadata::EValueUnit>(fMetadata.GetValueUnit());
    }

    // Return value at nearest frequency bin converted to outUnit
    double GetValueAtFrequency(double f, TRestHaloMetadata::EValueUnit outUnit) const;

    // Convenience overload: returns value in stored unit
    double GetValueAtFrequency(double f) const { return GetValueAtFrequency(f, GetStoredUnit()); }

    // Statistics (converted to outUnit)

    void PrintEvent() const override;
    void Initialize() override;

    // Draw the event as a graph (frequency vs value). Returns the pad used.
    virtual TPad* DrawEvent(const TString& option = "") override;
    // Provide a custom browser entry so TBrowser can show a drawable graph
    virtual void Browse(TBrowser* b) override;

    // conversion helpers
    static double VoltsToWatts(double v) { return v * v * 8.0; }
    static double WattsToVolts(double w) { return std::sqrt(std::max(0.0, w / 8.0)); }

    ClassDefOverride(TRestHaloEvent, 1);
};

#endif
