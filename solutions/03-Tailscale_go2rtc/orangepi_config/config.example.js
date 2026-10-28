// Zmieniaj settingValues w zaleznosci od potrzeb.

window.appConfig = {
  streamUrl: "/api/stream.mjpeg?src=esp_cam",
  endpoints: {
    temp: "/control/get_temp",
    rotateLeft: "/control/rotate_left",
    rotateRight: "/control/rotate_right",
    settings: "/control/set_camera"
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
