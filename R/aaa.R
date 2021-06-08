get_dims <- function(width, height, units, res) {
  dims <- c(width, height) * switch(
    units,
    'in' = res,
    'cm' = res / 2.54,
    'mm' = res / 25.4,
    'px' = 1,
    stop('Unknown unit. Please use either px, in, cm, or, mm', call. = FALSE)
  )
  max_dim <- getOption('ragg.max_dim', 5e4)
  if (any(dims > max_dim)) {
    stop(
      'One or both dimensions exceed the maximum (', max_dim, 'px).\n', 
      '- Use `options(ragg.max_dim = ...)` to change the max\n',
      '  Warning: May cause the R session to crash',
      call. = FALSE
    )
  }
  as.integer(dims)
}

validate_path <- function(path) {
  dir <- dirname(path)
  if (!dir.exists(dir)) {
    dir.create(dir, recursive = TRUE)
  }
  dir <- normalizePath(dir)
  file.path(dir, basename(path))
}

#' @importFrom systemfonts register_font
#' @export
systemfonts::register_font

#' @importFrom systemfonts register_variant
#' @export
systemfonts::register_variant

#' @importFrom systemfonts font_feature
#' @export
systemfonts::font_feature

#' @importFrom textshaping get_font_features
#' @export
textshaping::get_font_features
