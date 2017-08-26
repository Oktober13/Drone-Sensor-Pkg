# Drone-Sensor-Pkg
Code for Ocean Alliance's onboard drone wind strength, direction, and altitude sensor.
Developed by L. Zuehsow c. 8-23-17.


# hc-12 messenger.ino
Run on Arduino Nano equipped with 3 Rev. P Wind Sensors, an Adafruit Barometer (BMP 280), and a Sparkfun GARMIN Lidar-LITE V3. This code collects and sends data packets from the sensors to the paired transmitter.

![wind](https://github.com/Oktober13/Drone-Sensor-Pkg/blob/ima/images/IMG_5803.JPG)

# hc-12 receiver.ino
Run on Arduino Uno equipped with an i2c LCD screen. Parses data received and outputs to screen.

![receiver](https://github.com/Oktober13/Drone-Sensor-Pkg/blob/ima/images/IMG_5809.JPG)
