#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <sys/time.h>
int ind = 0,udpflag = 0;
fd_set rset;
int sockset[32] = {0};
int clientcommand[32] = {0};
int client[32]={0};
char name[32][2000]={{}};

void *connection_handler(void *socket_desc){
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[2000];
    int id2 = ind;
      ind+=1;
    //Receive a message from client
    int flag = 0;
    sockset[id2]=sock;
    //tcp
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {//printf("client message%d:%s",id2,client_message);
        //printf("%s,%d,%d\n",name[id2],id2,client[id2]);

       
         //logout:
        if(strncmp("LOGOUT",client_message,6)==0)
        { 
          printf(" Rcvd LOGOUT request\n");
         //printf("logout client name:%s,id:%d,id2:%d\n",name[id2],client[id2],id2);
          write(sock , "OK!\n", 4);
          
          client[id2] = 0; 
          
           bzero(client_message, strlen(client_message)); continue;}
  

        //login
   
         if(strncmp("LOGIN",client_message,5)==0){
               char copy[2000];
               char* cpy = client_message;
               strcpy(copy,cpy+6);
               char *pm = strtok(copy,"\n");
               strcpy(copy,pm);
                printf(" Rcvd LOGIN request for userid %s\n",copy);
               if(strlen(copy)<4 || strlen(copy)>16){
                write(sock , "ERROR Invalid userid\n", 21);
                printf(" Sent ERROR (Invalid userid)\n");
                bzero(client_message, strlen(client_message));
                continue;}

               for(int i=0;i<32;i++){if(strcmp(name[i],copy)==0) 
                {write(sock , "ERROR Already connected\n", 24);
                 printf(" Sent ERROR (Already connected)\n");
                bzero(client_message, strlen(client_message)); flag=1;break;}}
               if(flag==1){flag=0;continue;}
               client[id2]=1;strcpy(name[id2],copy);
               
               write(sock , "OK!\n", 4);
                bzero(client_message, strlen(client_message));
               
                continue;
            }
              //who
             if(strncmp("WHO",client_message,3)==0){
                 printf(" Rcvd WHO request\n");
                 int ii=4;
                 char bew[2000];
                 strcpy(bew,"OK!\n");

                 for(int i=0;i<32;i++){
                     if(client[i]==1){strcat(bew,name[i]);strcat(bew,"\n");}
                 }

                 write(sock , bew,strlen(bew));
                
                 
                 bzero(client_message, strlen(client_message)); continue;
               }
              //send
               if(strncmp("SEND",client_message,4)==0){
                     // printf("sender name:%s\n",name[id2]);
                      
                      char message[2000];
			char mess[3][2000],newmessage[2000],slen[2000];
			char *pp = client_message;
			strcpy(newmessage,pp+5);
			strcpy(message,"FROM ");
			char *pch = strtok(newmessage,"\n");
			int it=0;
					      
			while(pch!=NULL){
			strcpy(mess[it],pch);
			pch = strtok(NULL,"\n");
			it++;
			}
			pch = strtok(mess[0]," ");
			strcpy(mess[0],pch);
                        strcat(message,name[id2]);
                        strcat(message," "); 
                        sprintf(slen,"%ld",strlen(mess[1]));
                        printf(" Rcvd SEND request to userid %s\n",mess[0]);
                        if(strlen(mess[1])<4 || strlen(mess[1])>990){
                           write(sock,"ERROR Invalid msglen\n",21);
                           printf(" Sent ERROR Invalid msglen\n");
                           bzero(client_message, strlen(client_message));  
                           break;
                        }
                        strcat(message,slen);
                        strcat(message," "); 
                        strcat(message,mess[1]); 
                       strcat(message,"\n");
                       //printf("%s\n",message);
                     
                      for(it=0;it<32;it++){
                            if(client[it]==0){continue;} 
                            if( strcmp(name[it],mess[0])==0 ){
                               write(sock , "OK!\n", 4);                             
                                write(sockset[it],message,strlen(message)); 
                               bzero(client_message, strlen(client_message));  
                               break;
                                }
                      }
                      if(it==32){write(sock,"ERROR Unknown userid\n",21); 
                         printf(" Sent ERROR Unknown userid\n");
                         bzero(client_message, strlen(client_message));continue;}
                     
               }
             //broadcast
              if(strncmp("BROADCAST",client_message,9)==0){
                      printf(" Rcvd BROADCAST request\n");
                      char message[2000];
			char mess[3][2000],newmessage[2000],slen[2000];
			char *pp = client_message;
			strcpy(newmessage,pp+10);
			strcpy(message,"FROM ");
			char *pch = strtok(newmessage,"\n");
			int it=0;
					      
			while(pch!=NULL){
			strcpy(mess[it],pch);
			pch = strtok(NULL,"\n");
			it++;
			}
			
                        strcat(message,name[id2]);
                        strcat(message," "); 
                        sprintf(slen,"%ld",strlen(mess[1]));
                        strcat(message,slen);
                        strcat(message," "); 
                        strcat(message,mess[1]); 
                       strcat(message,"\n");
                       //printf("%s\n",message);
                       write(sock , "OK!\n", 4); 
                      for(it=0;it<32;it++){
                            if(client[it]==0){continue;} 
                            write(sockset[it],message,strlen(message)); 
                               bzero(client_message, strlen(client_message));  
                               
                      }
                      
                         bzero(client_message, strlen(client_message));continue;

              }
            //chare

           //printf("TCP client #%d: %s\n",id2,client_message);


        bzero(client_message, strlen(client_message)); 
    }

   
            

    if(read_size == 0)
    {    
        puts(" Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);
    close(sock);
    pthread_exit(NULL); 
}

int main(int argc , char **argv)
{
    unsigned short port = atoi(argv[1]);
    int socket_desc , client_sock , c , *new_sock,udpfd,nready,maxfd;
    char buffer[2000];
    struct sockaddr_in server , client2;
    printf(" Started server\n");
    printf(" Listening for TCP connections on port: %d\n",port);
    printf(" Listening for UDP datagrams on port: %d\n",port);
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
    }
    

    //Prepare the sockaddr_in structure
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( port );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    //puts("bind done");     
    // binding server addr structure to udp sockfd 
    if( bind(udpfd, (struct sockaddr*)&server, sizeof(server))<0){
        perror("udp bind failed");
        return 1;
    } 
    //Listen
    listen(socket_desc , 3);

    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);


    //Accept and incoming connection
    /*puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);*/
    FD_ZERO(&rset);
    if(socket_desc>=udpfd){maxfd = socket_desc+1;}
    else{maxfd=udpfd+1;}
    while( 1 )
    {
        FD_SET(socket_desc,&rset);
        FD_SET(udpfd,&rset);
        udpflag=0;
        //select the ready descrptor
        nready = select(maxfd,&rset,NULL,NULL,NULL);
        //if tcp
        if(FD_ISSET(socket_desc,&rset)){
        client_sock = accept(socket_desc, (struct sockaddr *)&client2, (socklen_t*)&c);
        //puts("Connection accepted");
        printf(" RCVD incoming TCP connection from\n");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        //for(int i=0;i<32;i++){if(sockset[i]==0){sockset[i]=client_sock;}}
        if( pthread_create( &sniffer_thread,NULL,connection_handler,(void*) new_sock) < 0)
        {perror("could not create thread");return 1;}
       
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL); // was commented before
        //puts("Handler assigned");
         
      }
       //if udp
       if(FD_ISSET(udpfd,&rset)){
          udpflag=1;        
          char client_message[2000];
          
          int len = sizeof(client2); 
          recvfrom(udpfd, client_message, 2000, 0,(struct sockaddr*)&client2, &len);
          printf(" Rcvd incoming UDP datagram from\n");
           //printf("clientmessage udp:%s\n",client_message);
          //send
          
          if(strncmp("SEND",client_message,4)==0){
               printf(" Rcvd SEND request\n");
               sendto(udpfd,"SEND not supported over UDP",27,0,
                     (struct sockaddr*)&client2, sizeof(client2));
               bzero(client_message, strlen(client_message));continue;
          } 
          //share
         if(strncmp("SHARE",client_message,5)==0){
              printf(" Rcvd SHARE request\n");
              sendto(udpfd,"SHARE not supported over UDP",28,0,
                    (struct sockaddr*)&client2, sizeof(client2));
              bzero(client_message, strlen(client_message));continue;
         }
         //who
          if(strncmp("WHO",client_message,3)==0){
            printf(" Rcvd WHO request\n");
          char bew[2000];
          strcpy(bew,"OK!\n");
         for(int i=0;i<32;i++){
           if(client[i]!=0){strcat(bew,name[i]);strcat(bew,"\n");}}
          sendto(udpfd,bew,2000,0,(struct sockaddr*)&client2, sizeof(client2));
          }
         //braodcast
        if(strncmp("BROADCAST",client_message,9)==0){
           printf(" Rcvd BROADCAST request\n");
        char message[2000];
	char mess[3][2000],newmessage[2000],slen[2000];
	char *pp = client_message;
	strcpy(newmessage,pp+10);
	strcpy(message,"FROM ");
	char *pch = strtok(newmessage,"\n");
	int it=0;
	while(pch!=NULL){
	strcpy(mess[it],pch);
	pch = strtok(NULL,"\n");
	it++;
	}
	 strcat(message,"UDP-client");
         strcat(message," "); 
         sprintf(slen,"%ld",strlen(mess[1]));
         strcat(message,slen);
         strcat(message," "); 
         strcat(message,mess[1]); 
         strcat(message,"\n");
                       //printf("%s\n",message);
         sendto(udpfd , "OK!\n", 4,0,(struct sockaddr*)&client2, sizeof(client2)); 
         for(it=0;it<32;it++){
            if(client[it]==0){continue;}
          sendto(udpfd , message, 2000,0,(struct sockaddr*)&client2, sizeof(client2));   
            }

              bzero(client_message, strlen(client_message));continue;
         }

       }//end of udp
      //end loop
    }
    //if (client_sock < 0){ perror("accept failed");return 1;}
    return 0;

}

/*
 * This will handle connection for each client
 * */

