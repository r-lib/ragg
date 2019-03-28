.onLoad <- function(lib, pkg){
  if ( Sys.info()['sysname'] == "Windows" ) {
    Sys.setenv(RAGG_FONTCONFIG_PATH = file.path(lib, pkg, "fontconfig", "win") )
    Sys.setenv(RAGG_FONTCONFIG_FILE = file.path(lib, pkg, "fontconfig", "win", "fonts.conf") )
  } else if (file.exists("/usr/local/etc/fonts/fonts.conf")) { # Homebrew
    Sys.setenv(RAGG_FONTCONFIG_PATH = "/usr/local/etc/fonts")
    Sys.setenv(RAGG_FONTCONFIG_FILE = "/usr/local/etc/fonts/fonts.conf")
  } else if (file.exists("/opt/X11/lib/X11/fontconfig")) { # X11
    Sys.setenv(RAGG_FONTCONFIG_PATH = "/opt/X11/lib/X11/fontconfig")
    Sys.setenv(RAGG_FONTCONFIG_FILE = "/opt/X11/lib/X11/fontconfig/fonts.conf")
  } else if (Sys.info()['sysname'] == "Darwin") {
    Sys.setenv(RAGG_FONTCONFIG_PATH = file.path(lib, pkg, "fontconfig", "mac") )
    Sys.setenv(RAGG_FONTCONFIG_FILE = file.path(lib, pkg, "fontconfig", "mac", "fonts.conf") )
  }
}


# /usr/local/etc/fonts/fonts.conf
