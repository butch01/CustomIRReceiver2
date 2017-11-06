/**
 * CustomIrReceiver2.h
 *
 * This class handles Receiving infrared data with own RC Protocol
 */

#ifndef CUSTOMIRRECEIVER_H_
#define CUSTOMIRRECEIVER_H_


class CustomIRReceiver2
{
	public:


	// general stuff
	CustomIRReceiver2();
	//virtual ~CustomIRReceiver2();


	// for normal use
	int getAnalogValues(int id);
	int getDigitalValues(int id);
	void processData();

	// debug functions
	void debuglogPrintRingbuffer(int channelMin, int channelMax);
	void debuglogPrintFilteredDigitalValues();
	void debuglogPrintFilteredAnalaogValues();

	//private:
	void resetValues();
	void addReceivedValueToFilter(int channel, unsigned long timestamp, long receivedValue );
	void updateFilteredReceivedValues(int channel);
};

#endif /* CUSTOMIRRECEIVER_H_ */
