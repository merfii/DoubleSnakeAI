#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include "jsoncpp/json.h"
//#include "GUI.h"

#define MAX_MAP 25
#define MAX_DEPS 8
#define ABS(a) ((a>0)?(a):(-a))


using namespace std;

//0 左 1下 2右 3 上
const int dx[4]= {-1,0,1,0};
const int dy[4]= {0,1,0,-1};

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


class MapBasic
{
public:
    int m,n;
    bool obst[MAX_MAP][MAX_MAP];
    MapBasic()
    {
        for(int i=0; i<MAX_MAP; i++)
            for(int j=0; j<MAX_MAP; j++)
                obst[i][j]=false;
    }
};

class Map
{
public:

    MapBasic *pMapBasic;
    bool obstAndSnake[MAX_MAP][MAX_MAP];
    int m,n;

    Map(MapBasic *mapBasic,Snake &snake0,Snake &snake1)
    {
        pMapBasic=mapBasic;
        m=pMapBasic->m;
        n=pMapBasic->n;
        for(int i=0; i<=m; i++)
            for(int j=0; j<=n; j++)
                obstAndSnake[i][j]=pMapBasic->obst[i][j];

        list<int>::iterator itx,ity;
        for(itx=snake0.x.begin(),ity=snake0.y.begin();
                itx!=snake0.x.end(); itx++,ity++)
            obstAndSnake[*itx][*ity]=true;

        for(itx=snake1.x.begin(),ity=snake1.y.begin();
                itx!=snake1.x.end(); itx++,ity++)
            obstAndSnake[*itx][*ity]=true;

    }

    Map(Map &preMap,Snake &snake0,Snake &snake1)
    {
        pMapBasic=preMap.pMapBasic;
        m=pMapBasic->m;
        n=pMapBasic->n;

        for(int i=0; i<=m; i++)
            for(int j=0; j<=n; j++)
                obstAndSnake[i][j]=pMapBasic->obst[i][j];

        list<int>::iterator itx,ity;
        for(itx=snake0.x.begin(),ity=snake0.y.begin();
                itx!=snake0.x.end(); itx++,ity++)
            obstAndSnake[*itx][*ity]=true;

        for(itx=snake1.x.begin(),ity=snake1.y.begin();
                itx!=snake1.x.end(); itx++,ity++)
            obstAndSnake[*itx][*ity]=true;
    }
    bool isValid(int x,int y)
    {
        if(x>n || y>m || x<1 || y<1)
            return false;
        return !obstAndSnake[x][y];
    }
};

class Rank
{
public:
    float freedom;
    int layer;
    Rank()
    {
        freedom=0;
        layer=0;
    }

    Rank(float fr,int lay)
    {
        freedom=fr;
        layer=lay;
    }

    Rank& operator+=(const Rank &a)
    {
        freedom+=a.freedom;
        layer+=a.layer;
        return *this;
    }
};

Rank predict(Map &map,Snake &snake0,Snake &snake1,
             int dire,int depth,int steps)
{
    if(!map.isValid(snake0.getNextX(dire),snake0.getNextY(dire)))
    {
        return Rank(0,0);
    }
    if(steps>=MAX_DEPS)
    {
        int n=1;
        while(map.isValid(snake0.getNextX_n(dire,n),snake0.getNextY_n(dire,n)))
            n++;
        return Rank(n,0);
    }
    //可以走

    Snake *snake0_next=new Snake(snake0);
    Snake *snake1_next=new Snake(snake1);
    //己方行动
    snake0_next->move(dire,steps);
    //敌方行动
    int k;
    for(k=0; k<4; k++)
    {
        if(map.isValid(snake1.getNextX(k),snake1.getNextY(k)))
        {
            snake1_next->move(k,steps);
            break;
        }
    }
    if(k==4)
    {
        //敌方无处可走了，这样我方也无须再试探了
        //但是这种情况少量出现不应影响我方决策，大量出现说明成杀，应当偏爱
        delete snake0_next;
        delete snake1_next;
        return Rank(MAX_DEPS-depth,0);
    }
    if(snake0_next->x.front()==snake1_next->x.front()
            &&
            snake0_next->y.front()==snake1_next->y.front()
      )
    {
        //两蛇头撞在一起，我们不考虑这种情况
        delete snake0_next;
        delete snake1_next;
        return Rank(1,0);
    }

    Map *map_next=new Map(map,*snake0_next,*snake1_next);
    Rank rank(1,0);
    //己方行动方向
    for(int dire_next=0; dire_next<4; dire_next++)
    {
        if(ABS(dire_next-dire)==2)
            continue;

        //下一级预测
        rank+=predict(*map_next,*snake0_next,*snake1_next,
                      dire_next,depth+1,steps+1);

    }
    delete snake0_next;
    delete snake1_next;
    delete map_next;


    return rank;
}


