#' Draw to a PPM file
#' 
#' The PPM (Portable Pixel Map) format defines one of the simplest storage 
#' formats available for 
#' image data. It is basically a raw 8bit RGB stream with a few bytes of 
#' information in the start. It goes without saying, that this file format is
#' horribly inefficient and should only be used if you want to play around with
#' a simple file format, or need a file-based image stream.
#' 
#' @param filename The name of the file. Follows the same semantics as the file 
#'   naming in [grDevices::png()], meaning that you can provide a [sprintf()] 
#'   compliant string format to name multiple plots (such as the default value)
#' @param width,height The dimensions of the device
#' @param units The unit `width` and `height` is measured in, in either pixels 
#'   (`'px'`), inches (`'in'`), millimeters (`'mm'`), or centimeter (`'cm'`).
#' @param pointsize The default pointsize of the device in pt
#' @param background The background colour of the device
#' @param res The resolution of the device. This setting will govern how device
#'   dimensions given in inches, centimeters, or millimeters will be converted
#'   to pixels. Further, it will be used to scale text sizes and linewidths
#' @param bg Same as `background` for compatibility with old graphic device APIs
#'
#' @export
#' 
#' @examples 
#' file <- tempfile(fileext = '.ppm')
#' agg_ppm(file)
#' plot(sin, -pi, 2*pi)
#' dev.off()
#' 
agg_ppm <- function(filename = 'Rplot%03d.ppm', width = 480, height = 480, 
                    units = 'px', pointsize = 12, background = 'white', 
                    res = 72, bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  file <- validate_path(filename)
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  .Call("agg_ppm_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), PACKAGE = 'ragg')
  invisible()
}

#' Draw to a PNG file
#' 
#' The PNG (Portable Network Graphic) format is one of the most ubiquitous 
#' today, due to its versatiliity 
#' and widespread support. It supports transparency as well as both 8 and 16 bit
#' colour. The device uses default compression and filtering and will not use a
#' colour palette as this is less useful for antialiased data. This means that 
#' it might be possible to compress the resulting image even more if size is of
#' concern (though the defaults are often very good). In contrast to 
#' [grDevices::png()] the date and time will not be written to the file, meaning
#' that similar plot code will produce identical files (a good feature if used 
#' with version control). It will, however, write in the dimensions of the image
#' based on the `res` argument.
#' 
#' @inheritParams agg_ppm
#' @param bitsize Should the device record colour as 8 or 16bit 
#' 
#' @export
#' 
#' @examples 
#' file <- tempfile(fileext = '.png')
#' agg_png(file)
#' plot(sin, -pi, 2*pi)
#' dev.off()
#' 
agg_png <- function(filename = 'Rplot%03d.png', width = 480, height = 480, 
                    units = 'px', pointsize = 12, background = 'white', 
                    res = 72, bitsize = 8, bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  file <- validate_path(filename)
  if (!bitsize %in% c(8, 16)) {
    stop('Only 8 and 16 bit is supported', call. = FALSE)
  }
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  .Call("agg_png_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), as.integer(bitsize), PACKAGE = 'ragg')
  invisible()
}
#' Draw to a TIFF file
#' 
#' The TIFF (Tagged Image File Format) format is a very versatile raster image
#' storage format that supports 8 and 16bit colour mode, true transparency, as
#' well as a range of other features not relevant to drawing from R (e.g. 
#' support for different colour spaces). The storage mode of the image data is
#' not fixed and different compression modes are possible, in contrast to PNGs
#' one-approach-fits-all. The default (uncompressed) will result in much larger
#' files than PNG, and in general PNG is a better format for many of the graphic
#' types produced in R. Still, TIFF has its purposes and sometimes this file
#' format is explicetly requested.
#' 
#' @section Transparency:
#' TIFF have support for true transparency, meaning that the pixel colour is 
#' stored in pre-multiplied form. This is in contrast to pixels being stored in 
#' plain format, where the alpha values more function as a mask. The utility of
#' this is not always that important, but it is one of the benefits of TIFF over
#' PNG so it should be noted.
#' 
#' @inheritParams agg_png
#' @param compression The compression type to use for the image data. The 
#' standard options from the [grDevices::tiff()] function are available under 
#' the same name.
#' 
#' @note `'jpeg'` compression is only available if ragg is compiled with a 
#' version of `libtiff` where jpeg support has been turned on.
#' 
#' @export
#' 
#' @examples 
#' file <- tempfile(fileext = '.tiff')
#' # Use jpeg compression
#' agg_tiff(file, compression = 'lzw+p')
#' plot(sin, -pi, 2*pi)
#' dev.off()
#' 
agg_tiff <- function(filename = 'Rplot%03d.tiff', width = 480, height = 480, 
                    units = 'px', pointsize = 12, background = 'white', 
                    res = 72, compression = 'none', bitsize = 8, bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  file <- validate_path(filename)
  encoding <- switch(compression, 'lzw+p' = , 'zip+p' = 1L, 0L)
  compression <- switch(
    compression,
    'none' = 0L,
    'rle' = 2L,
    'lzw+p' = ,
    'lzw' = 5L,
    'jpeg' = 7L,
    'zip+p' = ,
    'zip' = 8L
  )
  if (!bitsize %in% c(8, 16)) {
    stop('Only 8 and 16 bit is supported', call. = FALSE)
  }
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  .Call("agg_tiff_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), as.integer(bitsize), compression, encoding, 
        PACKAGE = 'ragg')
  invisible()
}
#' Draw to a JPEG file
#' 
#' The JPEG file format is a lossy compressed file format developed in 
#' particular for digital photography. The format is not particularly 
#' well-suited for line drawings and text of the type normally associated with 
#' statistical plots as the compression algorithm creates noticable artefacts. 
#' It is, however, great for saving image data, e.g. heightmaps etc. Thus, for
#' standard plots, it would be better to use [agg_png()], but for plots that
#' includes a high degree of raster image rendering this device will result in
#' smaller plots with very little quality degradation.
#' 
#' @inheritParams agg_png
#' @param quality An integer between `0` and `100` defining the quality/size 
#' tradeoff. Setting this to `100` will result in no compression.
#' @param smoothing A smoothing factor to apply before compression, from `0` (no
#' smoothing) to `100` (full smoothing). Can also by `FALSE` (no smoothing) or 
#' `TRUE` (full smoothing).
#' @param method The compression algorithm to use. Either `'slow'`, `'fast'`, or
#' `'float'`. Default is `'slow'` which works best for most cases. `'fast'` 
#' should only be used when quality is below `97` as it may result in worse 
#' performance at high quality settings. `'float'` is a legacy options that 
#' calculate the compression using floating point precission instead of with 
#' integers. It offers no quality benefit and is often much slower.
#' 
#' @note Smoothing is only applied if ragg has been compiled against a jpeg 
#' library that supports smoothing.
#' 
#' @export
#' 
#' @examples 
#' file <- tempfile(fileext = '.jpeg')
#' agg_jpeg(file, quality = 50)
#' plot(sin, -pi, 2*pi)
#' dev.off()
#' 
agg_jpeg <- function(filename = 'Rplot%03d.jpeg', width = 480, height = 480, 
                     units = 'px', pointsize = 12, background = 'white', 
                     res = 72, quality = 75, smoothing = FALSE, method = 'slow', 
                     bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  file <- validate_path(filename)
  quality <- min(100, max(0, quality))
  if (is.logical(smoothing)) smoothing <- if (smoothing) 100 else 0
  smoothing <- min(100, max(0, smoothing))
  method <- match.arg(tolower(method), c('slow', 'fast', 'float'))
  method <- match(method, c('slow', 'fast', 'float')) - 1L
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  .Call("agg_jpeg_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), as.integer(quality), as.integer(smoothing), method, 
        PACKAGE = 'ragg')
  invisible()
}
#' Draw to a PNG file, modifying transparency on the fly
#' 
#' The graphic engine in R only supports 8bit colours. This is for the most part
#' fine, as 8bit gives all the fidelity needed for most graphing needs. However,
#' this may become a limitation if you need to plot thousands of very 
#' translucent shapes on top of each other. 8bit only afford a minimum of 1/255
#' alpha, which may end up accumulating to fully opaque at some point. This 
#' device allows you to create a 16bit device that modifies the alpha level of 
#' all incomming colours by a fixed multiplier, thus allowing for much more 
#' translucent colours. The device will only modify transparent colour, so if
#' you pass in an opaque colour it will be left unchanged.
#' 
#' @inheritParams agg_ppm
#' @param alpha_mod A numeric between 0 and 1 that will be multiplied to the 
#' alpha channel of all transparent colours
#' 
#' @export
#' @keywords internal
#' 
agg_supertransparent <- function(filename = 'Rplot%03d.png', width = 480, 
                                 height = 480, units = 'px', pointsize = 12, 
                                 background = 'white', res = 72, alpha_mod = 1, 
                                 bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  file <- validate_path(filename)
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  .Call("agg_supertransparent_c", file, dim[1], dim[2], as.numeric(pointsize), 
        background, as.numeric(res), as.double(alpha_mod), PACKAGE = 'ragg')
  invisible()
}

#' Draw to a buffer that can be accessed directly
#' 
#' Usually the point of using a graphic device is to create a file or show the 
#' graphic on the screen. A few times we need the image data for further 
#' processing in R, and instead of writing it to a file and then reading it back
#' into R the `agg_capture()` device lets you get the image data directly from 
#' the buffer. In contrast to the other devices this device returns a function,
#' that when called will return the current state of the buffer.
#' 
#' @inheritParams agg_ppm
#' 
#' @return A function that when called returns the current state of the buffer.
#' The return value of the function depends on the `native` argument. If `FALSE`
#' (default) the return value is a `matrix` of colour values and if `TRUE` the 
#' return value is a `nativeRaster` object.
#' 
#' @importFrom grDevices dev.list dev.off dev.cur dev.capture dev.set
#' @export
#' 
#' @examples 
#' cap <- agg_capture()
#' plot(1:10, 1:10)
#' 
#' # Get the plot as a matrix
#' raster <- cap()
#' 
#' # Get the plot as a nativeRaster
#' raster_n <- cap(native = TRUE)
#' 
#' dev.off()
#' 
#' # Look at the output
#' plot(as.raster(raster))
#' 
agg_capture <- function(width = 480, height = 480, units = 'px', pointsize = 12, 
                        background = 'white', res = 72, bg) {
  if (deparse(sys.call(), nlines = 1, width.cutoff = 500) == 'dev(filename = filename, width = dim[1], height = dim[2], ...)') {
    units <- 'in'
  }
  dim <- get_dims(width, height, units, res)
  background <- if (missing(bg)) background else bg
  name <- paste0('agg_capture_', sample(.Machine$integer.max, 1))
  .Call("agg_capture_c", name, dim[1], dim[2], as.numeric(pointsize), 
        background, as.numeric(res), PACKAGE = 'ragg')
  cap <- function(native = FALSE) {
    current_dev = dev.cur()
    if (names(current_dev)[1] == name) {
      return(dev.capture(native = native))
    }
    all_dev <- dev.list()
    if (!name %in% names(all_dev)) {
      stop('The device (', name, ') is no longer open', call. = FALSE)
    }
    dev.set(all_dev[name])
    on.exit(dev.set(current_dev))
    dev.capture(native = native)
  }
  invisible(cap)
}
