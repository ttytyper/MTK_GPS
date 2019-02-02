# Arduino library for talking to MediaTek MT3318, MT3320 and MT3339 GPS receivers

This library extends TinyGPS++ to enable sending commands to the GPS module.

TinyGPS++ itself requires one-way communication. You would use it's encode() function to pass it characters. Such data could come from an arbitrary source, not necessarily an actual GPS. However, this library requires two-way communication to both send commands and receive acknowledgements and therefore needs to connect to a stream. A very basic example boils down to:
	
	MTK_GPS mtk(Serial1);
	void setup() {
		Serial1.begin(9600);
	}
	void loop() {
		mtk.loop();
	}


Please refer to the [official datasheet] for a full explanation of all the functionality.

Only a few of the available commands have been added to the library at this point, but more are on their way. Most of them are pretty easy to add.

[official datasheet]: https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf
