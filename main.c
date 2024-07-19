/**
 *  main.c        Main application to command NXT robot
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
#include "bt.h"
#include "rv.h"

#define NXTMSGDIR "/usr/local/bin"

int main(int argc, char *argv[])
{
	pthread_t task_bt, task_state;
	nxt_t *nxt;
	SHAREDINFO info;
	NXTINFO nxtinf;

	int prev_cmd = NO_ORDER;
	info.cmd = STOP;

	char state = 0;

	/* state flags init */
	info.isrecording = 0;
	info.isPlaying   = 0;
	info.running     = 1;

	/* init mutexes */
	pthread_mutex_init(&(info.state_mtx), NULL);
	pthread_cond_init(&(info.play_cond), NULL);
	pthread_cond_init(&(info.exec_cond), NULL);

	/****************************************************************************/
	/************************** Initializations *********************************/
	/****************************************************************************/
	
	/* pocketphinx load init */
	rv_init(&(info.ps));	   

	/* connect to NXT */
	lejos_init(&nxt);
	lejos_search(nxt);
	lejos_open(nxt);

	nxtinf.nxt   = nxt;
	nxtinf.state = &state;  

	/* Connect to headset */
	if(hs_connect(&(info.rd), &(info.sd), &(info.sco_handle), &(info.sco_mtu)) != -1){
	  
		/****************************************************************************/
		/************************* NXT behaviour code *******************************/
		/****************************************************************************/

		/* record from headset and recognize command */
		pthread_create(&task_bt, NULL, hs_data_rec, (void *)&info);

		/* receive NXT information state */
		pthread_create(&task_state, NULL, recv_state, (void *)&nxtinf);

		while(info.running){

			/* stop exec command when BT recording */
			pthread_mutex_lock(&(info.state_mtx));

			if(info.isrecording)
				pthread_cond_wait(&(info.exec_cond), &(info.state_mtx));

			pthread_mutex_unlock(&(info.state_mtx));

			/* read robot state code (threaded is better <- not yet) */
                        //recv_state(nxt, &state);

			/* get robot information state code */                                      
			switch(state){
				case NORMAL: /* execute order */

					if(info.cmd != UNKNOWN_ORDER && info.cmd != NO_ORDER){

						/* avoid execution of the same order :) */	
						if(prev_cmd != info.cmd){
							pthread_mutex_unlock(&(info.state_mtx));

							send_cmd(nxt, (char)info.cmd);
							fprintf(stderr, "[info] cmd ID = %d, state = NORMAL\n", info.cmd);
							prev_cmd = info.cmd;

							/* set non order */
							info.cmd = NO_ORDER;

							/* launch Bt task */
							info.isPlaying = 0;
							pthread_cond_signal(&(info.play_cond));

							pthread_mutex_unlock(&(info.state_mtx));
						}


					}
					else if(info.cmd == UNKNOWN_ORDER){
		   				printf("\n[!] unknown order!\n");

		   				/* BT -> HS : "UNKNOWN ORDER!" */
		   				hs_data_play(&info, NXTMSGDIR "/nxtmsg/wrong.msg");

						/* set non order */
						info.cmd = NO_ORDER;

					}

					break;

				case HIT:   /* stop when hit detected */

					printf("\n[!] hit detected!\n");

					/* BT -> HS : "HIT!" */
					hs_data_play(&info, NXTMSGDIR "/nxtmsg/hit.msg");

					fprintf(stderr, "[info] cmd ID = %d, state = HIT\n", info.cmd);
					break;

				case LOWBATTERY: /* exit program when battery low */

					printf("\n[!] battery low!\n");

					/* BT -> HS : "LOW BATTERY!" */
					hs_data_play(&info, NXTMSGDIR "/nxtmsg/lowbatt.msg");

					/* stop the system */
					info.running = 0;
					pthread_join(task_bt, NULL);
					pthread_join(task_state, NULL);
					
				
			} /* end switch-case block */


		} /* end loop robot behaviour */

	} /* end connection condition */

	/* free all ressources */
	lejos_close(nxt); // generate server exception!!!!?? -> because no wait for closing in server
	hs_disconnect(info);
	sleep(3);
	rv_close(&(info.ps));

	printf("\nexiting program... BYE!\n");
  
	return 0;
}
