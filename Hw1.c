/*******************************************************************
*File: Hw1.c
*Author: Kyle Bolin
*Procedures:
  main - sets up 2 mesage queues, then forks with the parents acting as a server while the child acts as a client 

********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <mqueue.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*****************
*int main()
* Author: Kyle Bolin
* Date: 2/1/22
* Description: sets up 2 message queues, then forks with the parent acting as a server and the child acting as a client
  -Parent: waits for a message from a client in mqd from the client (1,2,3,4) then performs the function assicioated with that number [1:getdomainname(),2:gethostname(),3:Uname(),4:return()], then returns the result through mqd2 to the client.
  -Child: written as a mainly as a test for the parent process, sends numbers to the parent process, then waits for the parent responce, then prints it off.
*Parameters: m
  -main O/P INT Status Code (Not Used)
*****************/

int main()
{
//message queues used for the processes 
mqd_t mqd = mq_open("/MQ1", O_CREAT | O_RDWR,  0600, NULL);
mqd_t mqd2 = mq_open("/MQ2", O_CREAT | O_RDWR,  0600, NULL);


int pid = 0;
pid = fork();
if (pid == 0)// child
{
  struct mq_attr attr2;
  char *buffer2;
  int priority2 = 0;

// askes the parent for the domain name if availible, then prints what is returned
  mq_send (mqd, "1", 1, 10);
  assert(mq_getattr (mqd, &attr2) != -1);
  buffer2 = calloc (attr2.mq_msgsize, 1);
  assert(buffer2 != NULL);
  mq_receive (mqd2, buffer2, attr2.mq_msgsize, &priority2);
  printf("getdomainname: %s \n", buffer2);

// askes the parent for the host name if availible, then prints what is returned
  mq_send (mqd, "2", 1, 10);
  assert(mq_getattr (mqd, &attr2) != -1);
  buffer2 = calloc (attr2.mq_msgsize, 1);
  assert(buffer2 != NULL);
  mq_receive (mqd2, buffer2, attr2.mq_msgsize, &priority2);
  printf("gethostname: %s \n", buffer2);

// askes the parent for all the parameters from the uname struct, then prints what is returned
  mq_send (mqd, "3", 1, 10);
  assert(mq_getattr (mqd, &attr2) != -1);
  buffer2 = calloc (attr2.mq_msgsize, 1);
  assert(buffer2 != NULL);
  mq_receive (mqd2, buffer2, attr2.mq_msgsize, &priority2);
  printf("Uname: %s \n", buffer2);

// sends the command for the parent process to end
  mq_send (mqd, "4", 1, 10);

// ends the child process
  return(0);
}



else // parent
{
    struct utsname myuname;
    char hostname[1024];
    unsigned int priority = 0;
    struct mq_attr attr;
    char *buffer;
    
    //this loop waits for a messagem then once it gets one executes one of 4 intructions and sends back the result, or if the message was bad, waits for another message.
    while(1)
  {
      assert(mq_getattr (mqd, &attr) != -1);
      buffer = calloc (attr.mq_msgsize, 1);
      assert(buffer != NULL);

      if ((mq_receive (mqd, buffer, attr.mq_msgsize, &priority)) == -1)
  printf ("Failed to receive message\n");

    //since switch staments in C cant use strings for cases I opted for chaining if statements using strcmp()    


    //getdomainname if 1 is recived
    if(strcmp(buffer,"1")==0)
    {
        getdomainname(hostname,1023);
        mq_send (mqd2, hostname, strlen(hostname),10);
    }
    //gethostname if 2 is recived
    else if(strcmp(buffer,"2")==0)
    {
        gethostname(hostname,1023);
        mq_send (mqd2, hostname, strlen(hostname), 10);
    }
    //Uname if 3 is recieved 
    else if(strcmp(buffer,"3")==0)
    {
        uname(&myuname);
        strcat(myuname.sysname," "); 
        strcat(myuname.sysname, myuname.nodename);
        strcat(myuname.sysname," ");
        strcat(myuname.sysname,myuname.release);
        strcat(myuname.sysname," ");
        strcat(myuname.sysname,myuname.version);
        strcat(myuname.sysname," ");
        strcat(myuname.sysname,myuname.machine); //theres probably a better way to manipulate strings in C but this works
        mq_send (mqd2, myuname.sysname, strlen(myuname.sysname), 10);
    }
    // ends the parent process if 4 was reciveed 
    else if(strcmp(buffer,"4")==0)
    {
          mq_close(mqd);
          mq_close(mqd2);
          mq_unlink("/MQ1");
          mq_unlink("/MQ2");
          free (buffer);
          buffer = NULL;
          return(0);
    }
      
  }


}// end of parent
}
