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



#include <FWakeMaster.hpp>
#include "shc_config.hpp"



using namespace std;



int main()
{
	cout << endl << endl << "******SHC NG******\r\nAuthor: Yakimov Y.A.";
	cout << endl << "Compiled on: " << __DATE__ " at " << __TIME__;

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

	cout << endl << "Found " << lineCnt << "modules.";

	netCfgFile.close();
}




