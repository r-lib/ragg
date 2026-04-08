# Capture drawing instructions without rendering

While the point of a graphics device is usually to render the graphics,
there are a few situations where you are instead interested in only
capturing the instructions required to render the graphics. While all
graphics devices can be retrofitted for that using
[`dev.control()`](https://rdrr.io/r/grDevices/dev2.html), they would
still render to their internal buffer even if you are only interested in
the recorded instructions, thus adding a performance penalty.
`agg_record()` is a device that does no rendering whatsoever, but has
recording turned on by default making it a no-overhead solution for plot
recording.

## Usage

``` r
agg_record(
  width = 480,
  height = 480,
  units = "px",
  pointsize = 12,
  background = "white",
  res = 72,
  scaling = 1,
  snap_rect = TRUE,
  bg
)
```

## Arguments

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

- bg:

  Same as `background` for compatibility with old graphic device APIs

## Examples

``` r
# Capture drawing instructions
agg_record()
plot(1:10, 1:10)
rec <- recordPlot()
dev.off()
#> agg_record_201d6318e98f 
#>                       2 

# Replay these on another device
file <- tempfile(fileext = '.png')
agg_png(file)
replayPlot(rec)
dev.off()
#> agg_record_201d6318e98f 
#>                       2 
```
