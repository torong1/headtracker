#define HT_PI 3.14159265f
#include "ht-api.h"
#include <opencv2/opencv.hpp>
//#include <QtNetwork/QUdpSocket>

using namespace std;
using namespace cv;

static volatile bool ht_quitp = false;

#ifdef __unix
#include <signal.h>

static void ht_quit_handler(int foo) {
    ht_quitp = true;
}

#endif

#pragma pack(push, 2)
struct THeadPoseData {
        double x, y, z, yaw, pitch, roll;
        long frame_number;
};
#pragma pack(pop)

int main(int argc, char** argv)
{
    bool start = false;
#ifdef __unix
    (void) signal(SIGTERM, ht_quit_handler);
    (void) signal(SIGHUP, ht_quit_handler);
    (void) signal(SIGINT, ht_quit_handler);
#endif
    ht_config_t config;

    config.classification_delay = 500;
    config.field_of_view = 52
	config.pyrlk_pyramids = 3;
    config.pyrlk_win_size_w = config.pyrlk_win_size_h = 21;
    config.max_keypoints = 200;
    config.keypoint_quality = 2;
    config.keypoint_distance = 1;
    config.keypoint_3distance = 2.9;
    config.keypoint_9distance = 8;
    config.force_width = 640;
    config.force_height = 480;
    config.force_fps = 0;
    config.camera_index = 0;
    config.ransac_num_iters = 50;
    config.ransac_max_reprojection_error = 3.2;
    config.ransac_max_inlier_error = 3.2;
    config.ransac_max_mean_error = 3.1;
    config.ransac_abs_max_mean_error = 10;
    config.debug = 1;
    config.ransac_min_features = 0.83;
    config.user_landmarks = false; //iniFile.value("use-bashed-coords").toBool();
    config.flandmark_delay = 65;

    headtracker_t* ctx = ht_make_context(&config, argc > 1 ? argv[1] : NULL);
    ht_result_t result;

#if 0
    QUdpSocket sock;
    QHostAddress addr("127.0.0.1");
    sock.bind(addr, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
#endif
    THeadPoseData pose;
    int frameno = 0;

    namedWindow("capture");

    while (!ht_quitp && ht_cycle(ctx, &result)) {
        if (result.filled) {
            start = true;
            pose.frame_number = frameno;
            pose.x = result.tx;
            pose.y = result.ty;
            pose.z = result.tz;
            pose.yaw = result.rotx;
            pose.pitch = result.roty;
            pose.roll = result.rotz;
            //sock.writeDatagram((const char*) &pose, sizeof(THeadPoseData), addr, 5550);
#if 1
            printf("POSE %.2f %.2f %.2f | %.2f %.2f %.2f\n",
                   pose.yaw, pose.pitch, pose.roll,
                   pose.x, pose.y, pose.z);
#endif
        } else if (start && argc > 1) {
            abort();
            break;
        }
        frameno++;
        ht_frame_t frame;
        ht_get_bgr_frame(ctx, &frame);
        if (frame.data) {
            Mat foo(frame.rows, frame.cols, CV_8UC3, frame.data);
            imshow("capture", foo);
            delete[] frame.data;
        }
        waitKey(1);
    }

    ht_free_context(ctx);
	return 0;
}

