 #include<string.h>
 #include<ctype.h>
 #include<malloc.h> /* malloc()等 */
 #include<limits.h> /* INT_MAX等 */
 #include<stdio.h> /* EOF(=^Z或F6),NULL */
 #include<stdlib.h> /* atoi() */
 #include<io.h> /* eof() */
 #include<math.h> /* floor(),ceil(),abs() */
 #include<process.h> /* exit() */
 /* 函数结果状态代码 */
 #define TRUE 1
 #define FALSE 0
 #define OK 1
 #define ERROR 0
 #define INFEASIBLE -1
 /* #define OVERFLOW -2 因为在math.h中已定义OVERFLOW的值为3,故去掉此行 */
 typedef int Status; /* Status是函数的类型,其值是函数结果状态代码，如OK等 */
 typedef int Boolean; /* Boolean是布尔类型,其值是TRUE或FALSE */
 
 #define MAX_NAME 5 /* 顶点字符串的最大长度+1 */
 #define MAX_INFO 20 /* 相关信息字符串的最大长度+1 */
 typedef int VRType;
 typedef char InfoType;
 typedef char VertexType[MAX_NAME];
 #define INFINITY INT_MAX /* 用整型最大值代替∞ */
 #define MAX_VERTEX_NUM 26 /* 最大顶点个数 */
 typedef enum{DG,DN,UDG,UDN}GraphKind; /* {有向图,有向网,无向图,无向网} */
 typedef struct
 {
   VRType adj; /* 顶点关系类型。对无权图，用1(是)或0(否)表示相邻否；对带权图，则为权值 */
   InfoType *info; /* 该弧相关信息的指针(可无) */
 }ArcCell,AdjMatrix[MAX_VERTEX_NUM][MAX_VERTEX_NUM]; /* 二维数组 */
 typedef struct
 {
   VertexType vexs[MAX_VERTEX_NUM]; /* 顶点向量 */
   AdjMatrix arcs; /* 邻接矩阵 */
   int vexnum,arcnum; /* 图的当前顶点数和弧数 */
   GraphKind kind; /* 图的种类标志 */
 }MGraph;
 
 
 Boolean visited[MAX_VERTEX_NUM]; /* 访问标志数组(全局量) */
 void(*VisitFunc)(VertexType); /* 函数变量 */
 typedef VRType QElemType; /* 队列元素类型 */
 typedef struct QNode
 {
   QElemType data;
   struct QNode *next;
 }QNode,*QueuePtr;

 typedef struct
 {
   QueuePtr front,rear; /* 队头、队尾指针 */
 }LinkQueue;
 
 typedef int PathMatrix[MAX_VERTEX_NUM][MAX_VERTEX_NUM]; /* 路径矩阵，二维数组 */
 typedef int ShortPathTable[MAX_VERTEX_NUM]; /* 最短距离表，一维数组 */
 
 
 int LocateVex(MGraph G,VertexType u)
 { /* 初始条件：图G存在，u和G中顶点有相同特征 */
   /* 操作结果：若G中存在顶点u，则返回该顶点在图中位置；否则返回-1 */
   int i;
   for(i=0;i<G.vexnum;++i)
     if(strcmp(u,G.vexs[i])==0)
       return i;
   return -1;
 }

 void CreateFUDG(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，由文件构造没有相关信息的无向图G */
   int i,j,k;
   char filename[13];
   VertexType va,vb;
   FILE *graphlist;
   printf("请输入数据文件名(f7-1.txt或f7-2.txt)：");
   scanf("%s",filename);
   graphlist=fopen(filename,"r");/* 打开数据文件，并以graphlist表示 */
   fscanf(graphlist,"%d",&(*G).vexnum);
   fscanf(graphlist,"%d",&(*G).arcnum);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     fscanf(graphlist,"%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=0; /* 图 */
       (*G).arcs[i][j].info=NULL; /* 没有相关信息 */
     }
   for(k=0;k<(*G).arcnum;++k)
   {
     fscanf(graphlist,"%s%s",va,vb);
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=(*G).arcs[j][i].adj=1; /* 无向图 */
   }
   fclose(graphlist); /* 关闭数据文件 */
   (*G).kind=UDG;
 }

 void CreateFUDN(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，由文件构造没有相关信息的无向网G */
   int i,j,k,w;
   char filename[13];
   VertexType va,vb;
   FILE *graphlist;
   printf("请输入数据文件名：");
   scanf("%s",filename);
   graphlist=fopen(filename,"r"); /* 打开数据文件，并以graphlist表示 */
   fscanf(graphlist,"%d",&(*G).vexnum);
   fscanf(graphlist,"%d",&(*G).arcnum);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     fscanf(graphlist,"%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=INFINITY; /* 网 */
       (*G).arcs[i][j].info=NULL; /* 没有相关信息 */
     }
   for(k=0;k<(*G).arcnum;++k)
   {
     fscanf(graphlist,"%s%s%d",va,vb,&w);
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=(*G).arcs[j][i].adj=w; /* 无向网 */
   }
   fclose(graphlist); /* 关闭数据文件 */
   (*G).kind=UDN;
 }

 void CreateDG(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，构造有向图G */
   int i,j,k,l,IncInfo;
   char s[MAX_INFO];
   VertexType va,vb;
   printf("请输入有向图G的顶点数,弧数,弧是否含其它信息(是:1,否:0): ");
   scanf("%d,%d,%d",&(*G).vexnum,&(*G).arcnum,&IncInfo);
   printf("请输入%d个顶点的值(<%d个字符):\n",(*G).vexnum,MAX_NAME);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     scanf("%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=0; /* 图 */
       (*G).arcs[i][j].info=NULL;
     }
   printf("请输入%d条弧的弧尾 弧头(以空格作为间隔): \n",(*G).arcnum);
   for(k=0;k<(*G).arcnum;++k)
   {
     scanf("%s%s%*c",va,vb);  /* %*c吃掉回车符 */
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=1; /* 有向图 */
     if(IncInfo)
     {
       printf("请输入该弧的相关信息(<%d个字符): ",MAX_INFO);
       gets(s);
       l=strlen(s);
       if(l)
       {
         (*G).arcs[i][j].info=(char*)malloc((l+1)*sizeof(char)); /* 有向 */
         strcpy((*G).arcs[i][j].info,s);
       }
     }
   }
   (*G).kind=DG;
 }

 void CreateDN(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，构造有向网G */
   int i,j,k,w,IncInfo;
   char s[MAX_INFO];
   VertexType va,vb;
   printf("请输入有向网G的顶点数,弧数,弧是否含其它信息(是:1,否:0): ");
   scanf("%d,%d,%d",&(*G).vexnum,&(*G).arcnum,&IncInfo);
   printf("请输入%d个顶点的值(<%d个字符):\n",(*G).vexnum,MAX_NAME);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     scanf("%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=INFINITY; /* 网 */
       (*G).arcs[i][j].info=NULL;
     }
   printf("请输入%d条弧的弧尾 弧头 权值(以空格作为间隔): \n",(*G).arcnum);
   for(k=0;k<(*G).arcnum;++k)
   {
     scanf("%s%s%d%*c",va,vb,&w);  /* %*c吃掉回车符 */
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=w; /* 有向网 */
     if(IncInfo)
     {
       printf("请输入该弧的相关信息(<%d个字符): ",MAX_INFO);
       gets(s);
       w=strlen(s);
       if(w)
       {
         (*G).arcs[i][j].info=(char*)malloc((w+1)*sizeof(char)); /* 有向 */
         strcpy((*G).arcs[i][j].info,s);
       }
     }
   }
   (*G).kind=DN;
 }

 void CreateUDG(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，构造无向图G */
   int i,j,k,l,IncInfo;
   char s[MAX_INFO];
   VertexType va,vb;
   printf("请输入无向图G的顶点数,边数,边是否含其它信息(是:1,否:0): ");
   scanf("%d,%d,%d",&(*G).vexnum,&(*G).arcnum,&IncInfo);
   printf("请输入%d个顶点的值(<%d个字符):\n",(*G).vexnum,MAX_NAME);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     scanf("%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=0; /* 图 */
       (*G).arcs[i][j].info=NULL;
     }
   printf("请输入%d条边的顶点1 顶点2(以空格作为间隔): \n",(*G).arcnum);
   for(k=0;k<(*G).arcnum;++k)
   {
     scanf("%s%s%*c",va,vb); /* %*c吃掉回车符 */
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=(*G).arcs[j][i].adj=1; /* 无向图 */
     if(IncInfo)
     {
       printf("请输入该边的相关信息(<%d个字符): ",MAX_INFO);
       gets(s);
       l=strlen(s);
       if(l)
       {
         (*G).arcs[i][j].info=(*G).arcs[j][i].info=(char*)malloc((l+1)*sizeof(char));
	 /* 无向，两个指针指向同一个信息 */
         strcpy((*G).arcs[i][j].info,s);
       }
     }
   }
   (*G).kind=UDG;
 }

 void CreateUDN(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，构造无向网G。算法7.2 */
   int i,j,k,w,IncInfo;
   char s[MAX_INFO];
   VertexType va,vb;
   printf("请输入无向网G的顶点数,边数,边是否含其它信息(是:1,否:0): ");
   scanf("%d,%d,%d",&(*G).vexnum,&(*G).arcnum,&IncInfo);
   printf("请输入%d个顶点的值(<%d个字符):\n",(*G).vexnum,MAX_NAME);
   for(i=0;i<(*G).vexnum;++i) /* 构造顶点向量 */
     scanf("%s",(*G).vexs[i]);
   for(i=0;i<(*G).vexnum;++i) /* 初始化邻接矩阵 */
     for(j=0;j<(*G).vexnum;++j)
     {
       (*G).arcs[i][j].adj=INFINITY; /* 网 */
       (*G).arcs[i][j].info=NULL;
     }
   printf("请输入%d条边的顶点1 顶点2 权值(以空格作为间隔): \n",(*G).arcnum);
   for(k=0;k<(*G).arcnum;++k)
   {
     scanf("%s%s%d%*c",va,vb,&w); /* %*c吃掉回车符 */
     i=LocateVex(*G,va);
     j=LocateVex(*G,vb);
     (*G).arcs[i][j].adj=(*G).arcs[j][i].adj=w; /* 无向 */
     if(IncInfo)
     {
       printf("请输入该边的相关信息(<%d个字符): ",MAX_INFO);
       gets(s);
       w=strlen(s);
       if(w)
       {
         (*G).arcs[i][j].info=(*G).arcs[j][i].info=(char*)malloc((w+1)*sizeof(char));
         /* 无向，两个指针指向同一个信息 */
         strcpy((*G).arcs[i][j].info,s);
       }
     }
   }
   (*G).kind=UDN;
 }

 void CreateGraph(MGraph *G)
 { /* 采用数组(邻接矩阵)表示法，构造图G。算法7.1改 */
   printf("请输入图G的类型(有向图:0,有向网:1,无向图:2,无向网:3): ");
   scanf("%d",&(*G).kind);
   switch((*G).kind)
   {
     case DG: CreateDG(G); /* 构造有向图 */
              break;
     case DN: CreateDN(G); /* 构造有向网 */
              break;
     case UDG:CreateUDG(G); /* 构造无向图 */
              break;
     case UDN:CreateUDN(G); /* 构造无向网 */
   }
 }

 void DestroyGraph(MGraph *G)
 { /* 初始条件：图G存在。操作结果：销毁图G */
   int i,j,k=0;
   if((*G).kind%2) /* 网 */
     k=INFINITY; /* k为两顶点之间无边或弧时邻接矩阵元素的值 */
   for(i=0;i<(*G).vexnum;i++) /* 释放弧或边的相关信息(如果有的话) */
     if((*G).kind<2) /* 有向 */
     {
       for(j=0;j<(*G).vexnum;j++)
         if((*G).arcs[i][j].adj!=k) /* 有弧 */
           if((*G).arcs[i][j].info) /* 有相关信息 */
           {
             free((*G).arcs[i][j].info);
             (*G).arcs[i][j].info=NULL;
           }
     } /* 加括号为避免if-else对配错 */
     else /* 无向 */
       for(j=i+1;j<(*G).vexnum;j++) /* 只查上三角 */
         if((*G).arcs[i][j].adj!=k) /* 有边 */
           if((*G).arcs[i][j].info) /* 有相关信息 */
           {
             free((*G).arcs[i][j].info);
             (*G).arcs[i][j].info=(*G).arcs[j][i].info=NULL;
           }
   (*G).vexnum=0; /* 顶点数为0 */
   (*G).arcnum=0; /* 边数为0 */
 }

 VertexType* GetVex(MGraph G,int v)
 { /* 初始条件：图G存在，v是G中某个顶点的序号。操作结果：返回v的值 */
   if(v>=G.vexnum||v<0)
     exit(ERROR);
   return &G.vexs[v];
 }

 Status PutVex(MGraph *G,VertexType v,VertexType value)
 { /* 初始条件：图G存在，v是G中某个顶点。操作结果：对v赋新值value */
   int k;
   k=LocateVex(*G,v); /* k为顶点v在图G中的序号 */
   if(k<0)
     return ERROR;
   strcpy((*G).vexs[k],value);
   return OK;
 }

 int FirstAdjVex(MGraph G,VertexType v)
 { /* 初始条件：图G存在，v是G中某个顶点 */
   /* 操作结果：返回v的第一个邻接顶点的序号。若顶点在G中没有邻接顶点，则返回-1 */
   int i,j=0,k;
   k=LocateVex(G,v); /* k为顶点v在图G中的序号 */
   if(G.kind%2) /* 网 */
     j=INFINITY;
   for(i=0;i<G.vexnum;i++)
     if(G.arcs[k][i].adj!=j)
       return i;
   return -1;
 }

 int NextAdjVex(MGraph G,VertexType v,VertexType w)
 { /* 初始条件：图G存在，v是G中某个顶点，w是v的邻接顶点 */
   /* 操作结果：返回v的(相对于w的)下一个邻接顶点的序号，若w是v的最后一个邻接顶点，则返回-1 */
   int i,j=0,k1,k2;
   k1=LocateVex(G,v); /* k1为顶点v在图G中的序号 */
   k2=LocateVex(G,w); /* k2为顶点w在图G中的序号 */
   if(G.kind%2) /* 网 */
     j=INFINITY;
   for(i=k2+1;i<G.vexnum;i++)
     if(G.arcs[k1][i].adj!=j)
       return i;
   return -1;
 }

 void InsertVex(MGraph *G,VertexType v)
 { /* 初始条件：图G存在，v和图G中顶点有相同特征 */
   /* 操作结果：在图G中增添新顶点v(不增添与顶点相关的弧，留待InsertArc()去做) */
   int i,j=0;
   if((*G).kind%2) /* 网 */
     j=INFINITY;
   strcpy((*G).vexs[(*G).vexnum],v); /* 构造新顶点向量 */
   for(i=0;i<=(*G).vexnum;i++)
   {
     (*G).arcs[(*G).vexnum][i].adj=(*G).arcs[i][(*G).vexnum].adj=j;
     /* 初始化新增行、新增列邻接矩阵的值(无边或弧) */
     (*G).arcs[(*G).vexnum][i].info=(*G).arcs[i][(*G).vexnum].info=NULL; /* 初始化相关信息指针 */
   }
   (*G).vexnum++; /* 图G的顶点数加1 */
 }

 Status DeleteVex(MGraph *G,VertexType v)
 { /* 初始条件：图G存在，v是G中某个顶点。操作结果：删除G中顶点v及其相关的弧 */
   int i,j,k;
   VRType m=0;
   if((*G).kind%2) /* 网 */
     m=INFINITY;
   k=LocateVex(*G,v); /* k为待删除顶点v的序号 */
   for(j=0;j<(*G).vexnum;j++)
     if((*G).arcs[j][k].adj!=m) /* 有入弧或边 */
     {
       if((*G).arcs[j][k].info) /* 有相关信息 */
         free((*G).arcs[j][k].info); /* 释放相关信息 */
       (*G).arcnum--; /* 修改弧数 */
     }
   if((*G).kind<2) /* 有向 */
     for(j=0;j<(*G).vexnum;j++)
       if((*G).arcs[k][j].adj!=m) /* 有出弧 */
       {
         if((*G).arcs[k][j].info) /* 有相关信息 */
           free((*G).arcs[k][j].info); /* 释放相关信息 */
         (*G).arcnum--; /* 修改弧数 */
       }
   for(j=k+1;j<(*G).vexnum;j++) /* 序号k后面的顶点向量依次前移 */
     strcpy((*G).vexs[j-1],(*G).vexs[j]);
   for(i=0;i<(*G).vexnum;i++)
     for(j=k+1;j<(*G).vexnum;j++)
       (*G).arcs[i][j-1]=(*G).arcs[i][j]; /* 移动待删除顶点之后的矩阵元素 */
   for(i=0;i<(*G).vexnum;i++)
     for(j=k+1;j<(*G).vexnum;j++)
       (*G).arcs[j-1][i]=(*G).arcs[j][i]; /* 移动待删除顶点之下的矩阵元素 */
   (*G).vexnum--; /* 更新图的顶点数 */
   return OK;
 }

 Status InsertArc(MGraph *G,VertexType v,VertexType w)
 { /* 初始条件：图G存在，v和w是G中两个顶点 */
   /* 操作结果：在G中增添弧<v,w>，若G是无向的，则还增添对称弧<w,v> */
   int i,l,v1,w1;
   char s[MAX_INFO];
   v1=LocateVex(*G,v); /* 尾 */
   w1=LocateVex(*G,w); /* 头 */
   if(v1<0||w1<0)
     return ERROR;
   (*G).arcnum++; /* 弧或边数加1 */
   if((*G).kind%2) /* 网 */
   {
     printf("请输入此弧或边的权值: ");
     scanf("%d",&(*G).arcs[v1][w1].adj);
   }
   else /* 图 */
     (*G).arcs[v1][w1].adj=1;
   printf("是否有该弧或边的相关信息(0:无 1:有): ");
   scanf("%d%*c",&i);
   if(i)
   {
     printf("请输入该弧或边的相关信息(<%d个字符)：",MAX_INFO);
     gets(s);
     l=strlen(s);
     if(l)
     {
       (*G).arcs[v1][w1].info=(char*)malloc((l+1)*sizeof(char));
       strcpy((*G).arcs[v1][w1].info,s);
     }
   }
   if((*G).kind>1) /* 无向 */
   {
     (*G).arcs[w1][v1].adj=(*G).arcs[v1][w1].adj;
     (*G).arcs[w1][v1].info=(*G).arcs[v1][w1].info; /* 指向同一个相关信息 */
   }
   return OK;
 }

 Status DeleteArc(MGraph *G,VertexType v,VertexType w)
 { /* 初始条件：图G存在，v和w是G中两个顶点 */
   /* 操作结果：在G中删除弧<v,w>，若G是无向的，则还删除对称弧<w,v> */
   int v1,w1,j=0;
   if((*G).kind%2) /* 网 */
     j=INFINITY;
   v1=LocateVex(*G,v); /* 尾 */
   w1=LocateVex(*G,w); /* 头 */
   if(v1<0||w1<0) /* v1、w1的值不合法 */
     return ERROR;
   (*G).arcs[v1][w1].adj=j;
   if((*G).arcs[v1][w1].info) /* 有其它信息 */
   {
     free((*G).arcs[v1][w1].info);
     (*G).arcs[v1][w1].info=NULL;
   }
   if((*G).kind>=2) /* 无向，删除对称弧<w,v> */
   {
     (*G).arcs[w1][v1].adj=j;
     (*G).arcs[w1][v1].info=NULL;
   }
   (*G).arcnum--;
   return OK;
 }

 void DFS(MGraph G,int v)
 { /* 从第v个顶点出发递归地深度优先遍历图G。算法7.5 */
   int w;
   visited[v]=TRUE; /* 设置访问标志为TRUE(已访问) */
   VisitFunc(G.vexs[v]); /* 访问第v个顶点 */
   for(w=FirstAdjVex(G,G.vexs[v]);w>=0;w=NextAdjVex(G,G.vexs[v],G.vexs[w]))
     if(!visited[w])
       DFS(G,w); /* 对v的尚未访问的序号为w的邻接顶点递归调用DFS */
 }

 void DFSTraverse(MGraph G,void(*Visit)(VertexType))
 { /* 初始条件：图G存在，Visit是顶点的应用函数。算法7.4 */
   /* 操作结果：从第1个顶点起，深度优先遍历图G，并对每个顶点调用函数Visit一次且仅一次 */
   int v;
   VisitFunc=Visit; /* 使用全局变量VisitFunc，使DFS不必设函数指针参数 */
   for(v=0;v<G.vexnum;v++)
     visited[v]=FALSE; /* 访问标志数组初始化(未被访问) */
   for(v=0;v<G.vexnum;v++)
     if(!visited[v])
       DFS(G,v); /* 对尚未访问的顶点v调用DFS */
   printf("\n");
 }


  void InitQueue(LinkQueue *Q)
 { /* 构造一个空队列Q */
   (*Q).front=(*Q).rear=(QueuePtr)malloc(sizeof(QNode));
   if(!(*Q).front)
     exit(OVERFLOW);
   (*Q).front->next=NULL;
 }

 void DestroyQueue(LinkQueue *Q)
 { /* 销毁队列Q(无论空否均可) */
   while((*Q).front)
   {
     (*Q).rear=(*Q).front->next;
     free((*Q).front);
     (*Q).front=(*Q).rear;
   }
 }

 void ClearQueue(LinkQueue *Q)
 { /* 将Q清为空队列 */
   QueuePtr p,q;
   (*Q).rear=(*Q).front;
   p=(*Q).front->next;
   (*Q).front->next=NULL;
   while(p)
   {
     q=p;
     p=p->next;
     free(q);
   }
 }

 Status QueueEmpty(LinkQueue Q)
 { /* 若Q为空队列，则返回TRUE，否则返回FALSE */
   if(Q.front->next==NULL)
     return TRUE;
   else
     return FALSE;
 }

 int QueueLength(LinkQueue Q)
 { /* 求队列的长度 */
   int i=0;
   QueuePtr p;
   p=Q.front;
   while(Q.rear!=p)
   {
     i++;
     p=p->next;
   }
   return i;
 }

 Status GetHead_Q(LinkQueue Q,QElemType *e) /* 避免与bo2-6.c重名 */
 { /* 若队列不空，则用e返回Q的队头元素，并返回OK，否则返回ERROR */
   QueuePtr p;
   if(Q.front==Q.rear)
     return ERROR;
   p=Q.front->next;
   *e=p->data;
   return OK;
 }

 void EnQueue(LinkQueue *Q,QElemType e)
 { /* 插入元素e为Q的新的队尾元素 */
   QueuePtr p=(QueuePtr)malloc(sizeof(QNode));
   if(!p) /* 存储分配失败 */
     exit(OVERFLOW);
   p->data=e;
   p->next=NULL;
   (*Q).rear->next=p;
   (*Q).rear=p;
 }

 Status DeQueue(LinkQueue *Q,QElemType *e)
 { /* 若队列不空，删除Q的队头元素，用e返回其值，并返回OK，否则返回ERROR */
   QueuePtr p;
   if((*Q).front==(*Q).rear)
     return ERROR;
   p=(*Q).front->next;
   *e=p->data;
   (*Q).front->next=p->next;
   if((*Q).rear==p)
     (*Q).rear=(*Q).front;
   free(p);
   return OK;
 }

 void QueueTraverse(LinkQueue Q,void(*vi)(QElemType))
 { /* 从队头到队尾依次对队列Q中每个元素调用函数vi() */
   QueuePtr p;
   p=Q.front->next;
   while(p)
   {
     vi(p->data);
     p=p->next;
   }
   printf("\n");
 }
 
 void BFSTraverse(MGraph G,void(*Visit)(VertexType))
 { /* 初始条件：图G存在，Visit是顶点的应用函数。算法7.6 */
   /* 操作结果：从第1个顶点起，按广度优先非递归遍历图G,并对每个顶点调用函数Visit一次且仅一次 */
   int v,u,w;
   LinkQueue Q; /* 使用辅助队列Q和访问标志数组visited */
   for(v=0;v<G.vexnum;v++)
     visited[v]=FALSE; /* 置初值 */
   InitQueue(&Q); /* 置空的辅助队列Q */
   for(v=0;v<G.vexnum;v++)
     if(!visited[v]) /* v尚未访问 */
     {
       visited[v]=TRUE; /* 设置访问标志为TRUE(已访问) */
       Visit(G.vexs[v]);
       EnQueue(&Q,v); /* v入队列 */
       while(!QueueEmpty(Q)) /* 队列不空 */
       {
         DeQueue(&Q,&u); /* 队头元素出队并置为u */
         for(w=FirstAdjVex(G,G.vexs[u]);w>=0;w=NextAdjVex(G,G.vexs[u],G.vexs[w]))
           if(!visited[w]) /* w为u的尚未访问的邻接顶点的序号 */
           {
             visited[w]=TRUE;
             Visit(G.vexs[w]);
             EnQueue(&Q,w);
           }
       }
     }
   printf("\n");
 }

 void Display(MGraph G)
 { /* 输出邻接矩阵存储表示的图G */
   int i,j;
   char s[7];
   switch(G.kind)
   {
     case DG: strcpy(s,"有向图");
              break;
     case DN: strcpy(s,"有向网");
              break;
     case UDG:strcpy(s,"无向图");
              break;
     case UDN:strcpy(s,"无向网");
   }
   printf("%d个顶点%d条边或弧的%s。顶点依次是: ",G.vexnum,G.arcnum,s);
   for(i=0;i<G.vexnum;++i) /* 输出G.vexs */
     printf("%s ",G.vexs[i]);
   printf("\nG.arcs.adj:\n"); /* 输出G.arcs.adj */
   for(i=0;i<G.vexnum;i++)
   {
     for(j=0;j<G.vexnum;j++)
       printf("%11d",G.arcs[i][j].adj);
     printf("\n");
   }
   printf("G.arcs.info:\n"); /* 输出G.arcs.info */
   printf("顶点1(弧尾) 顶点2(弧头) 该边或弧的信息：\n");
   for(i=0;i<G.vexnum;i++)
     if(G.kind<2) /* 有向 */
     {
       for(j=0;j<G.vexnum;j++)
         if(G.arcs[i][j].info)
           printf("%5s %11s     %s\n",G.vexs[i],G.vexs[j],G.arcs[i][j].info);
     } /* 加括号为避免if-else对配错 */
     else /* 无向,输出上三角 */
       for(j=i+1;j<G.vexnum;j++)
         if(G.arcs[i][j].info)
           printf("%5s %11s     %s\n",G.vexs[i],G.vexs[j],G.arcs[i][j].info);
 }
 
  void ShortestPath_DIJ(MGraph G,int v0,PathMatrix P,ShortPathTable D)
 { /* 用Dijkstra算法求有向网G的v0顶点到其余顶点v的最短路径P[v]及带权长度 */
   /* D[v]。若P[v][w]为TRUE，则w是从v0到v当前求得最短路径上的顶点。 */
   /* final[v]为TRUE当且仅当v∈S，即已经求得从v0到v的最短路径 算法7.15 */
   int v,w,i,j,min;
   Status final[MAX_VERTEX_NUM]; /* 辅助矩阵，为真表示该顶点到v0的最短距离已求出，初值为假 */
   for(v=0;v<G.vexnum;++v)
   {
     final[v]=FALSE; /* 设初值 */
     D[v]=G.arcs[v0][v].adj; /* D[]存放v0到v的最短距离，初值为v0到v的直接距离 */
     for(w=0;w<G.vexnum;++w)
       P[v][w]=FALSE; /* 设P[][]初值为FALSE，没有路径 */
     if(D[v]<INFINITY) /* v0到v有直接路径 */
       P[v][v0]=P[v][v]=TRUE; /* 一维数组p[v][]表示源点v0到v最短路径通过的顶点 */
   }
   D[v0]=0; /* v0到v0距离为0 */
   final[v0]=TRUE; /* v0顶点并入S集 */
   for(i=1;i<G.vexnum;++i) /* 其余G.vexnum-1个顶点 */
   { /* 开始主循环，每次求得v0到某个顶点v的最短路径，并将v并入S集 */
     min=INFINITY; /* 当前所知离v0顶点的最近距离，设初值为∞ */
     for(w=0;w<G.vexnum;++w) /* 对所有顶点检查 */
       if(!final[w]&&D[w]<min) /*在S集之外的顶点中找离v0最近的顶点，并将其赋给v，距离赋给min */
       {
	 v=w;
	 min=D[w];
       }
     final[v]=TRUE; /* 将v并入S集 */
     for(w=0;w<G.vexnum;++w) /* 根据新并入的顶点，更新不在S集的顶点到v0的距离和路径数组 */
       if(!final[w]&&min<INFINITY&&G.arcs[v][w].adj<INFINITY&&(min+G.arcs[v][w].adj<D[w]))
       { /* w不属于S集且v0→v→w的距离＜目前v0→w的距离 */
	 D[w]=min+G.arcs[v][w].adj; /* 更新D[w] */
	 for(j=0;j<G.vexnum;++j) /* 修改P[w]，v0到w经过的顶点包括v0到v经过的顶点再加上顶点w */
	   P[w][j]=P[v][j];
	 P[w][w]=TRUE;
       }
   }
 }

 int main()
 {
   int i, j;
   MGraph g;
   PathMatrix p; /* 二维数组，路径矩阵 */
   ShortPathTable d; /* 一维数组，最短距离表 */
   CreateDN(&g); /* 构造有向网g */
   Display(g); /* 输出有向网g */
   ShortestPath_DIJ(g,0,p,d);/*以g中位置为0的顶点为源点，球其到其余各顶点的最短距离。存于d中 */
   printf("最短路径数组p[i][j]如下:\n");
   for(i=0;i<g.vexnum;++i)
   {
     for(j=0;j<g.vexnum;++j)
       printf("%2d",p[i][j]);
     printf("\n");
   }
   printf("%s到各顶点的最短路径长度为：\n",g.vexs[0]);
   for(i=0;i<g.vexnum;++i)
     if(i!=0)
       printf("%s-%s:%d\n",g.vexs[0],g.vexs[i],d[i]);
 
   return 0;
 }

 
 
 
