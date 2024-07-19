/**
 * nxt.h          File header for NXT robot communication
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
 
#ifndef NXT_H
#define NXT_H

#include <stdio.h>
#include <stdlib.h>

#include <nxt/error.h>
#include <nxt/lowlevel.h>
#include <nxt/samba.h>

/* Robot orders */
#define NO_ORDER       -2
#define UNKNOWN_ORDER  -1

#define STOP		0
#define GO		1
#define BACK		2
#define LEFT		3
#define RIGHT		4

/* robot sent codes */
#define NORMAL		9
#define LOWBATTERY	73
#define HIT		89

typedef struct {
	nxt_t *nxt;
	char  *state;

}NXTINFO;

int  handle_error(nxt_t *nxt, char *msg, nxt_error_t err);
void USBToStr(char *str);

void lejos_init(nxt_t **nxt);
void lejos_search(nxt_t *nxt);
void lejos_open(nxt_t *nxt);
void lejos_close(nxt_t *nxt);

void send_cmd(nxt_t *nxt, char cmd);
//void recv_state(nxt_t *nxt, char *state);
void *recv_state(void *nxtinf);

#endif
