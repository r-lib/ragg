test_that("agg_png 8bit generates file", {
  file <- tempfile(fileext = '.png')
  agg_png(file)
  plot(1:10, 1:10)
  dev.off()

  expect_gt(file.info(file)$size, 0)

  unlink(file)
})

test_that("agg_png 16bit generates file", {
  file <- tempfile(fileext = '.png')
  agg_png(file, bitsize = 16)
  plot(1:10, 1:10)
  dev.off()

  expect_gt(file.info(file)$size, 0)

  unlink(file)
})
