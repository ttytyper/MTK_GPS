# Library for talking to MT3318, MT3320 and MT3339 GPS receivers from GlobalTop

This library extends TinyGPS++ to enable sending commands to the GPS module.

TinyGPS++ itself requires one-way communication. You would use it's encode() function to pass it characters. Such data could come from an arbitrary source, not necessarily an actual GPS. However, this library requires two-way communication to both send commands and receive acknowledgements and therefore needs to connect to a stream. A very basic example boils down to:
	
	PMTK_GPS pmtk(Serial1);
	void setup() {
		Serial1.begin(9600);
	}
	void loop() {
		pmtk.loop();
	}


Please refer to the [official datasheet] for a full explanation of all the functionality.

[official datasheet]: https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf
