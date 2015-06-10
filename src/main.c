#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "msp_config.h"
#include "msp_buffer.h"
#include "msp_input_v4l2.h"
#include "msp_output_sdl.h"

void mainloop(msp_input_v4l2_watcher *watcher)
{
	msp_buffer raw,rgb,gs;
	msp_buffer_init(&gs,CAM_WIDTH*CAM_HEIGHT*1);
	if(-1==msp_output_sdl_show_gs8(&gs,"SDL buffer"))
		fprintf(stderr,"EVERYTHING IS FUCKED\n");

	for(;;)
	{	
		raw = msp_input_v4l2_get_frame(watcher,3);
		msp_buffer_yuy2_to_gs8(&raw,&gs);
		msp_output_sdl_render();	
	}

	msp_buffer_kill(&gs);
}


int main(int argc, char **argv)
{
	msp_input_v4l2_watcher video1_in;
	
	msp_input_v4l2_initialize_watcher(&video1_in,"/dev/video2");
	msp_input_v4l2_init(&video1_in);
        mainloop(&video1_in);
	msp_input_v4l2_close(&video1_in);
	
        fprintf(stderr, "\n");
        return 0;
}
