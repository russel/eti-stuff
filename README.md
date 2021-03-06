# eti-stuff

"eti-stuff" is an attempt to understand the eti structure as defined in ETS 300 799.

[eti-cmdline](#eti-cmdline) is based on the dab-cmdline software  with code
included from [dabtools](https://github.com/Opendigitalradio/dabtools) to actually decode the eti frames.
It is - as the name suggests - a command line version.

eti-cmdline now supports a whole range of devices,

 - RTLSDR 2832 based dabsticks,
 - AIRspy devices,
 - SDRPlay RSP devices using the 2.13 SDRplay library,
 - SDRPlay RSP devices using the 3.06/7 SDRplay library,
 - Adalm Pluto devices,
 - HACKRF devices,
 - LIMESDR devices

When constructing, select the input device of choice in the CMake command

By piping the output from eti-cmdline into dablib_gtk, a more or less complete DAB receiver exists.

      
You can use dablin or dablin_gtk from https://github.com/Opendigitalradio/dablin by running
      
      eti-cmdline-xxx -C 11C -G 80 | dablin_gtk
      
where xxx refers to the input device being supported, one of (`rtlsdr`, `sdrplay`, `sdrplay-v3', `pluto', `airspy`, `hackrf', `limesdr', `rawfiles`, `wavfiles`).
      
# Disclaimer

The software is under development and most likely contains errors.

eti-stuff is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.


# Copyright

Copyright Jan van Katwijk <J.vanKatwijk@gmail.com>.
Lazy Chair Computing

This software is part of the Qt-DAB collection, Qt-DAB is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version, taking into account the licensing conditions of the parts of the software that are derived from wotk of others.

This software uses parts of dabtools. Excerpt from the README of dabtools reads

"dabtools is written by Dave Chapman <dave@dchapman.com>
   
Parts of the code in eti-backend are copied verbatim (or with trivial modifications) from David Crawley's OpenDAB and hence retain his copyright."

Obviously, the copyrights for the parts copied (or directly derived) from the dabtools remain with Dave Chapman.
