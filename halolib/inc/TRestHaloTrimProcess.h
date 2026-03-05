/*************************************************************************
 * TRestHaloTrimProcess
 *
 * Process that converts incoming halo spectra from volts (V_RMS) to power
 * (watts) when needed and trims the leading/trailing bins.
 *
 *************************************************************************/

#ifndef RESTProc_TRestHaloTrimProcess
#define RESTProc_TRestHaloTrimProcess

#include "TRestEventProcess.h"
#include "TRestHaloEvent.h"

/// Process that converts and trims halo spectra
class TRestHaloTrimProcess : public TRestEventProcess {
private:
    TRestEvent* fEvent;  //! pointer to input event

    // Number of bins to trim from the beginning and end of the spectrum
    // This member is persisted so it can be configured from RML or ROOT/C++
    // macros via the metadata interface.
    Int_t fTrimBins = 800; //< number of bins to trim

    void Initialize() override;

public:
    RESTValue GetInputEvent() const override { return fEvent; }
    RESTValue GetOutputEvent() const override { return fEvent; }

    void InitProcess() override;
    const char* GetProcessName() const override { return "HaloTrim"; }

    TRestEvent* ProcessEvent(TRestEvent* eventInput) override;

    void EndProcess() override;

    void PrintMetadata() override {
        BeginPrintProcess();
        printf("  TrimBins=%d\n", fTrimBins);
        EndPrintProcess();
    }

    // Allow runtime configuration from C++ / ROOT macros
    void SetTrimBins(Int_t n) { fTrimBins = n; }
    Int_t GetTrimBins() const { return fTrimBins; }

    TRestHaloTrimProcess();
    ~TRestHaloTrimProcess();

    ClassDefOverride(TRestHaloTrimProcess, 1);
};

#endif
