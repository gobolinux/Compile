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

#ifndef _KUDZU_H_
#define _KUDZU_H_

/* kudzu: it grows on you */

/* level of debugging output */
#undef DEBUG_LEVEL

#ifdef DEBUG_LEVEL
#define DEBUG(s...) fprintf(stderr,s)
#else
#define DEBUG(s...) ;
#endif

#include "device.h"
#include "serial.h"
#include "psaux.h"
#include "usb.h"

/* Do any preinitialization of device data */
/* Only required if you desire to change the default device lists */
int initializeDeviceList(void);
int initializeBusDeviceList(enum deviceBus bus);

/* Free any driver lists initialized with initializeDeviceList() */
void freeDeviceList();

/* Probe for devices of the specified class, on the specified bus,
 * with the specified class. Returns a NULL-terminated array of
 * device (or subclass) pointers */
struct device ** probeDevices ( enum deviceClass probeClass, 
			      enum deviceBus probeBus,
			      int probeFlags
			       );

/* Read a config file for a device list */
struct device ** readDevices ( char *fn );

/* Like readDevices, except pass the file directly */
struct device ** readDevs ( FILE *f );

/* Write the NULL-terminated device pointer array to a file, in a
 * format suitable for readDevices() */
int writeDevices ( char *fn, struct device **devlist);

/* Compares the two lists. 
 * retlist1 and retlist2 contain items that are unique to list1/list2. */
int listCompare( struct device **list1, struct device **list2,
		struct device ***retlist1, struct device ***retlist2);

/* Remove device 'dev' from list 'devlist'.
 * Returns the new list head. */
struct device *listRemove(struct device *devlist, struct device *dev);

/* Match network devices in devlist with their configured names */
void matchNetDevices(struct device *devlist);


typedef struct device *(newFunc)(struct device *);
typedef int (initFunc)();
typedef struct device *(probeFunc)(enum deviceClass, int, struct device *);

struct bus {
	enum deviceBus busType;
	char *string;
	struct device *(*newFunc)(struct device *);
	int (*initFunc)(char *filename);
	void (*freeFunc)();
	struct device *(*probeFunc)(enum deviceClass, int, struct device *);
};

extern struct bus buses[];

struct kudzuclass {
	enum deviceClass classType;
	char *string;
};

extern struct kudzuclass classes[];

extern char *module_file;
extern float kernel_release;
#endif
