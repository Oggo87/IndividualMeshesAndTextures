# D3D Settings

This section contains entries that affect the Direct3D rendering of GP4. Currently, the only entries available are for forcing the z-buffer format.

```ini
[D3D]
ForceZBufferFormat = false
ZBufferFormat = D16
```

## Force Z-Buffer Format
When set to `true`/`1`, this setting forces GP4 to use the z-buffer format specified in the `ZBufferFormat` setting. This can be useful for improving the visual quality of the game by reducing z-fighting and other depth-related artifacts.

## Z-Buffer Format

This setting specifies the z-buffer format to use when `ForceZBufferFormat` is enabled. The following formats are available:

* `D16` - 16-bit z-buffer bit depth. (D3DFMT_D16 and the default for GP4)
* `D15S1` - 16-bit z-buffer bit depth where 15 bits are reserved for the depth channel and 1 bit is reserved for the stencil channel. (D3DFMT_D15S1)
* `D24X8` - 32-bit z-buffer bit depth using 24 bits for the depth channel. (D3DFMT_D24X8)
* `D24X4S4` - 32-bit z-buffer bit depth using 24 bits for the depth channel and 4 bits for the stencil channel. (D3DFMT_D24X4S4)
* `D24S8` - 32-bit z-buffer bit depth using 24 bits for the depth channel and 8 bits for the stencil channel. (D3DFMT_D24S8)
* `D32` - 32-bit z-buffer bit depth. (D3DFMT_D32)

All formats above are, supposedly, natively supported by GP4's rendering engine, but some of them might not work properly (e.g. you might experience a blank screen). The default format is D3DFMT_D16.