#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<list>
#include<string>
#include "jsoncpp/json.h"

#define MAX_MAP 32  //本来是25 为了优化对齐改为2^5
#define FREE_DEPS 6
#define KILL_DEPS 8
#define FREE_LOW_LEVEL 80

#define ABS(a) ((a>0)?(a):(-a))

using namespace std;

//0 左 1下 2右 3 上
static const int dx[4]= {-1,0,1,0};
static const int dy[4]= {0,1,0,-1};
static int debug_msg;

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

/*
    void sniff()    //蛇头向四个可能的方向运动 用于降低路径规划的敏感性
    {
        Point head=points.front();
        for(int dire=0;dire<4;dire++)
        {
            points.push_front(Point(head.x+dx[dire],head.y+dy[dire]));
        }
    }
*/
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
        memcpy(&(obstAndSnake[0][0]),&(pMapBasic->obst[0][0]),MAX_MAP*MAX_MAP*sizeof(obstAndSnake[0][0]));

        list<Point>::iterator it;
        for(it=snake0.points.begin();it!=snake0.points.end();it++)
            obstAndSnake[it->x][it->y]=true;

        for(it=snake1.points.begin();it!=snake1.points.end();it++)
            obstAndSnake[it->x][it->y]=true;
    }

    Map(Map &preMap,Snake &snake0,Snake &snake1)
    {
        pMapBasic=preMap.pMapBasic;
        w=pMapBasic->w;
        h=pMapBasic->h;

        memcpy(&(obstAndSnake[0][0]),&(pMapBasic->obst[0][0]),MAX_MAP*MAX_MAP*sizeof(obstAndSnake[0][0]));

        list<Point>::iterator it;
        for(it=snake0.points.begin();it!=snake0.points.end();it++)
            obstAndSnake[it->x][it->y]=true;

        for(it=snake1.points.begin();it!=snake1.points.end();it++)
            obstAndSnake[it->x][it->y]=true;
    }

    bool isValid(const Point &p)
    {
        if(p.x>w || p.x<1 || p.y>h || p.y<1)
            return false;
        return !obstAndSnake[p.x][p.y];
    }
};

class Rank
{
public:
    int dire;
    float freedom;

    Rank()
    {
        dire=0;
        freedom=-1;
    }

    Rank(int d,float fr)
    {
        dire=d;
        freedom=fr;
    }

    Rank& operator+=(const Rank &a)
    {
        freedom+=a.freedom;
        return *this;
    }
};

typedef struct _Block
{
    int val;  //地图中的距离
    int index;  //堆中的索引值
    //int xy=x*MAX_MAP+y;  //地图中的坐标 直接用数组索引值替代了
    int back;   //为了推算路径 指向上一步节点
}Block;

#ifdef LOCAL
//#include "Heap.hpp"
void updateShortPath(int sm[MAX_MAP][MAX_MAP]);
#endif

#define MIN(X,Y) ((X)<(Y)?(X):(Y))
//小顶堆

class MinHeap
{

    Block *heap[MAX_MAP*MAX_MAP];
    int heap_size;

public:

    MinHeap(Block *vertices)
    {
        heap_size=0;
        vertices[0].val=-1;
        vertices[0].index=0;
        heap[0]=&vertices[0];
    }

    int getSize()
    {
        return heap_size;
    }

    void insert(Block *x)
    {
        if(x==NULL)
            return ;

        int i;
        for(i = ++heap_size; heap[i/2]->val > x->val; i/=2)
        {
             heap[i]=heap[i/2];
             heap[i]->index=i;
        }
        heap[i]=x;
        heap[i]->index=i;
    }

    Block *removeMin()
    {

        if(heap_size==0)
            return NULL;

        Block *min=heap[1];
        Block *last=heap[heap_size];
        heap_size--;
        int child;

        int i;
        for(i=1;i*2<=heap_size;i=child)
        {
                /* Find smaller child */
            child =i*2;
            if(child!=heap_size && heap[child+1]->val < heap[child]->val)
                    child++;
                /* Percolate one level */
            if(last->val > heap[child]->val)
            {
                heap[i]=heap[child];
                heap[i]->index=i;
            }
            else
                break;
        }
        heap[i]=last;
        heap[i]->index=i;

        return min;
    }


