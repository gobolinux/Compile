/* Copyright 1999-2003 Red Hat, Inc.
 *
 * This software may be freely redistributed under the terms of the GNU
 * public license.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * probe serial port for PnP/Legacy devices
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <libgen.h>

#include <asm/types.h>
#include <linux/serial.h>

#include "serial.h"
#include "kudzu.h"

/* character strings ARE null-terminated in the following structure    */
/* these elements are marked with a (string) in the comment            */
/* If PnP device sent 6 bit data stream, we've xlated by a 0x20 offset */
/* When computing checksum, must remove this offset                    */
struct pnp_com_id {
    unsigned char xlate_6bit;      /* does this contain xlated data */
    unsigned char other_id[17];    /* backward compatibility with pre-PNP */
    unsigned char other_len;       /* length of the other_id */
    unsigned char pnp_rev[2];      /* PnP revision bytes */
    unsigned char pnp_rev_str[8];  /* PnP revision (string version) */
    unsigned char eisa_id[4];      /* EISA Mfr identifier (string) */
    unsigned char product_id[5];   /* Mfr determined product ID (string) */
    unsigned char serial_number[9];/* Optional dev serial number (string) */
    unsigned char class_name[33];  /* Optional PnP Class name (string) */
    unsigned char driver_id[42];   /* Optional compat device IDs (string) */
    unsigned char user_name[42];   /* Optional verbose product descr (string)*/
    unsigned char checksum[2];     /* Optional checksum */
};

/* there are two possible bytes to signify the start of a PnP ID string */
#define BeginPnP1 0x28
#define BeginPnP2 0x08

/* Likewise, two possible stop bytes */
#define EndPnP1   0x29
#define EndPnP2   0x09

/* these chars indicate extensions to the base dev id exist */
#define ExtendPnP1 0x5c
#define ExtendPnP2 0x3c

#define PNP_COM_MAXLEN 256

/* results from initiating hardware probe of a hardware device */
#define PNP_COM_FATAL      1       /* system error, check errno  */
#define PNP_COM_FAIL       2       /* probe ok, but found nothing */
#define PNP_COM_OK         3       /* probe ok, we found it      */

/* types of devices we might find */
/* if PNP_COM_PNPDEV is NOT set, its a legacy device */
#define PNP_COM_MOUSE      1       /* its a mouse */
#define PNP_COM_MODEM      2       /* its a modem */
#define PNP_COM_OTHER      4       /* device is there, cant tell what kind */
#define PNP_COM_NOEXIST    8       /* no device seen */
#define PNP_COM_PNPDEV     512     /* its a PNP device */

static void serialFreeDevice(struct serialDevice *dev) {
	if (dev->pnpmfr) free(dev->pnpmfr);
	if (dev->pnpmodel) free(dev->pnpmodel);
	if (dev->pnpcompat) free(dev->pnpcompat);
	if (dev->pnpdesc) free(dev->pnpdesc);
        freeDevice((struct device *)dev);
}

static void serialWriteDevice(FILE *file, struct serialDevice *dev)
{
    writeDevice(file, (struct device *) dev);
    if (dev->pnpmfr)
	fprintf(file,"pnpmfr: %s\n",dev->pnpmfr);
    if (dev->pnpmodel)
	fprintf(file,"pnpmodel: %s\n",dev->pnpmodel);
    if (dev->pnpcompat)
	fprintf(file,"pnpcompat: %s\n",dev->pnpcompat);
    if (dev->pnpdesc)
	fprintf(file,"pnpdesc: %s\n",dev->pnpdesc);
}

static int serialCompareDevice( struct serialDevice *dev1, struct serialDevice *dev2)
{
    int x;
    
    x = compareDevice((struct device *)dev1, (struct device *)dev2);
    if (x && x!=2) return x;
    if (dev1->pnpmfr && dev2->pnpmfr && strcmp(dev1->pnpmfr,dev2->pnpmfr))
	return 1;
    if ((!dev1->pnpmfr || !dev2->pnpmfr) && (dev1->pnpmfr != dev2->pnpmfr))
	return 1;
    if (dev1->pnpmodel && dev2->pnpmodel && strcmp(dev1->pnpmodel,dev2->pnpmodel))
	return 1;
    if ((!dev1->pnpmodel || !dev2->pnpmodel) && (dev1->pnpmodel != dev2->pnpmodel))
	return 1;
    if (dev1->pnpcompat && dev2->pnpcompat && strcmp(dev1->pnpcompat,dev2->pnpcompat))
	return 1;
    if ((!dev1->pnpcompat || !dev2->pnpcompat) && (dev1->pnpcompat != dev2->pnpcompat))
	return 1;
    if (dev1->pnpdesc && dev2->pnpdesc && strcmp(dev1->pnpdesc,dev2->pnpdesc))
	return 1;
    if ((!dev1->pnpdesc || !dev2->pnpdesc) && (dev1->pnpdesc != dev2->pnpdesc))
	return 1;
    return x;
}


