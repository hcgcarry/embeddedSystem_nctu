#include <fcntl.h> 
#include <fstream>
#include <linux/fb.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/ioctl.h>

struct framebuffer_info
{
    uint32_t bits_per_pixel;    // framebuffer depth
    uint32_t xres_virtual;      // how many pixel in a row in virtual screen
};

struct framebuffer_info get_framebuffer_info(const char *framebuffer_device_path);

int main(int argc, const char *argv[])
{
    cv::Mat image;
    cv::Size2f image_size;
    
    framebuffer_info fb_info = get_framebuffer_info("/dev/fb0");
    std::ofstream ofs("/dev/fb0");

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
    cv::Mat framebuffer_compat;
    switch (framebuffer_depth) {
        case 16:
            //cv::cvtColor(image, framebuffer_compat, cv::COLOR_RGBA2BGR565);
            cv::cvtColor(image, framebuffer_compat, cv::COLOR_BGR2BGR565);
        break;
        // case 32: {
        //         std::vector<cv::Mat> split_bgr;
        //         cv::split(image, split_bgr);
        //         split_bgr.push_back(cv::Mat(image_size,CV_8UC1,cv::Scalar(255)));
        //         cv::merge(split_bgr, framebuffer_compat);
        //         for (int y = 0; y < image_size.height ; y++) {
        //             ofs.seekp(y*framebuffer_width*4);
        //             ofs.write(reinterpret_cast<char*>(framebuffer_compat.ptr(y)),image_size.width*4);
        //         }
        //     } 
        // break;
    }
    

    // output to framebufer row by row
    // move to the next written position of output device framebuffer by "std::ostream::seekp()".
    // posisiotn can be calcluated by "y", "fb_info.xres_virtual", and "fb_info.bits_per_pixel".
    // http://www.cplusplus.com/reference/ostream/ostream/seekp/

    // write to the framebuffer by "std::ostream::write()".
    // you could use "cv::Mat::ptr()" to get the pointer of the corresponding row.
    // you also have to count how many bytes to write to the buffer
    // http://www.cplusplus.com/reference/ostream/ostream/write/
    // https://docs.opencv.org/3.4.7/d3/d63/classcv_1_1Mat.html#a13acd320291229615ef15f96ff1ff738

    for (int y = 0; y < image_size.height ; y++) {
        //*2 is because one pixel has two byte
                ofs.seekp(y*framebuffer_width*2);
                ofs.write(reinterpret_cast<char*>(framebuffer_compat.ptr(y)),image_size.width*2);
    }    

    return 0;
}


struct framebuffer_info get_framebuffer_info(const char* framebuffer_device_path) {
    struct framebuffer_info info;
    struct fb_var_screeninfo screen_info;
    int fd = -1;
    fd = open(framebuffer_device_path, O_RDWR);
    if (fd >= 0) {
        if (!ioctl(fd, FBIOGET_VSCREENINFO, &screen_info)) {
            info.xres_virtual = screen_info.xres_virtual;
            info.bits_per_pixel = screen_info.bits_per_pixel;
        }
    }
    return info;
};