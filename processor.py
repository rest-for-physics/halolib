#!/usr/bin/env python3
"""Save SpectrumSumFile_ TGraphs as separate TRestHaloEvent objects.

Usage:
    python3 save_spectrum_tgraphs.py input.root output.root [libpath]

This script reads TGraph objects whose names start with "SpectrumSumFile_"
from `input.root`, converts them to `TRestHaloEvent` objects and writes each
event into `output.root` using the same name as the original TGraph.
Also want to:
- try trimming
- try fitting
- try combining
"""
import sys
import os
from ROOT import gSystem, TFile


def discover_lib(provided_path=''):
    candidates = []
    if provided_path:
        candidates.append(provided_path)
    # repo-local install
    here = os.path.dirname(__file__)
    candidates.append(os.path.join(here, '..', '..', 'rest-install', 'lib', 'libhalolib.so'))
    # LD_LIBRARY_PATH entries
    ld = os.environ.get('LD_LIBRARY_PATH', '')
    for p in ld.split(':'):
        if p:
            candidates.append(os.path.join(p, 'libhalolib.so'))
    for c in candidates:
        try:
            if c and os.path.exists(os.path.expanduser(c)):
                return os.path.expanduser(c)
        except Exception:
            continue
    return ''


def main():
    if len(sys.argv) < 3:
        print('Usage: processor.py input.root output.root [libpath]')
        return

    infile = sys.argv[1]
    outfile = sys.argv[2]
    libpath = sys.argv[3] if len(sys.argv) > 3 else ''

    lib_to_load = discover_lib(libpath)
    gSystem.Load(lib_to_load)

    try:
        from ROOT import TRestHaloEvent, TRestHaloMetadata, TRestHaloTrimProcess
    except Exception as e:
        print('Failed to import REST classes from ROOT. Ensure libhalolib is available.')
        print('Error:', e)
        return

    # Read input and create the output after doing the desired processesing 
    fin = TFile.Open(infile)
    fout = TFile.Open(outfile, 'RECREATE')

    processed = 0
    for key in fin.GetListOfKeys():
        obj = key.ReadObj()
        if (obj.ClassName() == 'TGraph') or obj.InheritsFrom('TGraph'):
            name = key.GetName()
            if not name.startswith('SpectrumSumFile_'):
                continue
            if processed == processed // 10 * 10:  # Print progress every 10 processed objects
                print('Converting:', name)
            npts = obj.GetN()
            xs = obj.GetX()
            ys = obj.GetY()
            freqs = [xs[i] for i in range(npts)]
            vals_v = [ys[i] for i in range(npts)]
            vals_w = [TRestHaloEvent.VoltsToWatts(v) for v in vals_v]

            ev = TRestHaloEvent()
            # TRestHaloMetadata enum members may not be exposed to PyROOT;
            # use integer value 1 for `W` (see TRestHaloMetadata::EValueUnit)
            ev.SetSpectrum(freqs, vals_w, 1)
            
            # Populate metadata (attached to the event and serialized with it)
            meta = ev.GetMetadata()
            meta.SetValueUnit(1) # 0 = Volts, 1 = Watts 
            meta.SetNumFreqPoints(npts)
            if npts > 1:
                # estimate resolution and center frequency from the graph
                res = freqs[1] - freqs[0]
                meta.SetResolutionBandwidth(res)
                meta.SetCenterFrequency((freqs[0] + freqs[-1]) / 2.0)
            meta.SetExperimentName('ConvertedFromTGraph')
            meta.SetNotes('Converted with processor.py')
            # Note: PyROOT may not expose TObject::SetName for this class,
            # so instead write a separate TGraph named 'Spectrum_<name>'.

            # Trim the event using TRestHaloTrimProcess (8000 bins)
            proc = TRestHaloTrimProcess()
            proc.SetTrimBins(14000)
            out_ev = proc.ProcessEvent(ev)
            if out_ev:
                write_ev = out_ev
            else:
                print('Trimming failed for', name, '- writing original event')
                write_ev = ev

            # write object into output file using the same name
            fout.cd()
            fout.WriteObject(write_ev, name)
            processed += 1


    fout.Close()
    fin.Close()
    print('Finished. Written', processed, 'TRestHaloEvent objects to', outfile)


if __name__ == '__main__':
    main()
