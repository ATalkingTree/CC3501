#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <sys/time.h>

using namespace cv;

Mat hsv_img;

int main()
{
    // Open the video camera.
    std::string pipeline = "libcamerasrc"
        " ! video/x-raw, width=800, height=600" // camera needs to capture at a higher resolution
        " ! videoconvert"
        " ! videoscale"
        " ! video/x-raw, width=400, height=300" // can downsample the image after capturing
        " ! videoflip method=rotate-180" // remove this line if the image is upside-down
        " ! appsink drop=true max_buffers=2";
    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
    if(!cap.isOpened()) {
        printf("Could not open camera.\n");
        return 1;
    }

    // Create the OpenCV window
    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
    cv::Mat frame;

    // Create a control window
    cv::namedWindow("Control", cv::WINDOW_AUTOSIZE);
    cv::Mat hsv_img;

    int iLowH = 0;
    int iHighH = 179;

    int iLowS = 0;
    int iHighS = 255;

    int iLowV = 0;
    int iHighV = 255;

    int openingSize = 1;
    int closingSize = 1;

    // Create trackbars in "Control" window
    cv::createTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
    cv::createTrackbar("HighH", "Control", &iHighH, 179);

    cv::createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
    cv::createTrackbar("HighS", "Control", &iHighS, 255);

    cv::createTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
    cv::createTrackbar("HighV", "Control", &iHighV, 255);

    cv::createTrackbar("Opening Size", "Control", &openingSize, 20); 
    cv::createTrackbar("Closing Size", "Control", &closingSize, 20); 

    // Measure the frame rate - initialise variables
    int frame_id = 0;
    timeval start, end;
    gettimeofday(&start, NULL);

    for(;;) {
        if (!cap.read(frame)) {
            printf("Could not read a frame.\n");
            break;
        }

        // Convert to HSV colour space
        cvtColor(frame, hsv_img, COLOR_BGR2HSV);

        // Threshold the image
        Mat thresh_img;
        inRange(hsv_img, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), thresh_img);

        // Morphology
        if (openingSize >= 1) {
            Mat openingStructuringElement = getStructuringElement(MORPH_RECT, Size(openingSize, openingSize));
            morphologyEx(thresh_img, thresh_img, MORPH_OPEN, openingStructuringElement);
        }

        if (closingSize >= 1) {
            Mat closingStructuringElement = getStructuringElement(MORPH_RECT, Size(closingSize, closingSize));
            morphologyEx(thresh_img, thresh_img, MORPH_CLOSE, closingStructuringElement);
        }
        
        // Calculate moments
        Moments m = moments(thresh_img, true);
        if (m.m00 > 0) {
            // Calculate the center of mass
            double x = m.m10 / m.m00;
            double y = m.m01 / m.m00;

            // Print the center of mass coordinates
            printf("Center of mass coordinates: x = %.2f, y = %.2f\n", x, y);
        }

        // Show the thresholded image
        cv::imshow("Thresholded", thresh_img);

        // Show frame
        cv::imshow("Camera", frame);

        // Measure the frame rate
        frame_id++;
        if (frame_id >= 30) {
            gettimeofday(&end, NULL);
            double diff = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
            printf("30 frames in %f seconds = %f FPS\n", diff, 30/diff);
            frame_id = 0;
            gettimeofday(&start, NULL);
        }

        // Allow openCV to process GUI events
        cv::waitKey(1);
    }

    // Free the camera 
    cap.release();
    return 0;
}
