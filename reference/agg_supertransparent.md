# Draw to a PNG file, modifying transparency on the fly

The graphic engine in R only supports 8bit colours. This is for the most
part fine, as 8bit gives all the fidelity needed for most graphing
needs. However, this may become a limitation if you need to plot
thousands of very translucent shapes on top of each other. 8bit only
afford a minimum of 1/255 alpha, which may end up accumulating to fully
opaque at some point. This device allows you to create a 16bit device
that modifies the alpha level of all incomming colours by a fixed
multiplier, thus allowing for much more translucent colours. The device
will only modify transparent colour, so if you pass in an opaque colour
it will be left unchanged.

## Usage

``` r
agg_supertransparent(
  filename = "Rplot%03d.png",
  width = 480,
  height = 480,
  units = "px",
  pointsize = 12,
  background = "white",
  res = 72,
  scaling = 1,
  snap_rect = TRUE,
  alpha_mod = 1,
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

- alpha_mod:

  A numeric between 0 and 1 that will be multiplied to the alpha channel
  of all transparent colours

- bg:

  Same as `background` for compatibility with old graphic device APIs
