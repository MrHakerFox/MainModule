/*
 * main.cpp
 *
 * Description	:
 *
 *  Created on	: 08.06.2013
 *      Author	: Fox
 *
 *  Revisions	:
 */



#include <iostream>
#include <fstream>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <stdio.h>



#include <FWakeMaster.hpp>
#include <FWakeExtCmd.hpp>
#include "shc_config.hpp"
#include <FPHY.hpp>




using namespace std;



FWakeMaster wmaster;
char serPort[ 20 ];



/*
	Function 'usage'

	Description: this function prints the user help on command line arguments

	IN:		nothing

	OUT:	nothing
 */
void usage(void)
{
	cout << endl << "Usage:" << endl;
	cout <<  "\t-p<serial port name> - port that can be used to communicate with module";
	cout << endl << "For example: shcng.exe -pcom254" << endl;
}



void wakeDelay()
{
	Sleep( 500 );
	//usleep( 50000 );
}



bool isTimeEq( int h0, int m0 )
{
	time_t rawtime;
	time( & rawtime );
	struct tm* lt = localtime( &rawtime );

	return ( lt->tm_hour == h0 ) && ( lt->tm_min == m0 );
}



bool isTimeBetween( int h0, int m0, int h1, int m1  )
{
	time_t rawtime;
	time( & rawtime );
	struct tm* lt = localtime( &rawtime );

	int min0 = h0 * 60 + m0;
	int min1 = h1 * 60 + m1;
	int min = lt->tm_hour * 60 + lt->tm_min;

	// 23:31 1411
	// 6:0 360

	if( ( min1 == min0 ) && ( min1 == min ) )
	{
		return true;
	}

	if( min0 > min1 )
	{
		int delta = 1440 - min0;
		min0 = 0;

		min1 += delta;

		min += delta;
		if( min > 1440 )
			min -= 1440;
	}

	return ( ( min >= min0 ) && ( min <= min1 ) );
}



