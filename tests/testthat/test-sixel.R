# Encode a matrix of packed RGB integers (0xRRGGBB, one per pixel, rows =
# image rows) to a sixel string via the internal C++ encoder
sixel_encode <- function(mat) {
  height <- nrow(mat)
  width <- ncol(mat)
  px <- as.vector(t(mat))
  buf <- as.raw(rbind(
    bitwAnd(bitwShiftR(px, 16), 0xFF),
    bitwAnd(bitwShiftR(px, 8), 0xFF),
    bitwAnd(px, 0xFF)
  ))
  .Call(
    "sixel_encode_c",
    buf,
    as.integer(width),
    as.integer(height),
    PACKAGE = "ragg"
  )
}

test_that("single-colour image encodes correctly", {
  expect_identical(
    sixel_encode(matrix(0x000000, nrow = 1, ncol = 1)),
    "\x1bP0;1;0q\"1;1;1;1#0;2;0;0;0$#0@\x1b\\"
  )
})

test_that("heights not divisible by 6 produce a partial final band", {
  expect_identical(
    sixel_encode(matrix(0xFFFFFF, nrow = 7, ncol = 2)),
    "\x1bP0;1;0q\"1;1;2;7#0;2;100;100;100$#0~~-$#0@@\x1b\\"
  )
})

test_that("bands are separated for taller images", {
  expect_identical(
    sixel_encode(matrix(0x000000, nrow = 12, ncol = 1)),
    "\x1bP0;1;0q\"1;1;1;12#0;2;0;0;0$#0~-$#0~\x1b\\"
  )
})

test_that("multiple colours are palette-sorted and indexed", {
  mat <- matrix(c(0x000000, 0xFFFFFF), nrow = 1, ncol = 2)
  expect_identical(
    sixel_encode(mat),
    "\x1bP0;1;0q\"1;1;2;1#0;2;100;100;100#1;2;0;0;0$#0?@$#1@\x1b\\"
  )
})

test_that("runs of 4+ identical pixels use RLE notation", {
  expect_identical(
    sixel_encode(matrix(0x000000, nrow = 1, ncol = 10)),
    "\x1bP0;1;0q\"1;1;10;1#0;2;0;0;0$#0!10@\x1b\\"
  )
})

test_that("device opens, draws, and closes without error", {
  # Output goes to fd 1 so it can't be captured from R; this only verifies
  # the device lifecycle (output is escape-sequence noise in the test log)
  agg_sixel(width = 100, height = 100)
  par(mar = rep(0, 4))
  plot.new()
  expect_no_error(dev.off())
})
