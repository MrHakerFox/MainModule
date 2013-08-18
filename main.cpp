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
	usleep( 50000 );
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

	time_t rawtime;
	time( & rawtime );
	struct tm* localTime = localtime( &rawtime );

	printf( "\r\nLocal time: %02u.%02u.%04u %02u:%02u:%02u", localTime->tm_mday, localTime->tm_mon + 1, localTime->tm_year + 1900,
																localTime->tm_hour, localTime->tm_min, localTime->tm_sec );

	bool shcFirstRun = true;

	while( 1 )
	{
		for( uint8_t addr = 0x08; addr < 0x0c; addr++ )
		{
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

			wakeDelay();
			wmaster.exchange( 0x08, FWakeCMD::HUB_PORCH_GET_ST_TEMP_RH );

			uint8_t rh = wmaster.getData( 2 );

			wakeDelay();
			wmaster.exchange( 0x0a, FWakeCMD::FWAKE_CMD_AI6DI2_GET_TEMPS );

			// Get key from OP
			wakeDelay();
			wmaster.setDataInd( wmaster.getData( 2 ), 0 );
			wmaster.setDataInd( wmaster.getData( 3 ), 1 );
			wmaster.setDataInd( wmaster.getData( rh ), 2 );
			wmaster.exchange( 0x0b, FWakeCMD::OPSIMPLE_GET_STATE, 1 );
			if( wmaster.getData( 0 ) == 1 )
			{
				cellarLight = !cellarLight;
			}

			wakeDelay();
			wmaster.setDataInd( 8, 0 );
			wmaster.exchange( 0x09, cellarLight ? FWakeCMD::DO4PWM2_SET_DI : FWakeCMD::DO4PWM2_CLR_DI, 1 );

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




