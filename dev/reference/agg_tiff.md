# Draw to a TIFF file

The TIFF (Tagged Image File Format) format is a very versatile raster
image storage format that supports 8 and 16bit colour mode, true
transparency, as well as a range of other features not relevant to
drawing from R (e.g. support for different colour spaces). The storage
mode of the image data is not fixed and different compression modes are
possible, in contrast to PNGs one-approach-fits-all. The default
(uncompressed) will result in much larger files than PNG, and in general
PNG is a better format for many of the graphic types produced in R.
Still, TIFF has its purposes and sometimes this file format is
explicetly requested.

## Usage

``` r
agg_tiff(
  filename = "Rplot%03d.tiff",
  width = 480,
  height = 480,
  units = "px",
  pointsize = 12,
  background = "white",
  res = 72,
  scaling = 1,
  snap_rect = TRUE,
  compression = "none",
  bitsize = 8,
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

- compression:

  The compression type to use for the image data. The standard options
  from the [`grDevices::tiff()`](https://rdrr.io/r/grDevices/png.html)
  function are available under the same name.

- bitsize:

  Should the device record colour as 8 or 16bit

- bg:

  Same as `background` for compatibility with old graphic device APIs

## Note

`'jpeg'` compression is only available if ragg is compiled with a
version of `libtiff` where jpeg support has been turned on.

## Transparency

TIFF have support for true transparency, meaning that the pixel colour
is stored in pre-multiplied form. This is in contrast to pixels being
stored in plain format, where the alpha values more function as a mask.
The utility of this is not always that important, but it is one of the
benefits of TIFF over PNG so it should be noted.

## Examples

``` r
file <- tempfile(fileext = '.tiff')
# Use jpeg compression
agg_tiff(file, compression = 'lzw+p')
plot(sin, -pi, 2*pi)
dev.off()
#> agg_record_20043ecbac07 
#>                       2 
```
