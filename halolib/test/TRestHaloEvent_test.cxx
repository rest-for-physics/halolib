#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "TRestHaloEvent.h"

int main() {
    std::cout << "=== TRestHaloEvent Basic Tests ===" << std::endl;
    
    TRestHaloEvent e;
    std::vector<double> freq = {10.0, 20.0, 30.0, 40.0};
    std::vector<double> volts = {0.5, 1.0, 0.25, 2.0};

    e.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS);

    const auto &p = e.GetValues();
    const auto &v = e.GetValues();
    const auto &unc = e.GetUncertainties();

    const double tol = 1e-12;
    
    // Check that uncertainties are initialized to computed standard deviation
    std::cout << "Testing computed uncertainties..." << std::endl;
    assert(!e.GetMetadata().GetUncertaintiesProvided());
    assert(unc.size() == v.size());
    for (double u : unc) {
        assert(u > 0);  // Should have computed positive std dev
    }
    
    // conversion check via GetValueAtFrequency in W
    std::cout << "Testing unit conversion..." << std::endl;
    for (size_t i = 0; i < v.size(); ++i) {
        double expected = v[i] * v[i] * 8.0;
        double got = e.GetValueAtFrequency(freq[i], TRestHaloMetadata::W);
        if (std::abs(got - expected) > tol) {
            std::cerr << "Conversion mismatch at " << i << ": got " << got
                      << " expected " << expected << "\n";
            return 2;
        }
    }

    // Test with provided uncertainties
    std::cout << "Testing provided uncertainties..." << std::endl;
    TRestHaloEvent e2;
    std::vector<double> unc_provided = {0.01, 0.02, 0.015, 0.03};
    e2.SetSpectrum(freq, volts, unc_provided, TRestHaloMetadata::V_RMS);
    
    assert(e2.GetMetadata().GetUncertaintiesProvided());
    const auto &unc2 = e2.GetUncertainties();
    assert(unc2.size() == unc_provided.size());
    for (size_t i = 0; i < unc2.size(); ++i) {
        assert(std::abs(unc2[i] - unc_provided[i]) < 1e-12);
    }

    TRestHaloEvent empty;
    assert(empty.GetValueAtFrequency(10.0) == 0.0);

    std::cout << "All checks passed!" << std::endl;
    return 0;
}
