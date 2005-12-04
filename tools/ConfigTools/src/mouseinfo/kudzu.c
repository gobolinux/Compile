#include <stdlib.h>
/* Copyright 1999-2004 Red Hat, Inc.
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "kudzu.h"

struct kudzuclass classes[] = {
	{ CLASS_MOUSE, "MOUSE"},
	{ CLASS_USB, "USB"},
	{ 0, NULL }
};

struct bus buses[] = {
	{ BUS_USB, "USB", (newFunc *)usbNewDevice, usbReadDrivers, usbFreeDrivers, usbProbe },
	{ BUS_PSAUX, "PSAUX", (newFunc *)psauxNewDevice, NULL, NULL, psauxProbe },
	{ BUS_SERIAL, "SERIAL", (newFunc *)serialNewDevice, NULL, NULL, serialProbe },
	{ 0, NULL, NULL, NULL, NULL, NULL }
};

int initializeBusDeviceList(enum deviceBus busSet) {
	int bus;
	
	for (bus=0;buses[bus].string;bus++) {
	  if ((busSet & buses[bus].busType) && buses[bus].initFunc) {
	      buses[bus].initFunc(NULL);
	  }
	}
	return 0;
}

struct device *newDevice(struct device *old, struct device *new) {
    if (!old) {
	if (!new) {
	    new = malloc(sizeof(struct device));
	    memset(new,'\0',sizeof(struct device));
	}
	    new->type = CLASS_UNSPEC;
    } else {
	    new->type = old->type;
	    if (old->device) new->device = strdup(old->device);
	    if (old->driver) new->driver = strdup(old->driver);
	    if (old->desc) new->desc = strdup(old->desc);
	    new->detached = old->detached;
    }
    new->newDevice = newDevice;
    new->freeDevice = freeDevice;
    new->compareDevice = compareDevice;
    return new;
}

void freeDevice(struct device *dev) {
    if (!dev) {
	    printf("freeDevice(null)\n");
	    abort(); /* return; */
    }
    if (dev->device) free (dev->device);
    if (dev->driver) free (dev->driver);
    if (dev->desc) free (dev->desc);
    free (dev);
}

void freeDeviceList() {
	int bus;
	
	for (bus=0;buses[bus].string;bus++)
	  if (buses[bus].freeFunc)
	    buses[bus].freeFunc();
}

void writeDevice(FILE *file, struct device *dev) {
	int bus, class = CLASS_UNSPEC, i;

	if (!file) {
		printf("writeDevice(null,dev)\n");
		abort();
	}
	if (!dev) {
		printf("writeDevice(file,null)\n");
		abort();
	}
	bus = 0;
	for (i = 0; buses[i].busType; i++)
	  if (dev->bus == buses[i].busType) {
		  bus = i;
		  break;
	  }
        for (i = 0; classes[i].classType; i++)
	  if (dev->type == classes[i].classType) {
		  class = i;
		  break;
	  }
	fprintf(file,"-\nclass: %s\nbus: %s\ndetached: %d\n",
		classes[class].string,buses[bus].string,dev->detached);
	if (dev->device) 
	  fprintf(file,"device: %s\n",dev->device);
	fprintf(file,"driver: %s\ndesc: \"%s\"\n",dev->driver,dev->desc);
}

int compareDevice(struct device *dev1, struct device *dev2) {
	if (!dev1 || !dev2) return 1;
	if (dev1->type != dev2->type) return 1;
	if (dev1->bus != dev2->bus) return 1;
	
	/* Look - a special case!
	 * If it's just the driver that changed, we might
	 * want to act differently on upgrades.
	 */
	if (strcmp(dev1->driver,dev2->driver)) return 2;
	return 0;
}

/* used to sort device lists by a) type, b) device, c) description */
static int devCmp( const void *a, const void *b )
{
        const struct device *one,*two;
	int x,y,z,zz;
	
	one=((const struct device **)a)[0];
	two=((const struct device **)b)[0];
	x=one->type - two->type;
	if (one->device && two->device)
	  y=strcmp(one->device,two->device);
	else {
		y = one->device - two->device;
	}
	z=two->index - one->index;
	zz=strcmp(one->desc,two->desc);
	if (x)
	  return x;
	else if (y)
	  return y;
	else if (z)
	  return z;
	else
	  return zz;
}

struct device ** probeDevices ( enum deviceClass probeClass,
			      enum deviceBus probeBus,
			      int probeFlags
			      ) {
	struct device *devices=NULL,**devlist=NULL;
	int numDevs=0, bus, x, index=0;
	enum deviceClass cl=CLASS_UNSPEC;

	for (bus=0;buses[bus].string;bus++) {
	    if ( (probeBus & buses[bus].busType) &&
		 !(probeBus == BUS_UNSPEC &&
		  buses[bus].busType & BUS_DDC))
		if (buses[bus].probeFunc) {
		    DEBUG("Probing %s\n",buses[bus].string);
		    devices = buses[bus].probeFunc(probeClass,
						   probeFlags, devices);
		}
	    if ((probeFlags & PROBE_ONE) && (devices))
		break;
	}
	
	if (devices == NULL)
		return NULL;
	
	while (devices) {
		devlist=realloc(devlist, (numDevs+2) * sizeof(struct device *));
		devlist[numDevs]=devices;
		devlist[numDevs+1]=NULL;
		numDevs++;
		devices=devices->next;
	}
	
	qsort(devlist, numDevs, sizeof(struct device *), devCmp);
	
	/* We need to sort the network devices by module name. Fun. */
	for (x=0; devlist[x]; x++) {
		devlist[x]->next = devlist[x+1];
	}
	
	devices = devlist[0];
	for (x = 0; x < numDevs ; x++) {
		devlist[x] = devices;
		devices = devices->next;
	}

	for (x=0;devlist[x];x++) {
		if (devlist[x]->type!=cl) {
			index = 0;
		}
		devlist[x]->index = index;
		cl = devlist[x]->type;
		index++;
	}
	return devlist;
}

char *bufFromFd(int fd) {
	struct stat sbuf;
	char *buf = NULL;
	unsigned long bytes = 0;
	char tmpbuf[16384];
	
	fstat(fd,&sbuf);
	if (sbuf.st_size) {
		buf = malloc(sbuf.st_size + 1);
		memset(buf,'\0',sbuf.st_size + 1);
		read(fd, buf, sbuf.st_size);
		buf[sbuf.st_size] = '\0';
	} else {
		memset(tmpbuf,'\0', sizeof(tmpbuf));
		while (read(fd, tmpbuf, sizeof(tmpbuf)) > 0) {
			buf = realloc(buf, bytes + sizeof(tmpbuf));
			memcpy(buf + bytes, tmpbuf, sizeof(tmpbuf));
			bytes += sizeof(tmpbuf);
			memset(tmpbuf, '\0', sizeof(tmpbuf));
		}
	}
	close(fd);
	return buf;
}

