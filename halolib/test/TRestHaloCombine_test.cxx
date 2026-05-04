#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include "TRestHaloEvent.h"
#include "TRestHaloCombine.h"

int main() {
    std::cout << "=== TRestHaloCombine Tests ===" << std::endl;
    
    // Test combining two overlapping spectra
    std::cout << "Testing basic combination..." << std::endl;
    TRestHaloEvent s1, s2;
    std::vector<double> freq1 = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> vals1 = {1.0, 1.0, 1.0, 1.0, 1.0};
    std::vector<double> unc1 = {0.1, 0.1, 0.1, 0.1, 0.1};
    
    std::vector<double> freq2 = {4.0, 5.0, 6.0, 7.0, 8.0};
    std::vector<double> vals2 = {2.0, 2.0, 2.0, 2.0, 2.0};
    std::vector<double> unc2 = {0.2, 0.2, 0.2, 0.2, 0.2};
    
    s1.SetSpectrum(freq1, vals1, unc1, TRestHaloMetadata::V_RMS);
    s2.SetSpectrum(freq2, vals2, unc2, TRestHaloMetadata::V_RMS);
    
    TRestHaloCombine combiner;
    combiner.AddEvent(&s1);
    combiner.AddEvent(&s2);
    
    TRestHaloEvent* result = combiner.Combine();
    assert(result != nullptr);
    assert(result->GetMetadata().GetUncertaintiesProvided());
    
    const auto &result_freq = result->GetFrequencies();
    const auto &result_vals = result->GetValues();
    const auto &result_unc = result->GetUncertainties();
    
    // Should span 1-8 Hz
    assert(result_freq.size() == 8);
    assert(std::abs(result_freq.front() - 1.0) < 1e-9);
    assert(std::abs(result_freq.back() - 8.0) < 1e-9);
    
    // Check non-overlapping regions are preserved
    assert(std::abs(result_vals[0] - 1.0) < 1e-9);  // bin at 1 Hz (only in s1)
    assert(std::abs(result_unc[0] - 0.1) < 1e-9);   // same uncertainty
    
    assert(std::abs(result_vals[7] - 2.0) < 1e-9);  // bin at 8 Hz (only in s2)
    assert(std::abs(result_unc[7] - 0.2) < 1e-9);   // same uncertainty
    
    // Check combined region (bins at 4-5 Hz)
    // For chi2 combination: C = (A/dA^2 + B/dB^2) / (1/dA^2 + 1/dB^2)
    // With vals A=1, dA=0.1, B=2, dB=0.2:
    // weight_A = 1/0.01 = 100, weight_B = 1/0.04 = 25
    // C = (1*100 + 2*25) / (100 + 25) = 150/125 = 1.2
    double expected_combined = (1.0 * 100.0 + 2.0 * 25.0) / (100.0 + 25.0);
    assert(std::abs(result_vals[3] - expected_combined) < 1e-9);  // bin at 4 Hz
    
    std::cout << "Combined spectrum spans " << result_freq.size() << " bins from "
              << result_freq.front() << " to " << result_freq.back() << " Hz" << std::endl;
    std::cout << "Overlapping bin (4 Hz): value=" << result_vals[3] 
              << " (expected " << expected_combined << "), uncertainty=" << result_unc[3] << std::endl;

    // Test three-event sequential combination
    std::cout << "Testing three-event sequential combination..." << std::endl;
    TRestHaloEvent s3;
    std::vector<double> freq3 = {6.0, 7.0, 8.0, 9.0, 10.0};
    std::vector<double> vals3 = {3.0, 3.0, 3.0, 3.0, 3.0};
    std::vector<double> unc3 = {0.3, 0.3, 0.3, 0.3, 0.3};
    s3.SetSpectrum(freq3, vals3, unc3, TRestHaloMetadata::V_RMS);
    
    TRestHaloCombine combiner3;
    combiner3.AddEvent(&s1);
    combiner3.AddEvent(&s2);
    combiner3.AddEvent(&s3);
    
    TRestHaloEvent* result3 = combiner3.Combine();
    assert(result3 != nullptr);
    
    const auto &result3_freq = result3->GetFrequencies();
    // Should span 1-10 Hz
    assert(result3_freq.size() == 10);
    assert(std::abs(result3_freq.front() - 1.0) < 1e-9);
    assert(std::abs(result3_freq.back() - 10.0) < 1e-9);
    
    std::cout << "Three-event combined spectrum spans " << result3_freq.size() 
              << " bins from " << result3_freq.front() << " to " << result3_freq.back() 
              << " Hz" << std::endl;

    std::cout << "\nAll checks passed!" << std::endl;
    return 0;
}
