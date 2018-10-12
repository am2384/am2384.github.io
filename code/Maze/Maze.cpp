#include "Maze.h"

Maze::Maze()
{
	for(int i = 0;i < 11;i++)
	{
		Ex[i] = 0b0;
		R[i] = 0b0;
	}
	
  
}
  bool Maze::treasure_read(int r, int c)
  {
    return (bitRead(mat[r][c], 4));
  }

  void Maze::treasure_write(int r, int c, bool x)
  {
    bitWrite(mat[r][c], 4, x);
  }
  
  String Maze::walls_read(int r, int c)
  {
    return "North="+String(north_read(r,c))\
    +",East=" + String(east_read(r,c))\
    +",West=" + String(west_read(r,c))\
    +",South=" + String(south_read(r,c));
  }
  String Maze::color_read(int r, int c)
  {
     return (bitRead(mat[r][c], 5) == 1) ? "red":"blue";
  }
  
  void Maze::color_write(int r, int c, bool x )
  {
	  bitWrite(mat[r][c], 5, x);
  }
   

    void Maze::north_write(int r, int c, bool x)
    {
      bitWrite(mat[r][c], 3, x);
    }
    void Maze::east_write(int r, int c, bool x)
    { 
      bitWrite(mat[r][c], 2, x);
    }
    void Maze::south_write(int r, int c, bool x)
    {
      bitWrite(mat[r][c], 1, x);
    }
    void Maze::west_write(int r, int c, bool x)
    {
      bitWrite(mat[r][c], 0, x);
    }
    

    bool Maze::north_read(int r, int c)
    {
      return bitRead(mat[r][c], 3);
    }
    bool Maze::east_read(int r, int c)
    {
      return bitRead(mat[r][c], 2);
    }
    bool Maze::west_read(int r, int c)
    {
      return bitRead(mat[r][c], 1);
    }
    bool Maze::south_read(int r, int c)
    {
      return bitRead(mat[r][c], 0);
    }

  

    /*square=0, triangle=1, diamond=2   */
    void Maze::shape_write(int r, int c, int shape)
    {
      if(shape==0)
      {
        bitWrite(mat[r][c], 7, 0);
        bitWrite(mat[r][c], 6, 0);
      } else if(shape==1)
      {
        bitWrite(mat[r][c], 7, 0);
        bitWrite(mat[r][c], 6, 1);
      }else if(shape==2)
      {
        bitWrite(mat[r][c], 7, 1);
        bitWrite(mat[r][c], 6, 0);
      }
    }

    String Maze::shape_read(int r, int c)
    {
      if(bitRead(mat[r][c], 7)==1) return "diamond";
      else if(bitRead(mat[r][c], 6)==1) return "triangle";
      else return "square";
    }
    String Maze::disp(int r, int c)
    {
		if(is_explored(r,c)==1)
		{
			if(treasure_read(r,c)==1)
			{
				if(robot_read(r,c))
				{
					return String(r)+","+String(c)+","+walls_read(r,c)+",tcolor="+\
					color_read(r,c)+",tshape="+shape_read(r,c) + "," + "Robot!";
				} else
				{
					return String(r)+","+String(c)+","+walls_read(r,c)+",tcolor="+\
					color_read(r,c)+",tshape="+shape_read(r,c) + "," + "No Robot!";
				}
				
			} else
			{
				if(robot_read(r,c))
				{
					return String(r)+","+String(c)+","+walls_read(r,c)+",No Treasure," + "Robot!";
				}
				else
				{
					return String(r)+","+String(c)+","+walls_read(r,c)+",No Treasure,No Robot";
				}
			}
			
		} else 
		{
			return "Not Explored";
		}
    }
	
	void Maze::robot_write(int r, int c, bool x)
	{
		//byte R[11];
		int num = r*9 + c;
		int ind = num/8;
		bitWrite(R[int(num/8)],num - 8*int(ind), x );
	}
	
	void Maze::explored(int r, int c, bool x)
	{
		int num = r*9 + c;
		int ind = num/8;
		bitWrite(Ex[int(num/8)],num - 8*int(ind), x );
	}
	
	bool Maze::robot_read(int r, int c)
	{
		int num = r*9 + c;
		int ind = num/8;
		return bitRead(R[int(num/8)],num - 8*int(ind));
	}
	
	bool Maze::is_explored(int r, int c)
	{
		int num = r*9 + c;
		int ind = num/8;
		return bitRead(Ex[int(num/8)],num - 8*int(ind));
	}