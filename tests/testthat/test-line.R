render_line <- function(colour, linewidth, linetype) {
  dev <- agg_capture()
  grid::grid.segments(
    x0 = 0.1,
    y0 = 0.1,
    x1 = 0.9,
    y1 = 0.9,
    gp = grid::gpar(col = colour, lwd = linewidth, lty = linetype)
  )
  out <- dev()
  dev.off()
  out
}

test_that("line stroke works", {
  line <- table(render_line('black', 14, 'solid'))
  expect_equal(line[['white']], 223760)
  expect_equal(line[['black']], 5060)

  line <- table(render_line('black', 14, 'dotdash'))
  expect_equal(line[['black']], 3062)

  line <- table(render_line('blue', 14, 'solid'))
  expect_equal(line[['blue']], 5060)

  line <- table(render_line('#DE2D7633', 14, 'solid'))
  expect_equal(line[['#F8D5E4']], 5060)
})
