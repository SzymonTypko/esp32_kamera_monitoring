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
  const nextValue = value + (action == "increase" ? 1 : -1);

  if (settingUpdatesInProgress.has(settingName)) {
    return;
  }

  settingUpdatesInProgress.add(settingName);
  button.disabled = true;
  sendRequest(appConfig.endpoints.settings[settingName][action], `${settingName}: ${nextValue}`)
    .then((result) => {
      if (result.success) {
        appConfig.settingValues[settingName] = nextValue;
        document.getElementById(`${settingName}Value`).value = nextValue;
        updateSettingButtons(settingName);
      } else {
        button.disabled = result.status === 400;
      }
    })
    .finally(() => {
      settingUpdatesInProgress.delete(settingName);
    });
}

function updateSettingButtons(settingName) {
  const setting = document.querySelector(`.setting[data-setting="${settingName}"]`);
  setting.querySelectorAll("[data-action]").forEach((button) => {
    button.disabled = false;
  });
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
});

cameraImage.src = appConfig.streamUrl;
window.addEventListener("pagehide", () => cameraImage.removeAttribute("src"));
window.addEventListener("pageshow", () => {
  if (streamEnabled) {
    cameraImage.src = appConfig.streamUrl;
  }
});
