// GFXI
// by Bram Stolk
// bram.stolk@canonical.com
// LICENSE: GLPv3

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>	// for assert()
#include <fcntl.h>	// for open()
#include <unistd.h>	// for close()
#include <errno.h>	// for EINVAL
#include <string.h>	// for strerror()
#include <inttypes.h>	// for PRIu64

#include <xf86drm.h>
#include <xf86drmMode.h>

// Command line arguments.
static const char* opt_class = "plane";

#define MAXFILT 16
static int         flt_cnt = 0;
static char        flt_keys[MAXFILT][80];
static char        flt_vals[MAXFILT][80];

#if 0
// Potential use: human readable output.
static const char* connector_name(int co)
{
	if (co == DRM_MODE_CONNECTOR_Unknown) return "Unknown";
	if (co == DRM_MODE_CONNECTOR_VGA) return "VGA";
	if (co == DRM_MODE_CONNECTOR_DVII) return "DVI-I";
	if (co == DRM_MODE_CONNECTOR_DVID) return "DVI-D";
	if (co == DRM_MODE_CONNECTOR_DVIA) return "DVI-A";
	if (co == DRM_MODE_CONNECTOR_Composite) return "composite";
	if (co == DRM_MODE_CONNECTOR_SVIDEO) return "s-video";
	if (co == DRM_MODE_CONNECTOR_LVDS) return "LVDS";
	if (co == DRM_MODE_CONNECTOR_Component) return "component";
	if (co == DRM_MODE_CONNECTOR_9PinDIN) return "9-pin DIN";
	if (co == DRM_MODE_CONNECTOR_DisplayPort) return "DP";
	if (co == DRM_MODE_CONNECTOR_HDMIA) return "HDMI-A";
	if (co == DRM_MODE_CONNECTOR_HDMIB) return "HDMI-B";
	if (co == DRM_MODE_CONNECTOR_TV) return "TV";
	if (co == DRM_MODE_CONNECTOR_eDP) return "eDP";
	if (co == DRM_MODE_CONNECTOR_VIRTUAL) return "Virtual";
	if (co == DRM_MODE_CONNECTOR_DSI) return "DSI";
	if (co == DRM_MODE_CONNECTOR_DPI) return "DPI";
	return "unknown";
}
#endif


// Checks to see if a property value passes all the filters that have been set.
static int prop_value_passes(drmModePropertyPtr prop, uint64_t pval)
{
	// Check all filters
	for (int i=0; i<flt_cnt; ++i)
	{
		const char* key = flt_keys[i];
		const char* val = flt_vals[i];
		// If filter name and property name match, check values.
		//fprintf(stderr,"propname: %s\n", prop->name);
		if (!strcmp(key, prop->name))
		{
			uint64_t reqval = 0xffffffffffffffff;
			if (prop->count_enums>0)
			{
				for (int j=0; j<prop->count_enums; ++j)
				{
					const char* nm = prop->enums[j].name;
					if (!strcmp(nm, val))
						reqval = prop->enums[j].value;
				}
			}
			else
			{
				reqval = atoi(val);
			}
			return reqval == pval;
		}
	}
	// No matching filter was set: the property value passes.
	return 1;
}


static int all_props_pass(int fd, drmModeObjectPropertiesPtr props)
{
	const int nump = props->count_props;
	for (int i=0; i<nump; ++i)
	{
		const uint64_t actualval = props->prop_values[i];
		drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[i]);
		if (!prop_value_passes(prop, actualval))
			return 0;
	}
	return 1;
}


// List the connectors.
static int list_conn(int fd, drmModeResPtr res)
{
	const int numcon = res->count_connectors;
	//fprintf(stderr, "Device has %d connectors.\n", numcon);

	for (int i=0; i<numcon; ++i)
	{
		const int connid = res->connectors[i];
		drmModeConnectorPtr connector = drmModeGetConnectorCurrent(fd, connid);
		if (!connector)
			continue;
		//int connected = connector->connection == DRM_MODE_CONNECTED;
		drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, connid, DRM_MODE_OBJECT_CONNECTOR);
		if (!props)
		{
			fprintf(stderr, "Failed to get properties for connector %d\n", connid);
			continue;
		}
		if (all_props_pass(fd, props))
			fprintf(stdout, "%d\n", connid);
		drmModeFreeObjectProperties(props);
		drmModeFreeConnector(connector);
	}
	return numcon;
}


