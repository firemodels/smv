# part2iso

Particle-to-isosurface conversion extracted from the deprecated smokezip
-part2iso path. The original smokezip sources are unchanged.

Usage: part2iso [-s sourcedir] casename[.smv]

Conversion runs sequentially for each available PRT5 file. It writes a particle
boundary .iso, a .tiso for each particle property, and a case .isosmv
listing those outputs. The isosurface level remains 0.5, as in smokezip.
Output files are written beside the particle inputs.

Only mesh dimensions, particle classes, and particle file entries are read
from the SMV file. Compression, INI settings, demo generation, file cleanup,
and particle-to-object conversion are excluded.

Build using the repository CMake target part2iso, or run the matching
make_part2iso script in a platform directory under Build/part2iso.
The platform builds use the shared libraries under Build/LIBS.