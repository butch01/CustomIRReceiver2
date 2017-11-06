// Do not remove the include below
#include "CustomIRReceiver2.h"
#include "IRremote.h"
#include "MyRemoteControlProtocol.h"





// DEFINE DEBUGGING
	#define IS_DEBUG 1 // enable debug logging
	#define DEBUG_LOG_RINGBUFFER_MIN 3
	#define DEBUG_LOG_RINGBUFFER_MAX 4


// DEFINE APPLICATION STUFF
	#define NUMBER_OF_FILTER_ENTRIES 1
	#define NUMBER_OF_FILTER_CHANNELS 4
	#define FILTER_TIMEOUT_MILLIS 750
	#define NUMBER_OF_FILTER_ELEMENTS 2

	#define NUMBER_OF_DIGITAL_VALUES 6
	#define NUMBER_OF_ANALOG_VALUES 4



	#define IR_RECEIVE_PIN A5


// Application global variables and objects
	// IR receiver
		IRrecv irrecv(IR_RECEIVE_PIN);
		decode_results results;

	// Protocol Parsing
		MyRemoteControlProtocol myRCProtocol;
		int analogValues[NUMBER_OF_ANALOG_VALUES]; // is only temporary result for current channel. Will not hold it for all channels to save memory
		int digitalValues[NUMBER_OF_DIGITAL_VALUES]; // is only temporary result for current channel. Will not hold it for all channels to save memory

		// stores results in ringbuffer
		/**
		 * description of 3 dimensional array of lastResultsRingBuffer
		 * 1st dimension: RC Channel
		 * 2nd dimension: Number of entries of ringbuffer elements, which holds results from IR receiver. All these values are used for calc the average of values
		 * 3rd dimension: number of values we store per entry. Currently we store 2 elements: [0] timestamp as long [1] received data by IR receiver as long
		 */
		unsigned long lastResultsRingBuffer[NUMBER_OF_FILTER_CHANNELS][NUMBER_OF_FILTER_ENTRIES][NUMBER_OF_FILTER_ELEMENTS];
		int  lastResultsRingbufferCurrentId = 0;


CustomIRReceiver2::CustomIRReceiver2()
{
	Serial.begin(9600);
	// instanciate the infrared library on given pin

	// start the IR Receiver
	irrecv.enableIRIn();
	Serial.println("IR Receiver enabled");



	// initialize the ring buffer array
		for (int channel=0; channel<NUMBER_OF_FILTER_CHANNELS; channel++)
		{
			for (int entry=0;entry<NUMBER_OF_FILTER_ENTRIES;entry++)
			{
				for (int element=0; element<NUMBER_OF_FILTER_ELEMENTS; element++)
				{
					// initialize with 0
					lastResultsRingBuffer[channel][entry][element]=0;
				}
			}
		}

	// reset / initialize the temporary values of  of analog and digital results.
	resetValues();
}


/**
 * set values for temporary arrays of analogValues[] and digitalValues[] to 0.
 */
void CustomIRReceiver2::resetValues()
{
	  // initialize analog values
	  for (int i=0; i<NUMBER_OF_ANALOG_VALUES;i++)
	  {
		  // initialize with 0
		  analogValues[i]=0;
	  }

	  // initialize digital values
	  for (int i=0; i<NUMBER_OF_DIGITAL_VALUES;i++)
	  {
		  // initialize with 0
		  digitalValues[i]=0;
	  }
}



void CustomIRReceiver2::processData()
{
	#if IS_DEBUG
		Serial.println("in CustomIRReceiver2::processData()");
	#endif
	// if received new transmission -> store it in ringbuffer
	if (irrecv.decode(&results))
	{
		// if we are able to decode
		//Serial.println(results.decode_type);
		Serial.print("received: ");
		Serial.println(results.value, BIN);
		Serial.println(results.value, HEX);

		// update the filter:
//		myRCProtocol.setProtocolValue(results.value); // needed for getting rc channel
//		addReceivedValueToFilter(myRCProtocol.getRCChannel(), millis(), results.value); // using current millis as timestamp because it is near enough to received timestamp


		irrecv.resume(); // Receive the next value
	}

  // in all cases do followig stuff.

  // debug the ringbuffer
	#if IS_DEBUG
		//debuglogPrintRingbuffer(DEBUG_LOG_RINGBUFFER_MIN,DEBUG_LOG_RINGBUFFER_MAX);
	#endif


  // calculate the filtered result (for channel 3 only)
	//updateFilteredReceivedValues(3);

	#if IS_DEBUG
		//debuglogPrintFilteredAnalaogValues();
		//debuglogPrintFilteredDigitalValues();
	#endif



}


/**
 * prints all ringbuffer for selected channels
 */
void CustomIRReceiver2::debuglogPrintRingbuffer(int channelMin, int channelMax)
{
	Serial.println("output of ringbuffer:");
	Serial.print("  range: ");
	Serial.print(channelMin);
	Serial.print(" to ");
	Serial.println(channelMax);

	for (int channel=channelMin; channel < channelMax; channel++)
	{
		for (int entry=0;entry<NUMBER_OF_FILTER_ENTRIES;entry++)
		{
			Serial.print("  ch=");
			Serial.print(channel);
			Serial.print(" e=");
			Serial.print(entry);

			for (int element=0; element<NUMBER_OF_FILTER_ELEMENTS; element++)
			{
				switch (element)
				{
					case 0:
						Serial.print(" time= ");
						Serial.print(lastResultsRingBuffer[channel][entry][0]);
						break;
					case 1:
						Serial.print(" data=");
						Serial.println(lastResultsRingBuffer[channel][entry][1], BIN);
						break;
				}
			}

		}
	}
}


