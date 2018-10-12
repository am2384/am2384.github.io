#include <Maze.h>


/*Define a Maze object  */
Maze h;
void setup() {
  
  Serial.begin(9600);
}

void loop()
{
  // all write functions have the specification "write(row number, column number, value)"
  /* To write a presence of treaure in square (0,0), call this   */
  h.treasure_write(0,0, 1); // 1 for treasure, 0 for no treasure

  /* To write presence of walls in square (0,0), call these   */
  h.north_write(0,0,1); // 1 for northern wall, 0 for no northern wall
  h.south_write(0,0,0); // 1 for southern wall, 0 for no southern wall
  h.west_write(0,0,0); // 1 for western wall, 0 for no western wall
  h.east_write(0,0,1); // 1 for eastern wall, 0 for no eastern wall

  /*To store the color of treasure, call this    */
  h.color_write(0,0, 1); // 1 for red treasure, 0 for blue treasure

  /* To store the shape of treasure in square (0,0), call this   */ 
  h.shape_write(0,0,1); // 1 for triangle, 0 for square, 2 for diamond.

   /*To indicate that there is a robot in square (0,0), call this  */
  h.robot_write(0,0, 1);

  /*To mark square (0,0) as "explored", call this */
  h.explored(0,0, 1);
   
  /* To display all stored information about an explored square, call this
  this will return "not explored" if square not explored yet*/
  String s = h.disp(0,0);
  Serial.println(s);
  
  

  /*To read particular information such as treasure, color, north wall, etc about a particular cell, 
  call robot_read(r,c), treasure_read(r,c), color_read(r,c), north_read(r,c), is_explored(r,c), etc*/
  delay(10000);

}



