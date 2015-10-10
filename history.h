#include "iostream"
#include "fstream"
#include "list"
#include "string"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "string.h"

#ifndef HIST_FILE_NAME
#define HIST_FILE_NAME ".MyHist"
#endif

using namespace std;

int histLength=5;
list<string> histBuff;


char *strtrim( const char* text){

	char *text2 = (char*) malloc( strlen(text) );
	strcpy( text2, text);

	while(*text2 ==' ') text2++;
	int len= strlen(text2);
	while(text2[len-1]==' '){
		text2[len-1]==0;
		len--;
	}
	return text2;
}


void addToHist(char *cmd){

	// Replace the \n with \0
	char temp[strlen(cmd)];
	strcpy(temp, cmd);
	int i = 0;
	while( temp[i] != '\n' )
		i++;
	temp[i] = '\0';

	if( strcmp(strtrim(temp), (char*)"history")==0 && histBuff.size()>0 &&
			strcmp(strtrim(temp), histBuff.back().c_str() ) == 0)
		return;

	string str(temp);

	if(  histBuff.size() == histLength ){
		histBuff.pop_front();
	}

	histBuff.push_back( str );
}


void displayHist(){

	int count = 0;

	int histBuffSize = histBuff.size();

	for ( list<string>::iterator i = histBuff.begin() ; i != histBuff.end() ; ++i ) {
		cout << ++count << "  " << *i << endl;
	}
}


void persistHistoryToDisk(){

	int fd = creat( (char*)".MyHist", 00666 );

	if (fd==-1)
	{
		printf("History file could not be saved.\n");
	}
	ftruncate(fd, 0);
	
	for ( list<string>::iterator i = histBuff.begin() ; i != histBuff.end() ; ++i ) {
		write(fd, (*i).c_str(), strlen( (*i).c_str() ) );
		write(fd, "\n", 1);
	}
	close(fd);

}

void loadHistoryFromDisk(){

	histBuff.clear();

	int fd = open( (char*)".MyHist", 00666 ), i = 0;
	if (fd==-1) {
		return;
	}
	char ch, str[1024];

	while( read( fd, &ch, 1 ) > 0 ){
		if(ch=='\n'){
			string s(str);
			histBuff.push_back( s );
			i = 0;
			bzero(str, 1024 * sizeof(char));
		}
		else
			str[ i++ ] = ch;
	}
	close(fd);
}


char* stringToCharPointer( string str ){

	char *chArr = (char*) malloc( ( str.size() + 1 ) * sizeof(char) );
	copy( str.begin(), str.end(), chArr );
	chArr[ str.size() ] = '\0';

	return chArr;
}

/*
 * returns the command present line number "position" in the history file
 */ 
string getEventAtPosition( int position ){
	int i = 1;
	for( list<string>::iterator it = histBuff.begin(); it != histBuff.end(); it++ ){
		if(i == position)
			return *it;
		else
			i++;
	}
	return (char*)"";
}


char* getBangCommandFromHist( char *bangCommand){


	if( bangCommand[1]=='!' ){

		char *targetCmd = stringToCharPointer( histBuff.back() );
		return targetCmd;
	}
	else if( bangCommand[1]=='-' ){
		// handles the commands of the form !-10abc

		// Overwrite the trailing \n with \0
		bangCommand[ strlen(bangCommand)-1 ] = '\0';

		// Find the location of the command in the history file.
		char serialNum[512];
		bzero(serialNum, 5);
		int indexOfBangCmd=2;
		while ( indexOfBangCmd<strlen(bangCommand) && bangCommand[indexOfBangCmd]>='0' && bangCommand[indexOfBangCmd]<='9' ){
			serialNum[indexOfBangCmd-2] = bangCommand[indexOfBangCmd];
			indexOfBangCmd++;
		}
		serialNum[ indexOfBangCmd ] = '\0';
		
		// Fetch the command at the location
		int serialNumInHist = histBuff.size() - atoi(serialNum) + 1;

		if( serialNumInHist < 0 ){
			char *errmg = (char*)malloc(512);
			strcpy( errmg, (char*)"!-" );
			strcat( errmg, serialNum );
			strcat( errmg, (char*)": event not found" );
			return errmg;
		}

		string temp =  getEventAtPosition( serialNumInHist );
		char *targetCmd = stringToCharPointer( temp );

		char *targetCmdWithArg = (char*)malloc( strlen(targetCmd)+ strlen(bangCommand) - indexOfBangCmd );
		strcpy( targetCmdWithArg, targetCmd );
		int i = strlen(targetCmdWithArg);
		while( indexOfBangCmd < strlen(bangCommand) )
			targetCmdWithArg[ i++ ] = bangCommand[ indexOfBangCmd++ ];
		targetCmdWithArg[ i ] = '\0';

		return targetCmdWithArg;
	}
	else if( bangCommand[1]<='9' && bangCommand[1]>='0' ){

		// Overwrite the trailing \n with \0
		bangCommand[ strlen(bangCommand)-1 ] = '\0';

		char serialNum[512];
		int indexOfBangCmd=1;
		while ( indexOfBangCmd<strlen(bangCommand) && bangCommand[indexOfBangCmd]>='0' && bangCommand[indexOfBangCmd]<='9' ){
			serialNum[indexOfBangCmd-1] = bangCommand[indexOfBangCmd];
			indexOfBangCmd++;
		}
		serialNum[ indexOfBangCmd ] = '\0';
		int serialNumInHist = atoi(serialNum);

		string temp =  getEventAtPosition( serialNumInHist );
		char *targetCmd = stringToCharPointer( temp );

		char *targetCmdWithArg = (char*)malloc( strlen(targetCmd)+ strlen(bangCommand) - indexOfBangCmd );
		strcpy( targetCmdWithArg, targetCmd );
		int i = strlen(targetCmdWithArg);
		while( indexOfBangCmd < strlen(bangCommand) )
			targetCmdWithArg[ i++ ] = bangCommand[ indexOfBangCmd++ ];
		targetCmdWithArg[ i ] = '\0';


		return targetCmdWithArg;
	}

	return (char*)"";
}



