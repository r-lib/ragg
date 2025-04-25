check_numeric_scalar <- function(x, name) {
  if (length(x) != 1 || !is.numeric(x) || !is.finite(x)) {
    stop(paste0(name, " must be a numeric scalar"))
  }
}

get_dims <- function(width, height, units, res) {
  check_numeric_scalar(width, "width")
  check_numeric_scalar(height, "height")
  check_numeric_scalar(res, "res")
  dims <- c(width, height) *
    switch(
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
      'One or both dimensions exceed the maximum (',
      max_dim,
      'px).\n',
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
  path <- file.path(dir, basename(path))
  if (grepl("(?<!%)%(?!%)(?![#0 ,+-]*[0-9.]*[diouxX])", path, perl = TRUE)) {
    stop(
      "Invalid path. Only use C-style formatting intended for numbers (%d, %i, %o, %u, %x, %X)"
    )
  }
  path
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
