// 
//       Filename:  framebuffer.cpp
//    Description:  
//        Created:  20.12.2010 18:52:52
// 

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstdlib>

#include <iostream>


/* we need to do some more work to get that running.. */
#define DL_IOCTL_BLIT           0xAA
#define DL_IOCTL_COPY           0xAB
#define DL_IOCTL_BLIT_DB        0xAC
#define DL_IOCTL_EDID           0xAD

using namespace std;
int main (void) {
    // open device and read info
    int fd = open ("/dev/fb1", O_RDWR);
		if (fd < 0) { // error occured
			cerr << "Couldn't open framebuffer device. ERRNO = " << errno << endl;
			exit (-1);
		}

    struct fb_var_screeninfo screeninfo;
    ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo);
    cout << screeninfo.bits_per_pixel << endl;
    cout << "width = " << screeninfo.xres << endl; 
    cout << "height = " << screeninfo.yres << endl; 

    // continue if 16 bit color depth
    if (screeninfo.bits_per_pixel == 16) {
        // determine size
        int width = screeninfo.xres;
        int height = screeninfo.yres;

        // embed framebuffer into memory
        // *2 because we use 16 bit data
        __u16 *data = (__u16 *) mmap (0, width*height*2, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

        // process screen content line by line
        for (int row = 0; row < height; row++) {
            for (int column = 0; column< width; column++) {
                data[column+row*width] = 0xe111;
            }
        }

        /*
         * The following code is essential to work around the blit mechanism in udlfb. A
         * call to ioctl is used to write the needed parts from the buffer to the display.
         * The coordinates in int *coords should describe a rectangle surrounding all
         * areas which changed. Of course this rectangle should be as small as possible,
         * to avoid writing too much data.
         */
        int coords[4];
        coords[0] = 0;
        coords[1] = 0;
        coords[2] = screeninfo.xres_virtual;
        coords[3] = screeninfo.yres_virtual;

        if (ioctl (fd, DL_IOCTL_BLIT, &coords) == -21) {
            cerr << "bad.." << endl;
        }

        // mask framebuffer out of memory
        munmap (data, 2*width*height);
    } else {
        cerr << "fucked it up" << endl;
    }
    close (fd);
    return 0;
}
