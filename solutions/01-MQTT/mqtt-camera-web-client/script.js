const { broker, topics } = appConfig;
const options = {
  username: broker.username,
  password: broker.password,
  clientId: "XIAO-S3-Sense-" + Math.random().toString(16).substr(2, 8)
};

const statusEl = document.getElementById("status");
const imgEl = document.getElementById("cameraImage");
imgEl.alt = "Oczekiwanie na zdjęcie";
statusEl.innerText = "Łączenie z brokerem MQTT";
const client = mqtt.connect(broker.host, options);

client.on("connect", () => {
  statusEl.innerText = "Połączono. Oczekiwanie na zdjęcie";
  statusEl.style.color = "#4caf50";
  client.subscribe(topics.foto);
});

client.on("message", (topic, message) => {
  if (topic === topics.foto) {
    const base64Data = message.toString();
    imgEl.src = base64Data;

    const now = new Date();
    statusEl.innerText = "Odebrano zdjęcie: " + now.toLocaleTimeString();
    statusEl.style.color = "#2196f3";
  }
});

client.on("error", (err) => {
  statusEl.innerText = "Błąd połączenia: " + err.message;
  statusEl.style.color = "#f44336";
});

client.on("offline", () => {
  statusEl.innerText = "Rozłączono.";
  statusEl.style.color = "#ff9800";
});

function sendControl(topic, payload) {
  if (client && client.connected) {
    client.publish(topic, payload, (err) => {
      if (err) {
        console.error("Błąd wysyłania komendy:", err);
      } else {
        console.log(`Wysłano '${payload}' na topic '${topic}'`);
      }
    });
  } else {
    alert("Brak połączenia z serwerem MQTT");
  }
}

function updateSetting(input, topic, outputId) {
  document.getElementById(outputId).value = input.value;
  sendControl(topic, input.value);
}