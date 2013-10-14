Voxie
=====

Voxie is an open-source voxel editor and toolchain.
It features

  - Voxel editor
  - Block tool, selection tool, bucket fill tool, pencil tool
  - Block copy/paste and move
  - Model scaling and size optimization
  - Global palette (256 indexed colors)
  - 90 degree rotation
  - Export to COLLADA .dae
  - Preliminary support for per-frame animation
  - Support for model reference points (for e.g. a gun or similar)
  - Runtime scripts for Unity3D (not committed yet)

Voxie's editor is written in C++, Qt and Bullet Physics for maximum
performance.
Toolchain scripts are written in Python to make exporting to your favorite
format or engine much more approachable.

Why Voxie?
----------

Voxels are quickly becoming popular with indie game developers. Even though
many games exist that leverages voxels, the toolchains and editors for voxels
are either not available, proprietary or poor.

Few voxel editors support animation, and usually, you hardcode each animated
limb using your game engine. For Voxie, the idea was to support per-frame
animation like you would with traditional pixel art/animation. This has been
implemented using the arrow keys in Voxie, where the camera state is retained
between windows.

Voxie also uses a single, indexed palette for all models, which can be changed
per-game as necessary.

Alternatives to Voxie include:
  - [Qubicle](http://www.minddesk.com/) - proprietary, paid, no indexed
    palette or support for animation
  - [Sproxel](https://code.google.com/p/sproxel/) - last release in 2011, slow
    renderer, no support for animation
  - [Voxatron editor](http://www.lexaloffle.com/voxatron.php) - proprietary
  - [SLAB6, VOXED](http://advsys.net/ken/) - uses ancient VOXLAP engine,
    difficult to use, no animation

Using Voxie
-----------

Voxie supports the following shortcuts:

| Shortcut      | Function                                              |
| :------------ |:----------------------------------------------------- |
| Ctrl+S        | Save                                                  |
| Ctrl+C        | Copy blocks                                           |
| Ctrl+V        | Paste blocks                                          |
| C             | Pick color                                            |
| Insert        | Rapidly use tool primary                              |
| Delete        | Rapidly use tool secondary OR destroy selected blocks |
| Alt+LMB       | Rotate camera                                         |
| Alt+RMB       | Position camera                                       |
| Left          | Select previous window, retain camera                 |
| Right         | Select next window, retain camera                     |

File format
-----------

The vxi file format is Voxie's native voxel format. It is structured as follows
(no padding, little endian):

```
struct Voxel
{
    uint32 x_size, y_size, z_size;
    int32 x_offset, y_offset, z_offset;
    uint8 data[x_size * y_size * z_size]; // entries in palette
    Color palette[256];
    uint8 ref_point_count;
    ReferencePoint ref_points[ref_point_count];
}

// data indexed using
// palette[data[z + y * z_size + x * z_size * y_size]]

struct Color
{
    uint8 r, g, b;
}

struct ReferencePoint
{
    char * name; // NULL-terminated string
    int32 x, y, z;
}
```

In this file format, Z+ is up, Y+ is forward, and palette entry 255 is 'empty'
or 'air'.

Each model includes the global palette that was used when it was saved, in case
you do not want to use `palette.dat`.

The global palette file (`palette.dat`) follows the same structure as the model
palette, i.e.:

```
struct GlobalPalette
{
    Color palette[256];
}
```

License
-------

Voxie is licensed under the MIT license. Please see LICENSE.txt for the full
license details.