// List the controllers.
static int list_crtc(int fd, drmModeResPtr res)
{
	const int numcrtcs = res->count_crtcs;
	for (int i=0; i<numcrtcs; ++i)
	{
		uint32_t crtc_id = res->crtcs[i];
		drmModeCrtcPtr crtc = drmModeGetCrtc(fd, crtc_id);
		if (!crtc)
			continue;

		drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, crtc_id, DRM_MODE_OBJECT_CRTC);
		if (!props)
		{
			fprintf(stderr, "Failed to get properties for crtc %d\n", crtc_id);
			continue;
		}
		if (all_props_pass(fd, props))
			fprintf(stdout, "%d\n", crtc_id);
		drmModeFreeObjectProperties(props);
		drmModeFreeCrtc(crtc);
	}
	return numcrtcs;
}


// List the framebuffers.
static int list_frmb(int fd, drmModeResPtr res)
{
	const int numfbs = res->count_fbs;
	for (int i=0; i<numfbs; ++i)
	{
		const uint32_t fb_id = res->fbs[i];
		drmModeFBPtr fb = drmModeGetFB(fd, fb_id);
		if (!fb)
			continue;
		drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, fb_id, DRM_MODE_OBJECT_FB);
		if (!props)
		{
			fprintf(stderr, "Failed to get properties for framebuffer %d\n", fb_id);
			continue;
		}
		if (all_props_pass(fd, props))
			fprintf(stdout, "%d\n", fb_id);
		drmModeFreeObjectProperties(props);
		drmModeFreeFB(fb);
#if 0
		fprintf(stderr, "fb %d bpp=%d %dx%d depth=%d\n", fb_id, fb->bpp, fb->width, fb->height, fb->depth);
#endif
	}
	return numfbs;
}


// List the planes that match the property value.
static int list_plan(int fd, drmModeResPtr res)
{
	drmModePlaneRes* res_planes = drmModeGetPlaneResources(fd);
	if (!res_planes)
	{
		fprintf(stderr, "Failed to get plane resources: %s\n", strerror(errno));
		return errno;
	}

	const int numplanes = res_planes->count_planes;
	for (int p=0; p<numplanes; ++p)
	{
		drmModePlanePtr plane = drmModeGetPlane(fd, res_planes->planes[p]);
		if (!plane)
			continue;

		drmModeObjectPropertiesPtr props = drmModeObjectGetProperties(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
		if (!props)
		{
			fprintf(stderr, "Failed to get object properties: %s\n", strerror(errno));
			continue;
		}
		if (all_props_pass(fd, props))
			fprintf(stdout, "%d\n", plane->plane_id);
		drmModeFreeObjectProperties(props);
		drmModeFreePlane(plane);
	}
	return numplanes;
}


int main(int argc, char* argv[])
{
	const char* devenv = getenv("GFXI_DEVICE");
	const char* devname = devenv ? devenv : "/dev/dri/card0";

	int show_usage = 0;
	for (int i=1; i<argc; ++i)
	{
		const char* a = argv[i];
		const char* s = strchr(a, ':');
		if (s)
		{
			// Handle filter option
			const int idx = flt_cnt++;
			if (idx >= MAXFILT)
			{
				fprintf(stderr, "Too many filters on command line (%d)\n", idx+1);
				exit(2);
			}
			strncpy(flt_keys[idx], a, (s-a));
			strncpy(flt_vals[idx], s+1, sizeof(flt_vals[idx]));
		}
		else
		{
			// Handle class option
			if (!strcmp(a, "plane"))
				opt_class = a;
			else if (!strcmp(a, "connector"))
				opt_class = a;
			else if (!strcmp(a, "crtc"))
				opt_class = a;
			else if (!strcmp(a, "framebuffer"))
				opt_class = a;
			else
				show_usage = 1;
		}
	}

	if (show_usage)
	{
		fprintf(stderr, "Usage: %s [plane|connector|crtc|framebuffer] property:value\n", argv[0]);
		exit(1);
	}

	const int fd = open(devname, O_RDWR);
	if (fd < 0)
	{
		fprintf(stderr, "Failed to open %s (%s)\n", devname, strerror(errno));
		return errno;
	}

	const int setcapres0 = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
	if (setcapres0)
		fprintf(stderr, "Failed to set client capability: %s\n", strerror(errno));
	const int setcapres1 = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
	if (setcapres1)
		fprintf(stderr, "Failed to set client capability: %s\n", strerror(errno));

	const drmModeResPtr res = drmModeGetResources(fd);
	if (!res)
	{
		fprintf(stderr, "Could not get drm resources from %s\n", devname);
		return -EINVAL;
	}

	if (!strcmp(opt_class, "crtc"))
		list_crtc(fd, res);

	if (!strcmp(opt_class, "plane"))
		list_plan(fd, res);

	if (!strcmp(opt_class, "connector"))
		list_conn(fd, res);

	if (!strcmp(opt_class, "framebuffer"))
		list_frmb(fd, res);

	close(fd);
	return 0;
}

