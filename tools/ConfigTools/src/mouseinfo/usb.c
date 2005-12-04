/* Copyright 1999-2003 Red Hat, Inc.
 *
 * Changelog:
 *   04/03/2005 - lucasvr@gobolinux.org
 *                Big cleanup to only detect mouse devices
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <string.h>
#include <unistd.h>

#include <sys/utsname.h>

#include "usb.h"

#define MODDIR getenv("goboModules") == NULL ? "/System/Kernel/Modules" : getenv("goboModules")

#define loadModule(x)      (1)
#define removeModule(x)    (1)
#define freeConfModules(x) do { } while(0)
#define readConfModules(x) (0)
#define getAlias(x,y)      (0)
#define module_file        (NULL)


static void usbFreeDevice(struct usbDevice *dev)
{
	if (dev->usbmfr)
		free(dev->usbmfr);
	if (dev->usbprod)
		free(dev->usbprod);
	freeDevice((struct device *) dev);
}

static void usbWriteDevice(FILE * file, struct usbDevice *dev)
{
	writeDevice(file, (struct device *) dev);
	fprintf(file, "usbclass: %d\nusbsubclass: %d\nusbprotocol: %d\n",
		dev->usbclass, dev->usbsubclass, dev->usbprotocol);
	fprintf(file, "usbbus: %d\nusblevel: %d\nusbport: %d\nusbdev: %d\n",
		dev->usbbus, dev->usblevel, dev->usbport, dev->usbdev);
	fprintf(file, "vendorId: %04x\ndeviceId: %04x\n",
		dev->vendorId, dev->deviceId);
	if (dev->usbmfr)
		fprintf(file, "usbmfr: %s\n",dev->usbmfr);
	if (dev->usbprod)
		fprintf(file, "usbprod: %s\n",dev->usbprod);
}

static int usbCompareDevice(struct usbDevice *dev1, struct usbDevice *dev2)
{
	int x;

	if (!dev1 || !dev2)
		return 1;
	x = compareDevice((struct device *) dev1, (struct device *) dev2);
	if (x && x != 2)
		return x;
	return x;
}


struct usbDevice *usbNewDevice(struct usbDevice *old)
{
	struct usbDevice *ret;

	ret = malloc(sizeof(struct usbDevice));
	memset(ret, '\0', sizeof(struct usbDevice));
	ret =
	    (struct usbDevice *) newDevice((struct device *) old,
					   (struct device *) ret);
	ret->bus = BUS_USB;
	ret->newDevice = usbNewDevice;
	ret->freeDevice = usbFreeDevice;
	ret->writeDevice = usbWriteDevice;
	ret->compareDevice = usbCompareDevice;
	if (old && old->bus == BUS_USB) {
		ret->usbclass = old->usbclass;
		ret->usbsubclass = old->usbsubclass;
		ret->usbprotocol = old->usbprotocol;
		ret->usbbus = old->usbbus;
		ret->usblevel = old->usblevel;
		ret->usbport = old->usbport;
		ret->usbdev = old->usbdev;
		ret->vendorId = old->vendorId;
		ret->deviceId = old->deviceId;
		if (old->usbmfr)
			ret->usbmfr = strdup(old->usbmfr);
		if (old->usbprod)
			ret->usbprod = strdup(old->usbprod);
	}
	return ret;
}

struct usbdesc {
	unsigned int vendorId;
	unsigned int deviceId;
	char *desc;
	char *driver;
};

static struct usbdesc *usbDeviceList = NULL;
static int numUsbDevices = 0;

struct usbdrvinfo {
	unsigned int vendid;
	unsigned int devid;
	char *driver;
};

static struct usbdrvinfo *usbDrvList = NULL;
static int numUsbDrivers = 0;

static int devCmp(const void *a, const void *b)
{
	const struct usbdesc *one = a;
	const struct usbdesc *two = b;
	int x, y;

	x = one->vendorId - two->vendorId;
	y = one->deviceId - two->deviceId;
	if (x)
		return x;
	return y;
}

static int drvCmp(const void *a, const void *b)
{
	const struct usbdrvinfo *one = a;
	const struct usbdrvinfo *two = b;
	int x, y;
	
	x = one->vendid - two->vendid;
	y = one->devid - two->devid;
	if (x) return x;
	return y;
}


int usbReadDrivers(char *filename)
{
	int fd;
	char *b, *buf, *tmp, *ptr;
	unsigned int vend = 0, dev;
	struct usbdesc tmpdev;
	char *vendor = NULL;
	struct utsname utsbuf;
	char path[256];

	uname(&utsbuf);
	snprintf(path,255,"%s/%s/modules.usbmap", MODDIR, utsbuf.release);
	
	fd = open(path, O_RDONLY);
        if (fd < 0)
            return 0;

	if (fd >= 0) {
		char *drv;
		int vid, did;
		
		b = buf = bufFromFd(fd);
                if (!buf) return 0;

		while (*buf) {
 			ptr = buf;
			while (*ptr && *ptr != '\n')
				ptr++;
			if (*ptr) {
				*ptr = '\0';
				ptr++;
			}
			if (*buf == '#') {
				buf = ptr;
				continue;
			}
			tmp = buf;
			while (*tmp && !isspace(*tmp) && tmp < ptr) tmp++;
			*tmp = '\0';
			tmp++;
			drv = buf;
			buf = tmp;
			if (strtoul(buf,&buf,16) != 3) {
				buf = ptr;
				continue;
			}
			if (!buf) {
				buf = ptr;
				continue;
			}
			vid = strtoul(buf,&buf,16);
			if (!buf) {
				buf = ptr;
				continue;
			}
			did = strtoul(buf,NULL,16);
			usbDrvList = realloc(usbDrvList,(numUsbDrivers+1)*
					     sizeof(struct usbdrvinfo));
			usbDrvList[numUsbDrivers].vendid = vid;
			usbDrvList[numUsbDrivers].devid = did;
			usbDrvList[numUsbDrivers].driver = strdup(drv);
			numUsbDrivers++;
			buf = ptr;
		}
		free(b);
	}
	if (numUsbDrivers) {
		qsort(usbDrvList,numUsbDrivers,sizeof(struct usbdrvinfo),
		      drvCmp);
	}
	
	if (filename) {
		fd = open(filename, O_RDONLY);
		if (fd < 0)
			return -1;
	} else {
		fd = open("/usr/share/hwdata/usb.ids", O_RDONLY);
		if (fd < 0) {
			fd = open("./usb.ids", O_RDONLY);
			if (fd < 0)
				return -1;
		}
	}
	b = buf = bufFromFd(fd);
        if (!buf) return -1;

	while (*buf) {
		ptr = buf;
		while (*ptr && *ptr != '\n')
			ptr++;
		if (*ptr) {
			*ptr = '\0';
			ptr++;
		}
		if (!strncmp(buf,"# List of known device classes",30))
			break;
		if (*buf == '#') {
			buf = ptr;
			continue;
		}
		if (isalnum(*buf)) {
			tmp = buf;
			while (*tmp && !isspace(*tmp))
				tmp++;
			if (*tmp) {
				*tmp = '\0';
				do
					tmp++;
				while (isspace(*tmp));
			}
			vend = strtol(buf, NULL, 16);
			vendor = tmp;
		}
		if (*buf == '\t') {
			buf++;
			tmp = buf;
			while (*tmp && !isspace(*tmp))
				tmp++;
			if (*tmp) {
				*tmp = '\0';
				do
					tmp++;
				while (isspace(*tmp));
			}
			dev = strtol(buf, NULL, 16);
			if (vend && dev) {
				struct usbdrvinfo drvtmp, *sdev;
				
				tmpdev.vendorId = vend;
				tmpdev.deviceId = dev;
				tmpdev.driver = NULL;
				tmpdev.desc =
					malloc(strlen(tmp) + 2 + strlen(vendor));
				snprintf(tmpdev.desc,
					 strlen(tmp) + 2 + strlen(vendor), "%s %s",
					 vendor, tmp);
				usbDeviceList =
					realloc(usbDeviceList,
						(numUsbDevices +
						 1) * sizeof(struct usbdesc));
				drvtmp.vendid = vend;
				drvtmp.devid = dev;
				sdev = bsearch(&drvtmp, usbDrvList, numUsbDrivers,
					sizeof(struct usbdrvinfo), drvCmp);
				if (sdev)
					tmpdev.driver = strdup(sdev->driver);
				usbDeviceList[numUsbDevices] = tmpdev;
				numUsbDevices++;
			}
		}
		buf = ptr;
	}
	free(b);
	qsort(usbDeviceList, numUsbDevices, sizeof(struct usbdesc),
	      devCmp);
	return 0;
}

static void parseTopologyLine(char *line, struct usbDevice *usbdev)
{
	usbdev->usbbus = atoi(&line[8]);
	usbdev->usblevel = atoi(&line[15]);
	usbdev->usbport = atoi(&line[31]);
	usbdev->usbdev = atoi(&line[46]);
}

static enum deviceClass usbToKudzu(int usbclass, int usbsubclass, int usbprotocol)
{
	switch (usbclass) {
	case 1:
		if (usbsubclass == 02)
			return CLASS_AUDIO;
		else
			return CLASS_OTHER;
	case 2:
		switch (usbsubclass) {
		case 2:
			return CLASS_MODEM;
		case 6:
		case 7:
			return CLASS_NETWORK;
		default:
			return CLASS_OTHER;
		}
	case 3:
		switch (usbprotocol) {
		case 1:
			return CLASS_KEYBOARD;
		case 2:
			return CLASS_MOUSE;
		default:
			return CLASS_OTHER;
		}

	case 7:
		return CLASS_PRINTER;
	case 8:
		switch (usbsubclass) {
		case 2:
			return CLASS_CDROM;
		case 3:
			return CLASS_TAPE;
		case 4:
			return CLASS_FLOPPY;
		case 5:
			/* Could be a M/O drive. Could be a floppy drive. */
			return CLASS_OTHER;
		case 6:
			return CLASS_HD;
		}
	default:
		return CLASS_OTHER;
	}
}

