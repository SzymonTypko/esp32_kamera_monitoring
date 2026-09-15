#pragma once
#include <Arduino.h>

namespace WifiConfig {
    // Wifi dane
    constexpr char* ssid = "zastąp_to_loginem";
    constexpr char* password = "zastąp_to_hasłem";
}

namespace MQTTConfig {
    // MQTT dane
    constexpr char mqtt_server[] = "zastąp_to_adresem_serwera"; // np 34734fdf89fdsfd.s1.eu.hivemq.cloud 
    constexpr int mqtt_port = 8883; // lub inny port, jeśli twój broker używa innego
    constexpr char mqtt_user[] = "zastąp_to_loginem"; // jezeli nie masz hasla zostaw puste
    constexpr char mqtt_pass[] = "zastąp_to_hasłem";
    
    // MQTT tematy 
    const char topic_capture[] = "???/capture"; // zmień "???" np "kamera/dzialka/esp/capture" 
    const char topic_foto[] = "???/foto";
    const char topic_timestamp[] = "???/timestamp";
    const char topic_temp[] = "???/temp";
    const char topic_set_all[] = "???/set/#";
}

namespace CameraConfig {
    typedef enum {
        FRAMESIZE_96X96,    // 96x96
        FRAMESIZE_QQVGA,    // 160x120
        FRAMESIZE_QCIF,     // 176x144
        FRAMESIZE_HQVGA,    // 240x176
        FRAMESIZE_240X240,  // 240x240
        FRAMESIZE_QVGA,     // 320x240
        FRAMESIZE_CIF,      // 400x296
        FRAMESIZE_HVGA,     // 480x320
        FRAMESIZE_VGA,      // 640x480
        FRAMESIZE_SVGA,     // 800x600
        FRAMESIZE_XGA,      // 1024x768
        FRAMESIZE_HD,       // 1280x720
        FRAMESIZE_SXGA,     // 1280x1024
        FRAMESIZE_UXGA,     // 1600x1200
        // 3MP Sensors
        FRAMESIZE_FHD,      // 1920x1080
        FRAMESIZE_P_HD,     //  720x1280
        FRAMESIZE_P_3MP,    //  864x1536
        FRAMESIZE_QXGA,     // 2048x1536
        // 5MP Sensors
        FRAMESIZE_QHD,      // 2560x1440
        FRAMESIZE_WQXGA,    // 2560x1600
        FRAMESIZE_P_FHD,    // 1080x1920
        FRAMESIZE_QSXGA,    // 2560x1920
        FRAMESIZE_INVALID
    } default_framesize_t;

    typedef enum {
        OV9650_PID = 0x96,
        OV7725_PID = 0x77,
        OV2640_PID = 0x26,
        OV3660_PID = 0x3660,
        OV5640_PID = 0x5640,
        OV7670_PID = 0x76,
        NT99141_PID = 0x1410,
        GC2145_PID = 0x2145,
        GC032A_PID = 0x232a,
        GC0308_PID = 0x9b,
        BF3005_PID = 0x30,
        BF20A6_PID = 0x20a6,
        SC101IOT_PID = 0xda4a,
        SC030IOT_PID = 0x9a46,
        SC031GS_PID = 0x0031,
    } default_camera_pid_t;

    constexpr default_framesize_t default_frame_size = FRAMESIZE_SVGA; // dostosuj do swoich potrzeb i możliwości kamery
    constexpr default_camera_pid_t default_camera_pid = OV3660_PID; // to też
    constexpr bool vflip = true; //ustawienie pionowego odbicia obrazu
}