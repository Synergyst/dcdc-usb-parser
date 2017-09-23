/*
 * Copyright (c) 2011 by Mini-Box.com, iTuner Networks Inc.
 * Written by Nicu Pavel <npavel@mini-box.com>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <usb.h>

#include "dcdc-usb.h"

void showhelp(char *prgname)
{

    printf ("Usage: %s [OPTION]\n", prgname);
    printf ("Options:.\n");
    printf (" -a \t show all device settings\n");
    printf (" -h \t show help message\n");
    printf (" -v \t set voltage out value (float)\n");
}

int main(int argc, char **argv)
{
    struct usb_dev_handle *h;
    unsigned char data[MAX_TRANSFER_SIZE];
    int ret;
    char *s;
    int arg = 0, showall = 0, setvoltage = 0;
    double vout = 5.0;

    while ( ++arg < argc )
    {
        s = argv[arg];
        if (strncmp(s, "-a", 2) == 0)
            showall = 1;
        if (strncmp(s, "-h", 2) == 0)
        {
            showhelp(argv[0]);
            return 0;
        }
        if (strncmp(s, "-v", 2) == 0)
            if (arg + 1 < argc)
            {
                arg++;
                vout = strtod(argv[arg], NULL);
                if (vout != 0)
                    setvoltage = 1;
            }
    }
    h = dcdc_connect();

    if (h == NULL)
    {
        fprintf(stderr, "Cannot connect to DCDC-USB\n");
        return 1;
    }

    if (dcdc_setup(h) < 0)
    {
        fprintf(stderr, "Cannot setup device\n");
        return 2;
    }

    if (showall)
    {
        if ((ret = dcdc_get_status(h, data, MAX_TRANSFER_SIZE)) <= 0)
        {
            fprintf(stderr, "Failed to get status from device\n");
            return 3;
        }
        dcdc_parse_data(data, ret);
        return 0;
    }

    if (setvoltage)
    {
        fprintf(stderr, "setting output voltage to: %.2f\n", vout);
        dcdc_set_vout(h, vout);

    }

    if ((ret = dcdc_get_vout(h, data, MAX_TRANSFER_SIZE)) <= 0)
    {
            fprintf(stderr, "Failed to get voltage from device\n");
            return 3;
    }
    dcdc_parse_data(data, ret);

    return 0;
}

/*
 * Shared-object library blocks below
 */

#define P(t, v...) fprintf(stderr, t "\n", ##v)
static int bytes2int(unsigned char c1, unsigned char c2)
{
    int i = c1;
    i = i << 8;
    i = i | c2;

    return i;
}

static int byte2bits(unsigned char c)
{
    int i,n = 0;
    for (i = 0; i < 8; i++)
    {
        n = n * 10;
        if ((c >> i) & 1)
            n = n + 1;
    }
    return n;
}

static double byte2vout(unsigned char c)
{
    double rpot = ((double) c) * CT_RP / (double)257 + CT_RW;
    double voltage = (double)80 * ((double) 1 + CT_R1/(rpot + CT_R2));
    voltage = floor(voltage);
    return voltage/100;
}

void dcdc_parse_values(unsigned char *data)
{
    int mode, state, status;
    float ignition_voltage, input_voltage, output_voltage;

    mode = data[1];
    state = data[2];
    input_voltage = (float) data[3] * 0.1558f;
    ignition_voltage = (float) data[4] * 0.1558f;
    output_voltage = (float) data[5] * 0.1170f;
    status = data[6];
    switch(mode & 0x03)
    {
        case 0: P("mode: 0 (dumb)"); break;
        case 1: P("mode: 1 (automotive)"); break;
        case 2: P("mode: 2 (script)"); break;
        case 3: P("mode: 3 (ups)");break;
    }
    P("time config: %d", (mode >> 5) & 0x07);
    P("voltage config: %d", (mode >> 2) & 0x07);
    P("state: %d", state);
    P("input voltage: %.2f", input_voltage);
    P("ignition voltage: %.2f", ignition_voltage);
    P("output voltage: %2f", output_voltage);
    P("power switch: %s", ((status & 0x04) ? "On":"Off"));
    P("output enable: %s", ((status & 0x08) ? "On":"Off"));
    P("aux vin enable %s", ((status & 0x10) ? "On":"Off"));
    P("status flags 1: %d", byte2bits(data[6]));
    P("status flags 2: %d", byte2bits(data[7]));
    P("voltage flags: %d", byte2bits(data[8]));
    P("timer flags: %d", byte2bits(data[9]));
    P("flash pointer: %d", data[10]);
    P("timer wait: %d", bytes2int(data[11], data[12]));
    P("timer vout: %d", bytes2int(data[13], data[14]));
    P("timer vaux: %d", bytes2int(data[15], data[16]));
    P("timer pw switch: %d", bytes2int(data[17], data[18]));
    P("timer off delay: %d", bytes2int(data[19], data[20]));
    P("timer hard off: %d", bytes2int(data[21], data[22]));
    P("version: %d.%d", ((data[23] >> 5) & 0x07), (data[23] & 0x1F));
}

void dcdc_parse_cmd(unsigned char *data)
{
    if (data[1] == 0)
    {
        if (data[2] == CMD_READ_REGULATOR_STEP)
        {
            P("regulator step: %d", data[3]);
        }
        else
        {
            P("output voltage: %.2f", byte2vout(data[3]));
        }
    } else {
        if (data[2] == CMD_READ_REGULATOR_STEP) {
            P("regulator step not defined");
        }
    }
}

void dcdc_parse_internal_msg(unsigned char *data)
{
    P("Parsing INTERNAL MESSAGE: Not implemented");
}

