#' Draw to a PPM file
#' 
#' The PPM format defines one of the simplest storage formats available for 
#' image data. It is basically a raw 8bit RGB stream with a few bytes of 
#' information in the start. It goes without saying, that this file format is
#' horribly inefficient and should only be used if you want to play around with
#' a simple file format, or need a file-based image stream.
#' 
#' @param file The name of the file. Follows the same semantics as the file 
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
#'
#' @export
#' 
agg_ppm <- function(file = 'Rplot%03d.ppm', width = 480, height = 480, 
                    units = 'px', pointsize = 12, background = 'white', 
                    res = 72) {
  dim <- get_dims(width, height, units, res)
  .Call("agg_ppm_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), PACKAGE = 'ragg')
  invisible()
}

#' Draw to a PNG file
#' 
#' The PNG format is one of the most ubiquitous today, due to its versatiliity 
#' and widespread support. It supports transparancy as well as both 8 and 16 bit
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
agg_png <- function(file = 'Rplot%03d.png', width = 480, height = 480, 
                    units = 'px', pointsize = 12, background = 'white', 
                    res = 72, bitsize = 8) {
  if (!bitsize %in% c(8, 16)) {
    stop('Only 8 and 16 bit is supported', call. = FALSE)
  }
  dim <- get_dims(width, height, units, res)
  .Call("agg_png_c", file, dim[1], dim[2], as.numeric(pointsize), background, 
        as.numeric(res), as.integer(bitsize), PACKAGE = 'ragg')
  invisible()
}