/**
 * prints filtered digital results
 */
void CustomIRReceiver2::debuglogPrintFilteredDigitalValues()
{
	Serial.println("filtered digitalValues ");
	for (int i=0;i<NUMBER_OF_DIGITAL_VALUES;i++)
    {
    	Serial.print("  digital ");
		Serial.print(i);
    	Serial.print("=");
    	Serial.print(digitalValues[i]);
    	Serial.print("\n");
    }
}

/**
 * prints filtered analog results
 */
void CustomIRReceiver2::debuglogPrintFilteredAnalaogValues()
{
	Serial.println("filtered analogValues ");
	for (int i=0;i<NUMBER_OF_ANALOG_VALUES;i++)
    {
    	Serial.print("  analog ");
		Serial.print(i);
    	Serial.print("=");
    	Serial.print(analogValues[i]);
    	Serial.print("\n");
    }
}


/**
 * add received value to the filter
 */
void CustomIRReceiver2::addReceivedValueToFilter(int channel, unsigned long timestamp, long receivedValue)
{
	// increment the ringBuffer current id
	lastResultsRingbufferCurrentId++;

	// go back to 0 if out of bounds
	if (lastResultsRingbufferCurrentId > NUMBER_OF_FILTER_ENTRIES -1)
	{
		lastResultsRingbufferCurrentId=0;
	}

	// set the time
	lastResultsRingBuffer[channel][lastResultsRingbufferCurrentId][0]=timestamp;

	// set the value
	lastResultsRingBuffer[channel][lastResultsRingbufferCurrentId][1]=receivedValue;
}


/**
 * updates the filtered received values for a given channel.
 * global vars will be updated.
 * return the avg of the last NUMBER_OF_FILTER_ENTRIES,
 * but ignore entries older than FILTER_TIMEOUT_MILLIS
 *
 */
void CustomIRReceiver2::updateFilteredReceivedValues(int channel)
{
	resetValues();

	#if IS_DEBUG
		Serial.println("in updateFilteredReceivedValues");
	#endif

	int usedElements=0;

	// remember the current time for comparing all entries with same timestamp
	unsigned long currentTime = millis();

	// go through complete ringbuffer for given channel
	for (int entry=0; entry <NUMBER_OF_FILTER_ENTRIES; entry++)
	{
		// check if time is not older than timeout

		//if (lastResultsRingBuffer[channel][entry][0] > (currentTime - timeout) || (currentTime - timeout < 0) )
		if (currentTime - lastResultsRingBuffer[channel][entry][0] < FILTER_TIMEOUT_MILLIS && lastResultsRingBuffer[channel][entry][0] > 0 )
		{

			// mark element as used (because it is not timed out)
			usedElements++;

			// put result into protocol parser
			myRCProtocol.setProtocolValue(lastResultsRingBuffer[channel][entry][1]);

			#if IS_DEBUG
				Serial.print("  usedElements=");
				Serial.print(usedElements);
				Serial.print(" processing: entry=");
				Serial.print(entry);
				Serial.print(" time=");
				Serial.print(lastResultsRingBuffer[channel][entry][0]);
				Serial.print(" data=");
				Serial.print(lastResultsRingBuffer[channel][entry][1]);
				Serial.print("\n");
			#endif


			// process analog values

			#if IS_DEBUG
				Serial.print("  calculating Analog values=");
			#endif
			for (int i=0; i<NUMBER_OF_ANALOG_VALUES; i++)
			{
				// accumulate the global result value. Division by used element count comes later
				#if IS_DEBUG
					Serial.print("    analog ");
					Serial.print(i);
					Serial.print("=");
					Serial.print(analogValues[i]);
					Serial.print("+");
					Serial.print(myRCProtocol.getAnalog(i));
					Serial.print("\n");
				#endif

				analogValues[i]=analogValues[i]+myRCProtocol.getAnalog(i);

			}

			// process digital values
			for (int i=0; i<NUMBER_OF_DIGITAL_VALUES; i++)
			{
				// accumulate the global result value. Division by used element count comes later
				digitalValues[i]=digitalValues[i]+myRCProtocol.getDigital(i);
			}
		}
		else
		{
			#if IS_DEBUG
				Serial.print("  ignoring entry ");
				Serial.print(entry);
				Serial.print(". age=");
				Serial.print(millis() - lastResultsRingBuffer[channel][entry][0]);
				Serial.print("\n");
			#endif
		}
	}
	for (int i=0; i<NUMBER_OF_ANALOG_VALUES; i++)
	{
		#if IS_DEBUG
			Serial.print("  filterresult analog value ");
			Serial.print(i);
			Serial.print("=");
			Serial.print(analogValues[i]);
			Serial.print("/");
			Serial.print(usedElements);
			Serial.print("=");
			Serial.print(analogValues[i]/usedElements);
			Serial.print("\n");
		#endif
		analogValues[i]=analogValues[i]/usedElements;
	}

	for (int i=0; i<NUMBER_OF_DIGITAL_VALUES; i++)
	{
		if ( (digitalValues[i]*10/usedElements) >=5)
		{
			digitalValues[i]=1;
		}
		else
		{
			digitalValues[i] = 0;
		}
	}
}


/**
 * returns the value of a specific analog value to consumers
 */
int CustomIRReceiver2::getAnalogValues(int id)
{
	return analogValues[id];
}


/**
 * returns the value of a specific analog value to consumers
 */
int CustomIRReceiver2::getDigitalValues(int id)
{
	return digitalValues[id];
}