void dcdc_parse_mem(unsigned char *data)
{
    P("Parsing MEM READ IN: Not implemented");
}

/*
 * dcdc-usb-proto.c block below
 */

/* transforms user value to a value understood by device */
static int vout2dev(double vout)
{
    double rpot = (double)0.8 * CT_R1 / (vout - (double)0.8) - CT_R2;
    double result = (257 * (rpot-CT_RW) / CT_RP);

    if (result<0) result = 0;
    if (result>255) result = 255;

    return (unsigned char)result;
}

static int dcdc_send_command(struct usb_dev_handle *h, unsigned char cmd, unsigned char val)
{
    unsigned char c[3];
    c[0] = DCDCUSB_CMD_OUT;
    c[1] = cmd;
    c[2] = val;

    return dcdc_send(h, c, 3);
}

int dcdc_get_status(struct usb_dev_handle *h, unsigned char *buf, int buflen)
{
    unsigned char c[2];
    int ret = 0;

    if (buflen < MAX_TRANSFER_SIZE)
        return -1;

    c[0] = DCDCUSB_GET_ALL_VALUES;
    c[1] = 0;

    if (dcdc_send(h, c, 2) < 0)
    {
        fprintf(stderr, "Cannot send command to device\n");
        return -2;
    }

    if ((ret = dcdc_recv(h, buf, MAX_TRANSFER_SIZE, 1000)) < 0)
    {
        fprintf(stderr, "Cannot get device status\n");
        return -3;
    }

    return ret;
}

int dcdc_set_vout(struct usb_dev_handle *h, double vout)
{
    if (vout < 5) vout = 5;
    if (vout > 24) vout = 24;

    return dcdc_send_command(h, CMD_WRITE_VOUT, vout2dev(vout));
}

/* value will be shown on dcdc_parse_data() */
int dcdc_get_vout(struct usb_dev_handle *h, unsigned char *buf, int buflen)
{
    int ret = 0;

    if (buflen < MAX_TRANSFER_SIZE)
        return -1;

    if ((ret = dcdc_send_command(h, CMD_READ_VOUT, 0)) < 0)
        return ret;

    ret = dcdc_recv(h, buf, MAX_TRANSFER_SIZE, 1000);

    return ret;
}

int dcdc_parse_data(unsigned char *data, int size)
{
    if (data == NULL)
        return -1;

#ifdef DEBUG
    int i;
    for (i = 0; i < size; i++)
    {
        if (i % 8 == 0) fprintf(stderr, "\n");
        fprintf(stderr, "[%02d] = 0x%02x ", i, data[i]);
    }
    fprintf(stderr, "\n");
#endif

    switch(data[0])
    {
        case DCDCUSB_RECV_ALL_VALUES: dcdc_parse_values(data);break;
        case DCDCUSB_CMD_IN: dcdc_parse_cmd(data); break;
        case INTERNAL_MESG: dcdc_parse_internal_msg(data); break;
        case DCDCUSB_MEM_READ_IN: dcdc_parse_mem(data); break;
        default:
            fprintf(stderr, "Unknown message\n");
    }

    return 0;
}

/*
 * dcdc-usb-comm.c block below
 */

#define P(t, v...) fprintf(stderr, t "\n", ##v)

int dcdc_send(struct usb_dev_handle *h, unsigned char *data, int size)
{
    if (data == NULL)
        return -1;

    return usb_interrupt_write(h, USB_ENDPOINT_OUT + 1, (char *) data, size, 1000);
}

int dcdc_recv(struct usb_dev_handle *h, unsigned char *data, int size, int timeout)
{
    if (data == NULL)
        return -1;

    return usb_interrupt_read(h, USB_ENDPOINT_IN + 1, (char *) data, size, timeout);
}

struct usb_dev_handle * dcdc_connect()
{
    struct usb_bus *b;
    struct usb_device *d;
    struct usb_dev_handle *h = NULL;

    usb_init();
    usb_set_debug(0);
    usb_find_busses();
    usb_find_devices();

    for (b = usb_get_busses(); b != NULL; b = b->next)
    {
        for (d = b->devices; d != NULL; d = d->next)
        {
            if ((d->descriptor.idVendor == DCDC_VID) &&
               (d->descriptor.idProduct == DCDC_PID))
            {
                h = usb_open(d);
                break;
            }
        }
    }
        return h;
}

int dcdc_setup(struct usb_dev_handle *h)
{
    char buf[65535];

    if (h == NULL)
        return -1;

    if (usb_get_driver_np(h, 0, buf, sizeof(buf)) == 0)
    {
        if (usb_detach_kernel_driver_np(h, 0) < 0)
        {
            fprintf(stderr, "Cannot detach from kernel driver\n");
            return -2;
        }
    }

    if (usb_set_configuration(h, 1) < 0)
    {
        fprintf(stderr, "Cannot set configuration 1 for the device\n");
        return -3;
    }

    usleep(1000);

    if (usb_claim_interface(h, 0) < 0)
    {
        fprintf(stderr, "Cannot claim interface 0\n");
        return -4;
    }

    if (usb_set_altinterface(h, 0) < 0)
    {
        fprintf(stderr, "Cannot set alternate configuration\n");
        return -5;
    }

    if (usb_control_msg(h, USB_TYPE_CLASS + USB_RECIP_INTERFACE,
                0x000000a, 0x0000000, 0x0000000, buf, 0x0000000, 1000)
        < 0)
    {
        fprintf(stderr, "Cannot send control message\n");
        return -6;
    }

    return 0;
}
