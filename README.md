
<!-- README.md is generated from README.Rmd. Please edit that file -->

# ragg

<!-- badges: start -->

[![Travis build
status](https://travis-ci.org/thomasp85/ragg.svg?branch=master)](https://travis-ci.org/thomasp85/ragg)
[![CRAN
status](https://www.r-pkg.org/badges/version/ragg)](https://cran.r-project.org/package=ragg)
<!-- badges: end -->

This package provides graphic devices for R based on the AGG library
developed by the late Maxim Shemanarev. AGG provides both higher
performance and higher quality than the standard cairo based drivers
provided by grDevices, and is further system agnostic so that output
should be identical across operating systems.

## Installation

The package is currently under development and has only been configured
for Mac and Linux. You’ll need to have pkg-config, freetype, fontconfig,
libpng, and libtiff available on your system to compile the package. If
you have these libraries available you should be able to install the
package using devtools:

``` r
# install.packages('devtools')
devtools::install_github('thomasp85/ragg')
```
