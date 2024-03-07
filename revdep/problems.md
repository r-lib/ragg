# WhatsR

<details>

* Version: 1.0.1
* GitHub: NA
* Source code: https://github.com/cran/WhatsR
* Date/Publication: 2023-06-01 22:50:14 UTC
* Number of recursive dependencies: 140

Run `cloud_details(, "WhatsR")` for more info

</details>

## Newly broken

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Complete output:
      > library(testthat)
      > library(WhatsR)
      The legacy packages maptools, rgdal, and rgeos, underpinning the sp package,
      which was just loaded, were retired in October 2023.
      Please refer to R-spatial evolution reports for details, especially
      https://r-spatial.org/r/2023/05/15/evolution4.html.
      It may be desirable to make the sf package available;
    ...
      29: tryCatchOne(expr, names, parentenv, handlers[[1L]])
      30: tryCatchList(expr, classes, parentenv, handlers)
      31: tryCatch(code, testthat_abort_reporter = function(cnd) {    cat(conditionMessage(cnd), "\n")    NULL})
      32: with_reporter(reporters$multi, lapply(test_paths, test_one_file,     env = env, desc = desc, error_call = error_call))
      33: test_files_serial(test_dir = test_dir, test_package = test_package,     test_paths = test_paths, load_helpers = load_helpers, reporter = reporter,     env = env, stop_on_failure = stop_on_failure, stop_on_warning = stop_on_warning,     desc = desc, load_package = load_package, error_call = error_call)
      34: test_files(test_dir = path, test_paths = test_paths, test_package = package,     reporter = reporter, load_helpers = load_helpers, env = env,     stop_on_failure = stop_on_failure, stop_on_warning = stop_on_warning,     load_package = load_package, parallel = parallel)
      35: test_dir("testthat", package = package, reporter = reporter,     ..., load_package = "installed")
      36: test_check("WhatsR")
      An irrecoverable exception occurred. R is aborting now ...
      Segmentation fault (core dumped)
    ```

