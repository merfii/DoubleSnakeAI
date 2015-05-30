#ifndef HEAP_HPP_INCLUDED
#define HEAP_HPP_INCLUDED


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
        Block *last=heap[heap_size--];
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
        //printf("i=%d ",i);
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



    void debug_dump()
    {

        printf("%d  ",heap[1]->val);

        printf("%d ",heap[2]->val);
        printf("%d  ",heap[3]->val);

        printf("%d ",heap[4]->val);
        printf("%d ",heap[5]->val);
        printf("%d ",heap[6]->val);
        printf("%d  ",heap[7]->val);

        printf("%d ",heap[8]->val);
        printf("%d ",heap[9]->val);
        printf("%d ",heap[10]->val);
        printf("%d ",heap[11]->val);
        printf("%d ",heap[12]->val);
        printf("%d ",heap[13]->val);
        printf("%d ",heap[14]->val);
        printf("%d \n",heap[15]->val);



       // printf("heap_size:%d \n",heap_size);

    }
};







#endif // HEAP_HPP_INCLUDED
