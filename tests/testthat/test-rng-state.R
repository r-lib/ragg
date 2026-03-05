test_that("agg devices do not advance global RNG state on creation", {

  set.seed(123)
  dev <- agg_capture()
  handle <- dev()
  dev.off()
  a1 <- runif(1)
  b1 <- .Random.seed

  set.seed(123)
  a2 <- runif(1)
  b2 <- .Random.seed

  expect_equal(a1, a2)
  expect_equal(b1, b2)
})

test_that("agg_record does not advance global RNG state on creation", {
  # with ragg
  set.seed(456)
  agg_record()
  dev.off()
  a1 <- runif(1)
  b1 <- .Random.seed

  set.seed(456)
  a2 <- runif(1)
  b2 <- .Random.seed

  expect_equal(a1, a2)
  expect_equal(b1, b2)
})
