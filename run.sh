#!/bin/bash -
php -n -d extension=ev.so  -dextension_dir=./.libs "$@"