struct serialDevice * serialNewDevice(struct serialDevice *dev) {
    struct serialDevice *ret;
    
    ret = malloc(sizeof(struct serialDevice));
    memset(ret,'\0',sizeof(struct serialDevice));
    ret=(struct serialDevice *)newDevice((struct device *)dev,(struct device *)ret);
    ret->bus = BUS_SERIAL;
    ret->newDevice = serialNewDevice;
    ret->freeDevice = serialFreeDevice;
    ret->writeDevice = serialWriteDevice;
    ret->compareDevice = serialCompareDevice;
    if (dev && dev->bus == BUS_SERIAL) {
	if (dev->pnpmfr)
	    ret->pnpmfr=strdup(dev->pnpmfr);
	if (dev->pnpmodel)
	    ret->pnpmodel=strdup(dev->pnpmodel);
	if (dev->pnpcompat)
	    ret->pnpcompat=strdup(dev->pnpcompat);
	if (dev->pnpdesc)
	    ret->pnpdesc=strdup(dev->pnpdesc);
    }
    return ret;
}

/*
 * wait_input - wait until there is data available on fd,
 * for the length of time specified by *timo (indefinite
 * if timo is NULL).
 */

static int wait_for_input (int fd, struct timeval *timo) {
    fd_set ready;
    int n;
    
    FD_ZERO(&ready);
    FD_SET(fd, &ready);

    n = select(fd+1, &ready, NULL, &ready, timo);
    return n;
}

static int open_serial_port( char *port ) {
    int fd;

    DEBUG("opening serial port %s...", port);
    
    fd = open( port, O_RDWR | O_NONBLOCK);
    if (fd < 0) {
	DEBUG("failed.\n");
	return -1;
    } else {
	DEBUG("successful.\n");
    }
    
    /* reset file so it is no longer in non-blocking mode */
    if (fcntl(fd, F_SETFL, 0) < 0) {
	close(fd);
	DEBUG("Failed to set port to non-blocking mode\n");
	return -1;
    }
    
    return fd;
}

/* <0 means ioctl error occurred */    
static int get_serial_lines( int fd ) {
    int modem_lines;
    
    ioctl(fd, TIOCMGET, &modem_lines);
    return modem_lines;
}

/* <0 means ioctl error occurred */    
static int set_serial_lines( int fd, int modem_lines ) {
    return ioctl(fd, TIOCMSET, &modem_lines);
}

/* set serial port to 1200 baud, 'nbits' bits, 1 stop, no parity */
static int setup_serial_port( int fd, int nbits, struct termios *attr ) {

    DEBUG("setting up serial port\n");
    
    attr->c_iflag = IGNBRK | IGNPAR;
    attr->c_cflag = 0;
    attr->c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD | PARENB);
    attr->c_cflag |=  CREAD | CLOCAL; /*| CRTSCTS ; */
    if (nbits == 7)
	attr->c_cflag |= CS7 | CSTOPB;
    else
	attr->c_cflag |= CS8;
    attr->c_oflag = 0;
    attr->c_lflag = 0;

    attr->c_cc[VMIN] = 1;
    attr->c_cc[VTIME] = 5;

    if (cfsetospeed( attr, B1200))
	return -1;
    if (cfsetispeed( attr, B1200))
	return -1;
    return tcsetattr(fd, TCSANOW, attr);
}

/* Initialize the serial port to a known state *before* probing. This is
 * apparently required for some Logitech mice, who will stubbornly refuse
 * to respond to PnP probes after they've been opened by gpm or XFree.
 */

static int init_port(int fd) {
    struct termios attr;
    
    if (tcgetattr(fd,&attr))
	return 1;
    
    cfsetospeed(&attr, B2400);
    cfsetispeed(&attr, B2400);
    attr.c_iflag = IXON | ICRNL;
    attr.c_cflag = CLOCAL | HUPCL | CREAD | B9600 | CS8;
    attr.c_oflag = 0;
    attr.c_lflag = 0;
    return tcsetattr(fd, TCSANOW, &attr);
}


