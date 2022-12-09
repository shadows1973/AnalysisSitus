set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

# Generate facets
generate-facets -very-rough

# Make temprary folder.
set subDir "/generate_facets_002/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in gltf.
set resultName "result_002.glb"
save-gltf -filename $tmpDir$resultName

# Check size.
set expectedSize 121128
set precision 20
set size [file size $tmpDir$resultName]
if { [expr abs($expectedSize - $size) ] <= $precision } {
  file delete -force $tmpDir
} else {
 # file delete -force $tmpDir
  return -code error "Error: actual size of $resultName is $size"
}