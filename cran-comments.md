This is a small patch release on the request of cran. It reintroduces a fix to
an overflow that was fixed in the 1.0.0 release that CRAN rolled back. It also
skips testing text rendering on cran since not all cran machines supports it
(solaris)

## Test environments
* local R installation, R 4.0.3
* ubuntu 16.04 (on travis-ci), R 4.0.3
* win-builder (devel)

## R CMD check results

0 errors | 0 warnings | 0 note