static void parseDescriptorLine(char *line, struct usbDevice *usbdev)
{
	usbdev->usbclass = atoi(&line[30]);
	usbdev->usbsubclass = atoi(&line[44]);
	usbdev->usbprotocol = atoi(&line[52]);
	usbdev->type =
		usbToKudzu(usbdev->usbclass, usbdev->usbsubclass,
			   usbdev->usbprotocol);
	if (usbdev->device) {
		free(usbdev->device);
		usbdev->device = NULL;
	}
	free(usbdev->driver);

	switch (usbdev->type) {
	case CLASS_MOUSE:
		usbdev->driver = strdup("genericwheelusb");
		usbdev->device = strdup("input/mice");
		break;
	case CLASS_KEYBOARD:
		usbdev->driver = strdup("keybdev");
		usbdev->type = CLASS_KEYBOARD;
		break;
	case CLASS_FLOPPY:
	case CLASS_CDROM:
	case CLASS_HD:
		usbdev->driver = strdup("usb-storage");
		break;
	case CLASS_AUDIO:
		usbdev->driver = strdup("snd-usb-audio");
		break;
	default:
		usbdev->driver = strdup("unknown");
		break;
	}
}

static void parseIdLine(char *line, struct usbDevice *usbdev)
{
	usbdev->vendorId = strtol(&line[11], NULL, 16);
	usbdev->deviceId = strtol(&line[23], NULL, 16);
}

