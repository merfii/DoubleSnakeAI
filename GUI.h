#ifndef GUI_H_INCLUDED
#define GUI_H_INCLUDED

#define REPAINT_INTERVAL 20     //ms
#define WINDOW_W 1000
#define WINDOW_H 700
#define BLOCK_SIZE 50
#define ABS(a) ((a>0)?(a):(-a))

#include<list>
using namespace  std;

void extern_init();
void runMain();
void keyPressed(int key);
void stopRun();
void DISPLOCK();
void DISPUNLOCK();


//0 вС 1об 2ср 3 ио
static const int dx[4]= {-1,0,1,0};
static const int dy[4]= {0,1,0,-1};

class Point
{
public:
    int x,y;

    Point()
    {
        x=0;y=0;
    }

    Point(const Point &p)
    {
        x=p.x;
        y=p.y;
    }

    Point(int _x,int _y)
    {
        x=_x;
        y=_y;
    }

    bool operator==(Point &a)
    {
        return (a.x==x)&&(a.y==y);
    }
};


class Snake
{
public:

    list<Point> points;

    Snake()
    {
    }

    Snake(const Snake &old)
    {
        points=old.points;
    }

    void addHead(Point head)
    {
        points.push_front(head);
    }

    bool whetherGrow(int steps)  //本回合是否生长
    {
        if (steps<=9) return true;
        if ((steps-9)%3==0) return true;
        return false;
    }

    void move(int dire)
    {
        Point head=points.front();
        points.push_front(Point(head.x+dx[dire],head.y+dy[dire]));
    }

    void deleteTail(int steps)
    {
        if(!whetherGrow(steps))
        {
            points.pop_back();
        }
    }

    Point getNext(int dire)
    {
        return Point(
            points.front().x+dx[dire] ,
            points.front().y+dy[dire]
                     );
    }


    Point getNextN(int dire,int n)
    {
        return Point(
            points.front().x+n*dx[dire],
            points.front().y+n*dy[dire]
                     );
    }
};


class MapBasic
{
public:
    int w,h;
    bool obst[25][25];
    int obst_count;
    MapBasic()
    {
        for(int i=0; i<25; i++)
            for(int j=0; j<25; j++)
                obst[i][j]=false;
    }
    int Xarray[100],Yarray[100];
};

void updateDisp(MapBasic &mb,Snake &snk0,Snake &snk1);
#endif // GUI_H_INCLUDED