/* Request for PnP info from serial device                      */
/* See page 6 of the pnpcom doc from Microsoft                  */
/* Return code tells us what happened                           */
/*                                                              */
/* PNP_COM_FATAL      - error, errno has reason                 */
/* PNP_COM_OK         - probe initiated successfully            */
static int init_pnp_com_seq1( int fd ) {
    int modem_lines;
    int temp;
    int dsr_status;
    int rc = PNP_COM_OK;
    struct termios portattr;

    DEBUG("initializing 1st PNP sequence\n");
    if (init_port(fd))
      return PNP_COM_FATAL;

    modem_lines = get_serial_lines(fd);

    /* COM port initialization, check for device enumerate */
    
    /* turn on DTR, turn off RTS */
    modem_lines |= TIOCM_DTR;
    modem_lines &= ~TIOCM_RTS;
    set_serial_lines(fd, modem_lines);

    /* wait 200ms for DSR=1 */
    usleep(200000);

    dsr_status = get_serial_lines(fd) & TIOCM_DSR;
    /* see if we got DSR coming up */

    if (!dsr_status) {
	DEBUG("Device did not set DSR\n");
    }

    /* COM port Setup, 1st phase */
    temp = tcgetattr(fd, &portattr);
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* now we set port to be 1200 baud, 7 bits, no parity, 1 stop bit */
    temp = setup_serial_port( fd, 7, &portattr );
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* we drop DTR and RTS */
    modem_lines &= ~( TIOCM_RTS | TIOCM_DTR);
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* bring DTR back up */
    modem_lines |= TIOCM_DTR;
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* Wait for response, 1st phase */
    modem_lines |= TIOCM_RTS;
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    return rc;
}


/* Request for PnP info from serial device                      */
/* See page 6 of the pnpcom doc from Microsoft                  */
/* Always returns PNP_COM_OK                                    */
static int init_pnp_com_seq2( int fd ) {
    int modem_lines;
    int rc = PNP_COM_OK;

    DEBUG("initializing 2nd PNP sequence\n");

    modem_lines = get_serial_lines(fd);

    /* COM port setup, 2nd phase */
    /* turn off DTR and RTS */
    modem_lines &= ~(TIOCM_DTR | TIOCM_RTS);
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* wait for response, 2nd phase */
    /* turn on DTR and RTS */
    modem_lines |= (TIOCM_DTR | TIOCM_RTS);
    set_serial_lines(fd, modem_lines);
    usleep(200000);
    
    return rc;
}


/* Request for PnP info from serial modem device                */
/* Uses ATI9 code, may not do anything but return 'ERROR'       */
/* Return code tells us what happened                           */
/*                                                              */
/* PNP_COM_FATAL      - error, errno has reason                 */
/* PNP_COM_OK         - probe initiated successfully            */
/* PNP_COM_FAIL       - DSR never came on - try alterntives     */
/*                          means (ATI9?) to get PnP string     */
static int init_pnp_com_ati9( int fd ) {
    int modem_lines;
    int temp;
    int done;
    int respindex;
    int starttime;
    unsigned char resp[100], buf[2];
    struct timeval timo;
    struct termios portattr;

    DEBUG("Querying ATI9 info from modem\n");
    modem_lines = get_serial_lines(fd);

    /* turn off RTS */
    modem_lines &= ~TIOCM_RTS;
    set_serial_lines(fd, modem_lines);

    /* wait 200ms for DSR=1 */
    usleep(200000);

    /* now we set port to be 1200 baud, 8 bits, no parity, 1 stop bit */
    temp = tcgetattr(fd, &portattr);
    if (temp < 0) {
	modem_lines |= TIOCM_DTR | TIOCM_RTS;
	set_serial_lines(fd, modem_lines);
	return PNP_COM_FATAL;
    }

    /* goto 1200 baud, 8 bits */
    temp = setup_serial_port( fd, 8, &portattr );
    if (temp < 0) {
	modem_lines |= TIOCM_DTR | TIOCM_RTS;
	set_serial_lines(fd, modem_lines);
	return PNP_COM_FATAL;
    }

    /* turn on DTR and RTS */
    modem_lines = get_serial_lines(fd);
    modem_lines |= TIOCM_RTS | TIOCM_DTR;
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* send the 'AT' command */
    DEBUG("Sending ATI9 command to modem\n");
    
    write(fd, "ATI9\r", 5);
    
    /* start reading - read the AT command back */
    done     = 0;
    respindex= 0;
    starttime=time(NULL);
    memset(resp, 0, sizeof(resp));
    while (!done) {
	timo.tv_sec=0;
	timo.tv_usec=250000;
	if (wait_for_input(fd, &timo) > 0) {
	    temp = read( fd, buf, 1 );
	    if (temp < 0) {
		if (errno != EAGAIN)
		    return PNP_COM_FATAL;
	    } else {
		resp[respindex++] = buf[0];
		resp[respindex] = 0;
	    }
	} else
	    done = 1;

	/* shouldnt run more than 5 seconds */
	if (time(NULL)-starttime > 5 )
	    done = 1;

	if (respindex > 6)
	    done = 1;

	if (strstr(resp, "ATI9\r"))
	    done = 1;

	DEBUG("ATI9 probe ->%d \"%s\"\n",respindex, resp);
    }
    
    /* see if we saw the 'OK' response */
    if (strstr(resp, "("))
	return PNP_COM_OK;
    else
	return PNP_COM_FAIL;
    
    return PNP_COM_OK;
}

