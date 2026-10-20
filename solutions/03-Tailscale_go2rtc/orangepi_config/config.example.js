// przykladowy config  nie polecam zmieniac settingLimits a zalecam zmieniac settingValues w zaleznosci od potrzeb

window.appConfig = {
  streamUrl: "/api/stream.mjpeg?src=esp_cam",
  endpoints: {
    led: "/api/frame.jpeg?src=toggle_led",
    rotateLeft: "/api/frame.jpeg?src=rotate_left",
    rotateRight: "/api/frame.jpeg?src=rotate_right",
    settings: {
      brightness: {
        increase: "/api/frame.jpeg?src=brightness_up",
        decrease: "/api/frame.jpeg?src=brightness_down"
      },
      contrast: {
        increase: "/api/frame.jpeg?src=contrast_up",
        decrease: "/api/frame.jpeg?src=contrast_down"
      },
      saturation: {
        increase: "/api/frame.jpeg?src=saturation_up",
        decrease: "/api/frame.jpeg?src=saturation_down"
      },
      sharpness: {
        increase: "/api/frame.jpeg?src=sharpness_up",
        decrease: "/api/frame.jpeg?src=sharpness_down"
      },
      quality: {
        increase: "/api/frame.jpeg?src=quality_up",
        decrease: "/api/frame.jpeg?src=quality_down"
      }
    }
  },
  settingValues: {
    brightness: 2,
    contrast: 2,
    saturation: 2,
    sharpness: 2,
    quality: 12
  },
  settingLimits: {
    brightness: { min: 0, max: 4 },
    contrast: { min: 0, max: 4 },
    saturation: { min: 0, max: 4 },
    sharpness: { min: 0, max: 4 },
    quality: { min: 10, max: 63 }
  }
};
