test_that("agg_tiff 8bit generates file", {
  file <- tempfile(fileext = '.tiff')
  agg_tiff(file)
  plot(1:10, 1:10)
  dev.off()
  
  expect_gt(file.info(file)$size, 0)
  
  unlink(file)
})

test_that("agg_tiff 16bit generates file", {
  file <- tempfile(fileext = '.tiff')
  agg_tiff(file, bitsize = 16)
  plot(1:10, 1:10)
  dev.off()
  
  expect_gt(file.info(file)$size, 0)
  
  unlink(file)
})
