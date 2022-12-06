# gfxi
Graphics Information tool that shows usage of DRM resources.

## Description

The gfxi tool can be used to quickly query DRM information.

It will answer typical questions like:

*"What are the active CRTCs?"*
```
$ ./gfxi crtc ACTIVE:1
80
```

*"What planes type Overlay are currently in use on the single active CRTC?"*
```
$ ./gfxi plane type:Overlay CRTC_ID:`./gfxi crtc ACTIVE:1`
40
```

*"How many cursor planes does the second GPU device provide?"*
```
$ GFXI_DEVICE=/dev/dri/card1 ./gfxi plane type:Cursor | wc -l
0
```

## Usage

By default, gfxi will list planes.
To list other resources, specify which type on the command line.

The output can be filtered on property values with a key:value annotation.

```
./gfxi [--annotate] [plane|connector|crtc|framebuffer] [PROP:VALUE ... PROP:VALUE]
```
Currently, to see which properties are available for filtering, it is best to use drm_info.

If you do not set the `GFXI_DEVICE` environment variable, `/dev/dri/card0` will be used as graphics device.

To get more human-friendly output, you can tell gfxi to annotate the object IDs it finds with the `--annotate` flag:
```
$ ./gfxi --annotate connector
236	# DP (connected)
249	# HDMI-A (disconnected)
255	# HDMI-A (disconnected)
```

## Dependencies

gfxi depends on the [libdrm](https://gitlab.freedesktop.org/mesa/drm) library.

## Rationale

Much of these questions could be answered by running the
[drm_info](https://gitlab.freedesktop.org/emersion/drm_info) tool and interpreting its output.

The gfxi tool is a more succint approach that removes the need of parsing drm_info output.
It is also intended to be fast enough so that it can run constantly to catch transient use of overlay planes.

In the future, this tool will have run-time monitoring capabilities.

## Author

[gfxi](github.com:canonical/gfxi.git) is by Bram Stolk (bram.stolk@canonical.com)

## License

GPLv3


