#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	printf ("output stdout info!\n");
	fprintf (stderr, "output stderr info!\n");
	
	printf (" \
			===========> Stream Mode: TS Input -> Transcoding -> Mux Remux \
			===========> Output Video Format: MPEG4 AVC \
			audio out sample rate is 48000 \
			Get 4 recv video buffer from driver \
			Video buffer 0: addr 2b7b0000 phy addr 13b30000 \
			Video buffer 1: addr 2b7d0000 phy addr 13b40000 \ 
			Video buffer 2: addr 2b7e0000 phy addr 13b50000 \
			Video buffer 3: addr 2b7f0000 phy addr 13b60000 \
			Get 0 recv audio buffer from driver \
			Get 0 recv frpt buffer from driver \
			  Enter 0 to stop transcoding \
			  during encoding, use the following additional commands (then press enter): \ 
			  PLEASE DON'T TRY DURING TRANSCODING \
			  type \'dd\' to switch to dual-mono mode \
			  type \'ds\' to switch to dual-stereo mode \
			  type \'dm\' to switch to mono mode \
			  type \'dr###\' to set new audio bitrate ### \
			  type \'a0\' to change to default aspect ratio \
		          type \'a1\' to change to 1:1 aspect ratio \
			  type \'a2\' to change to 4:3 aspect ratio \
		  	  type \'a3\' to change to 16:9 aspect ratio \
		    	  type \'a4\' to change to 2.1:1 aspect ratio \
			  type \'nx\' to change noise and comb filters where x \
			  [mux] =0-f. \
		 e.g. n0 means turn off filters. nf means enable all filters \
		 For ASI/PSI streams, use the following additional commands (then press enter): \
		   type \'g\' to get ts info \
	           type \'u\' to update AVC dynamic change parameters \
		   type \'f\' to get firmware state \
		   type \'c\' to flush output \
		   type \'p\' to pause/resume TS output \
		 Notes: after hit any key, the duration control will restart...\
			");
	printf ("\n\r TS Input PSI Lock = 0\n");
	sleep(1);
	printf ("   TS Input Video Lock      = 0\n");
	sleep(1);
	printf ("TS Input Audio Lock      = 0\n");
	sleep(1);
	printf ("TS Input PCR Lock     = 0\n");
	sleep(1);
	printf ("    TS Input Video CC Error = 0\n");
	printf ("TS Input Audio CC Error = 0\n");
	printf ("TS Output = YES\n");
	printf ("TS Video Syntax Error = 0\n");
	printf ("TS Video Timestamp Error = 0\n");
	sleep(1);
	printf ("TS Discontinuous Timestamp = 0\n");
	printf ("Video Output = YES\n");
	printf ("Audio Output = YES\n");

	return 0;
}
