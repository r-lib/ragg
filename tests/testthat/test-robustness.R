# Regression tests for crashes and state corruption found in a C++ code
# review. These exercise device behaviour when R callbacks fail or receive
# degenerate geometry.

error_grob <- function() {
  registerS3method(
    "makeContent",
    "ragg_error_grob",
    function(x) stop("boom"),
    envir = asNamespace("grid")
  )
  grid::grob(cl = "ragg_error_grob")
}

# Centre pixel of the current agg_capture device as an rgb vector
capture_centre <- function() {
  raster <- dev.capture(native = FALSE)
  as.vector(grDevices::col2rgb(raster[nrow(raster) / 2, ncol(raster) / 2])[, 1])
}

test_that("error inside a mask callback does not corrupt the device", {
  skip_if_not_installed("grid")
  library(grid)

  agg_capture(width = 100, height = 100, units = "px")

  expect_error(
    grid.rect(
      gp = gpar(fill = "red"),
      vp = viewport(mask = error_grob())
    ),
    "boom"
  )

  # The device must return to normal rendering afterwards
  grid.newpage()
  grid.rect(gp = gpar(fill = "blue"))
  centre <- capture_centre()
  # grid leaves the device lock set when a callback errors (also happens on
  # the pdf device, so it is an R-side issue, not ragg)
  suppressWarnings(dev.off())

  expect_equal(centre, c(0, 0, 255))
})

test_that("error inside group and tiling pattern callbacks does not corrupt the device", {
  skip_if_not_installed("grid")
  library(grid)

  agg_capture(width = 100, height = 100, units = "px")

  expect_error(grid.group(error_grob()), "boom")
  grid.newpage()
  grid.rect(gp = gpar(fill = "blue"))

  bad_tile <- pattern(
    error_grob(),
    width = unit(5, "mm"),
    height = unit(5, "mm"),
    extend = "repeat"
  )
  expect_error(grid.rect(gp = gpar(fill = bad_tile)), "boom")
  grid.newpage()
  grid.rect(gp = gpar(fill = "blue"))
  centre <- capture_centre()
  # grid leaves the device lock set when a callback errors (also happens on
  # the pdf device, so it is an R-side issue, not ragg)
  suppressWarnings(dev.off())

  expect_equal(centre, c(0, 0, 255))
})

test_that("degenerate tiling pattern is a clean error, not a crash", {
  skip_if_not_installed("grid")
  library(grid)

  agg_capture(width = 100, height = 100, units = "px")

  # A tile smaller than a device pixel used to segfault in the tile renderer
  tiny_tile <- pattern(
    rectGrob(gp = gpar(fill = "red")),
    width = unit(0.01, "pt"),
    height = unit(0.01, "pt"),
    extend = "repeat"
  )
  expect_error(
    grid.rect(gp = gpar(fill = tiny_tile)),
    "Invalid tile dimensions"
  )

  # The device must still work afterwards
  grid.newpage()
  grid.rect(gp = gpar(fill = "blue"))
  centre <- capture_centre()
  # grid leaves the device lock set when a callback errors (also happens on
  # the pdf device, so it is an R-side issue, not ragg)
  suppressWarnings(dev.off())

  expect_equal(centre, c(0, 0, 255))
})
