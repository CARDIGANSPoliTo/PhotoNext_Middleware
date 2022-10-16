/*******************************************************************************
* The client will take care of the name of the interrogator used.              *
* Then according to the name of this interrogator it will proceed.             *
* Wrong interrogator name or Missing of the interrogator name --> ERROR.       *
* A listener object is created at the beginning.                               *
* Listener is initialized and prepared to start listening for any data arrival *
* So in other words the Client will be the Listener itself.                    *
* Client will also be responsible for creating the collection name that will   *
* inserted in the MongoDB server.                                              *
* Collection name is created as a global variable since it will be used by     *
* other objects while inserting data in MongoDB specially by MONGODAO object.  *
* Closing Client Signal Handling is present that will Break the While loops in *
* the listener and parser object by making i_connected and i_running to false  *
* Closing Client by a signal can be done by : CTRL+Z or CTRL+C                 *
 *******************************************************************************/

/*******************************************************************************
* Included Libraries.
*******************************************************************************/
#include "../../include/SmartScanBlue/client.hpp"

/*******************************************************************************
* Global Variables.
*******************************************************************************/
Listener ssi;
string collectionName;

using namespace std;
using namespace std::chrono;

/*******************************************************************************
* Creating Collection Name Function.
*******************************************************************************/
string create_collname(std::string interrogatorName)
{

    /*****************************************************************
    *                                                                *
    * Creating the name of the collection as YYYYMMDDHHMMSSmmmm      *
    * --> added milliseconds                                          *
    * --> added Name of Interrogator                                  *
    *                                                                *
    *****************************************************************/
  	
    struct timespec current_time;
    uint64_t curr_time;
    time_t T = time(NULL);
    struct tm tm = *localtime(&T);
    clock_gettime(CLOCK_REALTIME, &current_time);
    char coll[28];
    sprintf(coll,"SMARTSCAN_%04d%02d%02d%02d%02d%02d%03d", 1900+tm.tm_year, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,(int)current_time.tv_nsec/1000000);

    std::string collname = coll;
    return collname;

}
/*******************************************************************************
* Signal handling Mongodb.
*******************************************************************************/
void closing_client(int signal)
{

    std::cout << std::endl << "----------------------------------------------------------" << std::endl;
    std::cout << "Exiting client with Signal(" << signal << ")                  " << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;

    syslog(LOG_INFO, "Exiting client");

    /*****************************************************************************
    * Updating i_running and i_connected To Break The Recieving Data While Loops.*
    *****************************************************************************/
    ssi.i_running.store(false);
    ssi.i_connected.store(false);
}

/**********************************
* MAIN PROCESS
**********************************/
int main(int argc, char **argv)
{

  signal(SIGINT, closing_client);
  signal(SIGTSTP, closing_client);
  printf("a\n");
  collectionName = create_collname("SMARTSCAN");
  printf("a\n");
  ssi.i_init();
  ssi.i_running.store(true);
  ssi.ssi_listen();
 return STATUS_OK;
}

