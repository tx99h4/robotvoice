/**
 * btcomm.c	      Bluetooth management (connection, record voice, etc.)
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

#include "bt.h"

//#define TESTBT

static int rfcomm_connect(bdaddr_t *src, bdaddr_t *dst, uint8_t channel)
{
	struct sockaddr_rc addr;
	int s;

	if ((s = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM)) < 0) {
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, src);
	addr.rc_channel = 0;
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(s);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, dst);
	addr.rc_channel = channel;
	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
		close(s);
		return -1;
	}

	return s;
}

static int sco_connect(bdaddr_t *src, bdaddr_t *dst, uint16_t *handle, uint16_t *mtu)
{
	struct sockaddr_sco addr;
	struct sco_conninfo conn;
	struct sco_options opts;
	int s, size;

	if ((s = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO)) < 0) {
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, src);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		close(s);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, dst);

	if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0 ){
		close(s);
		return -1;
	}

	memset(&conn, 0, sizeof(conn));
	size = sizeof(conn);

	if (getsockopt(s, SOL_SCO, SCO_CONNINFO, &conn, &size) < 0) {
		close(s);
		return -1;
	}

	memset(&opts, 0, sizeof(opts));
	size = sizeof(opts);

	if (getsockopt(s, SOL_SCO, SCO_OPTIONS, &opts, &size) < 0) {
		close(s);
		return -1;
	}

	if (handle)
		*handle = conn.hci_handle;

	if (mtu)
		*mtu = opts.mtu;

	return s;
}

int hs_scan(void){

	inquiry_info *ii = NULL;
	int max_rsp, num_rsp;
	int dev_id, sock, len, flags;
	char addr[19] = { 0 };
	char name[248] = { 0 };

	/* Open BT adapter */
	dev_id = hci_get_route(NULL);
	sock = hci_open_dev( dev_id );
	if (dev_id < 0 || sock < 0) {
		perror("opening socket");
		return -1;
	}

	len  = 8;
	max_rsp = 255;
	flags = IREQ_CACHE_FLUSH;
	ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

	printf("Scanning for headset...\n"); 

	/* inquire information device */
	num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
	if( num_rsp < 0 ){
		perror("hci_inquiry");
		return -1;
	}   

	ba2str(&(ii)->bdaddr, addr);
	memset(name, 0, sizeof(name));
	if (hci_read_remote_name(sock, &(ii)->bdaddr, sizeof(name), name, 0) < 0){
		strcpy(name, "[unknown]");
		return -1;
	}

	if(strcmp(addr, HS_BTADDR) != 0){
		printf("Headset not found!\nSwitch headset to pairing mode if not.\n");
		return -1;
	}

	printf("Found Headset: %s, %s\n\n", addr, name);

	free(ii);
	close(sock);

	return 0;

}

int hs_connect(int *rd, int *sd, uint16_t *sco_handle, uint16_t *sco_mtu){

	bdaddr_t bdaddr;

	str2ba(HS_BTADDR, &bdaddr);

	if ((*rd = rfcomm_connect(BDADDR_ANY, &bdaddr, 2)) < 0) {
		perror("Can't connect RFCOMM channel");
		return -1;
	}

	fprintf(stderr, "RFCOMM channel connected\n");

	if ((*sd = sco_connect(BDADDR_ANY, &bdaddr, sco_handle, sco_mtu)) < 0) {
		perror("Can't connect SCO audio channel");
		close(*rd);
		return -1;
	}

	fprintf(stderr, "SCO audio channel connected (handle %d, mtu %d)\n", *sco_handle, *sco_mtu);

    return 0;

}

/* tell if the buffer contain sound activity or silence data */
static inline int silence(short *buf, int rlen){

	unsigned int mean = 0;
	int i;
	
	int size = rlen/2;

	/* compute mean on non-null set of data */
	for(i = 0 ; i < size ; i++)
		if(buf[i] < 0)
			mean += -buf[i];
		else
			mean += buf[i];

	mean /= size;

	/* apply filter decision */
	if(mean <= THSLD_MAX*4)
		return 1; /* silence */
	else {
		printf("\n%d ", mean);
		return 0; /* sound */
	}

}

/* Detect HS button push */
static inline int switch_off(char *buf){

	char atcmd[11];

	memset(atcmd, 0, sizeof(atcmd));
	sscanf(buf, "%s", atcmd);

	if(buf)
		if (strcmp(atcmd, "AT+CKPD=200") == 0)
			return 1;

	return 0;
	
}