int main( int argc, char *argv[] )
{
	cout << endl << endl << "******SHC NG******\r\nAuthor: Yakimov Y.A.";
	cout << endl << "Compiled on: " << __DATE__ " at " << __TIME__;

	if( argc < 2 )
	{
		usage();
		return 0;
	}

	while( ( argc > 1 ) && ( argv[1][0] == '-' ) )
	{
		switch ( argv[1][1] )
		{
			case 'p':
				strcpy( serPort, &argv[1][2] );
				break;

			default:
				cout << endl << "Wrong Argument: " << argv[1];
				usage();
				return 0;
		}

		++argv;
		--argc;
	}

	cout << endl << endl << "Retrieving configuration of network...";
	fstream netCfgFile;
	netCfgFile.open( NET_CFG_FILE.c_str() );

	if( !netCfgFile.is_open() )
	{
		cout << endl << "Can't open network configuration file!" << endl;
		return 0;
	}

	cout << endl << endl;
	uint32_t lineCnt = 0;
	while( netCfgFile.good() )
	{
		string line;
		getline( netCfgFile, line );

		if( line.length() )
		{
			if( line.at( 0 ) != '#' )
			{
				cout << line << endl;
				lineCnt++;
			}
		}
	}

	cout << endl << "Found " << lineCnt << " modules.";

	netCfgFile.close();


	cout << endl << "Trying to open communication port...";

	if( wakePHYOpen( serPort ) == FRetVals::OK )
		cout << "OK";
	else
	{
		cout << "FAILED!";
		return 0;
	}

	cout << endl << endl << "Trying to scan modules...";

	//wmaster.setData( 0, 0 );
	//wmaster.exchange( 0x09, 0x44, 1 );

	//usleep( 50000 );

	//wmaster.setData( 8, 0 );
	//wmaster.exchange( 0x09, 0x43, 1 );

	bool cellarLight = false;
	bool streetToiletLight = false;

	time_t rawtime;
	time( & rawtime );
	struct tm* localTime = localtime( &rawtime );

	printf( "\r\nLocal time: %02u.%02u.%04u %02u:%02u:%02u", localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year + 1900,
																localTime->tm_hour, localTime->tm_min, localTime->tm_sec );

	bool shcFirstRun = true;

	while( 1 ) // 0x09, 0x0b
	{
		for( uint8_t addr = 0x08; addr < 0x0c; addr++ )
		{

			if( addr != 0x09 && addr != 0x0b )
			{
				continue;
			}

			//Sleep( 50 );
						wakeDelay();
			FRetVals::value result = wmaster.exchange( addr );

			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}


			// Get key from OP
			wakeDelay();
			wmaster.setDataInd( wmaster.getData( 2 ), 0 );
			wmaster.setDataInd( wmaster.getData( 3 ), 1 );
			wmaster.setDataInd( 0, 2 );
			result = wmaster.exchange( 0x0b, FWakeCMD::OPSIMPLE_GET_STATE, 3 );
			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}

			time( & rawtime );
			localTime = localtime( &rawtime );

			if( ( ( localTime->tm_hour == 0 ) && ( localTime->tm_min == 0 ) ) || shcFirstRun )
			{
				wakeDelay();
				wmaster.setDataInd( localTime->tm_mday, 0 );
				wmaster.setDataInd( localTime->tm_mon + 1, 1 );
				wmaster.setDataInd( localTime->tm_year + 1900 - 2000, 2 );
				wmaster.setDataInd( localTime->tm_hour, 3 );
				wmaster.setDataInd( localTime->tm_min, 4 );
				wmaster.setDataInd( localTime->tm_sec, 5 );
				wmaster.exchange( 0x0b, FWakeCMD::OPSIMPLE_SET_DT, 6 );
			}

			if( isTimeBetween( 7, 0, 23, 0 ) )
			{
				// Kitchen 100
				wmaster.setDataInd( 0 );
			}
			else
			{
				// Kitchen 30
				wmaster.setDataInd( 30 );
			}

			wakeDelay();
			result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_SET_PWM1, 1 );
			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}

			if( isTimeBetween( 21, 0, 23, 30 ) || isTimeBetween( 6, 1, 8, 0 ) )
			{
				// KitToilet 100
				wmaster.setDataInd( 100 );
			}
			else
			{
				if( isTimeBetween( 23, 31, 6, 0 ) )
				{
					// KitToilet 30
					wmaster.setDataInd( 30 );
				}
				else
				{
					// KitToilet off
					wmaster.setDataInd( 0 );
				}
			}

			wakeDelay();
			result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_SET_PWM0, 1 );
			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}

			// Door-light
			wmaster.setDataInd( 0x0e );
			if( isTimeBetween( 20, 55, 7, 0 ) )
			{
				wakeDelay();
				result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_SET_DI, 1 );
			}
			else
			{
				wakeDelay();
				result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_CLR_DI, 1 );
			}
			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}


			// Toilet light
			/*wmaster.setDataInd( 4 );
			if( isTimeBetween( 20, 0, 2, 30 ) )
			{
				wakeDelay();
				result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_SET_DI, 1 );
			}
			else
			{
				wakeDelay();
				result = wmaster.exchange( 0x09, FWakeCMD::DO4PWM2_CLR_DI, 1 );
			}
			if( result != FRetVals::OK )
			{
				printf( "\r\n\r\n\r\nAddr: 0x%02X", addr );
				FRetVals::printMsg( result );

				FWakeHeader lastTx = wmaster.getLastTxHeader();
				FWakeHeader lastRx = wmaster.getLastRxHeader();

				printf( "\r\nLast Tx Address: 0x%02X", lastTx.addr );
				printf( "\r\nLast Tx Command: 0x%02X", lastTx.cmd );
				printf( "\r\nLast Tx Num Of : 0x%02X", lastTx.n );

				printf( "\r\nLast Rx Address: 0x%02X", lastRx.addr );
				printf( "\r\nLast Rx Command: 0x%02X", lastRx.cmd );
				printf( "\r\nLast Rx Num Of : 0x%02X", lastRx.n );

				printf( "\r\n\r\nTx total: %i", wmaster.getTxTotal() );
				printf( "\r\nTx totalFailed: %i", wmaster.getTxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getTxTotalFailed() / ( float )wmaster.getTxTotal() * 100  );
				printf( "\r\n\r\nRx total: %i", wmaster.getRxTotal() );
				printf( "\r\nRx totalFailed: %i", wmaster.getRxTotalFailed() );
				printf( "\r\nError: %.2f", ( float )wmaster.getRxTotalFailed() / ( float )wmaster.getRxTotal() * 100  );
			}*/

			shcFirstRun = false;

		}
	}

	return 0;

	FRetVals::value result;

	uint8_t srcBuff[] = { 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0 };
	printf( "\r\nFill buffer with data..." );
	result = wmaster.setData( srcBuff, sizeof( srcBuff ) );
	FRetVals::printMsg( result );

	if( result != FRetVals::OK )
		return 0;

	uint8_t rxCmd, rxN;

	result = wmaster.exchange( 0x50, 0x7f, 16, &rxCmd, &rxN );
	FRetVals::printMsg( result );

	if( result != FRetVals::OK )
		return 0;

	printf( "\r\nRxCmd 0x%02x, RxN 0x%02x", rxCmd, rxN );

	if( rxN )
	{
		printf( "\r\nData: " );

		for( int i = 0; i < rxN; i++ )
			printf( "0x%02x ", wmaster.getData( i ) );
	}

}




