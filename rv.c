/**
 * rv.c           Recognition module
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
 
#include "rv.h"
#include "nxt.h"

#define MODELDIR "/usr/local/share/pocketsphinx/model"

//#define TESTRV

#ifdef TESTRV
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#endif

int rv_init(ps_decoder_t **ps){

        cmd_ln_t *config;

	config = cmd_ln_init(NULL, ps_args(), TRUE,
                             "-hmm", MODELDIR "/hmm/wsj1",
                             "-lm", MODELDIR "/lm/turtle/turtle.lm.DMP",
                             "-dict", MODELDIR "/lm/turtle/turtle.dic",
                             NULL);

        if (config == NULL)
                return 1;
	
        *ps = ps_init(config);
        if (*ps == NULL)
                return 1;

	return 0;

}

/* get ID of spoken word */
int rv_get_order_code(char const *hyp){

	char word[11];
	memset(word, 0, sizeof(word));

	if (hyp) {
		sscanf(hyp, "%s", word);

		if (strcmp(word, "GO") == 0)
			return GO;
		else if (strcmp(word, "BACKWARD") == 0)
			return BACK;
		else if (strcmp(word, "HALT") == 0)
		        return STOP;
		else if (strcmp(word, "LEFT") == 0)
		        return LEFT;
		else if (strcmp(word, "RIGHT") == 0)
		        return RIGHT;
		else if (strcmp(word, "") == 0)
		        return UNKNOWN_ORDER;
		
	}
	
	return NO_ORDER;

}

int rv_close(ps_decoder_t **ps){
	ps_free(*ps);
	return 0;
}

#ifdef TESTRV

int main(int argc, char **argv){

	ps_decoder_t *ps;
	int cmd;
	int fd;

	char buff[15];

        rv_init(&ps);
	if ((fd = open(argv[1], O_RDONLY)) < 0)
		perror("Can't open input/output file");
	
	rv_get_order_code(buff);

	close(fd);
	rv_close(ps);

	return 0;
}
#endif


