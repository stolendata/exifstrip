EXIFstrip - a simple JPEG APP/EXIF stripper
===========================================

Copyright (c) 2007 Robin Leffmann

This is a blunt attempt to strip a JPEG file of all its APP segments, meaning all EXIF tags, thumbnails, lossless- and JPS data etc.

It assumes that all JPEG segments are entropy-coded and that ICC color profiles always identify as "0xFFE2" segments, which seems to be true from the various files and picture sources I've tested, but, as I actually haven't read the JPEG/EXIF specifications I don't know if this is always the case, so your mileage may vary... :)

License
-------

This source code is released under the CC BY-NC-SA license, and comes
without any warranties of any kind.

http://creativecommons.org/licenses/by-nc-sa/3.0/
