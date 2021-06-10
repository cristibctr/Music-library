#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

extern int errno;

int port;

void removeNewline(char *myString)
{
	myString[strlen(myString)-1] = '\0';
}

void hashString(char *stringIn, char *stringOut)
{
	char buff = 0, comm[100] = "echo -n ";
	FILE *fpipe;
	int i;
	strcat(comm, stringIn);
	strcat(comm, " | sha256sum");
	fpipe = (FILE*)popen(comm,"r");
	for(i = 0; fread(&buff, sizeof(buff), 1, fpipe); i++)
	{
		stringOut[i] = buff;
	}
	stringOut[i+1] = '\0';
	pclose(fpipe);
}

int sd;
void addSong();
void voteSong();
void topSong();
void topSongGenre();
void commOnSong();
void delSong();
void removeVote();

int main (int argc, char *argv[])
{
  struct sockaddr_in server;
  char username[100],parola[100],msg[100], admin[100], hashPass[300];

  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  port = atoi (argv[2]);

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons (port);
  
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
	char logReg[10];
	bzero (logReg, 100);
	printf ("Register?(Yes/No)\n");
	fflush(stdout);
	read(0, logReg, 100);
	if(strstr(logReg, "Yes") || strstr(logReg, "yes"))
	{
		if (write (sd, "register", 100) <= 0)
		{
			perror ("[client]Eroare la write() spre server.\n");
			return errno;
		}
		bzero (username, 100);
		printf ("Username: ");
		fflush (stdout);
		read (0, username, 100);
		bzero (parola, 100);
		printf ("Password: ");
		fflush (stdout);
		read (0, parola, 100);
		bzero (admin, 100);
		printf ("Do you want the account to be admin? (Yes/No)\n");
		fflush (stdout);
		read (0, admin, 100);
		removeNewline(parola);
		removeNewline(username);
		removeNewline(admin);
		hashString(parola, hashPass);
		strcat(username, "|");
		strcat(username, hashPass);
		strcat(username, "|");
		strcat(username, admin);
		  
		if (write (sd, username, 100) <= 0)
		{
			perror ("[client]Eroare la write() spre server.\n");
			return errno;
		}
	}
	else {
		if (write (sd, "login", 100) <= 0)
		{
			perror ("[client]Eroare la write() spre server.\n");
			return errno;
		}
	}
	printf("\nLOGIN\n");
	printf("----------\n");
	fflush(stdout);
	int notConn = 1;
	while(notConn)
		{
		  	bzero (username, 100);
		  	printf ("Username:");
		  	fflush (stdout);
		  	read (0, username, 100);
			bzero (parola, 100);
			printf ("Password:");
			fflush (stdout);
			read (0, parola, 100);
			removeNewline(username);
			removeNewline(parola);
			hashString(parola, hashPass);
			strcat(username, "|");
			strcat(username, hashPass);
		  
		  if (write (sd, username, 100) <= 0)
		    {
		      perror ("[client]Eroare la write() spre server.\n");
		      return errno;
		    }

		  if (read (sd, msg, 100) < 0)
		    {
		      perror ("[client]Eroare la read() de la server.\n");
		      return errno;
		    }
			if(strcmp(msg,"success") == 0)
			{
				notConn = 0;
				printf("Login successful!\n");
			}
			else 
			{
				printf("Username or password incorrect.\n");
			}
		}
	bzero (msg, 100);
	read(sd, msg, 100); // verify account status
	printf("You now have access to the following commands:\n");
	printf("0. Quit\n");
	printf("1. Add song\n");
	printf("2. Vote song\n");
	printf("3. Show top songs\n");
	printf("4. Show top songs by genre\n");
	printf("5. Comment on song\n");
	int isAdmin = 0;
	if(strstr(msg, "yesadmin"))
	{
		isAdmin = 1;
		printf("6. Delete song\n");
		printf("7. Remove vote from user\n");
	}
	int quit = 1;
	while(quit)
	{
		bzero (msg, 100);
		read(0, msg, 100);
		removeNewline(msg);
		switch(msg[0]-'0')
		{
			case 0:
				quit = 0;
				write(sd,"0", 100);
				break;
			case 1:
				addSong();
				break;
			case 2:
				voteSong();
				break;
			case 3:
				topSong();
				break;
			case 4:
				topSongGenre();
				break;
			case 5:
				commOnSong();
				break;
			case 6:
				if(isAdmin)
				{
					delSong();
					break;
				}
			case 7:
				if(isAdmin)
				{
					removeVote();
					break;
				}
			default:
				printf("Incorrect number. Please try again.\n");
		}
	}
  //printf ("[client]Mesajul primit este: %s\n", msg);
  close (sd);
}