static void parseStringDescriptorLine(char *line, struct usbDevice *usbdev)
{
	int x;
	char *tmp;

	if ((tmp = strcasestr(line, "product")) != NULL) {
		if (usbdev->usbprod);
			free(usbdev->usbprod);
		usbdev->usbprod = strdup(tmp + 8);
		for (x = 0; usbdev->usbprod[x]; x++)
			if (usbdev->usbprod[x] == '\n')
				usbdev->usbprod[x] = '\0';
		for (x-=2; x>=0; x--) {
			if (isspace(usbdev->usbprod[x]))
				usbdev->usbprod[x] = '\0';
			else
				break;
		}
	}
	if ((tmp = strcasestr(line, "manufacturer")) != NULL) {
		if (usbdev->usbmfr);
			free(usbdev->usbmfr);
		usbdev->usbmfr = strdup(tmp + 13);
		for (x = 0; usbdev->usbmfr[x]; x++)
			if (usbdev->usbmfr[x] == '\n')
				usbdev->usbmfr[x] = '\0';
		for (x-=2; x>=0; x--) {
			if (isspace(usbdev->usbmfr[x]))
				usbdev->usbmfr[x] = '\0';
			else
				break;
		}
	}
}

void usbFreeDrivers()
{
	int x;
	if (usbDrvList) {
		for (x = 0; x < numUsbDrivers; x++) {
			free(usbDrvList[x].driver);
		}
		free(usbDrvList);
	}
	if (usbDeviceList) {
		for (x = 0; x < numUsbDevices; x++) {
			free(usbDeviceList[x].desc);
		}
		free(usbDeviceList);
	}
	usbDrvList = NULL;
	usbDeviceList = NULL;
	numUsbDrivers = 0;
	numUsbDevices = 0;
}