/* See if this is a legacy mouse device                         */
/* Only called if the PnP probe above failed                    */
/* We turn off the mouse via RS232 lines, then turn it on       */
/* If it spits out an 'M' character (at 1200 baud, 7N1)         */
/* it could be a mouse.                                         */
/*                                                              */
/* Return code tells us what happened                           */
/*                                                              */
/* PNP_COM_FATAL     - error, errno has reason                  */
/* PNP_COM_OK        - probe saw 'M'                            */
/* PNP_COM_FAIL      - Never saw the 'M' response               */

static int find_legacy_mouse( int fd ) {
    int modem_lines;
    int temp;
    int done;
    int starttime;
    unsigned char resp[2];
    struct timeval timo;
    struct termios portattr;

    DEBUG("looking for a legacy mouse\n");
    
    /* now we set port to be 1200 baud, 7 bits, no parity, 1 stop bit */
    temp = tcgetattr(fd, &portattr);
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* goto 1200 baud, etc etc*/
    temp = setup_serial_port( fd, 7, &portattr );
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* we drop DTR and RTS */
    modem_lines = get_serial_lines(fd);
    modem_lines &= ~( TIOCM_RTS | TIOCM_DTR);
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* bring them DTR back up */
    modem_lines |= TIOCM_DTR | TIOCM_RTS;
    set_serial_lines(fd, modem_lines);

    /* start reading - after first character we quit */
    done     = 0;
    starttime=time(NULL);
    while (!done) {
	timo.tv_sec=0;
	timo.tv_usec=250000;
	if (wait_for_input(fd, &timo) > 0) {
	    temp = read( fd, resp, 1 );
	    if (temp < 0) {
		if (errno != EAGAIN)
		    return PNP_COM_FATAL;
	    } else {
		done = 1;
	    }
	} else
	    done = 1;


	/* shouldnt run more than 2 seconds */
	if (time(NULL)-starttime > 2 )
	    done = 1;
    }
    if (*resp == 'M') {
	DEBUG("Found legacy mouse\n");
	return PNP_COM_OK;
    } else
	return PNP_COM_FAIL;
}

/* See if this is a legacy modem device                         */
/* Only called if the PnP probe above failed                    */
/* We send a '!AT' and see if we get an 'OK' back               */
/*                                                              */
/* Return code tells us what happened                           */
/*                                                              */
/* PNP_COM_FATAL     - error, errno has reason                  */
/* PNP_COM_OK        - probe saw 'OK'                           */
/* PNP_COM_FAIL      - Never saw the 'OK' response              */
static int find_legacy_modem( int fd ) {
    int modem_lines;
    int temp;
    int done;
    int respindex;
    int starttime;
    unsigned char resp[10], buf[2];
    struct timeval timo;
    struct termios portattr;

    DEBUG("looking for a legacy modem\n");
    
    /* now we set port to be 1200 baud, 8 bits, no parity, 1 stop bit */
    temp = tcgetattr(fd, &portattr);
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* goto 1200 baud, 8 bits */
    temp = setup_serial_port( fd, 8, &portattr );
    if (temp < 0) 
	return PNP_COM_FATAL;

    /* turn on DTR and RTS */
    modem_lines = get_serial_lines(fd);
    modem_lines |= TIOCM_RTS | TIOCM_DTR;
    set_serial_lines(fd, modem_lines);
    usleep(200000);

    /* send the 'AT' command */
    DEBUG("Sending AT command to modem\n");
    
    write(fd, "AT\r", 3);
    
    /* start reading - we'll get AT command back first, then modem response */
    done     = 0;
    respindex= 0;
    starttime=time(NULL);
    memset(resp, 0, sizeof(resp));
    while (!done) {
	timo.tv_sec=0;
	timo.tv_usec=250000;
	if (wait_for_input(fd, &timo) > 0) {
	    temp = read( fd, buf, 1 );
	    if (temp < 0) {
		if (errno != EAGAIN)
		    return PNP_COM_FATAL;
	    } else {
		resp[respindex++] = buf[0];
	    }
	} else
	    done = 1;

	/* shouldnt run more than 5 seconds */
	if (time(NULL)-starttime > 5 )
	    done = 1;

	if (respindex > 9)
	    done = 1;
    }
    
    /* see if we saw the 'OK' response */
    if (strstr(resp, "OK"))
	return PNP_COM_OK;
    else
	return PNP_COM_FAIL;
}

