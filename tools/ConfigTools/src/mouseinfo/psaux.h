/* Copyright 1999-2003 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _KUDZU_PSAUX_H_
#define _KUDZU_PSAUX_H_

#include "device.h"

struct psauxDevice {
    /* common fields */
    struct device *next;	/* next device in list */
	int index;
    enum deviceClass type;	/* type */
    enum deviceBus bus;		/* bus it's attached to */
    char * device;		/* device file associated with it */
    char * driver;		/* driver to load, if any */
    char * desc;		/* a description */
	int detached;
    void * classprivate;
    /* psaux-specific fields */
    struct psauxDevice *(*newDevice) (struct psauxDevice *dev);
    void (*freeDevice) (struct psauxDevice *dev);
    void (*writeDevice) (FILE *file, struct psauxDevice *dev);
	int (*compareDevice) (struct psauxDevice *dev1, struct psauxDevice *dev2);
};

struct psauxDevice *psauxNewDevice(struct psauxDevice *dev);
struct device *psauxProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist);

#endif