void addSong()
{
	char descr[300], name[100],genre[100], link[100], msg[300];
	write(sd, "1", 100);
	printf("Song name:\n");
	bzero (name, 100);
	read(0, name, 100);
	removeNewline(name);
	printf("Description:\n");
	bzero (descr, 100);
	read(0, descr, 100);
	removeNewline(descr);
	printf("Genre:\n");
	bzero (genre, 100);
	read(0, genre, 100);
	removeNewline(genre);
	printf("Link:\n");
	bzero (link, 100);
	read(0, link, 100);
	removeNewline(link);
	strcpy(msg,"INSERT INTO music (name, votes, link, descr) VALUES('");
	strcat(msg, name);
	strcat(msg, "', 0, '");
	strcat(msg, link);
	strcat(msg, "', '");
	strcat(msg, descr);
	strcat(msg, "');");
	strcat(msg, "INSERT INTO songgenre VALUES('");
	strcat(msg, name);
	strcat(msg, "', '");
	strcat(msg, genre);
	strcat(msg, "');");
	write(sd, msg, 300);
}

void voteSong()
{
	char name[100], msg[300], msgRet[10];
	write(sd, "2", 100);
	printf("Song name:\n");
	bzero (name, 100);
	read(0, name, 100);
	removeNewline(name);
	strcpy(msg, "UPDATE music SET votes = votes + 1 WHERE name = '");
	strcat(msg, name);
	strcat(msg, "';");
	write(sd, msg, 300);
	read(sd, msgRet, 10);
	if(strstr(msgRet, "denied"))
		printf("Not allowed to vote\n");
	else
		printf("Vote successful\n");
}

void topSong()
{
	char topSongs[1000];
	printf("Top 5 songs: \n");
	write(sd, "3", 100);
	bzero(topSongs, 1000);
	read(sd, topSongs, 1000);
	printf("%s\n", topSongs);
}

void topSongGenre()
{
	char topSongs[1000], genre[100];
	write(sd, "4", 100);
	printf("Genre:\n");
	bzero (genre, 100);
	read(0, genre, 100);
	removeNewline(genre);
	printf("Top 5 songs by genre: \n");
	write(sd, genre, 100);
	bzero(topSongs, 1000);
	read(sd, topSongs, 1000);
	printf("%s\n", topSongs);
}
void commOnSong()
{
	write(sd, "5", 100);
	char song[100], comm[1000];
	printf("Which song do you want to comment on?\n");
	bzero (song, 100);
	read(0, song, 100);
	removeNewline(song);
	write(sd, song, 100);
	printf("Type your comment: \n");
	bzero (comm, 1000);
	read(0, comm, 1000);
	removeNewline(comm);
	write(sd, comm, 1000);
}
void delSong()
{
	write(sd, "6", 100);
	char song[100];
	printf("Song you want to delete:\n");
	bzero (song, 100);
	read(0, song, 100);
	removeNewline(song);
	write(sd, song, 100);	
}

void removeVote()
{
	write(sd, "7", 100);
	char user[100];
	printf("Username of user you want to disable voting on:\n");
	bzero (user, 100);
	read(0, user, 100);
	removeNewline(user);
	write(sd, user, 100);
}
