/// Implementation for TRestHaloTrimProcess

#include "TRestHaloTrimProcess.h"
#include "TRestHaloEvent.h"
#include "TRestHaloMetadata.h"

ClassImp(TRestHaloTrimProcess);

TRestHaloTrimProcess::TRestHaloTrimProcess() : fEvent(nullptr), fTrimBins(8000) {}
TRestHaloTrimProcess::~TRestHaloTrimProcess() {}

void TRestHaloTrimProcess::Initialize() {
    // Nothing needed for now.
}

void TRestHaloTrimProcess::InitProcess() {
    // Called once before processing begins; nothing to init here yet.
}

TRestEvent* TRestHaloTrimProcess::ProcessEvent(TRestEvent* evInput) {
    if (!evInput) return nullptr;

    // Try to interpret input as TRestHaloEvent
    TRestHaloEvent* in = dynamic_cast<TRestHaloEvent*>(evInput);
    if (!in) {
        // Not a halo event, pass through unchanged
        return evInput;
    }

    const auto& freqs = in->GetFrequencies();
    const auto& vals = in->GetValues();
    size_t n = vals.size();
    if (n == 0 || freqs.size() != n) {
        // Malformed event: nothing to do
        return evInput;
    }

    // Convert values to power (W) if needed
    std::vector<double> converted;
    converted.reserve(n);
    auto stored = in->GetStoredUnit();
    for (size_t i = 0; i < n; ++i) {
        double v = vals[i];
        double p = v;
        if (stored == TRestHaloMetadata::V_RMS) {
            // Convert volts to watts (VoltsToWatts returns watts)
            p = TRestHaloEvent::VoltsToWatts(v);
        }
        // If already in W (watts) we keep as-is. Note: TRestHaloMetadata uses W
        converted.push_back(p);
    }

    // Determine trimming bounds
    size_t trim = static_cast<size_t>(std::max(0, fTrimBins));
    if (n <= 2 * trim) {
        // Too short after trimming: return converted (no trimming)
        // Create a new event with converted values and W unit
        TRestHaloEvent* out = new TRestHaloEvent();
        out->SetSpectrum(freqs, converted, TRestHaloMetadata::W);
        return out;
    }

    size_t first = trim;
    size_t last_excl = n - trim;
    std::vector<double> tfreqs;
    std::vector<double> tvals;
    tfreqs.reserve(last_excl - first);
    tvals.reserve(last_excl - first);
    for (size_t i = first; i < last_excl; ++i) {
        tfreqs.push_back(freqs[i]);
        tvals.push_back(converted[i]);
    }

    // Create new output event (power units)
    TRestHaloEvent* out = new TRestHaloEvent();
    out->SetSpectrum(tfreqs, tvals, TRestHaloMetadata::W);

    return out;
}

void TRestHaloTrimProcess::EndProcess() {
    // Nothing to finalize for now.
}

// Note: SetTrimBins/GetTrimBins are implemented inline in the header.
