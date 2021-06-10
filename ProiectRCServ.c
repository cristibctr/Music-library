#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

#define PORT 2024

extern int errno;

static int login(void *retMsg, int count, char **data, char **columns){
   int i;
   int *flag = (int*)retMsg;
   for(i = 0; i<count; i++){
      if(data[i] != NULL) *flag = 0;
   }
   
   return 0;
}
static int adminDB(void *retMsg, int count, char **data, char **columns){
   int *flag = (int*)retMsg;
   if(data[0][0] - '0' == 1) *flag = 1;
   
   return 0;
}
void breakMsg(char *firstString, char *secondString, char *msg)
{
			strncpy(firstString, msg, strchr(msg, '|')-msg);
			strcpy(secondString, strchr(msg, '|')+1);
			//printf ("[server]User:%s\n", firstString);
			//printf ("[server]Parola:%s\n", secondString); 
}

int main ()
{
  struct sockaddr_in server;
  struct sockaddr_in from;	
  char msg[300];
  char msgrasp[100]=" ";
  int sd;
  int clientNr = 1;
  char clientNrCh[100];
  int myNumber;
  char myNumberCh[10000];
  char dbRetMsg[100][300];
  pid_t pid;
	
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }

  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
  
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  if (listen (sd, 5) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

  while (1)
    {
      int client;
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      client = accept (sd, (struct sockaddr *) &from, &length);

      if (client < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}

      bzero (msg, 300);
      printf ("[server]Asteptam mesajul...\n");
      fflush (stdout);
	pid = fork();
      if(pid == 0)
	{
		close(sd);
		char username[100],parolaAndAdmin[100], parola[100], sql[200], admin[100];
		int rc;
		sqlite3 *db;
		rc = sqlite3_open("proiectRC.db", &db);
		if( rc ) {
	      		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	      		return(0);
	   	} else {
	      		fprintf(stderr, "Opened database successfully\n");
	   	}
		char *zErrMsg = 0;
		if (read (client, msg, 100) <= 0)
			{
			  perror ("[server]Eroare la read() de la client.\n");
			  close (client);
			  exit(0);
			}
		if(strcmp(msg,"register") == 0)
		{
			if (read (client, msg, 100) <= 0)
			{
			  perror ("[server]Eroare la read() de la client.\n");
			  close (client);
			  exit(0);
			}
			printf ("[server]Mesajul a fost receptionat...%s\n", msg);
			breakMsg(username, parolaAndAdmin, msg);
			breakMsg(parola, admin, parolaAndAdmin);
			if(strstr(admin, "Yes") || strstr(admin, "yes"))
				strcpy(admin, "1");
			else 
				strcpy(admin, "0");
			strcpy(sql, 	"INSERT INTO users (username, passw, admin, voteallw)" \
					"VALUES ('");
			strcat(sql, username);
			strcat(sql, "', '");
			strcat(sql, parola);
			strcat(sql, "', ");
			strcat(sql, admin);
			strcat(sql, ", 1);");
			rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
			if( rc != SQLITE_OK ) {
			   fprintf(stderr, "SQL error: %s\n", zErrMsg);
			   sqlite3_free(zErrMsg);
			} else {
			   fprintf(stdout, "Operation done successfully\n");
			}
		}
		int notConn = 1;
		
		while(notConn)
		{
		      if (read (client, msg, 100) <= 0)
			{
			  perror ("[server]Eroare la read() de la client.\n");
			  close (client);
			  exit(0);
			}
			
			printf ("[server]Mesajul a fost receptionat...%s\n", msg);
			breakMsg(username, parola, msg);
			strcpy(sql, "SELECT username FROM users WHERE username='");
			strcat(sql, username);
			strcat(sql,"' AND passw='");
			strcat(sql, parola);
			strcat(sql,"';");
			rc = sqlite3_exec(db, sql, login, &notConn, &zErrMsg);
			if( rc != SQLITE_OK ) {
			   fprintf(stderr, "SQL error: %s\n", zErrMsg);
			   sqlite3_free(zErrMsg);
			} else {
			   fprintf(stdout, "Operation done successfully\n");
			}
		      
			if(notConn)
				strcpy(msgrasp, "fail");
			else
				strcpy(msgrasp, "success");

		      if (write (client, msgrasp, 100) <= 0)
			{
			  perror ("[server]Eroare la write() catre client.\n");
			  exit(0);
			}
		      else
			printf ("[server]Mesajul a fost trasmis cu succes.\n");
		}
		printf("[server] Username si parola corecte\n");
		strcpy(sql, "SELECT admin FROM users WHERE username='");
		strcat(sql, username);
		strcat(sql,"' AND passw='");
		strcat(sql, parola);
		strcat(sql,"';");
		int isAdmin = 0;
		rc = sqlite3_exec(db, sql, adminDB, &isAdmin, &zErrMsg);
		if( rc != SQLITE_OK ) {
		   fprintf(stderr, "SQL error: %s\n", zErrMsg);
		   sqlite3_free(zErrMsg);
		} else {
		   fprintf(stdout, "Operation done successfully\n");
		}
		//printf("\nIs admin? %d\n",isAdmin);
		if(isAdmin)
			write(client, "yesadmin", 100);
		else
			write(client, "notadmin", 100);
		int isConnected = 1;
		sqlite3_stmt *stmt;
		rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);
		while(isConnected)
		{
			bzero (msg, 300);
			if (read (client, msg, 300) <= 0)
			{
			  perror ("[server]Eroare la read() de la client.\n");
			  close (client);
			  exit(0);
			}
			printf("Nr de la client: %s\n", msg);
			switch(msg[0]-'0')
			{
				case 0:
					isConnected = 0;
					break;
				case 1:
					if (read (client, msg, 300) <= 0)
					{
					  perror ("[server]Eroare la read() de la client.\n");
					  close (client);
					  exit(0);
					}
					printf("1. Comanda de la client: %s\n", msg);
					rc = sqlite3_exec(db, msg, NULL, NULL, NULL);
					break;
				case 2:
					strcpy(sql, "SELECT voteallw FROM users WHERE username = '");
					strcat(sql, username);
					strcat(sql, "';");
					if (read (client, msg, 300) <= 0)
					{
					  perror ("[server]Eroare la read() de la client.\n");
					  close (client);
					  exit(0);
					}
					printf("2. Comanda de la client: %s\n", msg);
					rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
					if(rc != SQLITE_OK) {
						//printf("no workey\n");
						close(client);
						exit(0);
					}
					sqlite3_step(stmt);
					int voteallw = sqlite3_column_int(stmt, 0);
					
					if(voteallw)
					{
						rc = sqlite3_exec(db, msg, NULL, NULL, NULL);
						write(client, "allowed", 10);
					}
					else
						write(client, "denied", 10);
					break;
				case 3:
					printf("topSong()\n");
					char topSongMsg[1000];
					topSongMsg[0] = '\0';
					rc = sqlite3_prepare_v2(db, "SELECT name, votes, link, descr FROM music ORDER BY votes DESC LIMIT 5;", -1, &stmt, NULL);
					if(rc != SQLITE_OK) {
						//printf("no workey\n");
						close(client);
						exit(0);
					}
					while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
					{
						//printf("%s:%d\n", sqlite3_column_text(stmt, 0), sqlite3_column_int(stmt, 1));
						strcat(topSongMsg, "Name: ");
						strcat(topSongMsg, sqlite3_column_text(stmt, 0));
						strcat(topSongMsg, " | Votes: ");
						strcat(topSongMsg, sqlite3_column_text(stmt, 1));
						strcat(topSongMsg, " | Link: ");
						strcat(topSongMsg, sqlite3_column_text(stmt, 2));
						strcat(topSongMsg, " | Description: ");
						strcat(topSongMsg, sqlite3_column_text(stmt, 3));
						strcat(topSongMsg, "\n");
					}
					sqlite3_finalize(stmt);
					write(client, topSongMsg, 1000);
					break;
				case 4:
					printf("topSongGenre()\n");
					if (read (client, msg, 300) <= 0)
					{
					  perror ("[server]Eroare la read() de la client.\n");
					  close (client);
					  exit(0);
					}
					char topSongMsgGen[1000];
					topSongMsgGen[0] = '\0';
					strcpy(sql, "SELECT name, votes, link, descr, genre FROM music JOIN songgenre ON music.name = songgenre.songname WHERE genre = '");
					strcat(sql, msg);
					strcat(sql, "' ORDER BY votes DESC LIMIT 5;");
					rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
					if(rc != SQLITE_OK) {
						//printf("no workey\n");
						close(client);
						exit(0);
					}
					while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
					{
						//printf("%s:%d\n", sqlite3_column_text(stmt, 0), sqlite3_column_int(stmt, 1));
						strcat(topSongMsgGen, "Name: ");
						strcat(topSongMsgGen, sqlite3_column_text(stmt, 0));
						strcat(topSongMsgGen, " | Votes: ");
						strcat(topSongMsgGen, sqlite3_column_text(stmt, 1));
						strcat(topSongMsgGen, " | Link: ");
						strcat(topSongMsgGen, sqlite3_column_text(stmt, 2));
						strcat(topSongMsgGen, " | Description: ");
						strcat(topSongMsgGen, sqlite3_column_text(stmt, 3));
						strcat(topSongMsgGen, " | Genre: ");
						strcat(topSongMsgGen, sqlite3_column_text(stmt, 4));
						strcat(topSongMsgGen, "\n");
					}
					sqlite3_finalize(stmt);
					write(client, topSongMsgGen, 1000);
					break;
				case 5:
					printf("commOnSong()\n");
					if (read (client, msg, 100) <= 0)
					{
					  perror ("[server]Eroare la read() de la client.\n");
					  close (client);
					  exit(0);
					}
					char songName[100], comm[1000];
					strcpy(songName, msg);
					if (read (client, msg, 1000) <= 0)
					{
					  perror ("[server]Eroare la read() de la client.\n");
					  close (client);
					  exit(0);
					}
					strcpy(comm, msg);
					strcpy(sql, "INSERT INTO comments (username, music_name, comm) VALUES('");
					strcat(sql, username);
					strcat(sql, "','");
					strcat(sql, songName);
					strcat(sql, "','");
					strcat(sql, comm);
					strcat(sql, "');");
					printf("%s\n",sql);
					rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
					break;
				case 6:
					if(isAdmin)
					{
						printf("delSong()\n");
						if (read (client, msg, 100) <= 0)
						{
						  perror ("[server]Eroare la read() de la client.\n");
						  close (client);
						  exit(0);
						}
						strcpy(sql, "DELETE FROM music WHERE name='");
						strcat(sql, msg);
						strcat(sql, "';");
						rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
						break;
					}
				case 7:
					if(isAdmin)
					{
						printf("removeVote()\n");
						if (read (client, msg, 100) <= 0)
						{
						  perror ("[server]Eroare la read() de la client.\n");
						  close (client);
						  exit(0);
						}
						strcpy(sql, "UPDATE users SET voteallw=0 WHERE username='");
						strcat(sql, msg);
						strcat(sql, "';");
						rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
						break;
					}
				default:
					printf("Recived incorrect number.\n");
			}
		}
		sqlite3_close(db);
		exit(0);
	}
	else 
	{
		close(client);
	}
      
    }				
}				
