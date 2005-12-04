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

#ifndef _KUDZU_SERIAL_H_
#define _KUDZU_SERIAL_H_

#include "device.h"

struct serialDevice {
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
	/* serial-specific fields */
	struct serialDevice *(*newDevice) (struct serialDevice *dev);
	void (*freeDevice) (struct serialDevice *dev);
	void (*writeDevice) (FILE *file, struct serialDevice *dev);
	int (*compareDevice) (struct serialDevice *dev1, struct serialDevice *dev2);
	char * pnpmfr;
	char * pnpmodel;
	char * pnpcompat;
	char * pnpdesc;
	
};

struct serialDevice *serialNewDevice(struct serialDevice *dev);
struct device *serialProbe(enum deviceClass probeClass, int probeFlags,
			struct device *devlist);

#endif
