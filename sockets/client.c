// Client side C/C++ program to demonstrate Socket
// programming
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include "client.h"

#define PORT 3000

int send_position(int player_id, float x, float y, float z, float rot_x, float rot_y, playerpositions *server_players)
{
  playerinfo local_info = {player_id, x, y+4, z, rot_x, rot_y};

	int sock = 0, valread, client_fd;
	struct sockaddr_in serv_addr;
	char buffer[1024] = { 0 };
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		// printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary
	// form


	// if (inet_pton(AF_INET, "159.196.6.181", &serv_addr.sin_addr)
	if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
		<= 0) {
		printf(
			"\nInvalid address/ Address not supported \n");
		return -1;
	}


  if ((client_fd
    = connect(sock, (struct sockaddr*)&serv_addr,
        sizeof(serv_addr)))
    < 0) {
    // printf("\nConnection Failed \n");
    return -1;
  }

  send(sock, &local_info, sizeof(local_info), 0);
  valread = read(sock, server_players, sizeof(playerpositions));

  // for (int i=0; i<10; i++)
  //   printf("%f %f %f\n", server_players->x_positions[i], server_players->y_positions[i], server_players->z_positions[i]);

	// closing the connected socket
	close(client_fd);
	return 0;
}
