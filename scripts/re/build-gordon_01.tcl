# Set working variables.
set datafile cad/gordon/test-cim.stp
set refErr   0.062

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 -g 2 3]

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