/*
void outputSnakeBody(int id)    //调试语句
{
	cout<<"Snake No."<<id<<endl;
	for (list<point>::iterator iter=snake[id].begin();iter!=snake[id].end();++iter)
		cout<<iter->x<<" "<<iter->y<<endl;
	cout<<endl;
}
*/

int main()
{
    string str;
    string temp;
    while (getline(cin,temp))
        str+=temp;

    Json::Reader reader;
    Json::Value input;
    reader.parse(str,input);

    MapBasic mapBasic;
    Snake snake0,snake1;
    mapBasic.n=
        input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘宽度
    mapBasic.m=
        input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //棋盘高度

    int x=input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //读蛇初始化的信息
    if (x==1)
    {
        snake0.push(1,1);
        snake1.push(mapBasic.n,mapBasic.m);
    }
    else
    {
        snake1.push(1,1);
        snake0.push(mapBasic.n,mapBasic.m);
    }
    //处理地图中的障碍物
    int obsCount=input["requests"][(Json::Value::UInt) 0]["obstacle"].size();

    for (int i=0; i<obsCount; i++)
    {
        int ox=input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
        int oy=input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
        mapBasic.obst[ox][oy]=true;
    }

    //根据历史信息恢复现场
    int total=input["responses"].size();

    int dire;
    for (int i=0; i<total; i++)
    {
        dire=input["responses"][i]["direction"].asInt();
        snake0.move(dire,i);

        dire=input["requests"][i+1]["direction"].asInt();
        snake1.move(dire,i);
    }

    Map map(&mapBasic,snake0,snake1);

    int best_dir=0;
    Rank rank,best_rank;
    for(dire=0; dire<4; dire++)
    {

        Snake *snake0_next=new Snake(snake0);
        Snake *snake1_next=new Snake(snake1);
        Map *map_next=new Map(map,*snake0_next,*snake1_next);
        rank=predict(*map_next,*snake0_next,*snake1_next,
                     dire,0,total);
        if(rank.freedom>best_rank.freedom)
        {
            best_rank=rank;
            best_dir=dire;
        }
        delete snake0_next;
        delete snake1_next;
        delete map_next;
    }

    //做出一个决策
    Json::Value ret;
    ret["response"]["direction"]=best_dir;

    char log[100];
    sprintf(log,"log%d",dire);
    string slog(log);

    ret["data"]=slog;
    //cout<<"cout."<<endl;

    Json::FastWriter writer;
    cout<<writer.write(ret)<<endl;

    return 0;
}
/*

#include <windows.h>
extern CRITICAL_SECTION displock;

void  extern_init()
{
    num=0;
    running=true;
}



void runMain()
{
    while(1)
    {
        if(!running)break;
        EnterCriticalSection(&displock);
        //进行绘图
        num+=2;
        if(num>250)num=0;
        LeaveCriticalSection(&displock);
        Sleep(50);
    }
}

/*
jsoncpp test code
void  preCal()
{
    Json::Value ret;

	char log[100];
	sprintf(log,"log%d",111);
	string slog(log);

	ret["data"]=slog;
	//cout<<"cout."<<endl;

	ret["response"]["direction"]=5;

    Json::FastWriter writer;
	cout<<writer.write(ret)<<endl;
}

*/