static void usbSearchAndAdd(struct usbDevice *usbdev, struct device **devlistptr,
			    enum deviceClass probeClass)
{
	struct usbdesc *searchDev, key;
	struct device *devlist = *devlistptr;
	
	key.vendorId = usbdev->vendorId;
	key.deviceId = usbdev->deviceId;
	searchDev = bsearch(&key, usbDeviceList, numUsbDevices,
			    sizeof(struct usbdesc), devCmp);
	if (searchDev) {
		free(usbdev->desc);
		usbdev->desc = strdup(searchDev->desc);
	        if (searchDev->driver) {
			free(usbdev->driver);
			usbdev->driver = strdup(searchDev->driver);
		}
	}
	if (!strcmp(usbdev->desc,"unknown") && usbdev->usbprod) {
		if (usbdev->usbmfr) {
			char buf[128];
			snprintf(buf,127,"%s %s",usbdev->usbmfr, usbdev->usbprod);
			usbdev->desc = strdup(buf);
		} else {
			usbdev->desc = strdup(usbdev->usbprod);
		}
	}
	
	if (strcasestr(usbdev->desc,"Wacom") &&
	    usbdev->type == CLASS_MOUSE) {
		free(usbdev->driver);
		usbdev->driver = strdup("wacom");
	}
		
	if (usbdev->type & probeClass) {
		usbdev->next = devlist;
		devlist = (struct device *)usbdev;
	} else {
		usbFreeDevice(usbdev);
	}
	*devlistptr = devlist;
}

static int
usbDeviceIgnore (struct usbDevice *usbdev, const char *line)
{
	int proto, alt;

	if (usbdev != NULL) {
		/* If the Proto is different, it's a different device */
		proto = atoi(&line[52]);
		if (proto != usbdev->usbprotocol) {
			return 0;
		}
	}

	/* Alt needs to be '0' */
	alt = atoi(&line[15]);
	if (alt != 0) {
		return 1;
	}

	return 0;
}

struct device *usbProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist)
{
	FILE *usbdevicelist;
	char line[255];
	struct usbDevice *usbdev = NULL, *tmpdev = NULL;
	int init_list = 0;

	if (probeClass & CLASS_MOUSE) {
		if (!usbDeviceList) {
			usbReadDrivers(NULL);
			init_list = 1;
		}
		

		usbdevicelist = fopen("/proc/bus/usb/devices", "r");
		if (usbdevicelist == NULL)
			return devlist;
		
		while (fgets(line, 255, usbdevicelist)) {	/* device info */
			switch (line[0]) {
			case 'T':
				if (usbdev != NULL) {
					usbSearchAndAdd(usbdev,&devlist, probeClass);
				}
				usbdev = usbNewDevice(NULL);
				usbdev->desc = strdup("unknown");
				usbdev->driver = strdup("unknown");
				parseTopologyLine(line, usbdev);
				break;
			case 'I':
				if (usbDeviceIgnore (usbdev, line) != 0) {
					break;
				}
				/* Interface number > 0 */
				if (atoi(line+8) > 0) {
					if (usbdev != NULL) {
						tmpdev = usbNewDevice(usbdev);
					        usbSearchAndAdd(usbdev,&devlist, probeClass);
						usbdev = tmpdev;
					}
				}
				parseDescriptorLine(line, usbdev);
				break;
			case 'P':
				parseIdLine(line, usbdev);
			case 'S':
				parseStringDescriptorLine(line, usbdev);
				break;
			default:
				break;
			}
		}
		if (usbdev != NULL) {
			usbSearchAndAdd(usbdev,&devlist, probeClass);
		}
		fclose(usbdevicelist);
	}
	
	if (usbDeviceList && init_list)
	  usbFreeDrivers();
	
	return devlist;
}
