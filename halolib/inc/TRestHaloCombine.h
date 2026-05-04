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

#ifndef REST_TRestHaloCombine
#define REST_TRestHaloCombine

#include "TRestHaloEvent.h"
#include <vector>

/// Class to combine multiple haloscope spectra using chi2-minimized weighted average
/// Implements combination of measurements A±dA and B±dB following:
/// C = (A/dA² + B/dB²) / (1/dA² + 1/dB²)
/// dC = √(1/(1/dA² + 1/dB²))
class TRestHaloCombine {
private:
    std::vector<TRestHaloEvent*> fEvents;  // Pointer to input events
    TRestHaloEvent* fCombinedEvent = nullptr;  // Result of combination
    
    /// Check if frequency bin sizes (resolution) are compatible between two events
    /// Returns true if bin sizes match to within relative tolerance, false otherwise
    bool SameFreq(const TRestHaloEvent* event1, 
                                  const TRestHaloEvent* event2) const;

    /// Helper method to combine two events with their overlapping region
    /// Returns a new event spanning both ranges with proper combination in overlap
    TRestHaloEvent* CombineTwoEvents(const TRestHaloEvent* event1,
                                     const TRestHaloEvent* event2);

public:
    TRestHaloCombine();
    ~TRestHaloCombine();

    /// Add an event to be combined (must have at least 2 before combining)
    void AddEvent(TRestHaloEvent* event);

    /// Combine all added events using chi2-minimized weighted average
    /// Returns the combined event, or nullptr if fewer than 2 events or incompatible frequencies
    TRestHaloEvent* Combine();

    /// Get the combined result (null if not yet combined)
    TRestHaloEvent* GetCombinedEvent() const { return fCombinedEvent; }

    /// Clear all stored events and result
    void Clear();

    /// Get number of events added
    size_t GetNumEvents() const { return fEvents.size(); }
};

#endif
