#include "Arduino.h"

class Maze
{
  public:
    Maze();
    bool treasure_read(int r, int c);
    void treasure_write(int r, int c, bool x);
    String walls_read(int r, int c);
    String color_read(int r, int c);
	
	/*red = 1, blue = 0*/
	void color_write(int r, int c, bool x );
    
    
    void north_write(int r, int c, bool x);
    void east_write(int r, int c, bool x);
    void south_write(int r, int c, bool x);
    void west_write(int r, int c, bool x);
	
    bool north_read(int r, int c);
    bool east_read(int r, int c);
    bool south_read(int r, int c);
    bool west_read(int r, int c);

    /*square=0, triangle=1, diamond=2   */
    void shape_write(int r, int c, int shape);

    
    String shape_read(int r, int c);

    String disp(int r, int c);
	
	void robot_write(int r, int c, bool x);
	
	void explored(int r, int c, bool x);
	
	bool robot_read(int r, int c);
	
	bool is_explored(int r, int c);
    
    
        
  private:
    byte mat[2][2];
	byte Ex[11];
	byte R[11];
    
};

