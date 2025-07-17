debugging <- FALSE

test_that("agg_webp_anim produces animated WebP output", {
  tmp <- tempfile(fileext = ".webp")
  nframes <- 8
  agg_webp_anim(tmp, width = 400, height = 300, delay = 500, loop = 0)

  colors <- rainbow(nframes)
  for (i in seq_len(nframes)) {
    plot.new()

    # Background with frame-specific pattern
    rect(0, 0, 1, 1, col = paste0(colors[i], "20"), border = NA)
    # Large frame number
    text(0.5, 0.7, paste("FRAME", i), cex = 3, col = colors[i], font = 2)
    # Moving circle to show progression
    x <- i / (nframes + 1)  # Position from left to right
    points(x, 0.3, pch = 19, cex = 4, col = colors[i])
    # Progress bar
    rect(0.1, 0.1, 0.1 + 0.8 * (i / nframes), 0.15, col = colors[i], border = "black")
    text(0.5, 0.05, paste(i, "of", nframes), cex = 1.5)

    dev.flush()
  }
  dev.off()

  expect_true(file.exists(tmp))
  expect_gt(file.info(tmp)$size, 1000)
  if (debugging) cat("Animated WebP created at:", tmp, "\n") else unlink(tmp)
})