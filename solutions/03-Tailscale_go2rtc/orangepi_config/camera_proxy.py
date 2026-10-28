from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.error import HTTPError, URLError
from urllib.parse import parse_qs, urlencode, urlsplit
from urllib.request import urlopen

ESP32_URL = "http://192.168.0.98"
PORT = 8081

SETTING_LIMITS = {
    "brightness": (0, 4),
    "contrast": (0, 4),
    "saturation": (0, 4),
    "sharpness": (0, 4),
    "quality": (10, 63),
}

ALLOWED_COMMANDS = {
    "get_temp",
    "rotate_left",
    "rotate_right",
}

class CameraProxyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        parsed_url = urlsplit(self.path)
        path = parsed_url.path.strip("/")
        if path.startswith("control/"):
            path = path[len("control/"):]

        if path == "set_camera":
            self.set_camera(parsed_url.query)
            return

        if path not in ALLOWED_COMMANDS:
            self.send_error(404, "Nieznana komenda")
            return

        self.forward_request(f"{ESP32_URL}/{path}")

    def set_camera(self, query):
        try:
            params = parse_qs(query, strict_parsing=True)
        except ValueError:
            self.send_error(400, "Nieprawidlowe parametry")
            return

        if set(params) != {"setting", "value"}:
            self.send_error(400, "Wymagane parametry setting i value")
            return

        if len(params["setting"]) != 1 or len(params["value"]) != 1:
            self.send_error(400, "Kazdy parametr moze wystapic tylko raz")
            return

        setting = params["setting"][0]
        value_text = params["value"][0]
        if setting not in SETTING_LIMITS:
            self.send_error(400, "Nieznane ustawienie kamery")
            return

        try:
            value = int(value_text)
        except ValueError:
            self.send_error(400, "Wartosc musi byc liczba calkowita")
            return

        minimum, maximum = SETTING_LIMITS[setting]
        if not minimum <= value <= maximum:
            self.send_error(400, f"Wartosc musi byc z zakresu {minimum}-{maximum}")
            return

        query_string = urlencode({"setting": setting, "value": value})
        self.forward_request(f"{ESP32_URL}/set_camera?{query_string}")

    def forward_request(self, target_url):
        try:
            with urlopen(target_url, timeout=5) as response:
                status = response.status
                body = response.read()
        except HTTPError as error:
            status = error.code
            body = error.read()
        except URLError as error:
            self.send_error(502, f"Nie mozna polaczyc z ESP32: {error.reason}")
            return

        self.send_response(status)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


if __name__ == "__main__":
    server = HTTPServer(("127.0.0.1", PORT), CameraProxyHandler)
    print(f"Proxy kamery nasluchuje na 127.0.0.1:{PORT}")
    server.serve_forever()
