const appConfig = window.appConfig;
const cameraImage = document.getElementById("cameraImage");
const statusElement = document.getElementById("status");
const streamButton = document.getElementById("streamButton");
const settingUpdatesInProgress = new Set();
let streamEnabled = true;

function sendRequest(url, description) {
  if (!url) {
    statusElement.textContent = `Brak endpointu: ${description}`;
    statusElement.style.color = "#f44336";
    return Promise.resolve({ success: false, status: null });
  }

  statusElement.textContent = `Wysyłanie: ${description}...`;
  statusElement.style.color = "#ff9800";

  return fetch(url)
    .then((response) => {
      if (!response.ok) {
        statusElement.textContent = `Błąd: ${description}.`;
        statusElement.style.color = "#f44336";
        console.error(`Błąd żądania (${description}): HTTP ${response.status}`);
        return { success: false, status: response.status };
      }
      statusElement.textContent = `Wysłano: ${description}.`;
      statusElement.style.color = "#4caf50";
      return { success: true, status: response.status };
    })
    .catch((error) => {
      statusElement.textContent = `Błąd: ${description}.`;
      statusElement.style.color = "#f44336";
      console.error(`Błąd żądania (${description}):`, error);
      return { success: false, status: null };
    });
}

function toggleStream() {
  streamEnabled = !streamEnabled;
  updateStreamVisibility();
  streamButton.textContent = streamEnabled ? "Wyłącz stream" : "Włącz stream";
}

function updateStreamVisibility() {
  if (streamEnabled && document.visibilityState === "visible") {
    if (!cameraImage.hasAttribute("src")) {
      cameraImage.src = appConfig.streamUrl;
    }
  } else {
    cameraImage.removeAttribute("src");
  }
}

function toggleLED() {
  sendRequest(appConfig.endpoints.led, "przełączenie LED");
}

function rotateCamera(direction) {
  const endpoint = direction == "left"
    ? appConfig.endpoints.rotateLeft
    : appConfig.endpoints.rotateRight;
  const description = direction == "left" ? "obrót w lewo" : "obrót w prawo";
  sendRequest(endpoint, description);
}

function changeSetting(settingName, slider, button) {
  if (settingUpdatesInProgress.has(settingName)) {
    return;
  }

  const value = Number(slider.value);
  const url = new URL(appConfig.endpoints.settings, window.location.href);
  url.searchParams.set("setting", settingName);
  url.searchParams.set("value", value);

  settingUpdatesInProgress.add(settingName);
  button.disabled = true;
  sendRequest(url, `${settingName}: ${value}`)
    .then((result) => {
      if (result.success) {
        appConfig.settingValues[settingName] = value;
      }
    })
    .finally(() => {
      settingUpdatesInProgress.delete(settingName);
      button.disabled = false;
    });
}

document.getElementById("streamButton").addEventListener("click", toggleStream);
document.getElementById("ledButton").addEventListener("click", toggleLED);
document.getElementById("rotateLeftButton").addEventListener("click", () => rotateCamera("left"));
document.getElementById("rotateRightButton").addEventListener("click", () => rotateCamera("right"));

document.querySelectorAll(".setting[data-setting]").forEach((setting) => {
  const settingName = setting.dataset.setting;
  const slider = document.getElementById(`${settingName}Input`);
  const output = document.getElementById(`${settingName}Value`);
  const limits = appConfig.settingLimits[settingName];
  const button = setting.querySelector('[data-action="send"]');

  slider.min = limits.min;
  slider.max = limits.max;
  slider.value = appConfig.settingValues[settingName];
  output.value = slider.value;

  slider.addEventListener("input", () => {
    output.value = slider.value;
  });

  button.addEventListener("click", () => {
    changeSetting(settingName, slider, button);
  });
});

updateStreamVisibility();
document.addEventListener("visibilitychange", updateStreamVisibility);
window.addEventListener("pagehide", () => cameraImage.removeAttribute("src"));
window.addEventListener("pageshow", updateStreamVisibility);
