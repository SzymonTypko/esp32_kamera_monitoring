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