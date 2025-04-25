render_rect <- function(fill, colour, linewidth, linetype) {
  dev <- agg_capture()
  grid::grid.rect(
    width = 0.8,
    height = 0.8,
    gp = grid::gpar(fill = fill, col = colour, lwd = linewidth, lty = linetype)
  )
  out <- dev()
  dev.off()
  out
}

test_that("rect stroke works", {
  rect <- table(render_rect(NA, 'black', 4, 'solid'))
  expect_equal(rect[['white']], 224256)
  expect_equal(rect[['black']], 3072)

  rect <- table(render_rect(NA, 'black', 4, 'dotdash'))
  expect_equal(rect[['black']], 1771)

  rect <- table(render_rect(NA, 'blue', 4, 'solid'))
  expect_equal(rect[['blue']], 3072)

  rect <- table(render_rect(NA, '#DE2D7633', 4, 'solid'))
  expect_equal(rect[['#F8D5E4']], 3072)
})

test_that("rect fill works", {
  rect <- table(render_rect('black', NA, 4, 'solid'))
  expect_equal(rect[['white']], 82944)
  expect_equal(rect[['black']], 147456)

  rect <- table(render_rect('blue', NA, 4, 'solid'))
  expect_equal(rect[['blue']], 147456)

  rect <- table(render_rect('#DE2D7633', NA, 4, 'solid'))
  expect_equal(rect[['#F8D5E4']], 147456)
})
