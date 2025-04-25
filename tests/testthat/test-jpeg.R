test_that("agg_jpeg generates file", {
  file <- tempfile(fileext = '.jpeg')
  agg_jpeg(file)
  plot(1:10, 1:10)
  dev.off()

  expect_gt(file.info(file)$size, 0)

  unlink(file)
})
