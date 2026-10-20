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
    return Promise.resolve(false);
  }

  statusElement.textContent = `Wysyłanie: ${description}...`;
  statusElement.style.color = "#ff9800";

  return fetch(url)
    .then((response) => {
      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }
      statusElement.textContent = `Wysłano: ${description}.`;
      statusElement.style.color = "#4caf50";
      return true;
    })
    .catch((error) => {
      statusElement.textContent = `Błąd: ${description}.`;
      statusElement.style.color = "#f44336";
      console.error(`Błąd żądania (${description}):`, error);
      return false;
    });
}

function toggleStream() {
  streamEnabled = !streamEnabled;
  if (streamEnabled) {
    cameraImage.src = appConfig.streamUrl;
  } else {
    cameraImage.removeAttribute("src");
  }
  streamButton.textContent = streamEnabled ? "Wyłącz stream" : "Włącz stream";
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

function changeSetting(settingName, action, button) {
  const value = appConfig.settingValues[settingName];
  const limits = appConfig.settingLimits[settingName];
  const nextValue = value + (action == "increase" ? 1 : -1);

  if (settingUpdatesInProgress.has(settingName) ||
      nextValue < limits.min || nextValue > limits.max) {
    return;
  }

  settingUpdatesInProgress.add(settingName);
  button.disabled = true;
  sendRequest(appConfig.endpoints.settings[settingName][action], `${settingName}: ${nextValue}`)
    .then((success) => {
      settingUpdatesInProgress.delete(settingName);
      button.disabled = false;

      if (success) {
        appConfig.settingValues[settingName] = nextValue;
        document.getElementById(`${settingName}Value`).value = nextValue;
        updateSettingButtons(settingName);
      }
    });
}

function updateSettingButtons(settingName) {
  const setting = document.querySelector(`.setting[data-setting="${settingName}"]`);
  const value = appConfig.settingValues[settingName];
  const limits = appConfig.settingLimits[settingName];
  setting.querySelector('[data-action="decrease"]').disabled = value <= limits.min;
  setting.querySelector('[data-action="increase"]').disabled = value >= limits.max;
}

document.getElementById("streamButton").addEventListener("click", toggleStream);
document.getElementById("ledButton").addEventListener("click", toggleLED);
document.getElementById("rotateLeftButton").addEventListener("click", () => rotateCamera("left"));
document.getElementById("rotateRightButton").addEventListener("click", () => rotateCamera("right"));

document.querySelectorAll(".setting[data-setting]").forEach((setting) => {
  const settingName = setting.dataset.setting;
  document.getElementById(`${settingName}Value`).value = appConfig.settingValues[settingName];
  setting.querySelectorAll("[data-action]").forEach((button) => {
    button.addEventListener("click", () => {
      changeSetting(settingName, button.dataset.action, button);
    });
  });
  updateSettingButtons(settingName);
});

cameraImage.src = appConfig.streamUrl;
window.addEventListener("pagehide", () => cameraImage.removeAttribute("src"));
window.addEventListener("pageshow", () => {
  if (streamEnabled) {
    cameraImage.src = appConfig.streamUrl;
  }
});
