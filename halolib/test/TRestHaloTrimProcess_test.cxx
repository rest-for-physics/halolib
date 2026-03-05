#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "TRestHaloEvent.h"
#include "TRestHaloTrimProcess.h"

int main() {
    // prepare a simple spectrum: 6 points
    TRestHaloEvent ev;
    std::vector<double> freq = {10, 20, 30, 40, 50, 60};
    std::vector<double> volts = {1, 2, 3, 4, 5, 6};

    ev.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS);

    // Case 1: trim 2 bins from each side -> remaining 2 bins (indices 2,3)
    TRestHaloTrimProcess proc;
    proc.SetTrimBins(2);
    TRestEvent* outBase = proc.ProcessEvent(&ev);
    TRestHaloEvent* out = dynamic_cast<TRestHaloEvent*>(outBase);
    if (!out) {
        std::cerr << "Expected TRestHaloEvent output\n";
        return 2;
    }

    const auto& ofreq = out->GetFrequencies();
    const auto& oval = out->GetValues();
    if (ofreq.size() != 2 || oval.size() != 2) {
        std::cerr << "Trimmed size mismatch: got " << ofreq.size() << "\n";
        return 3;
    }

    // expected values are volts^2 * 8 for indices 2 and 3 (3 and 4 volts)
    double tol = 1e-12;
    double exp0 = TRestHaloEvent::VoltsToWatts(3.0);
    double exp1 = TRestHaloEvent::VoltsToWatts(4.0);
    if (std::abs(oval[0] - exp0) > tol || std::abs(oval[1] - exp1) > tol) {
        std::cerr << "Converted values mismatch: got " << oval[0] << ", " << oval[1]
                  << " expected " << exp0 << ", " << exp1 << "\n";
        return 4;
    }

    if (out->GetStoredUnit() != TRestHaloMetadata::W) {
        std::cerr << "Output stored unit is not W\n";
        return 5;
    }

    // Case 2: trim too large -> n <= 2*trim -> should return converted but not trimmed
    proc.SetTrimBins(3); // now 2*trim == 6 -> equal to n
    TRestEvent* outBase2 = proc.ProcessEvent(&ev);
    TRestHaloEvent* out2 = dynamic_cast<TRestHaloEvent*>(outBase2);
    if (!out2) {
        std::cerr << "Expected TRestHaloEvent output (case 2)\n";
        return 6;
    }
    if (out2->GetFrequencies().size() != freq.size()) {
        std::cerr << "Case2: expected no trimming, but size changed\n";
        return 7;
    }
    // values should be converted to watts
    for (size_t i = 0; i < volts.size(); ++i) {
        double got = out2->GetValues()[i];
        double expect = TRestHaloEvent::VoltsToWatts(volts[i]);
        if (std::abs(got - expect) > tol) {
            std::cerr << "Case2 conversion mismatch at "<<i<<": got "<<got<<" expect "<<expect<<"\n";
            return 8;
        }
    }

    std::cout << "TRestHaloTrimProcess tests passed." << std::endl;
    return 0;
}
