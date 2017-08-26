# Drone-Sensor-Pkg
Code for Ocean Alliance's onboard drone wind strength, direction, and altitude sensor.

hc-12 messenger: Run on Arduino Nano equipped with 3 Rev. P Wind Sensors, an Adafruit Barometer (BMP 280), and a Sparkfun GARMIN Lidar-LITE V3. This code collects and sends data packets from the sensors to the paired transmitter.

hc-12 receiver: Run on Arduino Uno equipped with an i2c LCD screen. Parses data received and outputs to screen.
