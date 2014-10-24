/*
* Course: CS 100 Fall 2013
*
* First Name: <Harrison>
* Last Name: <Lee>
* Username: <hlee088>
* Email Address: <hlee088@ucr.edu>
*
* Assignment: <Homework #9>
*
* I hereby certify that the contents of this file represent
* my own original individual work. Nowhere herein is there
* code from any outside resources such as another individual,
* a website, or publishings unless specifically designated as
* permissible by the instructor or TA. */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <string>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sstream>
#include <pthread.h>
using namespace std;

const int MAXDATASIZE = 300;
pthread_mutex_t action_mutex = PTHREAD_MUTEX_INITIALIZER;

void copy(int fd_source, int fd_dest)
{
    int numbytes;
    char buf[BUFSIZ];
    while ((numbytes = read(fd_source, buf, BUFSIZ)) > 0)
        write(fd_dest, buf, numbytes);
    close(fd_source);
    close(fd_dest); 
}

void copyv2(string source, string destination)
{
    char* argv[4];
    argv[0] = "cp";
    strcpy(argv[1], source.c_str());
    strcpy(argv[2], destination.c_str());
    argv[3] = NULL;

    pid_t pid;
    switch(pid = fork())
    {
        case -1:
            cerr << "error fork()" << endl;
            exit(1);
            break;
        case 0: 
            execvp(argv[0], argv);
            cerr << "Error execvp()" << endl;
        default:
            wait(0);
            break;
    }
}

void mkdir(char* const path, int tid, string & dir_path)
{
    stringstream ss;
    ss << tid;
    string num = ss.str(); //converted tid into string
    
    string new_dir = "./Thread" + num + "files"; //make the Thread#files
	char* tmp = new char[new_dir.size()]; //copying string into char*
	strcpy(tmp, new_dir.c_str());
	char* arg_list[3];
    arg_list[0] = "mkdir";
    arg_list[1] = tmp;
    arg_list[2] = NULL;

    pid_t pid;
    switch(pid = fork())
    {
        case -1:
            cerr << "Errno: " << errno << endl;
            exit(errno);
            break;
        case 0: //child, do exec w/e here you dasdfs
            execvp(arg_list[0], arg_list);
            break;
        default:
            wait(0);
            break;
    }
	dir_path = new_dir;
	//steps to make the other folder
    string old_path(path);
    new_dir += old_path; //make the folder /etc or /bin in the new folder made
    char* str = new char[new_dir.size()];
    strcpy(str, new_dir.c_str());
    //converted the path
    char* argv[3];
    argv[0] = "mkdir";
    argv[1] = str;
    argv[2] = NULL;

    //pid_t pid;
    switch(pid = fork())
    {
        case -1:
            cerr << "Errno: " << errno << endl;
            exit(errno);
            break;
        case 0: //child, do exec w/e here you dasdfs
            execvp(argv[0], argv);
            break;
        default:
            wait(0);
            break;
    }
}

void run_Client(char* const argv[], int tid)
{
	int port = 51214; //port to use

    struct hostent *he;
    // get the host info
    if((he = gethostbyname(argv[1])) == NULL)
    {
        cerr << "gethostbyname(), in client" << endl;
        exit(1);
    }

    struct sockaddr_in their_addr; //client's info
    memset(&their_addr, '\0', sizeof(their_addr));   
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(port);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    
    int sockfd;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cerr << "socket() error in client";
        exit(1);
    }
    
	if(connect(sockfd, (struct sockaddr*)&their_addr, sizeof(their_addr)) == -1)
    {
        cerr << "connect() error in client" << endl;
        exit(1);
    } //connect successful, do stuff now
	
    //making directory
    //cout << "Making folders" << endl;
    string path_save;
    mkdir(argv[2], tid, path_save); //the name of the dir to copy 
    //path becomes ./Thread#files/etc
    
    //sending path to server
    if(write(sockfd, argv[2], sizeof(argv[2])) < 0)
    {
        cerr << "Error sending path to server" << endl;
        exit(1);
    }
    
    char fpath[BUFSIZ];
    while(read(sockfd, fpath, BUFSIZ) > 0) //receiving file names from server
    {
		string append(fpath);
        string path = path_save + append;
        //cout << "path: " << path << endl;
        int fd_source = open(fpath, O_RDWR | O_CREAT | O_TRUNC, 0666);
        int fd_dest = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
       copy(fd_source, fd_dest);
       //copyv2(fpath, path);
    }
    cout << "Finished copying " << argv[2] << endl;
    close(sockfd); //done with client
    cout << "\nClient #" << tid << " exiting...\n";
}

struct threadData
{
    int tid;
	char* argv[3];
};

void *threadWork(void* thread_id)
{
    //pthread_mutex_lock(&action_mutex);
    threadData* Data = (threadData*)thread_id;
    //cout << "Client #" << Data->tid << " connected!" << endl;
    //cerr << "Inside threadwork ";
    run_Client(Data->argv, Data->tid);
    //pthread_mutex_unlock(&action_mutex);
    pthread_exit((void*)thread_id);
}

int main(int argc, char *argv[])
{
	if (argc < 3) //didn't input hostname and directory
    {
        cerr << "./mtClient HostName Files/Directories" << endl;
        exit(1);
    }
    
    //making pthread array, 10 diff server
    pthread_t threads[10];
    threadData data[10];
    pthread_attr_t attr; //thread attribute
    pthread_attr_init(&attr); //Initialize and set thread detached attribute
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    void *status;
    int rc;
    
    cout << "Creating threads..." << endl;
    for(int tid = 0; tid < 10; tid++)
    {
		cout << "Making Thread #" << tid << endl;
        data[tid].tid = tid;
        //copy argument list over
        for (int i = 0; i < argc; i++)
        {
			data[tid].argv[i] = argv[i];
		}
			
        rc = pthread_create(&threads[tid], &attr, threadWork, (void*)&data[tid]);
        if (rc) //error
        {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    
    cout << "Joining threads now" << endl;
    pthread_attr_destroy(&attr); //free attr, wait for other threads
    for(int t = 0; t < 10; t++) 
    {
		rc = pthread_join(threads[t], &status);
        if (rc) 
        {
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}	
	}
   
    pthread_exit(NULL);
	return 0;
}
