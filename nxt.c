/**
 * nxt.c          NXT USB communication routines
 *
 * Copyright 2009 Muhiy-eddine Cherik <muhiy-eddine.cherik@heig-vd.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "nxt.h"

#define NXT_HANDLE_ERR(expr, nxt, msg)     \
  do {                                     \
    nxt_error_t nxt__err_temp = (expr);    \
    if (nxt__err_temp)                     \
      handle_error(nxt, msg, nxt__err_temp);  \
  } while(0)


int handle_error(nxt_t *nxt, char *msg, nxt_error_t err)
{
	printf("%s: %s\n", msg, nxt_str_error(err));
	if (nxt != NULL)
		nxt_close0(nxt);
	exit(err);
}

/**
 * Convert an USB string into a C formatted string
 */
void USBToStr(char * str)
{
	int i, len = (str[0] - 2)/2;
	for( i=0; i< len; i++)
		str[i] = str[i*2+2];
	str[len+1] = '\0';
}

/**
 * Intialization
 */
void lejos_init(nxt_t **nxt)
{
	NXT_HANDLE_ERR(nxt_init(nxt), NULL,"Error during library initialization");
}

/**
 * Scan for NXT
 */
void lejos_search(nxt_t *nxt)
{
	nxt_error_t err; 

	printf("Searching NXT...");
	err = nxt_find_nth(nxt, 0);
	if (err)
	{
		if (err == NXT_NOT_PRESENT)
			printf("NXT not found. Is it properly plugged in via USB?\n");
		else
			NXT_HANDLE_ERR(0, NULL, "Error while scanning for NXT");
			exit(1);
	}

	USBToStr(nxt_name(nxt));
	USBToStr(nxt_serial_no(nxt));
	printf("\nFound lejos powered NXT\nname: %s\tID: %s\n",nxt_name(nxt),nxt_serial_no(nxt));
}

/**
 * Open connection to lejOS
 */
void lejos_open(nxt_t *nxt)
{
	NXT_HANDLE_ERR(nxt_open0(nxt), NULL, "Error while trying to connect to NXT\nTry to launch app with root user");

	printf("Connected to %s\n\n",nxt_name(nxt));
}

/**
 * close connection with lejOS
 */
void lejos_close(nxt_t *nxt)
{
	NXT_HANDLE_ERR(nxt_close0(nxt), NULL,
		"Error while closing connection to NXT");

	printf("Connected to %s\n\n",nxt_name(nxt));
}

/**
 * Send order to NXT robot
 */
void send_cmd(nxt_t *nxt, char cmd)
{
	printf("send order... ");
	nxt_send_buf(nxt, &cmd, 1);
	printf("OK\n\n");
}

/**
 * Get robot state (battery low, hit detection)
 
void recv_state(nxt_t *nxt, char *state){
	printf("-\n");
	nxt_recv_buf(nxt, state, 1);
	printf("--\n");
}
*/
void *recv_state(void *nxtinf){
	
	NXTINFO *i = ((NXTINFO *)(nxtinf));	
	
	printf("state-\n");
	while(1)
		nxt_recv_buf(i->nxt, i->state, 1);
}
