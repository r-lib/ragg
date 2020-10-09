This is a major release to signify the new text layouting pipeline. Due to the 
more advanced layouting, this version takes on dependencies on HarfBuzz and 
Fribidi through the textshaping package. In turn it gains support for 
bidirectional and right-to-left text along with advanced OpenType features.

This release also fixes an ASAN issue present in the last release

## Test environments
* local R installation, R 4.0.1
* ubuntu 16.04 (on travis-ci), R 4.0.1
* win-builder (devel)

## R CMD check results

0 errors | 0 warnings | 0 note
