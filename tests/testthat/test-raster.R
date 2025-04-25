render_raster <- function(raster, interpolate, angle) {
  dev <- agg_capture()
  plot.new()
  graphics::rasterImage(
    raster,
    0.25,
    0.25,
    0.75,
    0.75,
    angle = angle,
    interpolate = interpolate
  )
  out <- dev()
  dev.off()
  out
}

test_that("raster works", {
  image <- matrix(grDevices::hcl(0, 80, seq(50, 80, 10)), nrow = 4, ncol = 5)
  raster <- table(render_raster(image, FALSE, 0))
  expect_equal(raster[['#E16A86']], 7380)
  expect_equal(raster[['#FFA2BC']], 7200)

  raster <- table(render_raster(image, TRUE, 0))
  expect_equal(raster[['#E16A86']], 180)
  expect_equal(raster[['#FFA2BC']], 3780)

  raster <- table(render_raster(image, FALSE, 30))
  expect_equal(raster[['#E16A86']], 7227)
  expect_equal(raster[['#FFA2BC']], 7127)

  raster <- table(render_raster(image, TRUE, 30))
  expect_equal(raster[['#E16A86']], 255)
  expect_equal(raster[['#FFA2BC']], 3628)
})
