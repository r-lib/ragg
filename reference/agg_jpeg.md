# Draw to a JPEG file

The JPEG file format is a lossy compressed file format developed in
particular for digital photography. The format is not particularly
well-suited for line drawings and text of the type normally associated
with statistical plots as the compression algorithm creates noticable
artefacts. It is, however, great for saving image data, e.g. heightmaps
etc. Thus, for standard plots, it would be better to use
[`agg_png()`](https://ragg.r-lib.org/reference/agg_png.md), but for
plots that includes a high degree of raster image rendering this device
will result in smaller plots with very little quality degradation.

## Usage

``` r
agg_jpeg(
  filename = "Rplot%03d.jpeg",
  width = 480,
  height = 480,
  units = "px",
  pointsize = 12,
  background = "white",
  res = 72,
  scaling = 1,
  snap_rect = TRUE,
  quality = 75,
  smoothing = FALSE,
  method = "slow",
  bg
)
```

## Arguments

- filename:

  The name of the file. Follows the same semantics as the file naming in
  [`grDevices::png()`](https://rdrr.io/r/grDevices/png.html), meaning
  that you can provide a
  [`sprintf()`](https://rdrr.io/r/base/sprintf.html) compliant string
  format to name multiple plots (such as the default value)

- width, height:

  The dimensions of the device

- units:

  The unit `width` and `height` is measured in, in either pixels
  (`'px'`), inches (`'in'`), millimeters (`'mm'`), or centimeter
  (`'cm'`).

- pointsize:

  The default pointsize of the device in pt. This will in general not
  have any effect on grid graphics (including ggplot2) as text size is
  always set explicitly there.

- background:

  The background colour of the device

- res:

  The resolution of the device. This setting will govern how device
  dimensions given in inches, centimeters, or millimeters will be
  converted to pixels. Further, it will be used to scale text sizes and
  linewidths

- scaling:

  A scaling factor to apply to the rendered line width and text size.
  Useful for getting the right dimensions at the resolution that you
  need. If e.g. you need to render a plot at 4000x3000 pixels for it to
  fit into a layout, but you find that the result appears to small, you
  can increase the `scaling` argument to make everything appear bigger
  at the same resolution.

- snap_rect:

  Should axis-aligned rectangles drawn with only fill snap to the pixel
  grid. This will prevent anti-aliasing artifacts when two rectangles
  are touching at their border.

- quality:

  An integer between `0` and `100` defining the quality/size tradeoff.
  Setting this to `100` will result in no compression.

- smoothing:

  A smoothing factor to apply before compression, from `0` (no
  smoothing) to `100` (full smoothing). Can also by `FALSE` (no
  smoothing) or `TRUE` (full smoothing).

- method:

  The compression algorithm to use. Either `'slow'`, `'fast'`, or
  `'float'`. Default is `'slow'` which works best for most cases.
  `'fast'` should only be used when quality is below `97` as it may
  result in worse performance at high quality settings. `'float'` is a
  legacy options that calculate the compression using floating point
  precission instead of with integers. It offers no quality benefit and
  is often much slower.

- bg:

  Same as `background` for compatibility with old graphic device APIs

## Note

Smoothing is only applied if ragg has been compiled against a jpeg
library that supports smoothing.

## Examples

``` r
file <- tempfile(fileext = '.jpeg')
agg_jpeg(file, quality = 50)
plot(sin, -pi, 2*pi)
dev.off()
#> agg_record_20e6114e9f37 
#>                       2 
```
