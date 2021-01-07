#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>


struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}


struct framebuffer_info
{
    uint32_t bits_per_pixel;    // depth of framebuffer
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
    uint32_t yres_virtual;
};

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path );

int main ( int argc, const char *argv[] )
{

    set_conio_terminal_mode();

    const int frame_rate = 10;

    // variable to store the frame get from video stream
    cv::Mat frame;

    // open video stream device
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a5d5f5dacb77bbebdcbfb341e3d4355c1
    cv::VideoCapture camera ( 2 );

    // get info of the framebuffer
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");

    // open the framebuffer device
    // http://www.cplusplus.com/reference/fstream/ofstream/ofstream/
    std::ofstream ofs("/dev/fb0");

    // check if video stream device is opened success or not
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a9d2ca36789e7fcfe7a7be3b328038585
    // if( !check )
    // {
    //     std::cerr << "Could not open video device." << std::endl;
    //     return 1;
    // }
    if(!camera.isOpened()) {
        std::cerr << "Could not open video device." << std::endl;
        return 1;
    }

    // set propety of the frame
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#a8c6d8c2d37505b5ca61ffd4bb54e9a7c
    // https://docs.opencv.org/3.4.7/d4/d15/group__videoio__flags__base.html#gaeb8dd9c89c10a5c63c139bf7c4f5704d
    int framebuffer_width = fb_info.xres_virtual;
    int framebuffer_height = fb_info.yres_virtual;
    int framebuffer_depth = fb_info.bits_per_pixel;
    //camera.set(CV_CAP_PROP_FRAME_WIDTH,framebuffer_width);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT,framebuffer_height);
    camera.set(CV_CAP_PROP_FPS,frame_rate);


    int frame_width= camera.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height=   camera.get(CV_CAP_PROP_FRAME_HEIGHT);
    cv::VideoWriter video("out.avi",CV_FOURCC('M','J','P','G'),10, cv::Size(frame_width,frame_height),true);


    unsigned char c = ' ';
    int image_file_counter = 0;
    while(c!='q')
    {
        while (!kbhit()) {
            
            // Read camera input frame
            camera >> frame;
            cv::Size2f frame_size = frame.size();
            cv::Mat framebuffer_compat;
            cv::cvtColor(frame, framebuffer_compat, cv::COLOR_BGR2BGR565);
                            
            // Display 
            for ( int y = 0; y < frame_size.height; y++ )
            {
                ofs.seekp(y * fb_info.xres_virtual * fb_info.bits_per_pixel / 8);
                ofs.write(reinterpret_cast<char*>(framebuffer_compat.ptr(y)), frame_size.width* fb_info.bits_per_pixel / 8);
            }

            // Save as video file
            video.write(frame);
        }

        /* consume the character */
        c = getch();
        if(c=='c'){
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,sizeof(buffer),"%d_%m_%Y_%H_%M_%S",timeinfo);
            std::string str(buffer);

            if (!frame.empty()) { cv::imwrite("Image_"+str+".png", frame); }

            printf("Image Saved!\n");   
        }

    }
    printf("Quit!\n");    

    // closing video stream
    // https://docs.opencv.org/3.4.7/d8/dfe/classcv_1_1VideoCapture.html#afb4ab689e553ba2c8f0fec41b9344ae6
    camera.release ( );
    video.release();

    return 0;
}

struct framebuffer_info get_framebuffer_info ( const char *framebuffer_device_path )
{
    struct framebuffer_info info;
    struct fb_var_screeninfo screen_info;
    int fd = -1;
    fd = open(framebuffer_device_path, O_RDWR);
    if (fd >= 0) {
        if (!ioctl(fd, FBIOGET_VSCREENINFO, &screen_info)) {
            info.xres_virtual = screen_info.xres_virtual; // ¤@¦æ¦³¦h¤Ö­Ópixel
            info.yres_virtual = screen_info.yres_virtual; // ¤@¦æ¦³¦h¤Ö­Ópixel
            info.bits_per_pixel = screen_info.bits_per_pixel; //¨C­Ópixelªº¦ì¼Æ
        }
    }
    return info;
};