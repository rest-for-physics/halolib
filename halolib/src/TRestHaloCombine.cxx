/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see https://gifna.unizar.es/trex                 *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see https://www.gnu.org/licenses/.                            *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

#include "TRestHaloCombine.h"
#include <cmath>
#include <algorithm>
#include <iostream>

TRestHaloCombine::TRestHaloCombine() {}

TRestHaloCombine::~TRestHaloCombine() {
    if (fCombinedEvent) delete fCombinedEvent;
}

void TRestHaloCombine::AddEvent(TRestHaloEvent* event) {
    if (event) {
        fEvents.push_back(event);
    }
}

bool TRestHaloCombine::SameFreq(const TRestHaloEvent* event1,
                                const TRestHaloEvent* event2) const {
    // Check that bin sizes (resolution) are compatible
    // Use relative tolerance of 1e-6
    const double tol = 1e-6;
    double res1 = event1->GetFrequencies().size() > 1 ? 
        event1->GetFrequencies()[1] - event1->GetFrequencies()[0] : 0;
    double res2 = event2->GetFrequencies().size() > 1 ? 
        event2->GetFrequencies()[1] - event2->GetFrequencies()[0] : 0;

    if (res1 <= 0 || res2 <= 0) {
        return false;  // Invalid resolution
    }

    double relDiff = std::abs(res1 - res2) / std::max(res1, res2);
    return relDiff < tol;
}

TRestHaloEvent* TRestHaloCombine::Combine() {
    // Require at least 2 events
    if (fEvents.size() < 2) {
        std::cerr << "TRestHaloCombine::Combine() - Error: Need at least 2 events, got "
                  << fEvents.size() << std::endl;
        return nullptr;
    }

    // Check frequency bin size compatibility of all events
    for (size_t i = 1; i < fEvents.size(); ++i) {
        if (!SameFreq(fEvents[0], fEvents[i])) {
            std::cerr << "TRestHaloCombine::Combine() - Error: Frequency bin sizes incompatible "
                      << "between event 0 and event " << i << std::endl;
            return nullptr;
        }
    }

    // Check that all events have same units
    auto unit0 = fEvents[0]->GetStoredUnit();
    for (size_t i = 1; i < fEvents.size(); ++i) {
        if (fEvents[i]->GetStoredUnit() != unit0) {
            std::cerr << "TRestHaloCombine::Combine() - Error: Different units detected. "
                      << "All input events must have the same units." << std::endl;
            return nullptr;
        }
    }

    // Start with the first event
    if (fCombinedEvent) delete fCombinedEvent;
    fCombinedEvent = new TRestHaloEvent(*fEvents[0]);

    // Sequentially combine with each subsequent event
    for (size_t i = 1; i < fEvents.size(); ++i) {
        TRestHaloEvent* result = CombineTwoEvents(fCombinedEvent, fEvents[i]);
        if (!result) {
            std::cerr << "TRestHaloCombine::Combine() - Error: Failed to combine event " << i << std::endl;
            return nullptr;
        }
        delete fCombinedEvent;
        fCombinedEvent = result;
    }

    return fCombinedEvent;
}

