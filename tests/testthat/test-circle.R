render_circle <- function(fill, colour, linewidth, linetype) {
  dev <- agg_capture()
  grid::grid.circle(
    r = 0.4,
    gp = grid::gpar(fill = fill, col = colour, lwd = linewidth, lty = linetype)
  )
  out <- dev()
  dev.off()
  out
}

test_that("circle stroke works", {
  circ <- table(render_circle(NA, 'black', 2, 'solid'))
  expect_equal(circ[['white']], 227079)
  expect_equal(circ[['black']], 402)

  circ <- table(render_circle(NA, 'black', 2, 'dotdash'))
  expect_equal(circ[['black']], 186)

  circ <- table(render_circle(NA, 'blue', 2, 'solid'))
  expect_equal(circ[['blue']], 402)

  circ <- table(render_circle(NA, '#DE2D7633', 2, 'solid'))
  expect_equal(circ[['#F8D5E4']], 472)
})

test_that("circle fill works", {
  circ <- table(render_circle('black', NA, 2, 'solid'))
  expect_equal(circ[['white']], 113941)
  expect_equal(circ[['black']], 115014)

  circ <- table(render_circle('blue', NA, 2, 'solid'))
  expect_equal(circ[['blue']], 115014)

  circ <- table(render_circle('#DE2D7633', NA, 2, 'solid'))
  expect_equal(circ[['#F8D5E4']], 115044)
})
