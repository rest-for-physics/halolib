#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "TRestHaloEvent.h"

int main() {
    TRestHaloEvent e;

    std::vector<double> freq = {10.0, 20.0, 30.0, 40.0};
    std::vector<double> volts = {0.5, 1.0, 0.25, 2.0};

    e.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS);

    const auto &p = e.GetValues();
    const auto &v = e.GetValues();

    const double tol = 1e-12;
    // conversion check via GetValueAtFrequency in W
    for (size_t i = 0; i < v.size(); ++i) {
        double expected = v[i] * v[i] * 8.0;
        double got = e.GetValueAtFrequency(freq[i], TRestHaloMetadata::W);
        if (std::abs(got - expected) > tol) {
            std::cerr << "Conversion mismatch at " << i << ": got " << got
                      << " expected " << expected << "\n";
            return 2;
        }
    }

    TRestHaloEvent empty;
    assert(empty.GetValueAtFrequency(10.0) == 0.0);

    std::cout << "All checks passed." << std::endl;
    return 0;
}
