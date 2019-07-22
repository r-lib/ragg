# ragg (development version)

* Avoid a bug when the call to start a device included too many characters (#16)
* Fix integer overflow runtime errors in agg source code 
  (`agg_scanline_storage_aa.h`), by changing storage to `long`
* Remove benchmarking vignettes as it was causing too much trouble on stripped
  down systems... They are still available on <https://ragg.r-lib.org>

# ragg 0.1.0

* Basic setup of package. png, tiff, ppm, and buffer capture support
* Added a `NEWS.md` file to track changes to the package.
