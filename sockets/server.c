// Server side C/C++ program to demonstrate Socket
// programming
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 3000

typedef struct {
  int id;
  float x, y, z;
  float rot_x, rot_y;
} playerinfo;

typedef struct {
  int ids[10];
  float x_positions[10];
  float y_positions[10];
  float z_positions[10];

  float x_rotations[10];
  float y_rotations[10];
} playerpositions;

playerpositions pinfo;

int main(int argc, char const* argv[])
{
  playerinfo current;
  int player_count = 0;

  for (int i=0; i<10; i++)
  {
    pinfo.ids[i] = 0;
    pinfo.x_positions[i] = 0;
    pinfo.y_positions[i] = 0;
    pinfo.z_positions[i] = 0;
    pinfo.x_rotations[i] = 0;
    pinfo.y_rotations[i] = 0;
  }


	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = { 0 };
	char* hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr*)&address,
			sizeof(address))
		< 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}


  while (1)
  {
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
          (socklen_t*)&addrlen))
      < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    valread = read(new_socket, &current, sizeof(current));
    printf("[id, x, y, z, rx, ry]: %d %f %f %f %f %f\n", current.id, current.x, current.y, current.z, current.rot_x, current.rot_y);
    int player_is_joined = 0;

    for (int i=0; i<10; i++)
    {
      if (pinfo.ids[i] == current.id)
      {
        player_is_joined = 1;
        pinfo.x_positions[i] = current.x;
        pinfo.y_positions[i] = current.y;
        pinfo.z_positions[i] = current.z;
        pinfo.x_rotations[i] = current.rot_x;
        pinfo.y_rotations[i] = current.rot_y;
        break;
      }
    }
    if (player_is_joined == 0 && player_count < 10)
    {
      pinfo.ids[player_count] = current.id;
      pinfo.x_positions[player_count] = current.x;
      pinfo.y_positions[player_count] = current.y;
      pinfo.z_positions[player_count] = current.z;
      pinfo.x_rotations[player_count] = current.rot_x;
      pinfo.y_rotations[player_count] = current.rot_y;
      player_count += 1;
    }

    send(new_socket, &pinfo, sizeof(playerpositions), 0);
    // printf("Emitting player positions to client\n");

  }

	// closing the connected socket
	close(new_socket);
	// closing the listening socket
	shutdown(server_fd, SHUT_RDWR);
	return 0;
}

