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

    // Test 1: Basic spectrum without uncertainties
    std::cout << "Testing basic spectrum (no uncertainties computed)..." << std::endl;
    e.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS);

    const auto &v = e.GetValues();
    const auto &unc = e.GetUncertainties();
    
    assert(v.size() == freq.size());
    assert(unc.empty());  // No uncertainties computed

    // Test 2: Unit conversion
    const double tol = 1e-12;
    std::cout << "Testing unit conversion (V_RMS to Watts)..." << std::endl;
    for (size_t i = 0; i < v.size(); ++i) {
        double expected = v[i] * v[i] * 8.0;
        double got = e.GetValueAtFrequency(freq[i], TRestHaloMetadata::W);
        if (std::abs(got - expected) > tol) {
            std::cerr << "Conversion mismatch at " << i << ": got " << got
                      << " expected " << expected << "\n";
            return 2;
        }
    }

    // Test 3: Spectrum with provided uncertainties
    std::cout << "Testing spectrum with uncertainties at creation..." << std::endl;
    TRestHaloEvent e2;
    std::vector<double> unc_provided = {0.01, 0.02, 0.015, 0.03};
    e2.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS, unc_provided);
    
    const auto &unc2 = e2.GetUncertainties();
    assert(unc2.size() == unc_provided.size());
    for (size_t i = 0; i < unc2.size(); ++i) {
        assert(std::abs(unc2[i] - unc_provided[i]) < 1e-12);
    }

    // Test 4: Adding uncertainties later with SetUncertainties
    std::cout << "Testing SetUncertainties method..." << std::endl;
    TRestHaloEvent e3;
    e3.SetSpectrum(freq, volts, TRestHaloMetadata::V_RMS);  // No uncertainties initially
    assert(e3.GetUncertainties().empty());
    
    e3.SetUncertainties(unc_provided);  // Add later
    assert(e3.GetUncertainties().size() == unc_provided.size());
    for (size_t i = 0; i < e3.GetUncertainties().size(); ++i) {
        assert(std::abs(e3.GetUncertainties()[i] - unc_provided[i]) < 1e-12);
    }

    // Test 5: Empty event
    std::cout << "Testing empty event..." << std::endl;
    TRestHaloEvent empty;
    assert(empty.GetValueAtFrequency(10.0) == 0.0);

    std::cout << "All checks passed!" << std::endl;
    return 0;
}