/* record BT data and send recognized order code */
void *hs_data_rec(void *info){

	SHAREDINFO *i = ((SHAREDINFO *)(info));
	
	/* BT connection parameters */
	int sd = i->sd;
	int rd = i->rd;

	fd_set rfds;
	struct timeval timeout;
	int maxfd;
        int sel, rlen, wlen;

	short bufv[1024], prevbufv[1024];
	char bufc[32];

	/* silence detection parameters */
	int total_read = 0;
	int onetime    = 1;
	int sil;
	
	/* RV parameters */
	ps_decoder_t *ps = i->ps;
	char const *hyp;
	char const *uttid;
	int32 score;

	/* Bluetooth ready notification bip! */
	write(rd, "RING\r\n", 6);

	//memset(prevbufv, 0, sizeof(prevbufv));

	maxfd = (rd > sd) ? rd : sd;

	/* avoid recognition of init noise */
	sleep(3);

	while(i->running){

		/* sleep until playing NXT information finished */
		pthread_mutex_lock(&(i->state_mtx));

		if(i->isPlaying)
			pthread_cond_wait(&(i->play_cond), &(i->state_mtx));
		pthread_mutex_unlock(&(i->state_mtx));

		FD_ZERO(&rfds);
		FD_SET(rd, &rfds);
		FD_SET(sd, &rfds);

		timeout.tv_sec = 0;
		timeout.tv_usec = 10000;

		if ((sel = select(maxfd + 1, &rfds, NULL, NULL, &timeout)) > 0) {

			/* AT command available ? */
			if (FD_ISSET(rd, &rfds)) {

				printf("$\n");
				memset(bufc, 0, sizeof(bufc));

				/* read RFCOMM AT-command */
				rlen = read(rd, bufc, sizeof(bufc));
				if (rlen > 0) {
					/* confirm with AT+OK */
					fprintf(stderr, "%s\n", bufc);
					wlen = write(rd, "OK\r\n", 4);
					printf("$$\n");
					/* stop system when AT+CKPD=200 (HS button pushed) */
					if(switch_off(bufc))
						i->running = 0;

				}
			}

			/* voice data available ? */
			if (FD_ISSET(sd, &rfds)) {
				memset(bufv, 0, sizeof(bufv));

				/* read voice data from HS */
				rlen = read(sd, bufv, sizeof(bufv));

				/* is buffer contain silence? */
				if(!(i->isrecording))
					sil = silence(bufv, rlen);

				/* voice activity detected?
				    -> start VR collecting data */
				if(onetime && !sil){
					ps_start_utt(ps, NULL);
					i->isrecording = 1;
					//ps_process_raw(ps, (int16 *)prevbufv, (int32)rlen/2, FALSE, FALSE);
				}

				/* collect only when recording mode */
				if(i->isrecording){
					ps_process_raw(ps, (int16 *)bufv, (int32)rlen/2, FALSE, FALSE);
					total_read += rlen;
					onetime = 0;
				}

				/* if complete sound collection
				   -> get recognized word */
				if(total_read >= MAX_DATA_RECORD){
					ps_end_utt(ps);
					hyp = ps_get_hyp(ps, &score, &uttid);

					//printf("-> Recognized: %s\n", hyp);
					i->cmd = rv_get_order_code(hyp);

					total_read     = 0;
					onetime        = 1;
					i->isrecording = 0;

					/* stop its own job */
					i->isPlaying = 1;					

					/* release processing NXT command thread */
					pthread_mutex_lock(&(i->state_mtx));
					pthread_cond_signal(&(i->exec_cond));
					pthread_mutex_unlock(&(i->state_mtx));

				}

				/* copy previous buffer data */
				//memcpy(prevbufv, bufv, rlen);

			} /* end data available condition */

		} /* end select() condition */
		
	} /* end while loop */

}

/* Play recorded message when exceptional case occured from robot */
void hs_data_play(SHAREDINFO *info, char const *sndfile){

	int sd = info->sd;
	uint16_t sco_mtu = info->sco_mtu;

	fd_set rfds;
	struct timeval timeout;

	int fd;
        int rlen, wlen, sel;
	unsigned char *p, buf[2048];

	/* activate playing */
	pthread_mutex_lock(&(info->state_mtx));
	info->isPlaying = 1;
	pthread_mutex_unlock(&(info->state_mtx));

	if ((fd = open(sndfile, O_RDONLY)) < 0)
		perror("Can't open input/output file");
	else {

		while(1){

			FD_ZERO(&rfds);
			FD_SET(sd, &rfds);

			timeout.tv_sec = 0;
			timeout.tv_usec = 10000;

			if ((sel = select(sd + 1, &rfds, NULL, NULL, &timeout)) > 0)

				if (FD_ISSET(sd, &rfds)) {

					memset(buf, 0, sizeof(buf));
					rlen = read(sd, buf, sizeof(buf));

					if(rlen > 0) {
						/* read from sound file until end */
						if((rlen = read(fd, buf, rlen)) == 0)
							break;
			
						wlen = 0;
						p = buf;

						/* play data block voice in HS */
						while (rlen > sco_mtu) {
							wlen += write(sd, p, sco_mtu);
							rlen -= sco_mtu;
							p += sco_mtu;
						}

						wlen += write(sd, p, rlen);
						
					}

				}
		}

		close(fd);

	}

	/* activate BT */
	pthread_mutex_lock(&(info->state_mtx));

	/* deactivate playing */
	info->isPlaying = 0;
	pthread_cond_signal(&(info->play_cond));

	pthread_mutex_unlock(&(info->state_mtx));

}

/* Close BT connection */
void hs_disconnect(SHAREDINFO info){

	/* stop incoming call */
	//write(rd, "AT+HUP\r\n", 8);
	close(info.sd);
	sleep(3);
	close(info.rd);

	printf("[info] headset disconnected.\n");

}


#ifdef TESTBT

int main(int argc, char **argv)
{
    SHAREDINFO info;
    pthread_t task_bt;

    /* pocketphinx load init */
    rv_init(&(info.ps));

    info.isPlaying = 0;

    if(hs_connect(&(info.rd), &(info.sd), &(info.sco_handle), &(info.sco_mtu)) != -1){  

	pthread_create(&task_bt, NULL, hs_data_rec, (void *)&info);
	while(1){
		printf("%d\n", info.cmd);
		if(info.cmd == 2)
			hs_data_play(&info, "./nxtmsg/hit.msg");
	sleep(1);
	
	}

	
     } 

    printf(">>>OWARI<<<\n");
	
    hs_disconnect(info);
    
    return 0;
}
#endif

