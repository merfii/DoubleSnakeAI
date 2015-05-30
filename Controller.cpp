
#include <windows.h>
#include <string>
#include <cstdio>
#include <vector>
#include "jsoncpp/json.h"
#include "GUI.h"

using  namespace std;

static struct{
bool running;
int keypressed;
}thread_comm;


static string mapData1("{\"requests\":[{\"y\":1,\"x\":1,\"width\":10,\"obstacle\":[{\"y\":5,\"x\":1},{\"y\":6,\"x\":15},{\"y\":9,\"x\":3},{\"y\":2,\"x\":13},{\"y\":9,\"x\":15},{\"y\":2,\"x\":1},{\"y\":9,\"x\":13},{\"y\":2,\"x\":3},{\"y\":5,\"x\":2},{\"y\":6,\"x\":14},{\"y\":5,\"x\":14},{\"y\":6,\"x\":2},{\"y\":9,\"x\":10},{\"y\":2,\"x\":6}],\"height\":15}],\"responses\":[]}");
static string mapData2("{\"requests\":[{\"y\":1,\"x\":1,\"width\":12,\"obstacle\":[{\"y\":5,\"x\":14},{\"y\":8,\"x\":3},{\"y\":7,\"x\":9},{\"y\":6,\"x\":8},{\"y\":2,\"x\":9},{\"y\":11,\"x\":8},{\"y\":5,\"x\":1},{\"y\":8,\"x\":16},{\"y\":5,\"x\":3},{\"y\":8,\"x\":14},{\"y\":9,\"x\":2},{\"y\":4,\"x\":15},{\"y\":1,\"x\":7},{\"y\":12,\"x\":10},{\"y\":9,\"x\":1},{\"y\":4,\"x\":16},{\"y\":1,\"x\":2},{\"y\":12,\"x\":15}],\"height\":16}],\"responses\":[]}");
static string mapData3("{\"requests\":[{\"y\":1,\"x\":1,\"width\":12,\"obstacle\":[{\"y\":9,\"x\":9},{\"y\":4,\"x\":4},{\"y\":12,\"x\":1},{\"y\":1,\"x\":12},{\"y\":5,\"x\":9},{\"y\":8,\"x\":4},{\"y\":1,\"x\":5},{\"y\":12,\"x\":8},{\"y\":3,\"x\":5},{\"y\":10,\"x\":8},{\"y\":11,\"x\":5},{\"y\":2,\"x\":8},{\"y\":9,\"x\":7},{\"y\":4,\"x\":6}],\"height\":12}],\"responses\":[]}");
static string mapData4("{\"requests\":[{\"y\":12,\"x\":11,\"width\":12,\"obstacle\":[{\"y\":7,\"x\":3},{\"y\":6,\"x\":9},{\"y\":10,\"x\":2},{\"y\":3,\"x\":10},{\"y\":9,\"x\":6},{\"y\":4,\"x\":6},{\"y\":6,\"x\":5},{\"y\":7,\"x\":7},{\"y\":2,\"x\":1},{\"y\":11,\"x\":11},{\"y\":5,\"x\":4},{\"y\":8,\"x\":8}],\"height\":11}],\"responses\":[]}");
static MapBasic mapBasic;
static Snake snake0,snake1;
static vector<string> history;
static int steps;    //当前(还没走)为第step回合 此时已有前step-1步的数据

static int resolve(string &boutString, int nstep);
static string snakeMove(string &boutString,int dir1,int dir2);
static int resolveDir(string ret);
string mai(string inputString);

void extern_init()
{
    thread_comm.running=true;
    thread_comm.keypressed=-1;
}


void runMain()
{
    history.push_back(mapData1);

restart:

    steps=resolve(history[0],0);
    updateDisp(mapBasic,snake0,snake1);
    string botret;
    int fp=steps;
    while(1)
    {
        int key;
        if(!thread_comm.running)break;
        Sleep(30);

        DISPLOCK();

        if(thread_comm.keypressed==-1)
        {
             DISPUNLOCK();
        }else
        {
            key=thread_comm.keypressed;
            thread_comm.keypressed=-1;
            DISPUNLOCK();

            switch(key)
            {
                //0 左 1下 2右 3 上
                case 0:
                case 1:
                case 2:
                case 3:
                    botret=mai(history[fp]);
                    if((int)history.size()<=fp+1)
                        history.push_back(string());
                    history[fp+1]=snakeMove(history[fp],resolveDir(botret),key);
                    fp++;
                    steps=fp;
                    break;

                case 10:
                    fp++;
                    if(fp>steps)
                        fp=steps;
                    break;

                case 11:
                    fp--;
                    if(fp<0)fp=0;
                    break;
                case 21:
                    history.clear();
                    history.push_back(mapData1);
                    goto restart;
                    break;
                case 22:
                    history.clear();
                    history.push_back(mapData2);
                    goto restart;
                    break;
                case 23:
                    history.clear();
                    history.push_back(mapData4);
                    goto restart;
                    break;
                case 24:
                history.clear();
                history.push_back(mapData3);
                goto restart;
                break;
            }
            resolve(history[fp],-1);
              //绘图
            updateDisp(mapBasic,snake0,snake1);
        }
    }
}

//返回当前(已走)为第几回合
static int resolve(string &boutString, int limit_step)
{
    Json::Reader reader;
    Json::Value input;
    reader.parse(boutString,input);

    snake0=Snake();
    snake1=Snake();
    mapBasic=MapBasic();


    mapBasic.w=
        input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘宽度 所给数据就是交叉的
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
    mapBasic.obst_count=obsCount;

    for (int i=0; i<obsCount; i++)
    {
        int ox=input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
        int oy=input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
        mapBasic.obst[ox][oy]=true;
        mapBasic.Xarray[i]=ox;
        mapBasic.Yarray[i]=oy;
    }
    int total=input["responses"].size();

    int dire;
    if(limit_step>=0)
        total=total<limit_step?total:limit_step;
    for (int i=0; i<total; i++)
    {
        dire=input["responses"][i]["direction"].asInt();
        snake0.deleteTail(i);
        snake0.move(dire);

        dire=input["requests"][i+1]["direction"].asInt();
        snake1.deleteTail(i);
        snake1.move(dire);
    }

    return total;

}

static string snakeMove(string &boutString,int dir0,int dir1)
{
    Json::Reader reader;
    Json::Value input;
    reader.parse(boutString,input);

    int total=input["responses"].size();
    input["responses"][total]["direction"]=dir0;
    input["requests"][total+1]["direction"]=dir1;
    Json::FastWriter writer;
	return writer.write(input);
}

static int resolveDir(string ret)
{
    //printf("Bot Ret:%s\n",ret.c_str());
    Json::Reader reader;
    Json::Value input;
    reader.parse(ret,input);
    return input["response"]["direction"].asInt();
    //printf("Json Ret: %d\n",ss);
}



void keyPressed(int key)
{
    DISPLOCK();
    thread_comm.keypressed=key;
    DISPUNLOCK();
}

void stopRun()
{
    DISPLOCK();
    thread_comm.running=false;
    DISPUNLOCK();
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
