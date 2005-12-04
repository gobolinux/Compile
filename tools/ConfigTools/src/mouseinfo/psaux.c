/* Copyright 1999-2004 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "kudzu.h"
#include "psaux.h"

static void psauxFreeDevice(struct psauxDevice *dev)
{
    freeDevice((struct device *) dev);
}

static void psauxWriteDevice(FILE *file, struct psauxDevice *dev)
{
    writeDevice(file, (struct device *)dev);
}

static int psauxCompareDevice(struct psauxDevice *dev1, struct psauxDevice *dev2)
{
    return compareDevice( (struct device *)dev1, (struct device *)dev2);
}

struct psauxDevice *psauxNewDevice(struct psauxDevice *old)
{
    struct psauxDevice *ret;
    
    ret = malloc(sizeof(struct psauxDevice));
    memset(ret, '\0', sizeof(struct psauxDevice));
    ret = (struct psauxDevice *) newDevice((struct device *) old, (struct device *) ret);
    ret->bus = BUS_PSAUX;
    ret->newDevice = psauxNewDevice;
    ret->freeDevice = psauxFreeDevice;
    ret->writeDevice = psauxWriteDevice;
    ret->compareDevice = psauxCompareDevice;
    return ret;
}

struct device *psauxProbe(enum deviceClass probeClass, int probeFlags,
			  struct device *devlist)
{
    struct psauxDevice *ps2dev = NULL;
    
    if (probeClass & CLASS_MOUSE || probeClass & CLASS_KEYBOARD) {
	    int fd;
	    char *buf, *start;
	    
	    fd = open("/proc/bus/input/devices", O_RDONLY);
	    if (fd < 0) return devlist;
	    buf = bufFromFd(fd);
	    
	    start = buf;
	    while (start && *start) {
		    while (*buf && *buf !='\n') buf++;
		    if (*buf == '\n') {
			    *buf = '\0';
			    buf++;
		    }
			    
		    if (!strncmp(start,"I:",2)) {
			    if (ps2dev && (ps2dev->type & probeClass)) {
				    if (devlist)
					    ps2dev->next = devlist;
				    devlist = (struct device *) ps2dev;
			    }
			    start = buf;
			    ps2dev = psauxNewDevice(NULL);
			    ps2dev->driver = strdup("ignore");
			    continue;
		    }
		 
		    if (!strncmp(start,"N: Name=",8)) {
			    char *tmp;
			    if (ps2dev->desc)
				    free(ps2dev->desc);
			    ps2dev->desc = strdup(start+9);
			    if ((tmp = strstr(ps2dev->desc,"\"")))
				    *tmp = '\0';
			    if (strstr(ps2dev->desc,"eyboard"))
				    ps2dev->type = CLASS_KEYBOARD;
			    else if (strstr(ps2dev->desc,"ouse") || strstr(ps2dev->desc,"Pad")) {
				    ps2dev->type = CLASS_MOUSE;
				    ps2dev->device = strdup("input/mice");
				    free(ps2dev->driver);
				    ps2dev->driver = strdup("generic3ps/2");
			    } else if (!strcmp(ps2dev->desc,"PC Speaker")) {
				    ps2dev->type = CLASS_OTHER;
				    free(ps2dev->driver);
				    ps2dev->driver = strdup("pcspkr");
			    } else if (strstr(ps2dev->desc,"Speaker") && strstr(ps2dev->desc,"Sparc")) {
				    ps2dev->type = CLASS_OTHER;
				    free(ps2dev->driver);
				    ps2dev->driver = strdup("sparcspkr");
			    } else {
				    ps2dev->type = CLASS_OTHER;
			    }
			    if (strstr(ps2dev->desc,"Synaptics TouchPad")) {
				    if (ps2dev->driver)
					    free(ps2dev->driver);
				    ps2dev->driver = strdup("synaptics");
			    }
			   if (strstr(ps2dev->desc,"AlpsPS/2 ALPS TouchPad")) {
				    if (ps2dev->driver)
					    free(ps2dev->driver);
				    ps2dev->driver = strdup("synaptics");
			    }
			    start = buf;
			    continue;
		    }
		    if (!strncmp(start,"P: Phys=usb",11)) {
			    psauxFreeDevice(ps2dev);
			    ps2dev = NULL;
			    start = buf;
			    continue;
		    }
		    start = buf;
	    }
	    if (ps2dev && ps2dev->type & probeClass) {
		    if (devlist)
			    ps2dev->next = devlist;
		    devlist = (struct device *) ps2dev;
	    }
    }
    return devlist;
}
