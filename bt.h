/**
 * bt.h           Bluetooth management (connection, record voice, etc.)
 *                and recognition module
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
 
#ifndef BT_H
#define BT_H

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sco.h>
#include <bluetooth/rfcomm.h>

#include <math.h>
#include <pthread.h>

#include "rv.h"

#define HS_BTADDR       "00:0C:78:76:30:AE"
#define MAX_DATA_RECORD	16000 /*24576*/

/* threshold parameter -> well defined, well result :) */	
#define THSLD_MAX	250

typedef struct {
	int rd, sd;	                /* RFCOMM and SCO descriptor */
	uint16_t sco_handle, sco_mtu;	/* SCO parameters */
	int cmd;                        /* NXT order */
	int running    :1;		/* application state */
	int isPlaying  :1;              /* playing state */
	int isrecording:1;		/* recording state*/
	pthread_cond_t play_cond;	/* condition on playing excp msg */
	pthread_cond_t exec_cond;	/**/
	pthread_mutex_t state_mtx;      /* NXT process mutex */
	ps_decoder_t *ps;               /* RV object */
}SHAREDINFO;

int hs_scan(void);
int hs_connect(int *rd, int *sd, uint16_t *sco_handle, uint16_t *sco_mtu);
void *hs_data_rec(void *info);
void hs_data_play(SHAREDINFO *info, char const *sndfile);
void hs_disconnect(SHAREDINFO info);

#endif