/* retrieve the PnP ID string */
/* timeout after 3 seconds   */
/* should probably set a 200 msec timeout per char, as spec says */
/* if no char received, we're done                              */
static int read_pnp_string( int fd, unsigned char *pnp_string, int *pnp_len, int pnp_stringbuf_size ) {
    int     pnp_index;
    int     temp, done, counter;
    int     seen_start;
    time_t  starttime;
    struct timeval timo;
    unsigned char buf[80];
    unsigned char end_char;

    DEBUG("Attempting to read PNP ID string\n");
    
    /* see if we have any input waiting */
    pnp_index  = 0;
    seen_start = 0;
    done       = 0;
    end_char   = 0;
    starttime=time(NULL);
    while (!done) {
	timo.tv_sec=0;
	timo.tv_usec=250000;
	if (wait_for_input(fd, &timo) > 0) {
	    temp = read( fd, buf, 1 );
	    if (temp < 0) {
		if (errno != EAGAIN)
		    return PNP_COM_FAIL;
	    } else {
		for (counter=0; counter < temp; counter++) {
		    pnp_string[pnp_index++] = buf[counter];
		    if (seen_start) {
			if (buf[counter] == end_char) {
			    done=1;
			    break;
			}
		    } else {
			if (buf[counter] == BeginPnP1) {
			    seen_start = 1;
			    end_char   = EndPnP1;
			} else if (buf[counter] == BeginPnP2) {
			    seen_start = 1;
			    end_char   = EndPnP2;
			}
		    }
		}
	    }
	} else
	    done = 1;
	
	/* shouldnt run more than 3 seconds */
	if (time(NULL)-starttime > 3 )
	    done = 1;

	if (pnp_index >= pnp_stringbuf_size)
	    done = 1;
    }
    pnp_string[pnp_index] = 0;
   *pnp_len=pnp_index;
    return PNP_COM_OK;
}

