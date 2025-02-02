# Set working variables.
set datafile cad/blends/0060_isolated_blends_test_13_noloc.brep

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-brep $datadir/$datafile
fit
#
if { [check-validity] != 1 } {
  error "Initial part is not valid."
}
#
print-summary
#
set initialToler [get-tolerance]

# Check Euler-Poincare property of the manifold before modification.
if { [check-euler 3] != 1 } {
  error "Euler-Poincare property is not equal to the expected value."
}

# Kill blends.
if { [kill-blend 229] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 230] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 116] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 137] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 208] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 233] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 214] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 212] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 119] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}
#
if { [kill-blend 121] != 1 } {
  error "Unexpected blend suppression result (success expected)."
}

# Check Euler-Poincare property of the manifold.
if { [check-euler 3] != 1 } {
  error "Euler-Poincare property does not hold after topological modification."
}

# Check orientations of vertices.
if { [check-vertices-ori] != 1 } {
  error "Some edges have non-distinguishable orientations of vertices."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}

# Check validity of the result.
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Check that tolernace has not significantly degraded.
set finalToler [get-tolerance]
puts "Final tolerance ($finalToler) vs initial tolerance ($initialToler)"
#
if { [expr $finalToler - $initialToler] > 1e-3 } {
  error "Significant tolerance degradation."
}
