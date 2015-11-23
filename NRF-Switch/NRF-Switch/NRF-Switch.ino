#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9, 10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
const int pin = 7;
bool isOn = false;
int offState = HIGH;
int onState = LOW;

void setup()
{
	Serial.begin(57600);
	printf_begin();
	printf("NRF-Switch\r\n");

	radio.begin();
	radio.setRetries(15, 15);
	radio.setPALevel(RF24_PA_MAX);
	radio.enableDynamicPayloads();
	radio.setDataRate(RF24_250KBPS);
	radio.openWritingPipe(pipes[0]);
	radio.openReadingPipe(1, pipes[1]);
	radio.startListening();
	radio.printDetails();

	pinMode(pin, OUTPUT);
	digitalWrite(pin, offState);
}

void loop()
{
	char* received = nrfReceive();
	if (received != NULL)
	{
		radio.stopListening();
		nrfCreateResponse(received);
		radio.startListening();
	}

	delay(50);
}

char* nrfReceive()
{
	if (radio.available()) {
		printf("Got message.\n\r");
		char RecvPayload[31] = "";

		int len = radio.getDynamicPayloadSize();
		radio.read(&RecvPayload, len);

		printf("Received: ");
		printf(RecvPayload);
		printf("\r\n");

		return RecvPayload;
	}

	return NULL;
}

void nrfCreateResponse(char* message)
{
	if (message[0] == '1')
	{
		int len = radio.getPayloadSize();
		if (isOn)
			radio.write("1", 1);
		else
			radio.write("0", 1);

		printf("Sent response.\n\r");
	}
	else if (message[0] == '2')
	{
		isOn = message[1] == '0' ? false : true;
		int len = radio.getPayloadSize();
		if (isOn)
		{
			digitalWrite(pin, onState);
			radio.write("1", 1);
		}
		else
		{
			digitalWrite(pin, offState);
			radio.write("0", 1);
		}

		printf("Changed status and sent response.\n\r");
	}
}