# M5Stack-PaperS3-UniRead

## Overview
**M5Stack-PaperS3-UniRead** is a custom firmware for the [M5Stack Paper S3](https://docs.m5stack.com/en/core/M5PaperS3) device designed to provide a high-quality E-Book reading experience. It is optimized for the E-Ink display, ensuring crisp text rendering and efficient battery usage.

## Features
- **EPub Support**: Read unprotected `.epub` files directly on your M5Stack Paper S3.
- **High-Fidelity Rendering**: Utilizes `lib_freetype` for high-quality font rendering with anti-aliasing support suitable for E-Ink screens.
- **SD Card Integration**: Load books and fonts from an SD card.
- **Wi-Fi File Upload**: Easily upload books and fonts wirelessly via a built-in web interface.
- **Custom Fonts**: Support for user-provided TrueType (.ttf) and OpenType (.otf) fonts.
- **Touch Interface**: Intuitive touch controls for navigation and settings.
- **Optimized Power Management**: Deep sleep and power-saving features to extend battery life.

## Getting Started

### Prerequisites
- **Hardware**: M5Stack Paper S3.
- **Software**:
  - [Visual Studio Code](https://code.visualstudio.com/)
  - [PlatformIO IDE Extension](https://platformio.org/platformio-ide)

### Installation
1.  **Clone the Repository**:
    ```bash
    git clone https://github.com/alexcircuits/M5Stack-PaperS3-UniRead.git
    cd M5Stack-PaperS3-UniRead
    ```

2.  **Open in PlatformIO**:
    Open the project folder in VS Code. PlatformIO should automatically detect the project and install necessary dependencies (based on `platformio.ini`).

3.  **Build and Upload**:
    - Connect your M5Stack Paper S3 via USB.
    - Click the **PlatformIO: Upload** button (right arrow icon) in the bottom status bar.
    - Alternatively, run:
      ```bash
      pio run -t upload
      ```

4.  **Prepare SD Card**:
    - Format your SD card to FAT32.
    - Create a `books` folder and place your `.epub` files there.
    - Create a `fonts` folder if you wish to use custom fonts.

### Usage
- **Navigation**: Tap the right side of the screen significantly to turn the page forward, left side to go back.
- **Menu**: Tap the center of the screen to toggle the menu bar.
- **File Upload**: Enable Wi-Fi from the settings, note the IP address, and visit it in your computer's browser to upload files.

## Credits
- Based on the work by Guy Turcotte and other contributors to the ESP32 EPub reader ecosystem.
- Uses `lib_freetype` for font rendering.

## License
MIT License. See `licenses.txt` for details.
