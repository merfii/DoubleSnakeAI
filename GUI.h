#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#define REPAINT_INTERVAL 40     //ms
#define SCREEN_W 1000
#define SCREEN_H 700
#define BLOCK_SIZE 50
#define ABS(a) ((a>0)?(a):(-a))

#include<list>
using namespace  std;

extern bool running;
extern int keypress;
void extern_init();
void runMain();

//0 左 1下 2右 3 上
static const int dx[4]= {-1,0,1,0};
static const int dy[4]= {0,1,0,-1};

class MapBasic
{
public:
    int m,n;
    bool obst[25][25];
    int Xarray[100];
    int Yarray[100];
    int obst_count;

    MapBasic()
    {
        m=1;n=1;
        obst_count=0;
        for(int i=0; i<25; i++)
            for(int j=0; j<25; j++)
                obst[i][j]=false;
    }
};


class Snake
{
public:
    list<int> x;
    list<int> y;
    int length;


    Snake()
    {
        length=0;
    }

    Snake(Snake &old)
    {
        length=old.length;
        x=old.x;
        y=old.y;

    }

    void push(int _x,int _y)
    {
        x.push_front(_x);
        y.push_front(_y);
        length++;
    }


    bool whetherGrow(int steps)  //本回合是否生长
    {
        if (steps<=9) return true;
        if ((steps-9)%3==0) return true;
        return false;
    }


    void move(int dire,int steps)
    {
        x.push_front(x.front()+dx[dire]);
        y.push_front(y.front()+dy[dire]);
        if(whetherGrow(steps))
        {
            length++;
        }
        else
        {
            x.pop_back();
            y.pop_back();
        }
    }

    int getNextX(int dir)
    {
        return x.front()+dx[dir];
    }

    int getNextY(int dir)
    {
        return y.front()+dy[dir];
    }


    int getNextX_n(int dir,int n)
    {
        return x.front()+n*dx[dir];
    }

    int getNextY_n(int dir,int n)
    {
        return y.front()+n*dy[dir];
    }
};

void disp(MapBasic &mb,Snake &snk0,Snake &snk1);

#endif // GUI_H_INCLUDED
