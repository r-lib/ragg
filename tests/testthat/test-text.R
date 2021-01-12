render_text <- function() {
  dev <- agg_capture()
  grid::grid.text(label = 'this is a test')
  out <- dev()
  dev.off()
  out
}

# We cannot test exact across systems because fonts may differ. We simply test
# if something is drawn
test_that("text works", {
  skip_on_cran() # Solaris don't have any text support on CRAN
  text <- table(render_text())
  expect_gt(length(text), 1) # Not only white
})
