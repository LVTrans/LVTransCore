# Hydraulic reference data

## Reservoir–pipe–reservoir

`labview_reservoir_pipe_reservoir.csv` transcribes the user's LabVIEW export
provided on 2026-10-29. Decimal commas were converted to decimal points. The
duplicate absolute-head columns were omitted; the reported timestamps and
H/Q values were preserved.

Configuration: dt=0.1 s, L=100 m, D=1 m, a_in=1200 m/s, lambda_in=500000,
rho=1000, f=0.01, initial pipe H=Q=0, both reservoir heads=100 m, and both
reservoir loss coefficients=100000. The author's supplied implementation uses
g=9.82 m/s² and the grid-adjusted wave speed in B.

The first nine reported times are 2.1, 3.1, ..., 10.1 s. Remaining reported
times are 11, 12, ..., 49 s. The regression compares all 48 value rows at
integer solver steps 21, 31, ..., 491, with tolerance 5.1e-7 for six-decimal
rounding. Timestamp agreement is asserted only for the first nine rows.
The later timestamp/phase discrepancy remains unconfirmed; matching the value
sequence does not establish agreement at exactly 11.0, 12.0, ..., 49.0 s.
Generated output retains the actual C++ simulation times (through 49.1 s).

## Reservoir–pipe–valve

`reservoir_pipe_valve.csv` is the historical g=9.806 reference and is retained
unchanged. The active regression uses `reservoir_pipe_valve_g982.csv` for the
LVTrans gravity convention.

The new fixture was generated independently of `lvtrans_core` using
`examples/reservoir_pipe_valve_old.cpp`, with its local `double g = 9.806;`
changed to `double g = 9.82;` in a temporary copy. Compile that standalone file
and run it in a temporary directory; `output_old.csv` is the reference output.
No other equations or parameters were changed. The new core output matches
that standalone output byte-for-byte at its printed precision.

## Combined pipe and reservoir export (2026-10-30)

`labview_pipe_and_reservoirs.csv` contains all 184 samples from the three new
LabVIEW exports, joined by their unchanged timestamps (0.3 through 183.3 s).
Decimal commas are converted to decimal points; six-decimal values and signs
are preserved. No timing shift is applied.

Column mapping:

- `reported_time`: Time (s), identical in all three exports.
- `H_0`, `H_N`: pipe H1-egl and H2-egl.
- `Q_0`, `Q_N`: pipe Q1 and Q2.
- `LR_Q2`: left reservoir Q2 (pipe-facing port).
- `RR_Q1`: right reservoir Q1 (pipe-facing port).

The negative-flow reservoir export is identified as left and the positive-flow
export as right, consistent with the user's earlier reservoir labels. Source
attachment IDs are respectively `245eaae3-cbf9-49ec-be2c-3c676d73922e` (left),
`fbdf0acc-36c6-4612-a60c-136ada01b330` (right), and
`0a05190d-fc00-4a07-95f0-f87aa9cb0ebc` (pipe). The integration test uses this fixture for all 184 timestamps. The older
fixture is retained separately.
