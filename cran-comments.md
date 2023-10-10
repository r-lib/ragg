A few bug fixes as well as prep for Arm Windows. The one revdep failure seems
to be related as the package is not using any ragg functionality.

## revdepcheck results

We checked 37 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 1 new problems
 * We failed to check 1 packages

Issues with CRAN packages are summarised below.

### New problems
(This reports the first line of each new failure)

* WhatsR
  checking tests ... ERROR

### Failed to check

* ursa (NA)

