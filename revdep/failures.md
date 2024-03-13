# ursa

<details>

* Version: 3.10.1
* GitHub: https://github.com/nplatonov/ursa
* Source code: https://github.com/cran/ursa
* Date/Publication: 2023-10-07 18:20:02 UTC
* Number of recursive dependencies: 122

Run `cloud_details(, "ursa")` for more info

</details>

## In both

*   checking whether package ‘ursa’ can be installed ... ERROR
    ```
    Installation failed.
    See ‘/tmp/workdir/ursa/new/ursa.Rcheck/00install.out’ for details.
    ```

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘gdalraster’
    ```

## Installation

### Devel

```
* installing *source* package ‘ursa’ ...
** package ‘ursa’ successfully unpacked and MD5 sums checked
** using staged installation
** libs
gcc -I"/opt/R/4.1.1/lib/R/include" -DNDEBUG   -I/usr/local/include   -fpic  -g -O2  -c init.c -o init.o
gcc -I"/opt/R/4.1.1/lib/R/include" -DNDEBUG   -I/usr/local/include   -fpic  -g -O2  -c ursa.c -o ursa.o
gcc -shared -L/opt/R/4.1.1/lib/R/lib -L/usr/local/lib -o ursa.so init.o ursa.o -L/opt/R/4.1.1/lib/R/lib -lR
installing to /tmp/workdir/ursa/new/ursa.Rcheck/00LOCK-ursa/00new/ursa/libs
** R
Error in parse(outFile) : 
  /tmp/workdir/ursa/new/ursa.Rcheck/00_pkg_src/ursa/R/package_vapour.R:125:94: unexpected input
124:       else
125:          ursa_value(a) <- vapour::gdal_raster_data(fname,bands=seq(a)) |> do.call(cbind,args=_
                                                                                                  ^
ERROR: unable to collate and parse R files for package ‘ursa’
* removing ‘/tmp/workdir/ursa/new/ursa.Rcheck/ursa’


```
### CRAN

```
* installing *source* package ‘ursa’ ...
** package ‘ursa’ successfully unpacked and MD5 sums checked
** using staged installation
** libs
gcc -I"/opt/R/4.1.1/lib/R/include" -DNDEBUG   -I/usr/local/include   -fpic  -g -O2  -c init.c -o init.o
gcc -I"/opt/R/4.1.1/lib/R/include" -DNDEBUG   -I/usr/local/include   -fpic  -g -O2  -c ursa.c -o ursa.o
gcc -shared -L/opt/R/4.1.1/lib/R/lib -L/usr/local/lib -o ursa.so init.o ursa.o -L/opt/R/4.1.1/lib/R/lib -lR
installing to /tmp/workdir/ursa/old/ursa.Rcheck/00LOCK-ursa/00new/ursa/libs
** R
Error in parse(outFile) : 
  /tmp/workdir/ursa/old/ursa.Rcheck/00_pkg_src/ursa/R/package_vapour.R:125:94: unexpected input
124:       else
125:          ursa_value(a) <- vapour::gdal_raster_data(fname,bands=seq(a)) |> do.call(cbind,args=_
                                                                                                  ^
ERROR: unable to collate and parse R files for package ‘ursa’
* removing ‘/tmp/workdir/ursa/old/ursa.Rcheck/ursa’


```
