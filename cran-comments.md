This release fixes a bug in the AGG source code where the font index was not 
considered in the freetype font manager, thus returning the wrong font when 
using font collection files.

## Test environments
* local OS X install, R 3.6.0
* ubuntu 14.04 (on travis-ci), R 3.6.0
* win-builder (devel and release)

## R CMD check results

0 errors | 0 warnings | 0 note
