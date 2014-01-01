# apache-sass

This is an Apache plugin that uses libsass to compile SCSS to CSS on the fly.

# Setup
In your httpd.conf:

```
LoadModule /path/to/mod_sass.so
AddHandler sass .scss
AddHandler sass .css
SassIncludePaths /foo/bar:/baz/qux
```

# How to compile

This is a bit rickety right now.

1. Get the apache dev headers.  On ubuntu, the package is either apache2-prefork-dev or apache2-threaded-dev.  I'm not sure which.
2. Build libsass in a directory that's a sibling to the directory holding this project.
3. To compile and install the plugin, run `sudo ./build.sh`.  The build system is very simple, so you shouldn't have any trouble peeling it apart if you have customized needs.

# Configuration

If you want Apache to compile .scss files when they are requested, add this to your httpd.conf

```
AddHandler sass .scss
```

If you want Apache to compile .scss files when someone requests a nonexistent .css file, but a similarly-named .scss file does exist, add this to your httpd.conf

```
AddHandler sass .css
```

To customize the include path the sass compiler uses, use the `SassIncludePaths` directive.  Paths are separated by colons:

```
SassIncludePaths /foo/bar:/baz/qux
```
