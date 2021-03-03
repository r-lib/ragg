This submission does not fix the outstanding CRAN ASAN issue as the nature of it
is still being investigated. However, fixes a single issue that in some rare
circumstances can result in system crashes. Because of the severity of the bug I
urge CRAN to consider moving forward with releasing this submission while the 
ASAN issue is being investigated.

## Test environments

* GitHub Actions (ubuntu-16.04): devel, release, oldrel, 3.5, 3.4, 3.3
* GitHub Actions (windows): release, oldrel
* GitHub Actions (macOS): release
* win-builder: devel

## R CMD check results

0 errors | 0 warnings | 0 note

## revdepcheck results

We checked 11 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 0 new problems
 * We failed to check 0 packages
 
