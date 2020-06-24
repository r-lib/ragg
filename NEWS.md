# ragg (development version)

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
