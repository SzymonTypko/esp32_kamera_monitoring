# Instalacja na Orange Pi

## 1. Zainstaluj i uruchom Tailscale

```bash
curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up
```

## 2. Zainstaluj go2rtc

```bash
curl -L https://github.com/AlexxIT/go2rtc/releases/latest/download/go2rtc_linux_arm64 -o go2rtc
chmod +x go2rtc
sudo mv go2rtc /usr/local/bin/
```

## 3. Skonfiguruj go2rtc

1. Skopiuj zawartość `orangepi_config/go2rtc.yaml` do `/etc/go2rtc.yaml` i ustaw w nim adres IP ESP32.
2. Skopiuj pliki `index.html`, `script.js`, `config.js` oraz `style.css` z `orangepi_config/` do `/etc/go2rtc_web/`.
3. Skopiuj `orangepi_config/go2rtc.service` do `/etc/systemd/system/go2rtc.service`.
4. Włącz i uruchom usługę:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable --now go2rtc
   sudo systemctl status go2rtc
   ```

## 4. Zainstaluj proxy do sterowania kamerą

Stream wideo obsługuje go2rtc, natomiast polecenia sterujące i wartości z suwaków przechodzą przez `camera_proxy.py`.

1. Ustaw `ESP32_URL` w `orangepi_config/camera_proxy.py` na adres ESP32.
2. Skopiuj skrypt i usługę na Orange Pi:

   1. Skopiuj zawartość `orangepi_config/camera_proxy.py` do `/opt/camera-proxy/camera_proxy.py`

   2. Skopiuj `orangepi_config/camera-proxy.service` do `/etc/systemd/system/camera-proxy.service`.

3. Włącz i uruchom proxy:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable --now camera-proxy
   sudo systemctl status camera-proxy
   ```

## 5. Skonfiguruj Tailscale Serve

Zachowaj trasę `/` do go2rtc na porcie `1984` i dodaj trasę `/control` do proxy na porcie `8081`:

```bash
sudo tailscale serve --bg 1984
sudo tailscale serve --bg --set-path=/control http://127.0.0.1:8081
tailscale serve status
```

Jak to wygląda u mnie:

```text
https://moj_sekretny_adres.ts.net (tailnet only)
|-- /        proxy http://127.0.0.1:1984
|-- /control proxy http://127.0.0.1:8081
```

## 6. Wgraj firmware na ESP32

Ustaw odpowiedni solution do wgrania w `platformio.ini`:

```ini
build_src_filter =
    ;+<solutions/01-MQTT/>

    ;+<solutions/02-Husarnet/> 

    +<solutions/03-Tailscale_go2rtc/> 
    
    +<core/>
```

W repozytorium uruchom:

```bash
pio run -e seeed_xiao_esp32s3 -t upload
```

### Dodatkowe info

Struktura plików:

```
/
├── etc/
│   ├── systemd/
│   │   └── system/
│   │       ├── go2rtc.service
│   │       └── camera-proxy.service
│   │
│   ├── go2rtc_web/
│   │   ├── index.html
│   │   ├── script.js
│   │   ├── config.js
│   │   └── style.css
│   │
│   └── go2rtc.yaml
│
└── opt/
    └── camera-proxy/
         └── camera_proxy.py
```

Panel wysyła ustawienia w formacie `/control/set_camera?setting=brightness&value=3`. Proxy i firmware sprawdzają dozwolone ustawienia i zakresy: jasność, kontrast, nasycenie i ostrość `0–4`, jakość `10–63`.

Serve udostępnia tylko w twoim tailnecie a Funnel na cały Inetrnet.
