# Locate a variable font with a weight axis on the system, or NULL if none
find_weight_variable_font <- function() {
  fonts <- systemfonts::system_fonts()
  fonts <- fonts[fonts$variable, , drop = FALSE]
  for (i in seq_len(nrow(fonts))) {
    info <- tryCatch(
      systemfonts::font_info(path = fonts$path[i], index = fonts$index[i]),
      error = function(e) NULL
    )
    if (is.null(info) || nrow(info) == 0) {
      next
    }
    axes <- info$variation[[1]]
    if ("wght" %in% names(axes)) {
      wght <- axes$wght
      # Need enough range for the two test settings to differ
      if (wght["min"] <= 100 && wght["max"] >= 900) {
        return(list(
          path = fonts$path[i],
          index = fonts$index[i],
          family = fonts$family[i]
        ))
      }
    }
  }
  NULL
}

render_glyphs_native <- function(font, wght, ids, x_offsets) {
  glyph_font <- grDevices::glyphFont(
    font$path,
    font$index,
    font$family,
    400,
    "normal",
    variations = list(wght = wght)
  )
  info <- grDevices::glyphInfo(
    ids,
    x = x_offsets,
    y = 0,
    font = 1,
    size = 30,
    fontList = grDevices::glyphFontList(glyph_font),
    width = 400,
    height = 40,
    hAnchor = grDevices::glyphAnchor(0, "left"),
    vAnchor = grDevices::glyphAnchor(0, "bottom"),
    col = "black"
  )
  dev <- agg_capture(width = 400, height = 80)
  grid::grid.newpage()
  grid::grid.glyph(
    info,
    x = grid::unit(0, "pt"),
    y = grid::unit(10, "pt"),
    hjust = "left",
    vjust = "bottom"
  )
  out <- dev()
  dev.off()
  out
}

# Count non-white pixels in the character colour matrix returned by agg_capture
ink_count <- function(capture) {
  sum(capture != "white")
}

test_that("variable font axes are honoured in glyph rendering", {
  skip_on_cran()
  skip_if(getRversion() < "4.6.0")

  font <- find_weight_variable_font()
  skip_if(is.null(font), "No variable font with a usable weight axis available")

  shaped <- textshaping::shape_text(
    "Hamburgefon",
    path = font$path,
    index = font$index,
    size = 30
  )$shape

  thin <- render_glyphs_native(font, 100, shaped$index, shaped$x_offset)
  black <- render_glyphs_native(font, 900, shaped$index, shaped$x_offset)

  # Both settings actually rendered something
  expect_gt(ink_count(thin), 0)
  expect_gt(ink_count(black), 0)

  # The two weight settings must produce different glyphs
  expect_false(identical(thin, black))

  # ... and repeating the light setting must reproduce the cached light
  # glyphs, not reuse the heavy ones
  thin_again <- render_glyphs_native(font, 100, shaped$index, shaped$x_offset)
  expect_identical(thin, thin_again)
})
