# instalacja tailscale

`curl -fsSL https://tailscale.com/install.sh | sh`

`sudo tailscale up`

# instalacja go2rtc

`curl -L https://github.com/AlexxIT/go2rtc/releases/latest/download/go2rtc_linux_arm64 -o go2rtc`

`chmod +x go2rtc`

`sudo mv go2rtc /usr/local/bin/`

`sudo nano /etc/go2rtc.yaml`

wklej zawartosc z orangepi_config/go2rtc.yaml

`sudo nano /etc/go2rtc_web/index.html`

wklej zawartosc z orangepi_config/index.html

`sudo nano /etc/systemd/system/go2rtc.service`

wklej zawartosc z orangepi_config/go2rtc.service

`sudo systemctl daemon-reload`

`sudo systemctl enable go2rtc`

`sudo systemctl start go2rtc`

`sudo systemctl status go2rtc`

jak jest zielone active running to raczej jest ok

---

`sudo tailscale funnel 1984` 

raczej z flagą --bg aby chodzilo w tle

uruchom to tylko jak chcesz wsytawic podgląd do internetu - osoby spoza sieci tailscale tez będą mogły wejść
