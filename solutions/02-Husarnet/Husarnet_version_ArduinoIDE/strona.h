#ifndef STRONA_H
#define STRONA_H

#include <Arduino.h>


const char STRONA[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="pl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Stream</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #1e1e1e;
            color: #ffffff;
            margin: 0;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }
        h1 {
            margin-bottom: 20px;
            font-size: 24px;
        }
        .stream-container {
            border: 4px solid #00adb5;
            border-radius: 8px;
            overflow: hidden;
            max-width: 90%;
            background: #000;
        }
        img {
            display: block;
            width: 100%;
            height: auto;
            max-height: 75vh;
        }
    </style>
</head>
<body>

    <h1>Podgląd na żywo</h1>

    <div class="stream-container">
        <img src="/stream" alt="Brak połączenia ze strumieniem wideo">
    </div>

</body>
</html>
)=====";
#endif