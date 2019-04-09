get_dims <- function(width, height, units, res) {
  dims <- c(width, height) * switch(
    units,
    'in' = res,
    'cm' = res / 2.54,
    'mm' = res / 25.4,
    'px' = 1,
    stop('Unknown unit. Please use either px, in, cm, or, mm', call. = FALSE)
  )
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
