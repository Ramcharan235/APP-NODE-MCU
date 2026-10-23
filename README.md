# NodeMCU Control (Android app)

Car + 4 relays + voice + servo controller for NodeMCU (ESP8266).

- `www/index.html`  – the app (UI + logic)
- `NodeMCU_Firmware.ino` – upload this to your NodeMCU with Arduino IDE
- `.github/workflows/build-apk.yml` – builds the APK automatically on GitHub (free)

## Get the APK
1. Create a free account at github.com and make a **new repository** (any name).
2. Upload ALL files of this folder to it (drag & drop in the browser: "Add file -> Upload files"; keep the `.github` folder).
3. Open the **Actions** tab -> "Build Android APK" -> wait ~5 minutes until it turns green.
4. Open that run -> scroll to **Artifacts** -> download `NodeMCU-Control-APK` (zip) -> unzip -> `app-debug.apk`.
5. Send the APK to your phone, tap it, allow "Install unknown apps", install.
