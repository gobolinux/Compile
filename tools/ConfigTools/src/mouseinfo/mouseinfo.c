#include <stdio.h>
#include <stdlib.h>
#include "device.h"
#include "kudzu.h"

char *
get_class_name(enum deviceClass class)
{
	switch (class) {
		case CLASS_MOUSE: return "Mouse";
		case CLASS_USB:   return "USB";
		default:          return "unknown";
	}
}

char *
get_bus_name(enum deviceBus bus)
{
	switch (bus) {
		case BUS_PSAUX:   return "PSAUX";
		case BUS_USB:     return "USB";
		case BUS_SERIAL:  return "Serial";
		default:          return "unknown";
	}
}

int
do_probe(enum deviceClass class, enum deviceBus bus)
{
	int i;
	struct device **currentDevs;

	DEBUG("probing class %s on bus %s\n", class_string, bus_string);
	initializeBusDeviceList(bus);
	currentDevs = probeDevices(class, bus, (PROBE_ALL | PROBE_SAFE));

	if (! currentDevs) {
		currentDevs = malloc(sizeof(struct device *));
		currentDevs[0] = NULL;
	}
	
	for (i=0; currentDevs[i]; ++i)
		currentDevs[i]->writeDevice(stdout, currentDevs[i]);

	freeDeviceList();
	for (i=0; currentDevs[i]; ++i)
		free(currentDevs[i]);
	
	return 0;
}

int
main(int argc, char **argv)
{
	int bus, class;
	enum deviceBus busList[] = { BUS_PSAUX, BUS_USB, BUS_SERIAL };
	enum deviceClass classList[] = { CLASS_MOUSE, CLASS_USB };
	
	
	for (bus=0; bus<sizeof(busList)/sizeof(enum deviceBus); ++bus)
		for (class=0; class<sizeof(classList)/sizeof(enum deviceClass); ++class)
			do_probe(classList[class], busList[bus]);

	return 0;
}
