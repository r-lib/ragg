test_that("agg_ppm generates file", {
  file <- tempfile(fileext = '.ppm')
  agg_ppm(file)
  plot(1:10, 1:10)
  dev.off()

  expect_gt(file.info(file)$size, 0)

  unlink(file)
})