    void ChangeAt(Block *x)
    {
        if(x==NULL || x->index==0)
            return;

        int i=x->index;
        if(heap[i]->val < heap[i/2]->val)
        {
            //改小了，应该上浮
            for(;  x->val < heap[i/2]->val; i/=2)
            {
                heap[i]=heap[i/2];
                heap[i]->index=i;
            }
            heap[i]=x;
            heap[i]->index=i;


        }else if(i*2>heap_size)
        {
            return;  //下面没东西 不用改
        }
        else if(heap[i]->index >
            (   heap_size==i*2 ?
                heap[i*2]->val
                :
                MIN(heap[i*2]->val,heap[i*2+1]->val)
            )  )
        {
            int child;
            //改大了，应该下沉
            for(     ; i*2<=heap_size;i=child)
            {
                /* Find smaller child */
                child =i*2;
                if(child!=heap_size && heap[child+1]->val < heap[child]->val)
                        child++;
                /* Percolate one level */
                if(x->val > heap[child]->val)
                {
                    heap[i]=heap[child];
                    heap[i]->index=i;

                }else
                {
                    break;
                }
            }
            heap[i]=x;
            heap[i]->index=i;

        }
        else
        {
            //没改多少，不用修正
            return;
        }
    }
};

void mapWeight(Map &map,Snake &snk0,Snake &snk1,int steps,Block vertices[MAX_MAP*MAX_MAP])
{
    int w=map.w;
    int h=map.h;

    Snake snake0(snk0);
    Snake snake1(snk1);
    snake0.deleteTail(steps);
    snake1.deleteTail(steps);

    //memset(&(weight[0][0],0,MAX_MAP*MAX_MAP*sizeof(weight[0][0]));
    for(int i=0; i<MAX_MAP; i++)
    {
        for(int j=0; j<MAX_MAP; j++)
        {
            vertices[i*MAX_MAP+j].index=0;
            vertices[i*MAX_MAP+j].back=0;
            if(map.pMapBasic->obst[i][j] || i>w || i<1 || j>h || j<1)
                vertices[i*MAX_MAP+j].val=-1;   //这个值大有讲究 设置为-1添加连通节点后更新方便, 又不影响找最短路径
            else
                vertices[i*MAX_MAP+j].val=1000;
        }
    }

    list<Point>::iterator it;

    for(it=snake1.points.begin();it!=snake1.points.end();it++)
        vertices[(it->x)*MAX_MAP+(it->y)].val=-1;


    it=snake0.points.begin();   //己方蛇头0步即可走到
    vertices[(it->x)*MAX_MAP+(it->y)].val=0;

    for(it++;it!=snake0.points.end();it++)
        vertices[(it->x)*MAX_MAP+(it->y)].val=-1;


    MinHeap minHeap(vertices);

    for(int i=1;i<MAX_MAP*MAX_MAP;i++)
    {
        if(vertices[i].val!=-1)
        {
            minHeap.insert(&vertices[i]);

        }
    }

    Block *minVertex;
    do{
        minVertex=minHeap.removeMin();
        if(minVertex)
        {
            minVertex->index=0;
            for(int dire=0;dire<4;dire++)
            {
                int x=(minVertex-vertices)/MAX_MAP+dx[dire];
                int y=(minVertex-vertices)%MAX_MAP+dy[dire];

                if(x*MAX_MAP+y >= MAX_MAP*MAX_MAP)
                    continue;
                Block *sur=&vertices[x*MAX_MAP+y];
                if(0== sur->index)
                    continue;

                if(sur->val
                   >
                  minVertex->val+1 )
                {
                    sur->val=minVertex->val+1;
                    sur->back=minVertex-vertices;
                    minHeap.ChangeAt(sur);

                }
            }
        }

    }while(minVertex!=NULL);
}


