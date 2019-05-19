//============================================================================
// Name        : AirPortMusic.cpp
// Author      : Jon Dellaria
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <DLog.h>
#include <LinuxCommand.h>
#include <Directory.h>
#include <unistd.h>
#include <sys/wait.h>
#include <algorithm>

#include "IsolatedContainer.h"

#define CHILDCONTAINERNAME "/home/jdellaria/eclipse-workspace/addFileDuplicate/Release/addFileDuplicate"


#include "configurationFile.h"
#include "ApplicationModes.h"

int main_event_handler();
int eventHandler();
int configApp();
void doRecurseDirectory(char const * text, int x);
int callChildContainer(string text);
int checkChildContainers();
int incrementCurrentlyActiveContainer();
using namespace std;

int songFD = 0;
DLog myLog;

configurationFile myConfig;
ApplicationModes myAppModes;
//#define NUMBEROFCONTAINERS 20
IsolatedContainer *myContainerPtr; //= new IsolatedContainer[NUMBEROFCONTAINERS];
int currentlyActiveContainer = 0;
long numberOfContainers = 0;

int fixDirectoryCount;


using namespace std;
bool cmdOptionExists(char** begin, char** end, const string& option);
char* getCmdOption(char ** begin, char ** end, const std::string & option);
int removeEmptyDirectory(const char* directory);
void doRemoveEmptyDirectories(char const * text, int x);
void myCallBack(char const * text, int event);
int waitForAllChildContainersToClose();

bool cmdOptionExists(char** begin, char** end, const string& option)
{
    return std::find(begin, end, option) != end;
}

char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

int main(int argc, char* argv[])
{
	int returnValue;
	string message;
	char ibuffer [33];
	const char *myFileName;

//	char jon[returnValue];

	myConfig.getConfiguration("config.xml");
	Directory myDirectory;
	time_t Start_t, End_t;
	int time_task1;

	configApp();
	// Set the action defaults
/*	if(cmdOptionExists(argv, argv+argc, "-containers")||cmdOptionExists(argv, argv+argc, "-c"))
	{

	}
*/
    char * STRnumberOfContainers = getCmdOption(argv, argv + argc, "-c");

    if (STRnumberOfContainers)
    {
    	char *pNext;
    	numberOfContainers = strtol (STRnumberOfContainers, &pNext, 10);
        // Do interesting things
        // ...
    }
    else
    {
    	numberOfContainers = 1;
    }
	std::cout << "Number of Containers " << numberOfContainers << "\n" ;
	IsolatedContainer *myDummyPtr = new IsolatedContainer[numberOfContainers];
	myContainerPtr = myDummyPtr;
	myAppModes.setPlayMode (PLAY_ACTION_PLAY);
//	myAppModes.setContinuous();
//	myAppModes.setManual();
//	myAppModes.setNetworkMode(NETWORK_ACTION_DISCONNECT);
	message = "airportAddress is: ";
	message.append(myConfig.airportAddress + "\n");
	myLog.print(logWarning, message);

    if( argc > 1 )
    {
        std::cout << "there are " << argc-1 << " (more) arguments, they are:\n" ;

 //       std::cout( argv+1, argv+argc, std::ostream_iterator<const char*>( std::cout, "\n" ) ) ;
        myFileName = argv[argc-1];
        std::cout << "myFileName: " << myFileName << "\n";
    }
	Start_t = time(NULL);    //record time that task 1 begins
	myDirectory.Recurse("/home/jdellaria/Desktop/doFiles", doRecurseDirectory);
	std::cout << "Waiting for children to close " << "\n";
	waitForAllChildContainersToClose();
	std::cout << "Deleting empty directories " << "\n";
	myDirectory.Recurse("/home/jdellaria/Desktop/doFiles", doRemoveEmptyDirectories);
	End_t = time(NULL);    //record time that task 1 ends
	time_task1 = difftime(End_t, Start_t);    //compute elapsed time of task 1

	message.clear();
	message.append("Task took ");
	char intbuffer[50];
	sprintf(intbuffer,"%d",time_task1);
	message.append(intbuffer);
	message.append(" seconds.");
	myLog.print(logWarning, message);
	message = "runContainer.cpp :";
	message.append(__func__);
	message.append(": runContainer exiting Normally");
	myLog.print(logWarning, message);
}

#define MAIN_EVENT_TIMEOUT 3 // sec unit
void doRemoveEmptyDirectories(char const * text, int x)
{
	if(DIRECTORYENTRYTYPE_DIR == x) // if a Directory, remove if it is empty
	{
		removeEmptyDirectory(text);
	}
}

class DirectoryCount {
public:
	DirectoryCount();
	virtual ~DirectoryCount();

	int Get(const char* startDirectory);
	void countDirectoryEntries(char const *  text, int event);

	int directoryCount;
};


void myCallBack(char const * text, int event)
{
	if(event == DIRECTORYENTRYTYPE_REG)
	{
		fixDirectoryCount++;
	}
}



void DirectoryCount::countDirectoryEntries(char const *  text, int event)
{
	directoryCount++;
}

DirectoryCount::DirectoryCount() {
	// TODO Auto-generated constructor stub
	directoryCount = 0;
}

DirectoryCount::~DirectoryCount() {
	// TODO Auto-generated destructor stub
}

