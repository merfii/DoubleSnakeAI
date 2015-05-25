#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include<ctime>
#include "jsoncpp/json.h"

#define MAX_MAP 25
#define MAX_DEPS 6
#define ABS(a) ((a>0)?(a):(-a))


using namespace std;

//0 左 1下 2右 3 上
static const int dx[4]= {-1,0,1,0};
static const int dy[4]= {0,1,0,-1};


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


    void move(int dire)
    {
        x.push_front(x.front()+dx[dire]);
        y.push_front(y.front()+dy[dire]);
        length++;
    }

    void deleteTail(int steps)
    {
        if(!whetherGrow(steps))
        {
            x.pop_back();
            y.pop_back();
            length--;
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
    int w,h;
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
    int w,h;

    Map(MapBasic *mapBasic,Snake &snake0,Snake &snake1)
    {
        pMapBasic=mapBasic;
        w=pMapBasic->w;
        h=pMapBasic->h;
        for(int i=0; i<=w; i++)
            for(int j=0; j<=h; j++)
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
        w=pMapBasic->w;
        h=pMapBasic->h;

        for(int i=0; i<=w; i++)
            for(int j=0; j<=h; j++)
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
        if(x>w || y>h || x<1 || y<1)
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
        freedom=-1;
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

static int calDire(Map &map,Snake &snake0,Snake &snake1,int steps,int depths);
static Rank predict(Map &oldmap,Snake &snk0,Snake &snk1,
             int dire,int steps,int depths)
{
    Snake *snake0=new Snake(snk0);
    Snake *snake1=new Snake(snk1);
    snake0->deleteTail(steps);
    snake1->deleteTail(steps);
    Map *map=new Map(oldmap,*snake0,*snake1);

    if(!map->isValid(snake0->getNextX(dire),snake0->getNextY(dire)))
    {
        delete snake0;
        delete snake1;
        delete map;
        return Rank(0,0);
    }

    if(depths<=0)
    {
        int n=1;
        while(map->isValid(snake0->getNextX_n(dire,n),snake0->getNextY_n(dire,n)))
            n++;
        if(n<0)
            printf("Bug Warnning! n=%d\n",n);
        delete snake0;
        delete snake1;
        delete map;
        return Rank(n,0);
    }
    //可以走

    //敌方行动
    snake1->move(calDire(*map,*snake1,*snake0,steps,depths-2));
    //己方行动
    snake0->move(dire);




    if(snake0->x.front()==snake1->x.front()
            &&
            snake0->y.front()==snake1->y.front()
      )
    {
        //两蛇头撞在一起
        delete snake0;
        delete snake1;
        delete map;
        return Rank(1,0);
    }

    Rank rank(1,0);
    //己方行动方向
    for(int dire_next=0; dire_next<4; dire_next++)
    {
        if(ABS(dire_next-dire)==2)
            continue;

        //下一级预测
        rank+=predict(*map,*snake0,*snake1,
                      dire_next,steps+1,depths-1);

    }
    delete snake0;
    delete snake1;
    delete map;

    return rank;
}

//static string turnString[4]={"Left","Down","Right","Up"};
static int calDire(Map &map,Snake &snake0,Snake &snake1,int steps,int depths)
{
    int best_dir=0;
    Rank rank,best_rank;
    for(int dire=0; dire<4; dire++)
    {
        rank=predict(map,snake0,snake1,
                     dire,steps,depths);

        if(rank.freedom>best_rank.freedom)
        {
            best_rank=rank;
            best_dir=dire;
        }
    }
    if(depths==MAX_DEPS)
        printf("Best freedom: %3.1f\n",best_rank.freedom);
    return best_dir;
}


#ifdef LOCAL
string mai(string inputString)
#else
int main()
#endif
{
    string str;
    string temp;
#ifdef LOCAL
    str=inputString;
#else
    while (getline(cin,temp))
        str+=temp;
#endif

    Json::Reader reader;
    Json::Value input;
    reader.parse(str,input);

    MapBasic mapBasic;
    Snake snake0,snake1;
    int steps;

    mapBasic.w=
        input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘宽度
    mapBasic.h=
        input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //棋盘高度

    int x=input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //读蛇初始化的信息
    if (x==1)
    {
        snake0.push(1,1);
        snake1.push(mapBasic.w,mapBasic.h);
    }
    else
    {
        snake1.push(1,1);
        snake0.push(mapBasic.w,mapBasic.h);
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
    steps=input["responses"].size();

    int dire;
    for (int i=0; i<steps; i++)
    {
        dire=input["responses"][i]["direction"].asInt();
        snake0.deleteTail(i);
        snake0.move(dire);

        dire=input["requests"][i+1]["direction"].asInt();
        snake1.deleteTail(i);
        snake1.move(dire);    }

    Map map(&mapBasic,snake0,snake1);

    //做出决策
    Json::Value ret;
    int dirr=calDire(map,snake0,snake1,steps,MAX_DEPS);
    ret["response"]["direction"]=dirr;

    Json::FastWriter writer;
#ifdef LOCAL
    return writer.write(ret);
#else
    cout<<writer.write(ret)<<endl;
    return 0;
#endif

}

