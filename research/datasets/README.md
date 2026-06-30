# Datasets

This directory holds ECG data used for CardioCore V1 research, prototyping, and
education. It exists so that experiments are reproducible and so that the origin
and licensing of every recording is unambiguous.

> **Research only.** CardioCore V1 is a research/prototyping/education platform,
> **not** a certified medical device. Nothing stored or referenced here is for
> diagnosis, treatment, patient monitoring, emergency use, or clinical
> decision-making.

## What may go here

Only two kinds of data are permitted in this directory (or referenced from it):

1. **Experimental recordings made with CardioCore V1** as part of bench/research
   work (e.g. signal-quality tests, function-generator inputs, ECG simulator
   inputs, or voluntary self-recordings by a consenting researcher in a
   non-clinical context).
2. **Properly licensed or open ECG datasets** that you are permitted to store or
   redistribute under their license, kept strictly for engineering and
   educational use.

## Hard rules

- **NO real patient or clinical data.** Do not store data sourced from clinical
  care, hospitals, or patients.
- **NO personally identifiable information (PII).** No names, dates of birth,
  medical record numbers, contact details, device serials tied to a person, or
  any other identifier. De-identify before committing; if in doubt, do not
  commit it.
- **Respect every dataset license.** Do not store or redistribute data whose
  license forbids it. Some open datasets may only be *referenced* (linked), not
  copied into this repo — honor that.
- **Document source and license** for every external dataset (see below). Data
  without a documented source and license must not be added.
- **Recordings from people require informed, voluntary consent** and must remain
  non-clinical. Keep consent records outside this repo.

## Metadata convention

Place a small sidecar file next to each dataset or recording, named
`<dataset_or_recording>.meta.md` (or a single `INDEX.md` per subfolder).
Suggested fields:

| Field     | Description                                                        |
|-----------|--------------------------------------------------------------------|
| `source`  | Where it came from (CardioCore V1 capture, or dataset name + URL)  |
| `license` | License / usage terms; for CardioCore captures, the repo's terms   |
| `date`    | Capture or retrieval date (YYYY-MM-DD)                             |
| `notes`   | Setup, signal source, electrode config, sample rate, caveats       |

Example (CardioCore V1 capture):

```
source:  CardioCore V1 — bench capture, ECG simulator input
license: TBD (see repository LICENSE)
date:    2026-06-29
notes:   8-channel, ADS1298, sample rate TBD, RLD enabled; sim @ 60 BPM, 1 mV
```

Example (external dataset):

```
source:  <dataset name> — <stable URL>
license: <exact license, e.g. ODC-By / CC BY 4.0 / dataset-specific>
date:    2026-06-29 (retrieved)
notes:   Stored by reference only per license; attribution required.
```

## Suggested layout

```
datasets/
  cardiocore-v1/      # recordings captured with the hardware
  external/           # licensed/open datasets or reference stubs
  README.md           # this file
```

## TBD

- Canonical on-disk file format(s) for CardioCore V1 captures — **TBD**.
- Default sample rate and per-channel gain documented in metadata — **TBD**.
- Whether large files are tracked via Git LFS or kept out of the repo — **TBD**.
- Approved list of referenced external datasets — **TBD**.
