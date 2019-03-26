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
