#' @export
#' 
agg_ppm <- function(file, width, height, pointsize, bg) {
  .Call("agg_ppm_c", file, as.integer(width), as.integer(height), 
        as.numeric(pointsize), bg, PACKAGE = 'ragg')
  invisible()
}
