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

 #include "TRestHaloMetadata.h"

ClassImp(TRestHaloMetadata)

TRestHaloMetadata::TRestHaloMetadata() {
    // default initialization handled in member declarations
}

TRestHaloMetadata::~TRestHaloMetadata() {}

void TRestHaloMetadata::PrintMetadata() {
    // print generic metadata info
    TRestMetadata::PrintMetadata();
    // print the haloscope-specific fields
    RESTMetadata << "=== Halo Metadata ===\n";
    RESTMetadata << " Experiment        : " << fExperimentName << "\n";
    RESTMetadata << " Run number        : " << fRunNumber << "\n";
    RESTMetadata << " Start time        : " << fStartTime << "\n";
    RESTMetadata << " Integration time  : " << fIntegrationTime << " s\n";
    RESTMetadata << " Frames/spectrum   : " << fFramesPerSpectrum << "\n";
    RESTMetadata << " RBW               : " << fResolutionBandwidth << " Hz\n";
    RESTMetadata << " Reference power   : " << fReferencePowerdBm << " dBm\n";
    RESTMetadata << " Preamp enabled    : " << fPreamplifierEnabled << "\n";
    RESTMetadata << " Center freq       : " << fCenterFrequency << " Hz\n";
    RESTMetadata << " Num spectra       : " << fNumSpectra << "\n";
    RESTMetadata << " Frequency points  : " << fNumFreqPoints << "\n";
    RESTMetadata << " Attenuation       : " << fAttenuation << " dB\n";
    RESTMetadata << " Notes             : " << fNotes << "\n";
    // print stored units
    RESTMetadata << " Stored value unit : " << GetValueUnitString() << "\n";
}

TString TRestHaloMetadata::GetValueUnitString() const {
    switch (fValueUnit) {
        case TRestHaloMetadata::W: return "W";
        case TRestHaloMetadata::V_RMS:
        default:
            return "V_RMS";
    }
}
