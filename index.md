# ragg

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
# install.packages('pak')
pak::pak('r-lib/ragg')
```

## Use

ragg provides drop-in replacements for the png, jpeg, and tiff graphic
devices provided by default from the grDevices packages and can both
produce png, jpeg and tiff files. Notable features, that sets itself
apart from the build-in devices, includes:

- Faster (up to 40% faster than anti-aliased cairo device)
- Direct access to all system fonts
- Advanced text rendering, including support for right-to-left text,
  emojis, and font fallback
- High quality anti-aliasing
- High quality rotated text
- Support 16-bit output
- System independent rendering (output from Mac, Windows, and Linux
  should be identical)

You can use it like any other device. The main functions are
[`agg_png()`](https://ragg.r-lib.org/reference/agg_png.md),
[`agg_jpeg()`](https://ragg.r-lib.org/reference/agg_jpeg.md) and
[`agg_tiff()`](https://ragg.r-lib.org/reference/agg_tiff.md), all of
which have arguments that closely match those of the
[`png()`](https://rdrr.io/r/grDevices/png.html),
[`jpeg()`](https://rdrr.io/r/grDevices/png.html) and
[`tiff()`](https://rdrr.io/r/grDevices/png.html) functions, so switching
over should be easy.

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

![A scatterplot created with ggplot2 using a fancy non-standard font and
mixing in emojis with
text](reference/figures/README-unnamed-chunk-3-1.png)

Further, it provides an
[`agg_capture()`](https://ragg.r-lib.org/reference/agg_capture.md)
device that lets you access the device buffer directly from your R
session.

``` r
cap <- agg_capture(width = 1000, height = 500, res = 144)
plot(1:10, 1:10)

scatter <- cap()
invisible(dev.off())

# Remove margins from raster plotting
par(mai = c(0, 0, 0, 0))
plot(as.raster(scatter))
```

![A very simple scatterplot captured as raster data and then drawn
again](reference/figures/README-unnamed-chunk-4-1.png)

### Use ragg with knitr

knitr supports png output from ragg by setting `dev = "ragg_png"` in the
chunk settings or globally with
`knitr::opts_chunk$set(dev = "ragg_png")`.

### Use ragg in RStudio

ragg can be used as the graphic back-end to the RStudio device (for
RStudio \>= 1.4) by choosing *AGG* as the backend in the graphics pane
in general options (see screenshot)

![A screenshot showing the RStudio Global Options window with focus on
the General pane and the Graphics tab. Here it shows AGG is selected as
Backend in order to tell RStudio to use
ragg](https://i.imgur.com/4XgiPWy.png)

Setting ragg as backend in RStudio

## Code of Conduct

Please note that the ‘ragg’ project is released with a [Contributor Code
of Conduct](https://ragg.r-lib.org/CODE_OF_CONDUCT.html). By
contributing to this project, you agree to abide by its terms.
