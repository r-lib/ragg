debugging <- FALSE

make_plot <- function() {
  set.seed(42)
  pal <- c("lightpink", "lightblue", "lightgreen", "lightyellow")
  par(mar = c(1, 0, 0, 0))
  plot.new()
  plot.window(0:1, 0:1, asp = 1)
  for (i in 1:50) {
    polygon(runif(3), runif(3), col = sample(pal, 1), border = "black")
  }
  mtext("Triangular Puzzle", side = 1, line = 0, cex = 0.8)
}

is_webp_file <- function(path) {
  if (!file.exists(path) || file.info(path)$size < 30) return(FALSE)
  con <- file(path, "rb"); on.exit(close(con))
  h <- readBin(con, "raw", 12)
  length(h) == 12 &&
    all(h[1:4] == charToRaw("RIFF")) && all(h[9:12] == charToRaw("WEBP"))
}

test_that("agg_webp generates lossless file", {
  file <- tempfile(pattern = "lossless", fileext = ".webp")
  agg_webp(file, width = 1024, height = 768)
  make_plot()
  dev.off()

  expect_true(is_webp_file(file))
  if (debugging) cat(sprintf("WebP at: %s\n", file)) else unlink(file)
})

test_that("agg_webp generates lossy file", {
  file <- tempfile(pattern = "lossy", fileext = ".webp")
  agg_webp(file, width = 1024, height = 768, lossy = TRUE, quality = 5)
  make_plot()
  dev.off()

  expect_true(is_webp_file(file))
  if (debugging) cat(sprintf("WebP at: %s\n", file)) else unlink(file)
})

test_that("agg_webp supports transparency", {
  file <- tempfile(pattern = "transp", fileext = ".webp")
  agg_webp(file, width = 1024, height = 768, background = "transparent")
  make_plot()
  dev.off()

  expect_true(is_webp_file(file))
  if (debugging) cat(sprintf("WebP at: %s\n", file)) else unlink(file)
})
