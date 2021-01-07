#include <fcntl.h> 
#include <fstream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>
#include <iostream>

struct framebuffer_info
{
    uint32_t bits_per_pixel;    // framebuffer depth
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
    uint32_t yres_virtual;      // how many pixel in a column in virtual screen
    
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[])
{
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
    if(!camera.isOpened()) {
        std::cerr << "Could not open video device." << std::endl;
        return 1;
    } 
    
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,fb_info.yres_virtual);
    cap.set(CV_CAP_PROP_FPS,frame_rate);
    cv::Size2f image_size;
    
    // read image file (sample.bmp) from opencv libs.
    // https://docs.opencv.org/3.4.7/d4/da8/group__imgcodecs.html#ga288b8b3da0892bd651fce07b3bbd3a56
    image= cv::imread("sample.bmp");

    // get image size of the image.
    // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a146f8e8dda07d1365a575ab83d9828d1
    image_size = image.size();

    // transfer color space from BGR to BGR565 (16-bit image) to fit the requirement of the LCD
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga397ae87e1288a81d2363b61574eb8cab
    // https://docs.opencv.org/3.4.7/d8/d01/group__imgproc__color__conversions.html#ga4e0972be5de079fed4e3a10e24ef5ef0
    int framebuffer_width = fb_info.xres_virtual;
    int framebuffer_depth = fb_info.bits_per_pixel;
    
    while (true)
    {
        cap >> frame;

        int framebuffer_width = fb_info.xres_virtual;
        int framebuffer_depth = fb_info.bits_per_pixel;
        cv::Size2f frame_size = frame.size();
        cv::Mat framebuffer_compat;
        
        cv::cvtColor(frame, framebuffer_compat, cv::COLOR_BGR2BGR565);


        for (int y = 0; y < frame_size.height ; y++) {
            ofs.seekp(y*framebuffer_width*2);
            ofs.write(reinterpret_cast<char*>(framebuffer_compat.ptr(y)),frame_size.width*2);
        }
    }
}


struct framebuffer_info get_framebuffer_info(const char* framebuffer_device_path) {
    struct framebuffer_info info;
    struct fb_var_screeninfo screen_info;
    int fd = -1;
    fd = open(framebuffer_device_path, O_RDWR);
    if (fd >= 0) {
        if (!ioctl(fd, FBIOGET_VSCREENINFO, &screen_info)) {
            info.xres_virtual = screen_info.xres_virtual;
            info.yres_virtual = screen_info.yres_virtual;
            info.bits_per_pixel = screen_info.bits_per_pixel;
        }
    }
    return info;
};