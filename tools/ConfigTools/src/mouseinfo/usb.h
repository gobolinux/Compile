/* Copyright 2003 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _KUDZU_USB_H
#define _KUDZU_USB_H

#include "device.h"
#include "kudzu.h"

struct usbDevice {
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
/* usb-specific fields */
    struct usbDevice *(*newDevice) (struct usbDevice *dev);
    void (*freeDevice) (struct usbDevice *dev);
    void (*writeDevice) (FILE *file, struct usbDevice *dev);
    int (*compareDevice) (struct usbDevice *dev1, struct usbDevice *dev2);
    int usbclass;
    int usbsubclass;
    int usbprotocol;
    int usbbus;
    int usblevel;
    int usbport;
    int usbdev;
    int vendorId;
    int deviceId;
    char *usbmfr;
    char *usbprod;
};

struct usbDevice *usbNewDevice(struct usbDevice *dev);
struct device *usbProbe(enum deviceClass probeClass, int probeFlags,
                        struct device *devlist);
int usbReadDrivers(char *filename);
void usbFreeDrivers(void);
#endif