int DirectoryCount::Get(const char* startDirectory)
{
	Directory myDirectory;

	myDirectory.Recurse(startDirectory, myCallBack);  // void (*callBack)(const char* directoryEntry, int directoryEntryType)
//	return(directoryCount);
	return(fixDirectoryCount);
}

int removeEmptyDirectory(const char* directory)
{
	int directoryCount;
	DirectoryCount myDirectoryCount;
	Directory myDirectory;
	string message;

	fixDirectoryCount = 0;
	directoryCount = myDirectoryCount.Get(directory);
//	cout << "directory Count:"  << directoryCount << ".\n";
	if (directoryCount == 0)
	{
		myDirectory.Remove(directory);
		message.clear();
		message.append("removeEmptyDirectory: directory removed:");
		message.append(directory);
		myLog.print(logWarning, message);
//		cout << "directory removed:"  << directory << ".\n";
	}
	else
	{
//				message.append("removeEmptyDirectory: directory removed:" + directory);
	}

}

void doRecurseDirectory(char const * text, int x)
{
	string message;
	if(DIRECTORYENTRYTYPE_REG == x) // if it is a file and a duplicate... then remove
	{
		message.clear();
		message.append(text);
		myLog.print(logWarning, message);
		callChildContainer(message);
	}
}

int callChildContainer(string originalFileName)
{
	char originalFileHash[36];
	char destinationFileHash[34];
	string destinationFileName;
	string destinationVersionFileName;
	int returnValue;
	string message;

	char const *darg[7]={CHILDCONTAINERNAME, "-d",  NULL,  NULL, NULL}; // -d tells child to delete file if duplicate
	darg[2] = originalFileName.c_str();

	do
	{
		std::cout << "----working on file " << originalFileName << " programID:" << myContainerPtr[currentlyActiveContainer].programID << "\n" ;
		if ( myContainerPtr[currentlyActiveContainer].programID == 0)
		{
			myContainerPtr[currentlyActiveContainer].OpenChild(darg);
			incrementCurrentlyActiveContainer();
			break;
		}
		else
		{
			incrementCurrentlyActiveContainer();
		}
		checkChildContainers();
	}while ( true);
	return 0;
}

int incrementCurrentlyActiveContainer()
{
	currentlyActiveContainer++;
	if (currentlyActiveContainer >= numberOfContainers)
	{
		currentlyActiveContainer = 0;
	}
	return 0;
}

int checkChildContainers()
{
	int x;
	for (x = 0; x < numberOfContainers; x++)
	{
//		std::cout << "----checkChildContainers x=" << x << " \n" ;
		if ( myContainerPtr[x].programID != 0)
		{
			if ( myContainerPtr[x].hasChildCompleted() > 0)
			{
				myContainerPtr[x].CloseChild();
			}
		}
	}
//	std::cout << "----exiting checkChildContainers " << " \n" ;
	return 0;
}

int waitForAllChildContainersToClose()
{
	int x;
	int allClosed = 0;
	do
	{
		allClosed = 0;
		for (x = 0; x < numberOfContainers; x++)
		{
//			std::cout << "----waitForAllChildContainersToClose x=" << x << " \n" ;
			if ( myContainerPtr[x].programID != 0)
			{
				if ( myContainerPtr[x].hasChildCompleted() > 0)
				{
					myContainerPtr[x].CloseChild();
					std::cout << "----waitForAllChildContainersToClose x=" << x << " has closed \n" ;
				}
			}
			else
			{
				allClosed++; //if any are open.. this will cause the while loop to [erform again.
			}

		}
	}while (allClosed != numberOfContainers);
	std::cout << "----exiting waitForAllChildContainersToClose " << " \n" ;
	return 0;
}

int configApp()
{
	string message;

	myLog.logFileName = myConfig.logFileName;
	myLog.printFile = myConfig.logPrintFile;
	myLog.printScreen = myConfig.logPrintScreen;
	myLog.printTime = myConfig.logPrintTime;

	if (myConfig.logValue.find("logDebug")!=string::npos)
	{
		myLog.logValue = logDebug;
		message = "myLog.logValue = logDebug";
		myLog.print(logInformation, message);
	}
	if (myConfig.logValue.find("logInformation")!=string::npos)
	{
		myLog.logValue = logInformation;
		message = "myLog.logValue = logInformation";
		myLog.print(logInformation, message);
	}
	if (myConfig.logValue.find("logWarning")!=string::npos)
	{
		myLog.logValue = logWarning;
		message = "myLog.logValue = lo(more) arguments, they aregWarning";
		myLog.print(logInformation, message);
	}
	if (myConfig.logValue.find("logError")!=string::npos)
	{
		myLog.logValue = logError;
		message = "myLog.logValue = logError";
		myLog.print(logInformation, message);
	}
	if (myConfig.playContinuous == true)
	{
		myAppModes.setContinuous();
		message = "Continuous Play Mode";
		myLog.print(logInformation, message);
	}
	else
	{
		myAppModes.setManual();
		message = "Manual Play Mode";
		myLog.print(logInformation, message);
	}
	return (1);
}

int eventHandler()
{
	int n;
	long iVolume;
	char buffer[1024];
	struct sockaddr_in from;
	char *ps=NULL;
	int returnValue = PLAY_ACTION_NORMAL;
	string message;

//	playMode = PLAY_ACTION_PLAY;
	bzero(buffer,1024);


	usleep(100); // let other processes have the CPU for 100 microsecondsint WaitForCommandToExit()

	return (returnValue);
}