static Rank connectness(Map &map,Snake &snake0,Snake &snake1,int steps)
{
    /*
    末-初+1=
    -1,0 => 0,1  左0
    0, 1 => 1,2  下1
    1, 0 => 2,1  右2
    0,-1 => 1,0  上3
    */
    static const int xy2dire[3][3]={
    {0,0,0},
    {3,0,1},
    {0,2,0}};

    int direSeries[20];
    int direSerCount;
    float best_jam=10000;int best_dire=0;
    Block m0[MAX_MAP*MAX_MAP];
    Block m1[MAX_MAP*MAX_MAP];


    mapWeight(map,snake0,snake1,steps,m0);

    for(int i=0;i<=map.w;i++)
    {
        for(int j=0;j<=map.h;j++)
        {
            if(m0[i*MAX_MAP+j].val<=0 || m0[i*MAX_MAP+j].val>20)
                continue;
            direSerCount=0;
            int idx=i*MAX_MAP+j;
            while(m0[idx].val!=0)
            {
                int mx=idx/MAX_MAP-m0[idx].back/MAX_MAP+1;  //+1是为了满足-1时能查表
                int my=idx%MAX_MAP-m0[idx].back%MAX_MAP+1;
                direSeries[direSerCount++]=xy2dire[mx][my];
                idx=m0[idx].back;
            }
            Snake snk0(snake0);
            for(int s=0;s<direSerCount;s++)
            {
                snk0.deleteTail(s+steps);
                snk0.move(direSeries[direSerCount-s-1]);
            }
            mapWeight(map,snake1,snk0,steps,m1);

            float S=0;
            for(int ii=0;ii<=map.w;ii++)
            {
                for(int jj=0;jj<=map.h;jj++)
                {
                    if(m1[ii*MAX_MAP+jj].val>20)
                        continue;
                    if(m1[ii*MAX_MAP+jj].val<0)
                    {
                        S+=1/2.0f;
                    }else
                    {
                        S+=1/(1+(float)m1[ii*MAX_MAP+jj].val);
                    }

                }
            }
            if(S<best_jam)
            {
                best_jam=S;
                best_dire=direSeries[direSerCount-1];

#ifdef LOCAL
                int out[MAX_MAP][MAX_MAP];
                for(int xx=0;xx<=MAX_MAP;xx++)
                {
                    for(int yy=0;yy<=MAX_MAP;yy++)
                    {
                        out[xx][yy]=m1[xx*MAX_MAP+yy].val;
                    }
                }

                list<Point>::iterator it;
                for(it=snk0.points.begin();it!=snk0.points.end();it++)
                    out[it->x][it->y]=500;

                updateShortPath(out);
#endif
            }
        }
    }


    return Rank(best_dire,best_jam);
}

static int simplePred(Map &map,Point p,int depths)
{
    if(depths<=0)
        return 0;
    if(!map.isValid(p))
        return 0;

    int x=p.x;
    int y=p.y;
    map.obstAndSnake[x][y]=true;

    int ret=simplePred(map,Point(x-1,y),depths-1)+
            simplePred(map,Point(x+1,y),depths-1)+
            simplePred(map,Point(x,y-1),depths-1)+
            simplePred(map,Point(x,y+1),depths-1)+1;

    map.obstAndSnake[p.x][p.y]=false;
    return ret;
}

static int simpleDire(Map &map,Snake &snake)
{

    int best_dire=0,max=-1;

    for(int dire=0; dire<4; dire++)
    {
        int ret=simplePred(map,snake.getNext(dire),4);
        if(ret>max)
        {
            max=ret;
            best_dire=dire;
        }
    }

    return best_dire;
}


static Rank predict(Map &oldmap,Snake &snk0,Snake &snk1,
             int dire,int steps,int depths)
{
    Snake snake0(snk0);
    Snake snake1(snk1);
    snake0.deleteTail(steps);
    snake1.deleteTail(steps);
    Map *map=new Map(oldmap,snake0,snake1);

    if(!map->isValid(snake0.getNext(dire)))
    {
        delete map;
        return Rank(dire,0);
    }

    if(depths<0)
    {
        int n=1;
        while(map->isValid(snake0.getNextN(dire,n)))
            n++;
        if(n<0)
            printf("Bug Warnning! n=%d\n",n);
        delete map;
        return Rank(dire,n);
    }
    //可以走

    //敌方行动
    snake1.move(simpleDire(*map,snake1));
    //己方行动
    snake0.move(dire);

    if(snake0.points.front()==snake1.points.front())
    {
        //两蛇头撞在一起
        delete map;
        return Rank(dire,1);
    }

    Rank rank(dire,1);
    //己方行动方向
    for(int dire_next=0; dire_next<4; dire_next++)
    {
        if(ABS(dire_next-dire)==2)
            continue;

        //下一级预测
        rank+=predict(*map,snake0,snake1,
                      dire_next,steps+1,depths-1);
    }
    delete map;
    return rank;
}


