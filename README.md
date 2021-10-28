
<!-- README.md is generated from README.Rmd. Please edit that file -->

# ragg <a href='https://ragg.r-lib.org'><img src='man/figures/logo.png' align="right" height="131.5" /></a>

<!-- badges: start -->

[![Codecov test
coverage](https://codecov.io/gh/r-lib/ragg/branch/main/graph/badge.svg)](https://app.codecov.io/gh/r-lib/ragg?branch=main)
[![CRAN
status](https://www.r-pkg.org/badges/version/ragg)](https://cran.r-project.org/package=ragg)
[![Lifecycle:
stable](https://img.shields.io/badge/lifecycle-stable-brightgreen.svg)](https://lifecycle.r-lib.org/articles/stages.html#stable)
[![R-CMD-check](https://github.com/r-lib/ragg/workflows/R-CMD-check/badge.svg)](https://github.com/r-lib/ragg/actions)
<!-- badges: end -->

This package provides graphic devices for R based on the AGG library
developed by the late Maxim Shemanarev. AGG provides both higher
performance and higher quality than the standard raster devices provided
by grDevices. For a comparison with the default devices, see the
[performance](https://ragg.r-lib.org/articles/ragg_performance.html) and
[quality](https://ragg.r-lib.org/articles/ragg_quality.html) vignettes.

## Installation

The package can be installed from CRAN with `install.packages('ragg')`
or, if the development version is desired, directly from github:

``` r
# install.packages('devtools')
devtools::install_github('r-lib/ragg')
```

## Use

ragg provides drop-in replacements for the png, jpeg, and tiff graphic
devices provided by default from the grDevices packages and can both
produce png, jpeg and tiff files. Notable features, that sets itself
apart from the build-in devices, includes:

-   Faster (up to 40% faster than anti-aliased cairo device)
-   Direct access to all system fonts
-   Advanced text rendering, including support for right-to-left text,
    emojis, and font fallback
-   High quality anti-aliasing
-   High quality rotated text
-   Support 16-bit output
-   System independent rendering (output from Mac, Windows, and Linux
    should be identical)

You can use it like any other device. The main functions are
`agg_png()`, `agg_jpeg()` and `agg_tiff()`, all of which have arguments
that closely match those of the `png()`, `jpeg()` and `tiff()`
functions, so switching over should be easy.

``` r
library(ragg)
library(ggplot2)

file <- knitr::fig_path('.png')

on_linux <- tolower(Sys.info()[['sysname']]) == 'linux'
fancy_font <- if (on_linux) 'URW Chancery L' else 'Papyrus'

agg_png(file, width = 1000, height = 500, res = 144)
ggplot(mtcars) + 
  geom_point(aes(mpg, disp, colour = hp)) + 
  labs(title = 'System fonts — Oh My! 😱') + 
  theme(text = element_text(family = fancy_font))
invisible(dev.off())

knitr::include_graphics(file)
```

<img src="man/figures/README-unnamed-chunk-3-1.png" width="100%" />

Further, it provides an `agg_capture()` device that lets you access the
device buffer directly from your R session.

``` r
cap <- agg_capture(width = 1000, height = 500, res = 144)
plot(1:10, 1:10)

scatter <- cap()
invisible(dev.off())

# Remove margins from raster plotting
par(mai = c(0, 0, 0, 0))
plot(as.raster(scatter))
```

<img src="man/figures/README-unnamed-chunk-4-1.png" width="70%" style="margin-left: 15%;" />

### Use ragg with knitr

knitr supports png output from ragg by setting `dev = "ragg_png"` in the
chunk settings or globally with
`knitr::opts_chunk$set(dev = "ragg_png")`.

### Use ragg in RStudio

ragg can be used as the graphic back-end to the RStudio device (for
RStudio \>= 1.4) by choosing *AGG* as the backend in the graphics pane
in general options (see screenshot)

![Setting ragg as backend in RStudio](https://i.imgur.com/4XgiPWy.png)

## Code of Conduct

Please note that the ‘ragg’ project is released with a [Contributor Code
of Conduct](https://ragg.r-lib.org/CODE_OF_CONDUCT.html). By
contributing to this project, you agree to abide by its terms.