/* parse the PnP ID string into components */
static int parse_pnp_string( unsigned char *pnp_id_string, int pnp_len,
		     struct pnp_com_id *pnp_id ) {
    unsigned char *p1, *p2;
    unsigned char *start;
    unsigned char *end;
    unsigned char *curpos;
    unsigned char *endfield;
    unsigned char *temppos;
    unsigned char *pnp_string;
    unsigned char end_char;

    int no_more_extensions=0;
    int stage;
    int len;
    unsigned short int checksum;
    char hex_checksum[5];

    char extension_delims[] = {EndPnP1, EndPnP2, ExtendPnP1, ExtendPnP2, 0};
    char end_delims[] = {EndPnP1, EndPnP2, 0};
    unsigned char* p1end = NULL;
    unsigned char* p2end = NULL;

    /* clear out pnp_id */
    memset(pnp_id, 0, sizeof(*pnp_id));

    /* copy pnp_string to temp space */
    pnp_string = alloca(pnp_len+1);
    memcpy(pnp_string, pnp_id_string, pnp_len+1);
    
    /* first find the start of the PnP part of string */
    p1 = memchr( pnp_string, BeginPnP1, pnp_len );
    p2 = memchr( pnp_string, BeginPnP2, pnp_len );


    if (p1) {
        int p_len = pnp_len - (p1 - pnp_string);
        p1end = memchr(p1, EndPnP1, p_len);
    }
    if (p2) {
        int p_len = pnp_len - (p2 - pnp_string);
        p2end = memchr(p2, EndPnP2, p_len);
    }

    /* use the one which points nearest to start of the string */
    /* and is actually defined                                */
    if ( p1 && p1end && p2 && p2end ) {
	start = (p1 < p2) ? p1 : p2;
    } else if ( p1 && p1end )
	start = p1;
    else if ( p2 && p2end )
	start = p2;
    else
	start = NULL;

    /* if no start then we're done */
    if (!start)
	return -1;
    
    /* the length of the initial part cannot be more than 17 bytes */
    if ((start - pnp_string) > 17)
	return -1;

    /* setup end character we are looking for based on the start character */
    if (start == p2) {
      pnp_id->xlate_6bit = 1;
      end_char = EndPnP2;
      /* we need to xlate data in PnP fields */
      /* remember to skip the revision fields (bytes 1 and 2 after start) */
      temppos=start;
      while (1) {
	  if (*temppos == EndPnP2) {
	     *temppos += 0x20;
	      break;
	  } else if (temppos != start+1 && temppos != start+2 )
	     *temppos += 0x20;
	  
	  temppos++;
      }	      
    } else {
      pnp_id->xlate_6bit = 0;
      end_char = EndPnP1;
    }
    
    /* move everything before the start of the PnP block */
    memcpy(pnp_id->other_id, pnp_string, start-pnp_string);
    pnp_id->other_len = start - pnp_string;

    /* now we get the PnP fields - all were zero'd out above */
    curpos = start+1;
    memcpy(pnp_id->pnp_rev,curpos,2);       curpos += 2;
    memcpy(pnp_id->eisa_id,curpos,3);       curpos += 3;
    memcpy(pnp_id->product_id,curpos,4);    curpos += 4;
    /* now we see if have extension fields */
    no_more_extensions = 0;
    stage = 0;
    while (!no_more_extensions) {
	if (*curpos == ExtendPnP1 || *curpos == ExtendPnP2) {
	    curpos++;
	    endfield = strpbrk(curpos, extension_delims);
	    if (!endfield)
		return -1;
	    /* if we reached the end of all PnP data, back off */
	    /* cause there is a checksum at the end of extension data */
	    if (*endfield == EndPnP1 || *endfield == EndPnP2)
		endfield -= 2;
	} else
	    break;

	len = endfield - curpos;
	switch (stage) {
	  case 0:
	    if (len != 8 && len != 0 )
		return -1;
	    
	    memcpy(pnp_id->serial_number,curpos,len);
	    curpos += len;
	    break;

	  case 1:
	    if (len > 33)
		return -1;
	    memcpy(pnp_id->class_name, curpos, len);
	    curpos = endfield;
	    break;

	  case 2:
	    if (len > 41)
		return -1;
	    memcpy(pnp_id->driver_id, curpos, len);
	    curpos = endfield;
	    break;

	  case 3:
	    if (len > 41)
		return -1;
	    memcpy(pnp_id->user_name, curpos, len);
	    curpos = endfield;
	    break;
	}
	stage++;
    }

    /* now find the end of all PnP data */
    end = strpbrk(curpos, end_delims);
    if (!end)
	return -1;
    
    /* if we had any extensions, we expect an checksum */
    if (stage != 0) {
	/* copy checksum into struct */
	memcpy(pnp_id->checksum, curpos, 2);
	
	/* compute the checksum as the sum of all PnP bytes, excluding */
	/* the two byte checksum. */
	checksum = 0;
	for (temppos=start; temppos <= end; temppos++) {
	    /* skip checksum in calculation */
	    if (temppos == (end-2) || temppos == (end-1))
		continue;
	    /* dont xlate the revision at start */
	    if (temppos != (start+1) && temppos != (start+2))
		checksum += *temppos - ((pnp_id->xlate_6bit) ? 0x20 : 0);
	    else
		checksum += *temppos;
	}
	sprintf(hex_checksum, "%.2X", checksum & 0xff);
	if (strncmp(hex_checksum, pnp_id->checksum, 2))
	    return -1;
    }

    /* checksum was ok, so we're done */
    return 0;
}

static int attempt_pnp_retrieve(int fd, char *pnp_string, int *pnp_strlen, int pnp_stringbuf_size) {
    int pnp_probe_status;
    struct pnp_com_id pnp_id;
    int tried_at_prodding=0, give_up=0;
    
    DEBUG("Attempting PNP information retrieval\n");

    while (!give_up) {
	pnp_probe_status = init_pnp_com_seq1(fd);
	if (pnp_probe_status == PNP_COM_FATAL)
	    return PNP_COM_FATAL;
	pnp_probe_status = read_pnp_string(fd, pnp_string, pnp_strlen,
					   pnp_stringbuf_size);
	if (pnp_probe_status == PNP_COM_FAIL) {
	    init_pnp_com_seq2(fd);	    /* always succeeds */

	    pnp_probe_status = read_pnp_string(fd, pnp_string, pnp_strlen,
					       pnp_stringbuf_size);
	}

	if (*pnp_strlen == 1 && pnp_string[0] == 'M') /* legacy mouse */
	    return PNP_COM_OK;
	/* see if we got anything useful, if not try AT command */
	/* to prod device into correct serial params */
	if (parse_pnp_string( pnp_string, *pnp_strlen, &pnp_id )<0) {
	    DEBUG("That failed.\n");
	    if (!tried_at_prodding) {
		DEBUG("Prod modem with AT command.\n");
		write(fd, "AT\r", 3);
		tried_at_prodding=1;
	    } else
		give_up = 1;
	} else
	    return PNP_COM_OK;
    }   

    /* normal PNP detection has failed. */
    /* try sending a ATI9 code to the modem to see if we get PnP id back */
    init_pnp_com_ati9(fd);
    read_pnp_string(fd, pnp_string, pnp_strlen, pnp_stringbuf_size );
    if (parse_pnp_string( pnp_string, *pnp_strlen, &pnp_id )<0) {
	*pnp_strlen = 0;
	pnp_string[0] = 0;
	return PNP_COM_FAIL;
    } else
	return PNP_COM_OK;
}

