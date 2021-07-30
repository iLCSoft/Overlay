# Overlay
[![Build](https://github.com/iLCSoft/Overlay/actions/workflows/linux.yml/badge.svg)](https://github.com/iLCSoft/Overlay/actions/workflows/linux.yml)
[![coverity](https://scan.coverity.com/projects/11936/badge.svg)](https://scan.coverity.com/projects/ilcsoft-overlay)

The package Overlay provides code for event overlay with Marlin

Overlay is distributed under the [GPLv3 License](http://www.gnu.org/licenses/gpl-3.0.en.html)

[![License](https://www.gnu.org/graphics/gplv3-127x51.png)](https://www.gnu.org/licenses/gpl-3.0.en.html)

## Notes

The Overlay processor can be used to overlay background events from an additonal set of LCIO files in a Marlin job. It requires Marlin v01-00 or higher. For details see ./doc/Overlay.pdf and the doxygen API doc.

The JoinEvents processor can be used to join events on a collection basis, i.e events from an additional input file  are read and all collections are added to the current event (provided they have a name that is different from all collections in the current event).

## License and Copyright
Copyright (C), Overlay Authors

Overlay is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License long with this program.  If not, see <http://www.gnu.org/licenses/>.
