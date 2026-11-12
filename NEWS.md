# ragg (development version)

* ragg is now variable font aware. Axis settings travel through shaping via
  the new `FontSettings2` C API in textshaping and are honoured both in
  regular text rendering and in the `glyph()` device callback (R >= 4.6.0),
  which now reads the variations carried by `grid::glyphFont()`. Variation
  coordinates are part of the AGG font cache key so glyphs are cached per
  instance of the font, and the device now advertises the `variableFonts`
  capability
* Glyph loading now bypasses embedded bitmap strikes in scalable fonts,
  which failed to render at specific font sizes with certain fonts (e.g.
  Calibri and Arial on Windows) and silently produced missing text (#219)
* Fixed undefined behaviour (flagged by ASAN as `memcpy-param-overlap`) when the
  font cache pool evicted a font, by using `memmove` for the overlapping array
  shift in the vendored AGG font cache (#222)
* Fixed a range of memory safety issues uncovered in a review of the C++ code:
  * Pixel buffer sizes are now computed with overflow checks, preventing heap
    corruption when opening devices with extreme dimensions
  * Sub-pixel tiling patterns are now a clean error instead of a crash in the
    tile renderer
  * Failed allocations when creating masks, patterns, and groups can no longer
    result in double frees
  * 16-bit TIFF output no longer passes a malformed ExtraSamples tag to libtiff
    which could crash on transparent backgrounds
* R errors or interrupts raised while recording clipping paths, masks,
  patterns, or groups no longer leak memory or leave the device in a broken
  recording state where subsequent drawing silently disappears
* Removed clip paths and masks are no longer referenced by the device,
  avoiding potential use-after-free
* Fixed a segfault that could occur when a font failed to load after another
  font had been used, due to a stale font cache
* Device ids are now unique across device types, fixing a potential font mixup
  when different device types were open simultaneously
* 16-bit devices now render masks with the correct colours (previously mask
  colours were misinterpreted, e.g. a solid black mask rendered almost white)
* libjpeg errors (e.g. write failures) no longer terminate the R session
* libpng write failures no longer leak file handles
* TIFF writing no longer leaks a file handle per saved page and is
  unicode-aware on Windows for both 8- and 16-bit output
* The `smoothing` and `method` arguments of `agg_jpeg()` are now respected
  (they were previously reset by libjpeg defaults)
* WebP save failures with `options(warn = 2)` no longer skip resource cleanup
* `drawGlyph()` now translates all glyph positions correctly inside patterns
  and no longer mutates R's coordinate arrays

# ragg 1.5.2

* Fixed a sanitizer issue from not correctly closing down the recording device

# ragg 1.5.1

* Fix support for the default symbol font on many linux distros
  (StandardSymbolPS) by ensuring proper fallback during character metric
  calculation (#136, #201)
* `agg_capture()` now reports the current "page number" as well as whether any
  drawing has occured since the last time the capture was taken as attributes
  in the return value (but only if you request native raster) (#204)
* Avoid compile time warnings from implicit casting during enum arithmetic
* `agg_capture()` and `agg_record()` no longer advance the RNG (#212)

# ragg 1.5.0

* Fix gradient rendering bug introduced by trying to fix #177 (#192)
* Add support for WebP output, both static and animated (#190, @klausbrunner)

# ragg 1.4.0

* Added `agg_record()` for optimised plot recording
* Added support for hold and flushing, mainly in service of Positron
* Added support for `saturate` composition operator
* Sanitise file name before plotting (#176)
* Tiff files can now include unicode characters on all platforms (#87)

# ragg 1.3.3

* Fixed a regression from upstream changes in Freetype (#172)

# ragg 1.3.2

* Fix a bug in how groups are defined and recalled

# ragg 1.3.1

* Make sure the linejoin/linemitre setting is honored when drawing rectangles
  (#162)
* Fix a bug with resetting clipping during group and pattern rendering
* Fix scaling of raster glyphs in drawGlyph

# ragg 1.3.0

* Added supported for new graphics enigine features: Groups, paths, luminance
  masks, and glyphs
* Add a switch (defaults to on) to snapping rectangles to the pixel grid when
  they are only drawn with fill (no stroke)
* Fixed a bug causing repeated warnings when using a font at 0 size (#130)
* Silence a bug in clang-ASAN that incorrectly reported sanitiser issues

# ragg 1.2.7

* Fix a stack imbalance bug

# ragg 1.2.6

* Fix symbol rendering bug on windows (#132)
* Add support for `dev.capabilities()` (#105)
* Prepare for Arm Windows

# ragg 1.2.5

* Fix a bug when rendering glyphs from a colour font that also provide greyscale
  glyphs (#105)
* Move `sprintf()` to `snprintf()` in the AGG source code to comply with Arm64
  deprecation
* Better guard against bad input (#116)

# ragg 1.2.4

* Fixed a regression that turned off line mitre support (#119)

# ragg 1.2.3

* Second attempt at rendering jpegs with transparent background. Now, the buffer
  is filled with solid white before any drawing happens.
* Fixed a bug that resulted in newlines being rendered as missing glyphs on some
  Linux versions (#111)

# ragg 1.2.2

* MacOS: configure script now uses the local system dependencies provided by
  CRAN via pkg-config. Autobrew libs are used as fallback on non-cran servers only.

# ragg 1.2.1

* Fix bug that caused R to crash when writing tiff files with transparent
  background (#97)

# ragg 1.2.0

* Add support for new graphic engine features:
  - Arbitrary clipping paths
  - Alpha masks
  - Linear and radial gradients
  - Tiling patterns
* Use white as background when passing in a fully transparent background colour
  to devices that doesn't support alpha (notably jpeg) (#91)
* ragg now defers symbol font resolving to systemfonts which makes it possible
  to register alternative symbol fonts using `register_font()` (#90)
* Filenames in UTF-8 are now treated correctly on Windows (#87)
* Fix size selection of non-scalable fonts when the requested size is bigger
  than the available

# ragg 1.1.3

* Use int32_t instead of int32 in old code
* Prepare for UCRT
* Better error message when failing to allocate memory for the buffer (#82)
* Increase storage size limits for paths (#80)

# ragg 1.1.2

* Fix bug in `agg_capture()` that resulted in premultiplied colour values being
  returned

# ragg 1.1.1

* Fix a bug in glyph dimension lookup that could cause system crashes
* Fix bug in font caching when multiple ragg devices are used simultaneously

# ragg 1.1.0

* Major version release to signify the much improved text support that includes
  full support for right-to-left scripts and bidirectional text (mix of RtL and
  LtR scripts). It further adds full support for OpenType features and
  non-scalable fonts.
* Re-exporting `register_font()`, `register_variant()`, and `font_feature()`
  from systemfonts
* Re-exporting `get_font_features()` from textshaping
* Use new textshaping API and handle font fallback correctly
* Add support for rendering colour fonts (#1)

# ragg 0.4.1

* Skip text tests on CRAN as no text is plottet on the CRAN solaris machine
* Fixed a bug resulting in system crash on certain systems, as well as
  clang-ASAN error. (#59)

# ragg 0.4.0

* ragg now requires the Harfbuzz and Fribidi libraries to be available when
  installing from source due to their dependency in the textshaping package.
* Move text shaping to the new textshaping package.
* Fix `agg_capture()` on big endian systems (#49, @QuLogic)
* Fix use of symbol font on Windows by moving to Segoe UI Symbol which has a
  Unicode charmap (#51)
* Better compatibility with knitr and `ggplot2::ggsave()`

# ragg 0.3.1

* Roll back support for new clipping options in the graphic engine as it was
  buggy.

# ragg 0.3.0

* Fix a bug when plotting partially transparent raster (#44)
* Add a `scaling` argument to all devices allowing you to change relative
  scaling of output.
* Horizontal and vertical text are now snapped to the pixel grid in order to
  improve rendering quality.
* Internal changes to prepare for coming updates to the graphic engine

# ragg 0.2.0

* Fix compilation on R <= 3.3 by including Rdynload.h explicitly
* Fix a performance regression when plotting text (#33)
* Fix erroneous width calculations of strings starting with a space on windows
  (#32)
* Fix a bug in `agg_capture()` where the output became mangled if device
  height != width
* Fix a bug in raster support where raster data did not get premultiplied before
  rendering (#38, @yixuan)
* Fix an integer overflow issue in the AGG source code

# ragg 0.1.5

* Fix compilation on macOS

# ragg 0.1.4

* Fix a bug in AGG's font manager that ignored the font index when it stored and
  retrieved cached faces

# ragg 0.1.3

* Fix bug preventing ragg from displaying 50% transparent black
* Another attempt at fixing compilation on mac build machines

# ragg 0.1.2

* Fix compilation on certain Linux systems by preferring dynamic libraries over
  static ones (#25, @jimhester).

# ragg 0.1.1

* Avoid a bug when the call to start a device included too many characters (#16)
* Fix integer overflow runtime errors in agg source code
  (`agg_scanline_storage_aa.h`), by changing storage to `long`
* Remove benchmarking vignettes as it was causing too much trouble on stripped
  down systems... They are still available on <https://ragg.r-lib.org>
* Better build setup to properly build on all macOS systems

# ragg 0.1.0

* Basic setup of package. png, tiff, ppm, and buffer capture support
* Added a `NEWS.md` file to track changes to the package.
