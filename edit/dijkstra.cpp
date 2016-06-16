

#define MAXVEC 9

#define INFINITY 65535

typedef int Pathmatrix[MAXVEC];
typedef int ShortPath[MAXVEC];


void Dijkstra(MGraph G, int v0, Pathmatrix *p, ShortPath *D)
{
	int v, w, k, min;
	int final[MAXVEC];//标记是否已经是确定值
	for(v=0; v<G.vecnum; ++v)
	{
		final[v] = 0;			//全部标记为估计值
		D[v] = G.matrix[v0][v];		//拷贝图中的一行
		p[v] = 0;			//路径标记为0
	}


	D[v0] = 0;				//从v0出发到v0的路径长度为0

	final[v0] = 1;				//标记为确定值
	
	for(v=0; v<G.vecnum; ++v)
	{
		min = INFINITY;
		for(w=0; w<G.vecnum; ++w)
		{
			if(!final[w] && D[w] < min)
			{
				k = w;
				min = D[w];
			}
		}

		final[k] = 1;			//标记从v到k的最短路径为确定值

		for(w=0; w<G.vecnum; ++w)
		{
			if(!final[w] && min + G.matrix[k][w] < D[w])
			{
				D[w] = min + G.matrix[k][w];
				p[w] = k;
			}
		}		
	}
}






