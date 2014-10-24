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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <pthread.h>
using namespace std;

//global constants
const int BACKLOG = 10;
const int MAXDATASIZE = 300;
//initialize a pthread, to lock later
//pthread_mutex_t action_mutex = PTHREAD_MUTEX_INITIALIZER;

void make_file_list(int new_fd)
{
    char path[MAXDATASIZE];
    if (read(new_fd, path, MAXDATASIZE-1) < 0)
    {
        cerr << "Error reading path from client" << endl;
        exit(1);
    }//reading the path from client
    //find /etc -maxdepth 1 -type f
    char* argv[7];
    argv[0] = "find";
    argv[1] = path;
    argv[2] = "-maxdepth";
    argv[3] = "1";
    argv[4] = "-type";
    argv[5] = "f";
    argv[6] = NULL; //null terminated arg list
    
    int fd = open("Files.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
     
    pid_t pid;
    switch(pid = fork())
    {
        case -1:
            cerr << "Errno: " << errno << endl;
            exit(errno);
            break;
        case 0: //child, using find to generate list of files in path
            if(dup2(fd, 1) == -1)
            {
                cerr << "dup2 failed, errno: "<< errno << endl;
                exit(1);
            }
            execvp(argv[0], argv);
            cerr << "execvp error" << endl;
            break;
        default:
            wait(0);
            break;
    }
   close(fd);
}


//~ struct threadData //holds the info to the threads
//~ {
    //~ int tid;
    //~ int sockfd;
//~ };
//~ 
//~ void *threadWork(void* tmp)
//~ {
    //~ //pthread_mutex_lock(&action_mutex);
    //~ threadData *Data = (threadData*)tmp;
    //~ int new_fd = Data->sockfd;
    //~ int tid = Data->tid;
     //~ 
    //~ cout << "Client #" << tid << endl;
//~ 
    //~ make_file_list(new_fd); //list of files in Files.txt now
    //~ 
	//~ ifstream files;
    //~ files.open("Files.txt");
	//~ string tmp_str;
	//~ while(files >> tmp_str)
    //~ {	
		//~ char buf[BUFSIZ]; 
		//~ strcpy(buf, tmp_str.c_str());
		//~ //cout << "file: " << buf << endl;
		//~ if(write(new_fd, buf, BUFSIZ) < 0)
		//~ {
			//~ cerr << "error in write, errno: " << errno << endl;
			//~ exit(1);
		//~ }
	//~ } 
    //~ close(new_fd); //close connection to client
    //~ //pthread_mutex_unlock(&action_mutex);
    //~ pthread_exit((void*)tmp);
//~ }

void run_Server()
{
	int port = 51214; //port to use
    int new_fd; //fd for client to connect to
    struct sockaddr_in my_addr;
    memset(&my_addr, '\0', sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), 0, 8);
	
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
      cerr << "socket() error in server!" << endl;
      exit(1);
    }
     
    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1)
    {
      cerr << "bind() error in server, errno " << errno << endl;
      exit(errno);
    }
      
    int sin_size = sizeof(struct sockaddr_in);
     	
    if(listen(sockfd, BACKLOG) == -1) //wait for client to connect
    {
         cerr << "listen() error!" << endl;
         exit(1);
    }
    
    cout << "Waiting for client to connect..." << endl;
    int n = 0;
    while(true) //keep acceptings clients
    {
        new_fd = accept(sockfd, (struct sockaddr *)&my_addr, (socklen_t *)&sin_size);
        //accepted the client 
        if(new_fd == -1)
            cerr << "accept() error in server" << endl;
        cout << "Client #" << n << " connected!" << endl;
        make_file_list(new_fd); //list of files in Files.txt now
    
		ifstream files;
		files.open("Files.txt");
		string tmp_str;
		while(files >> tmp_str)
		{	
			char buf[BUFSIZ]; 
			strcpy(buf, tmp_str.c_str());
			//cout << "file: " << buf << endl;
			if(write(new_fd, buf, BUFSIZ) < 0)
			{
				cerr << "error in write, errno: " << errno << endl;
				exit(1);
			}
		}
		n++; 
		close(new_fd); //close connection to client
		if (n == 10)
			break;
   }
   
    //~ pthread_t threads[10]; //can handle up to 10 clients
    //~ threadData data[10];
    //~ pthread_attr_t attr; //thread attribute
    //~ pthread_attr_init(&attr); //Initialize and set thread detached attribute
    //~ pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //~ void *status;
    //~ for(int tid = 0; tid < 10; tid++) //keep acceptings clients
	//~ {
        //~ new_fd = accept(sockfd, (struct sockaddr *)&my_addr, (socklen_t *)&sin_size);
        //~ //accepted the client 
        //~ if(new_fd == -1)
            //~ cerr << "accept() error in server" << endl;
		//~ 
		//~ cout << "Preparing Client #" << tid << endl;
        //~ data[tid].tid = tid;
        //~ data[tid].sockfd = new_fd;
        //~ int rc = pthread_create(&threads[tid], &attr, threadWork, (void*)&data[tid]);
        //~ if (rc)
        //~ {
			//~ printf("ERROR; return code from pthread_create() is %d\n", rc);
            //~ exit(-1);
		//~ }       
    //~ }
    //~ pthread_attr_destroy(&attr); //free attr, wait for other threads
	//~ for(int t = 0; t < 10; t++) 
    //~ {
		//~ int rc = pthread_join(threads[t], &status);
        //~ if (rc) 
        //~ {
			//~ printf("ERROR; return code from pthread_join() is %d\n", rc);
			//~ exit(-1);
		//~ }	
	//~ }
	//~ 
	//~ pthread_exit(NULL);
    //done with requests
    close(sockfd); //close server
    cout << "\nServer exiting..." << endl;
}

int main()
{
	run_Server(); //pass in directory
    return 0;
}
