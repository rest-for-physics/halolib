#include "TRestHaloEvent.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <TGraph.h>
#include <TCanvas.h>
#include <TString.h>
#include <TBrowser.h>

ClassImp(TRestHaloEvent);

TRestHaloEvent::TRestHaloEvent() {}
TRestHaloEvent::~TRestHaloEvent() {}

// Initialize is intentionally a no-op; events are simple containers.
// Required to satisfy TRestEvent pure virtual interface.
void TRestHaloEvent::Initialize() {}

void TRestHaloEvent::SetSpectrum(const std::vector<double>& freq,
                                 const std::vector<double>& values,
                                 TRestHaloMetadata::EValueUnit unit) {
    fFrequency = freq;
    fValues = values;
    fMetadata.SetValueUnit(static_cast<int>(unit));

    if (!fFrequency.empty()) {
        fStartFrequency = fFrequency.front();
        fStopFrequency = fFrequency.back();
        if (fFrequency.size() > 1)
            fResolution = fFrequency[1] - fFrequency[0];
        else
            fResolution = 0;
    } else {
        fStartFrequency = fStopFrequency = fResolution = 0;
    }
}

double TRestHaloEvent::GetValueAtFrequency(double f, TRestHaloMetadata::EValueUnit outUnit) const {
    if (fFrequency.empty() || fValues.empty()) return 0.0;
    auto it = std::lower_bound(fFrequency.begin(), fFrequency.end(), f);
    size_t idx = 0;
    if (it == fFrequency.end()) idx = fFrequency.size() - 1;
    else idx = std::distance(fFrequency.begin(), it);

    double raw = fValues[idx];
    auto inUnit = GetStoredUnit();
    // If units match, return raw (or converted to requested unit)
    if (inUnit == outUnit) return raw;

    if (inUnit == TRestHaloMetadata::V_RMS && outUnit == TRestHaloMetadata::W) {
        return VoltsToWatts(raw);
    }
    if (inUnit == TRestHaloMetadata::W && outUnit == TRestHaloMetadata::V_RMS) {
        return WattsToVolts(raw);
    }
    // fallback: no conversion known
    return raw;
}


void TRestHaloEvent::PrintEvent() const {
    printf("TRestHaloEvent: bins=%zu start=%.3fHz stop=%.3fHz res=%.3fHz\n",
           fFrequency.size(), fStartFrequency, fStopFrequency, fResolution);
    printf("  Stored unit: %s\n", fMetadata.GetValueUnitString().Data());
}


TPad* TRestHaloEvent::DrawEvent(const TString& option) {
    UNUSED(option);
    if (fFrequency.empty() || fValues.empty()) return nullptr;

    // Filter out non-finite points (NaN/Inf) to avoid plotting issues
    std::vector<double> xf; xf.reserve(fFrequency.size());
    std::vector<double> yf; yf.reserve(fValues.size());
    for (size_t i = 0; i < fFrequency.size() && i < fValues.size(); ++i) {
        double x = fFrequency[i];
        double y = fValues[i];
        if (std::isfinite(x) && std::isfinite(y)) {
            xf.push_back(x);
            yf.push_back(y);
        }
    }
    if (xf.empty() || yf.empty()) return nullptr;

    const int n = static_cast<int>(yf.size());
    TGraph* g = new TGraph(n, xf.data(), yf.data());

    TString title = GetName();
    if (title.IsNull()) title = "TRestHaloEvent";
    g->SetTitle(Form("%s;Frequency (Hz);Value", title.Data()));
    // use an explicit name for the graph so it can be found in the pad
    TString gname = TString("Spectrum_") + title;
    g->SetName(gname);
    g->SetLineWidth(2);
    g->SetMarkerStyle(20);

    // Create an explicit canvas (avoid implicit default-canvas edge cases)
    TString cname = TString("c_") + title;
    TCanvas* c = new TCanvas(cname, title, 800, 600);
    c->cd();
    g->Draw("AL");
    c->Modified();
    c->Update();
    return c;
}


void TRestHaloEvent::Browse(TBrowser* b) {
    if (!b) return;
    if (fFrequency.empty() || fValues.empty()) return;

    const int n = static_cast<int>(fValues.size());
    TGraph* g = nullptr;
    try {
        g = new TGraph(n, fFrequency.data(), fValues.data());
    } catch (...) {
        return;
    }
    TString gname = TString("Spectrum_") + GetName();
    if (gname.IsNull()) gname = "Spectrum";
    g->SetName(gname);
    g->SetTitle(Form("%s;Frequency (Hz);Value", gname.Data()));

    b->Add(g, gname.Data());
}
