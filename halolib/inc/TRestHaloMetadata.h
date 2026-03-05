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

#ifndef REST_TRestHaloMetadata
#define REST_TRestHaloMetadata

#include "TRestMetadata.h"

/// Dedicated metadata for halo/acquisition settings
class TRestHaloMetadata : public TRestMetadata {
private:
    double fStartTime = 0;               // Initial timestamp (Unix)
    double fIntegrationTime = 0;         // Seconds per spectrum
    int    fFramesPerSpectrum = 0;       // Frame averaging count
    double fResolutionBandwidth = 0;     // Hz
    double fReferencePowerdBm = 0;       // dBm
    bool   fPreamplifierEnabled = false; // true/false
    double fCenterFrequency = 0;         // Hz
    int    fNumSpectra = 0;
    int    fNumFreqPoints = 0;

    // optional metadata
    TString fExperimentName = "";
    int     fRunNumber = -1;
    double  fAttenuation = 0;            // dB if available
    TString fNotes = "";
    // unit for the stored values (0 = V_RMS, 1 = W)
    Int_t fValueUnit = 0; // use enum EValueUnit

public:
    // Units stored with the data. Prefer enum for clarity.
    enum EValueUnit {
        V_RMS = 0,
        W = 1
    };

    TRestHaloMetadata();
    ~TRestHaloMetadata() override;

    // --- Setters ---
    void SetStartTime(double v) { fStartTime = v; }
    void SetIntegrationTime(double v) { fIntegrationTime = v; }
    void SetFramesPerSpectrum(int v) { fFramesPerSpectrum = v; }
    void SetResolutionBandwidth(double v) { fResolutionBandwidth = v; }
    void SetReferencePower(double v) { fReferencePowerdBm = v; }
    void SetPreamplifierEnabled(bool v) { fPreamplifierEnabled = v; }
    void SetCenterFrequency(double v) { fCenterFrequency = v; }
    void SetNumSpectra(int v) { fNumSpectra = v; }
    void SetNumFreqPoints(int v) { fNumFreqPoints = v; }

    void SetExperimentName(const TString& n) { fExperimentName = n; }
    void SetRunNumber(int r) { fRunNumber = r; }
    void SetAttenuation(double a) { fAttenuation = a; }
    void SetNotes(const TString& n) { fNotes = n; }
    void SetValueUnit(Int_t u) { fValueUnit = u; }

    // --- Getters ---
    double GetStartTime() const { return fStartTime; }
    double GetIntegrationTime() const { return fIntegrationTime; }
    int    GetFramesPerSpectrum() const { return fFramesPerSpectrum; }
    double GetResolutionBandwidth() const { return fResolutionBandwidth; }
    double GetReferencePower() const { return fReferencePowerdBm; }
    bool   GetPreamplifierEnabled() const { return fPreamplifierEnabled; }
    double GetCenterFrequency() const { return fCenterFrequency; }

    int    GetNumSpectra() const { return fNumSpectra; }
    int    GetNumFreqPoints() const { return fNumFreqPoints; }

    TString GetExperimentName() const { return fExperimentName; }
    int     GetRunNumber() const { return fRunNumber; }
    double  GetAttenuation() const { return fAttenuation; }
    TString GetNotes() const { return fNotes; }
    Int_t GetValueUnit() const { return fValueUnit; }
    TString GetValueUnitString() const;

    void PrintMetadata() override;

    ClassDefOverride(TRestHaloMetadata, 1);
};

#endif
