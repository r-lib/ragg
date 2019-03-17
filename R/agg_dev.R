#' @export
#' 
agg_dev <- function(file, width, height, pointsize, bg) {
  .Call("agg_dev_c", file, as.integer(width), as.integer(height), 
        as.numeric(pointsize), bg, PACKAGE = 'ragg')
  invisible()
}
