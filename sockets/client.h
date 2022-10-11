#ifndef CLIENT_H
#define CLIENT_H 


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

int send_position(int player_id, float x, float y, float z, float rot_x, float rot_y, playerpositions *server_players);


#endif /* CLIENT_H */