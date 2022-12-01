# gfxi
Graphics Information tool that shows usage of DRM resources.

## Description

The gfxi tool can be used to quickly query DRM information.

It will answer typical questions like:

*"What are the active CRTCs?"*
`./gfxi crtc ACTIVE:1`

*"What planes are available of type Overlay on the single active CRTC?"*
`./gfxi plane type:Overlay CRTC_ID:`./gfxi crtc ACTIVE:1`

*"List all cursor planes"*
`./gfxi plane type:Cursor 

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


