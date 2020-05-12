render_polygon <- function(fill, colour, linewidth, linetype, linejoin) {
  dev <- agg_capture()
  grid::grid.polygon(
    x = c(0.5, 0.9, 0.5, 0.1), y = c(0.9, 0.5, 0.1, 0.5),
    gp = grid::gpar(fill = fill, col = colour, lwd = linewidth, 
                    lty = linetype, linejoin = linejoin)
  )
  out <- dev()
  dev.off()
  out
}

is_4plus <- packageVersion('base') > "4.0.0"

test_that("polygon stroke works", {
  polygon <- table(render_polygon(NA, 'black', 14, 'solid', 'round'))
  expect_equal(polygon[['white']], 217376)
  expect_equal(polygon[['black']], if (is_4plus) 9968 else 9976)
  
  polygon <- table(render_polygon(NA, 'black', 14, 'dotdash', 'round'))
  expect_equal(polygon[['black']], 5762)
  
  polygon <- table(render_polygon(NA, 'black', 14, 'solid', 'mitre'))
  expect_equal(polygon[['black']], if (is_4plus) 9984 else  9988)
  
  polygon <- table(render_polygon(NA, 'blue', 14, 'solid', 'round'))
  expect_equal(polygon[['blue']], if (is_4plus) 9968 else 9976)
  
  polygon <- table(render_polygon(NA, '#DE2D7633', 14, 'solid', 'round'))
  expect_equal(polygon[['#F8D5E4']], if (is_4plus) 9968 else 9976)
})

test_that("polygon fill works", {
  polygon <- table(render_polygon('black', NA, 4, 'solid', 'round'))
  expect_equal(polygon[['white']], 156288)
  expect_equal(polygon[['black']], 73344)
  
  
  polygon <- table(render_polygon('blue', NA, 4, 'solid', 'round'))
  expect_equal(polygon[['blue']], 73344)
  
  polygon <- table(render_polygon('#DE2D7633', NA, 4, 'solid', 'round'))
  expect_equal(polygon[['#F8D5E4']], 73344)
})