/*
近步成杀。
即按照这一走法，己方不死，对方无论怎么走都会死。
该情况属于硬性胜利条件，只要存在即实施，无须再看其他指标。

全局标志；
未走到底己方已死，失败
未走到底对方已无路可走，成功
走到底对方未死，失败
*/
static bool localKilling(Map &oldmap,Snake &snk0,Snake &snk1,
                         int dire,int steps,int depths)
{
    if(depths<0)
        return false;
    Snake snake0(snk0);
    Snake snake1(snk1);
    snake0.deleteTail(steps);
    snake1.deleteTail(steps);
    Map *map=new Map(oldmap,snake0,snake1);

    if(!map->isValid(snake0.getNext(dire)))
    {
        delete map;
        return false;
    }

    snake0.move(dire);

    for(int i=0;i<4;i++)
    {
        if(!map->isValid(snake1.getNext(i)))
        {
            continue;
        }

        Snake next(snake1);
        next.move(i);
        int j;
        for(j=0;j<4;j++)
        {
            if(localKilling(*map,snake0,next,j,steps+1,depths-1))
                break;
        }
        if(j>=4)
        {
            //对方这样走 你怎么走都赢不了 失败
            delete map;
            return false;
        }
    }
    delete map;
    return true;
}

//static string turnString[4]={"Left","Down","Right","Up"};
static int calDire(Map &map,Snake &snake0,Snake &snake1,int steps)
{
    for(int dire=0; dire<4; dire++)
    {
        if(localKilling(map,snake0,snake1,dire,steps,KILL_DEPS))
            return dire;
    }

    Rank rank[5];
    Rank best_rank(0,-1);
    for(int dire=0; dire<4; dire++)
    {
        rank[dire]=predict(map,snake0,snake1,
                     dire,steps,FREE_DEPS);
#ifdef LOCAL
        printf("%3.1f  ",rank[dire].freedom);
#endif
        if(rank[dire].freedom>best_rank.freedom)
        {
            best_rank=rank[dire];
            best_rank.dire=dire;
        }
    }

    rank[4]=connectness(map,snake0,snake1,steps);

    #ifdef LOCAL
    printf("\nfreedom:    %d   %3.1f\n",best_rank.dire,best_rank.freedom);
    printf("connectness %d %3.1f\n",rank[4].dire,rank[4].freedom);
    #endif

    if(best_rank.dire==rank[4].dire)
    {
        return best_rank.dire;
    }else if(rank[rank[4].dire].freedom>FREE_LOW_LEVEL)
    {
        if(rank[4].freedom<45)
           return rank[4].dire;
    }
    return best_rank.dire;

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
        snake0.addHead(Point(1,1));
        snake1.addHead(Point(mapBasic.w,mapBasic.h));
    }
    else
    {
        snake1.addHead(Point(1,1));
        snake0.addHead(Point(mapBasic.w,mapBasic.h));
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


    for (int i=0; i<steps; i++)
    {
        int dire;
        dire=input["responses"][i]["direction"].asInt();
        snake0.deleteTail(i);
        snake0.move(dire);

        dire=input["requests"][i+1]["direction"].asInt();
        snake1.deleteTail(i);
        snake1.move(dire);
    }

    Map map(&mapBasic,snake0,snake1);

    //做出决策
    Json::Value ret;
    int dire=calDire(map,snake0,snake1,steps);
    ret["response"]["direction"]=dire;
    ret["debug"]=debug_msg;
    Json::FastWriter writer;
#ifdef LOCAL
    return writer.write(ret);
#else
    cout<<writer.write(ret)<<endl;
    return 0;
#endif

}