struct device *serialProbe(enum deviceClass probeClass, int probeFlags,
			   struct device *devlist) {
    int fd;
    int temp;
    int pnp_strlen;
    int devicetype=-1;
    unsigned char pnp_string[100];
    char port[20];
    struct termios origattr;
    struct pnp_com_id pnp_id;
    struct serialDevice *serdev;
    struct stat sb;
    int maj, twelve=12;
    int console=-1;
    int stdin_line=-1;
    struct serial_struct si;

    DEBUG("Probing for serial ports\n");
    
    if (probeFlags & PROBE_SAFE) return devlist;

    if (!access("/initrd/rhgb-socket",F_OK)) return devlist;

    /* Are we on a serial console? */
    fstat(0,&sb);
    maj = major(sb.st_rdev);
    if (maj != 4 && (maj < 136 || maj > 143)) {
	if (ioctl (0, TIOCLINUX, &twelve) < 0) {
	    if (ioctl (0, TIOCGSERIAL, &si) >= 0) {
		if (si.line > 0) {
		    stdin_line = 1 << si.line;
		} else {
		    stdin_line = 0;
		}
	    } else stdin_line = 0;
	}
    }
	
    fd=open("/dev/console",O_RDWR);
    if (fd != -1) {
	fstat(fd,&sb);
	maj = major(sb.st_rdev);
	if (maj != 4 && (maj < 136 || maj > 143)) {
	    if (ioctl (fd, TIOCLINUX, &twelve) < 0) {
		if (ioctl (fd, TIOCGSERIAL, &si) >= 0) {
		    if (si.line > 0) {
			console = 1 << si.line;
		    } else {
			console = 0;
		    }
		} else console = 0;
	    }
	}
	close(fd);
    }
    
    
    if (
	(probeClass & CLASS_OTHER) ||
	(probeClass & CLASS_MOUSE) ||
	(probeClass & CLASS_MODEM) ||
	(probeClass & CLASS_PRINTER)
	) {
	int x;
	
	for (x=0; x<=3 ; x++) {
	    struct stat sbuf;
	    char lockfile[32];
	    if (x==console || x==stdin_line) continue;
	    snprintf(port,20,"/dev/ttyS%d",x);

	    /* Make sure it's not in use */
	    snprintf(lockfile,32,"/var/lock/LCK..ttyS%d",x);
	    if (!stat(lockfile,&sbuf)) {
		DEBUG("Port %s in use, skipping probe.\n",
		      port);
		continue;
	    }
	    memset(lockfile,'\0',32);
	    if (readlink("/dev/modem",lockfile,32)>0) {
		    if (!strcmp(basename(port),basename(lockfile))) {
			snprintf(lockfile,32,"/var/lock/LCK..modem");
			if (!stat(lockfile,&sbuf)) {
			    DEBUG("Port %s in use, skipping probe.\n",
				  port);
			    continue;
			}
		    }
	    }
	    
	    if ((fd=open_serial_port(port)) < 0) {
		continue;
	    }
	    /* save the current state of the port */
	    temp = tcgetattr(fd, &origattr);
	    if (temp < 0) {
		DEBUG("unable to retrieve port attributes...no port present?\n");
		close(fd);
		continue;
	    }
	    

	    /* try twiddling RS232 control lines and see if it talks to us */
	    devicetype=-1;
	    pnp_strlen = 0;
	    if (attempt_pnp_retrieve( fd, pnp_string, &pnp_strlen,
				      sizeof(pnp_string) - 1 ) == PNP_COM_FATAL)
		goto endprobe;
	    
	    /* see if we found any PnP signature */
	    if (pnp_strlen != 0) {
		/* fill in the PnP com structure */
		if (parse_pnp_string( pnp_string, pnp_strlen, &pnp_id )<0) {
		    DEBUG("Got PNP data back, but failed to parse.  Aborting\n");
		    goto endprobe;
		} else {
		    char *foo;
		    int len;

		    DEBUG("PNP data parsed.\n");
		    serdev = serialNewDevice(NULL);

		    if (pnp_id.user_name[0]) {
			serdev->pnpdesc = strdup(pnp_id.user_name);
			len = strlen(pnp_id.eisa_id) +
			    strlen(pnp_id.product_id) +
			    strlen(pnp_id.user_name) + 3;
			foo = malloc(len);
			snprintf(foo,len,"%s|%s %s",pnp_id.eisa_id,
				 pnp_id.product_id,pnp_id.user_name);
		    } else {
			len = strlen(pnp_id.eisa_id) +
			    strlen(pnp_id.product_id) + 3;
			foo = malloc(len);
			snprintf(foo,len,"%s|%s",pnp_id.eisa_id,
				 pnp_id.product_id);
		    }
		    if (serdev->desc) free(serdev->desc);		    
		    serdev->desc=strdup(foo);
		    serdev->device=strdup(port+5);
		    if (serdev->driver) free(serdev->driver);
		    serdev->driver=strdup("ignore");
		    serdev->pnpmfr = strdup(pnp_id.eisa_id);
		    serdev->pnpmodel = strdup(pnp_id.product_id);
			
		    free(foo);
		    foo=pnp_id.product_id;
		    if (pnp_id.driver_id) {
			if (strstr(pnp_id.driver_id,"PNP"))
			  foo = strstr(pnp_id.driver_id,"PNP")+3;
			    serdev->pnpcompat = strdup(pnp_id.driver_id);
		    }

		    if (*pnp_id.other_id == 'M' ||
			!strncmp(pnp_id.class_name, "MOUSE", 5) ||
			!strncmp(foo, "0F", 2)) {
			serdev->type = CLASS_MOUSE;
			if (!strncmp(serdev->desc, "|", 1)) {
			    free(serdev->desc);
			    serdev->desc=strdup("Generic Serial Mouse");
			}
			if (serdev->driver) free(serdev->driver);
			serdev->driver = strdup("generic");
		    }
		    else if (!strncmp(pnp_id.class_name, "MODEM", 5) ||
			     !strncmp(foo, "C", 1))
			serdev->type = CLASS_MODEM;
		    else if (!strncmp(pnp_id.class_name, "PRINTER", 7))
			serdev->type = CLASS_PRINTER;
		    else
			serdev->type = CLASS_OTHER;
		    if (serdev->type & probeClass) {
			if (devlist)
			    serdev->next = devlist;
			devlist = (struct device *)serdev;
			if (probeFlags & PROBE_ONE) {
			    tcsetattr(fd, TCSANOW, &origattr);
			    tcflush(fd, TCIOFLUSH);
			    close(fd);
			    return devlist;
			}
		    } else {
			serdev->freeDevice(serdev);
		    }
		    goto endprobe;
		}
	    } else {
		DEBUG("No PNP data received.\n");
		/* try to find a legacy device */

		temp = find_legacy_mouse(fd);
		if (temp == PNP_COM_FATAL) {
			goto endprobe;
		} else if (temp == PNP_COM_OK) {
		    if (probeClass & CLASS_MOUSE) {
			serdev=serialNewDevice(NULL);
			serdev->type = CLASS_MOUSE;
			serdev->device = strdup(port+5);
			serdev->driver= strdup("generic");
			serdev->desc = strdup("Generic Serial Mouse");
			if (devlist)
			    serdev->next = devlist;
			devlist = (struct device *)serdev;
			if (probeFlags & PROBE_ONE) {
			    tcsetattr(fd, TCSANOW, &origattr);
			    tcflush(fd, TCIOFLUSH);
			    close(fd);
			    return devlist;
			}
		    }
		    goto endprobe;
		} else {
		    DEBUG("Didn't see a legacy mouse.\n");
		    
		    temp = find_legacy_modem(fd);
		    if (temp == PNP_COM_FATAL) {
			goto endprobe;
		    } else if (temp == PNP_COM_OK) {
			DEBUG("Legacy modem signature seen.\n");
			if (probeClass & CLASS_MODEM) {
			    serdev=serialNewDevice(NULL);
			    serdev->type = CLASS_MODEM;
			    serdev->device = strdup(port+5);
			    serdev->driver= strdup("ignore");
			    serdev->desc = strdup("Generic Serial Modem");
			    if (devlist)
				serdev->next = devlist;
			    devlist = (struct device *)serdev;
			    if (probeFlags & PROBE_ONE) {
				tcsetattr(fd, TCSANOW, &origattr);
				tcflush(fd, TCIOFLUSH);
				close(fd);
				return devlist;
			    }
			}
			goto endprobe;
		    } else {
			DEBUG("Didnt see a legacy modem, game over.\n");
		    }
		}
	    }
	endprobe:
	    DEBUG("Restoring original port attributes\n");
	    tcsetattr(fd, TCSANOW, &origattr);
	    tcflush(fd, TCIOFLUSH);
	    close(fd);
	}
    }
    return devlist;
}