TRestHaloEvent* TRestHaloCombine::CombineTwoEvents(const TRestHaloEvent* event1,
                                                   const TRestHaloEvent* event2) {
    const auto& freq1 = event1->GetFrequencies();
    const auto& freq2 = event2->GetFrequencies();

    if (freq1.empty() || freq2.empty()) {
        std::cerr << "TRestHaloCombine::CombineTwoEvents() - Error: Empty frequency array" << std::endl;
        return nullptr;
    }

    // Find overlapping frequency range for these two events
    double fmin_overlap = std::max(freq1.front(), freq2.front());
    double fmax_overlap = std::min(freq1.back(), freq2.back());

    // Check if there's actual overlap
    if (fmin_overlap >= fmax_overlap) {
        std::cerr << "TRestHaloCombine::CombineTwoEvents() - Error: No overlapping frequency range. "
                  << "Event1: [" << freq1.front() << ", " << freq1.back() << "] Hz, "
                  << "Event2: [" << freq2.front() << ", " << freq2.back() << "] Hz" << std::endl;
        return nullptr;
    }

    // Build combined frequency array spanning both events
    std::vector<double> combinedFreq;
    for (double f : freq1) {
        bool found = false;
        for (double cf : combinedFreq) {
            if (std::abs(f - cf) < 1e-9) {
                found = true;
                break;
            }
        }
        if (!found) {
            combinedFreq.push_back(f);
        }
    }
    for (double f : freq2) {
        bool found = false;
        for (double cf : combinedFreq) {
            if (std::abs(f - cf) < 1e-9) {
                found = true;
                break;
            }
        }
        if (!found) {
            combinedFreq.push_back(f);
        }
    }
    std::sort(combinedFreq.begin(), combinedFreq.end());

    // Perform combination for each frequency bin
    std::vector<double> combinedValues;
    std::vector<double> combinedErrors;

    for (double freq_val : combinedFreq) {
        bool in1 = (freq_val >= freq1.front() - 1e-9) && (freq_val <= freq1.back() + 1e-9);
        bool in2 = (freq_val >= freq2.front() - 1e-9) && (freq_val <= freq2.back() + 1e-9);
        bool in_overlap = (freq_val >= fmin_overlap - 1e-9) && (freq_val <= fmax_overlap + 1e-9);

        double final_value = 0.0;
        double final_error = 0.0;

        if (in_overlap) {
            // Bin is in overlapping region: combine both events
            // Find bins in both events
            auto it1 = std::lower_bound(freq1.begin(), freq1.end(), freq_val);
            size_t idx1 = 0;
            if (it1 == freq1.end()) {
                idx1 = freq1.size() - 1;
            } else {
                idx1 = std::distance(freq1.begin(), it1);
                if (idx1 > 0 && std::abs(freq1[idx1-1] - freq_val) < std::abs(*it1 - freq_val)) {
                    idx1 = idx1 - 1;
                }
            }

            auto it2 = std::lower_bound(freq2.begin(), freq2.end(), freq_val);
            size_t idx2 = 0;
            if (it2 == freq2.end()) {
                idx2 = freq2.size() - 1;
            } else {
                idx2 = std::distance(freq2.begin(), it2);
                if (idx2 > 0 && std::abs(freq2[idx2-1] - freq_val) < std::abs(*it2 - freq_val)) {
                    idx2 = idx2 - 1;
                }
            }

            double value1 = event1->GetValues()[idx1];
            double value2 = event2->GetValues()[idx2];
            double stddev1 = event1->GetUncertainties()[idx1];
            double stddev2 = event2->GetUncertainties()[idx2];

            // Avoid division by zero
            if (stddev1 <= 0) stddev1 = 1e-10;
            if (stddev2 <= 0) stddev2 = 1e-10;

            // Chi2 weighted combination
            double weight1 = 1.0 / (stddev1 * stddev1);
            double weight2 = 1.0 / (stddev2 * stddev2);
            final_value = (value1 * weight1 + value2 * weight2) / (weight1 + weight2);
            final_error = std::sqrt(1.0 / (weight1 + weight2));
        } else if (in1 && !in2) {
            // Only in event1
            auto it1 = std::lower_bound(freq1.begin(), freq1.end(), freq_val);
            size_t idx1 = 0;
            if (it1 == freq1.end()) {
                idx1 = freq1.size() - 1;
            } else {
                idx1 = std::distance(freq1.begin(), it1);
                if (idx1 > 0 && std::abs(freq1[idx1-1] - freq_val) < std::abs(*it1 - freq_val)) {
                    idx1 = idx1 - 1;
                }
            }
            final_value = event1->GetValues()[idx1];
            final_error = event1->GetUncertainties()[idx1];
        } else if (!in1 && in2) {
            // Only in event2
            auto it2 = std::lower_bound(freq2.begin(), freq2.end(), freq_val);
            size_t idx2 = 0;
            if (it2 == freq2.end()) {
                idx2 = freq2.size() - 1;
            } else {
                idx2 = std::distance(freq2.begin(), it2);
                if (idx2 > 0 && std::abs(freq2[idx2-1] - freq_val) < std::abs(*it2 - freq_val)) {
                    idx2 = idx2 - 1;
                }
            }
            final_value = event2->GetValues()[idx2];
            final_error = event2->GetUncertainties()[idx2];
        }

        combinedValues.push_back(final_value);
        combinedErrors.push_back(final_error);
    }

    // Create combined event with per-bin uncertainties
    TRestHaloEvent* result = new TRestHaloEvent();
    result->SetSpectrum(combinedFreq, combinedValues, combinedErrors, event1->GetStoredUnit());

    return result;
}

void TRestHaloCombine::Clear() {
    fEvents.clear();
    if (fCombinedEvent) {
        delete fCombinedEvent;
        fCombinedEvent = nullptr;
    }
}
