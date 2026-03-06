# Draw an animation to a WebP file

The WebP format is a raster image format that provides improved lossless
(and lossy) compression for images on the web. Transparency is
supported.

## Usage

``` r
agg_webp_anim(
  filename = "Ranim.webp",
  width = 480,
  height = 480,
  units = "px",
  pointsize = 12,
  background = "white",
  res = 72,
  scaling = 1,
  snap_rect = TRUE,
  lossy = FALSE,
  quality = 80,
  delay = 100L,
  loop = 0L,
  bg
)
```

## Arguments

- filename:

  The name of the file. This function does not perform page number
  substitution as the other devices since it cannot produce multiple
  pages.

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

- lossy:

  Use lossy compression. Default is `FALSE`.

- quality:

  An integer between `0` and `100` defining either the quality (if using
  lossy compression) or the compression effort (if using lossless).

- delay:

  Per-frame delay in milliseconds (single integer)

- loop:

  Number of loops (0 = infinite)

- bg:

  Same as `background` for compatibility with old graphic device APIs

## See also

[`agg_webp()`](https://ragg.r-lib.org/dev/reference/agg_webp.md) for
static WebP images

## Examples

``` r
file <- tempfile(fileext = '.webp')
agg_webp_anim(file, delay = 100, loop = 0)
for(i in 1:10) {
  plot(sin(1:100 + i/10), type = 'l', ylim = c(-1, 1))
  dev.flush()
}
dev.off()
#> agg_record_1d4d7572e92e 
#>                       2 
```
