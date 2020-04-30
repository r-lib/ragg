render_polyline <- function(colour, linewidth, linetype, lineend, linejoin) {
  dev <- agg_capture()
  grid::grid.lines(x = c(0.1, 0.4, 0.8, 0.5, 0.9), y = c(0.9, 0.7, 0.1, 0.3, 0.8), 
                   gp = grid::gpar(col = colour, lwd = linewidth, 
                                   lty = linetype, linewidth = linewidth, 
                                   lineend = lineend))
  out <- dev()
  dev.off()
  out
}

test_that("polyline stroke works", {
  polyline <- table(render_polyline('black', 14, 'solid', 'round', 'round'))
  expect_equal(polyline[['white']], 218850)
  expect_equal(polyline[['black']], 9298)
  
  polyline <- table(render_polyline('black', 14, 'dotdash', 'round', 'round'))
  expect_equal(polyline[['black']], 5497)
  
  polyline <- table(render_polyline('black', 14, 'solid', 'butt', 'round'))
  expect_equal(polyline[['black']], 9219)
  
  polyline <- table(render_polyline('black', 14, 'solid', 'round', 'bevel'))
  expect_equal(polyline[['black']], 9298)
  
  polyline <- table(render_polyline('blue', 14, 'solid', 'round', 'round'))
  expect_equal(polyline[['blue']], 9298)
  
  polyline <- table(render_polyline('#DE2D7633', 14, 'solid', 'round', 'round'))
  expect_equal(polyline[['#F8D5E4']], 9298)
})
