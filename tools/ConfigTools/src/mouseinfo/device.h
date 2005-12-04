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


#ifndef _KUDZU_DEVICES_H_
#define _KUDZU_DEVICES_H_

#include <stdio.h>

enum deviceClass {
    /* device classes... this is somewhat ad-hoc */
	CLASS_UNSPEC = ~0,
	CLASS_OTHER = (1 << 0),
	CLASS_NETWORK = (1 << 1),
	CLASS_SCSI = (1 << 2),
	CLASS_MOUSE = (1 << 3),
	CLASS_AUDIO = (1 << 4),
	CLASS_CDROM = (1 << 5),
	CLASS_MODEM = (1 << 6),
	CLASS_VIDEO = (1 << 7),
	CLASS_TAPE = (1 << 8),
	CLASS_FLOPPY = (1 << 9),
	CLASS_SCANNER = (1 << 10),
	CLASS_HD = (1 << 11),
	CLASS_RAID = (1 << 12),
	CLASS_PRINTER = (1 << 13),
	CLASS_CAPTURE = (1 << 14),
	CLASS_KEYBOARD = (1 << 15),
	CLASS_MONITOR = (1 << 16),
	CLASS_USB = (1 << 17),
	CLASS_SOCKET = (1 << 18),
	CLASS_FIREWIRE = (1 << 19),
	CLASS_IDE = (1 << 20)
};

/* Update this if needed */
#define CLASS_LAST CLASS_IDE

enum deviceBus {
    /* 'bus' that a device is attached to... this is also ad-hoc */
    /* BUS_SBUS is sort of a misnomer - it's more or less Sun */
    /* OpenPROM probing of all various associated non-PCI buses */
    BUS_UNSPEC = ~0,
    BUS_OTHER = (1 << 0),
    BUS_PCI = (1 << 1),
    BUS_SBUS = (1 << 2),
    BUS_SERIAL = (1 << 3),
    BUS_PSAUX = (1 << 4),
    BUS_PARALLEL = (1 << 5),
    BUS_SCSI = (1 << 6),
    BUS_IDE = (1 << 7),
    /* Again, misnomer */
    BUS_KEYBOARD = (1 << 8),
    BUS_DDC = (1 << 9),
    BUS_USB = (1 << 10),
    BUS_ISAPNP = (1 << 11),
    BUS_MISC = (1 << 12),
    BUS_FIREWIRE = (1 << 13),
    BUS_PCMCIA = (1 << 14),
    BUS_ADB = (1 << 15),
    BUS_MACIO = (1 << 16),
    BUS_VIO = (1 << 17),
    BUS_S390 = (1 << 18)
};

struct device {
	/* This pointer is used to make lists by the library. */
	/* Do not expect it to remain constant (or useful) across library calls. */
	struct device *next;
	/* Used for ordering, and for aliasing (modem0, modem1, etc.) */
	int index;
	enum deviceClass type;	/* type */
	enum deviceBus bus;		/* bus it's attached to */
	char * device;		/* device file associated with it */
	char * driver;		/* driver to load, if any */
	char * desc;		/* a description */
	int detached;		/* should we care if it disappears? */
	void * classprivate;    /* data specific to a particular class */
	struct device *(*newDevice) (struct device *old, struct device *new);
	void (*freeDevice) (struct device *dev);
	void (*writeDevice) (FILE *file, struct device *dev);
	int (*compareDevice) (struct device *dev1, struct device *dev2);
};

struct device *newDevice(struct device *old, struct device *new);
void freeDevice(struct device *dev);
void writeDevice(FILE *file, struct device *dev);
int compareDevice(struct device *dev1, struct device *dev2);
struct device *readDevice(FILE *file);
char *bufFromFd(int fd);

/* Return everything found, even non-useful stuff */
#define PROBE_ALL       1
/* Don't do 'dangerous' probes that could do weird things (serial) */
#define PROBE_SAFE (1<<1)
/* Stop at first device found */
#define PROBE_ONE       (1<<2)
/* Do not load any modules */
#define PROBE_NOLOAD	(1<<3)
/* Return devices for which modules are currently loaded */
/* Only implemented for network cards currently */
#define PROBE_LOADED (1 << 31)

#endif
