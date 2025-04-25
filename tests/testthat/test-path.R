render_path <- function(fill, colour, linewidth, linetype, linejoin) {
  dev <- agg_capture()
  grid::grid.path(
    x = c(0.5, 0.9, 0.5, 0.1, 0.4, 0.4, 0.6, 0.6),
    y = c(0.9, 0.5, 0.1, 0.5, 0.4, 0.6, 0.6, 0.4),
    id.lengths = c(4, 4),
    gp = grid::gpar(
      fill = fill,
      col = colour,
      lwd = linewidth,
      lty = linetype,
      linejoin = linejoin
    ),
    rule = 'evenodd'
  )
  out <- dev()
  dev.off()
  out
}

test_that("path stroke works", {
  path <- table(render_path(NA, 'black', 14, 'solid', 'round'))
  expect_equal(path[['white']], 212800)
  expect_equal(path[['black']], 13776)

  path <- table(render_path(NA, 'black', 14, 'dotdash', 'round'))
  expect_equal(path[['black']], 7896)

  path <- table(render_path(NA, 'black', 14, 'solid', 'mitre'))
  expect_equal(path[['black']], 13824)

  path <- table(render_path(NA, 'blue', 14, 'solid', 'round'))
  expect_equal(path[['blue']], 13776)

  path <- table(render_path(NA, '#DE2D7633', 14, 'solid', 'round'))
  expect_equal(path[['#F8D5E4']], 13776)
})

test_that("path fill works", {
  path <- table(render_path('black', NA, 4, 'solid', 'round'))
  expect_equal(path[['white']], 165504)
  expect_equal(path[['black']], 64128)

  path <- table(render_path('blue', NA, 4, 'solid', 'round'))
  expect_equal(path[['blue']], 64128)

  path <- table(render_path('#DE2D7633', NA, 4, 'solid', 'round'))
  expect_equal(path[['#F8D5E4']], 64128)
})
