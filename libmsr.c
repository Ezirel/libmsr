#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "libmsr.h"

int
msr_dumpbits (uint8_t * buf, int len)
{
	int		bytes, i;

	for (bytes = 0; bytes < len; bytes++) {

		/*
		 * Note: we want to display the bits in the order in
		 * which they're read off the card, which means we
		 * have to decode each byte from most significant bit
		 * to least significant bit.
		 */

		for (i = 7; i > -1; i--) {
			if (buf[bytes] & (1 << i))
				printf("1");
			else
				printf("0");
		}
	}
	printf ("\n");
	return (0);
}

int
msr_getbit (uint8_t * buf, uint8_t len, int bit)
{
	int		byte;
	int		b;

	if (bit > (len * 8)) {
		printf ("%d > %d\n", bit, len * 8);
		return (-1);
	}

	byte = bit / 8;
	b = 7 - (bit % 8);
	if (buf[byte] & (1 << b))
		return (1);

	return (0);
}

int
msr_setbit (uint8_t * buf, uint8_t len, int bit, int val)
{
	int		byte;
	int		b;

	if (bit > (len * 8)) {
		printf ("%d > %d\n", bit, len * 8);
		return (-1);
	}

	byte = bit / 8;
	b = 7 - (bit % 8);

	if (val)
		buf[byte] |= 1 << b;
	else
		buf[byte] &= ~(1 << b);

	return (0);
}


int
msr_decode(uint8_t * inbuf, uint8_t inlen,
    uint8_t * outbuf, uint8_t * outlen, int bpc)
{
	uint8_t * b;
	uint8_t len;
	int ch = 0;
	char byte = 0;
	int i, x;

	len = inlen;
	b = inbuf;
	x = 0;

	for (i = 0; i < len * 8; i++) {
		byte |= msr_getbit (b, len, i) << ch;
		if (ch == (bpc - 1)) {
			/* Strip the parity bit */
			byte &= ~(1 << ch);
			if (bpc < 7)
				byte |= 0x30;
			else {
				if (byte < 0x20)
					byte |= 0x20;
				else {
					byte |= 0x40;
					byte -= 0x20;
				}
			}

			outbuf[x] = byte;
			x++;
			/* Don't overflow output buffer */
			if (x == *outlen)
				break;
#ifdef MSR_DEBUG
			printf ("%c", byte);
#endif
			ch = 0;
			byte = 0;
		} else
			ch++;
	}

#ifdef MSR_DEBUG
	printf ("\n");
#endif

	/* Output buffer was too small. */
	if (x == *outlen)
		return (-1);
	*outlen = x;

	return (0);
}