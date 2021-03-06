#include<cstdio>
#include<algorithm>
#include<cstring>
#include<cstdio>
#include<random>
#include<map>
#include<ctime>
#include<cassert>
#include<vector>
using namespace std;

/******************************************************************************
 * Constants to adjust
 *****************************************************************************/

const unsigned int RandomSeed = 987654321;
const int ConstructVC_Max_Tries = 200;
const int Local_Search_Iterations = 1000000;
const int RemoveBMS_Max_Tries = 100;

/******************************************************************************
 * End constants to adjust
 *****************************************************************************/
 
default_random_engine Gn(RandomSeed);

int readnum(){
	int res=0;
	char c=getchar();
	for(;!isdigit(c);c=getchar());
	for(;isdigit(c);c=getchar())res=res*10+c-'0';
	return res;
}

unsigned read_u() {
	unsigned res = 0;
	char c = getchar();
	while (!isdigit(c) && c > 0) c = getchar();
	if (c < 0) return res;
	for (; isdigit(c); c = getchar()) res = res * 10 + c - '0';
	return res;
}

typedef long long ll;

const int maxn=1010;
const int maxm=maxn*maxn;

//----------------------------------------------------------------

#define pop(stack) (stack[--stack##_p])
#define push(x, stack) (stack[stack##_p++]=(x))

class MaxCliqueFinder{
private:
	
	int on;
	bool oe[maxn][maxn];
	map<int,int>mp;
	int mp_num[maxn];
	
	int n,m;
	
	struct bEdge{
		int u,v;
	}be[maxm];
	int edge_weight[maxm];
	int deg[maxn];
	int edge_ind[maxn][maxn];
	int adj[maxn][maxn];
	
	ll step;
	ll timestamp[maxn];
	
	int dscore[maxn];
	
	int nc;
	bool in_cover[maxn];
	int nr;
	int remove_cand[maxn];
	int remove_cand_ind[maxn];
	
	int best_nc;
	bool best_in_cover[maxn];
	
	int uncover_stack[maxm];
	int uncover_stack_p;
	int uncover_stack_ind[maxm];
	
	bool conf_change[maxn];
	bool tabu[maxn];
	
	int tmp_nc;
	bool tmp_in_cover[maxn];
	int edge_ind_seq[maxm];
	
	
	void ResetRemoveCand();
	inline void Uncover(int e);
	inline void Cover(int e);
	void Add(int p);
	void Remove(int p);
	int ChooseRemove_MinLoss();
	int ChooseRemove_BMS(int times);
	int ChooseAddV(int remove_v1,int remove_v2);
	inline void UpdateBestSolution();
	void RemoveRedundant();
	void ConstructVC(int tries);
	void UpdateEdgeWeight();
	void LocalSearch(int iters);
	
	void InitComplimentGraph();
	void FindMinVertexCover();
	
public:

	void Init();
	void AddVertex(int u);
	void AddEdge(int u,int v);
	vector<int>FindMaxClique();
	
};

void MaxCliqueFinder::Init(){
	on=0;
	memset(oe,0,sizeof(oe));
	mp.clear();
	memset(mp_num,0,sizeof(mp_num));
	n=m=0;
	memset(deg,0,sizeof(deg));
	step=0;
	memset(timestamp,0,sizeof(timestamp));
	memset(dscore,0,sizeof(dscore));
	nc=0;
	memset(in_cover,0,sizeof(in_cover));
	nr=0;
	memset(remove_cand,0,sizeof(remove_cand));
	memset(remove_cand_ind,0,sizeof(remove_cand_ind));
	best_nc=0;
	memset(best_in_cover,0,sizeof(best_in_cover));
	uncover_stack_p=0;
	memset(conf_change,0,sizeof(conf_change));
	memset(tabu,0,sizeof(tabu));
	tmp_nc=0;
	memset(tmp_in_cover,0,sizeof(tmp_in_cover));
}
void MaxCliqueFinder::ResetRemoveCand(){
	int i;
	nr=0;
	for(i=1;i<=n;++i){
		if(in_cover[i]){
			remove_cand[++nr]=i;
			remove_cand_ind[i]=nr;
		}else{
			remove_cand_ind[i]=0;
		}
	}
}
inline void MaxCliqueFinder::Uncover(int e){
	uncover_stack_ind[e]=uncover_stack_p;
	push(e,uncover_stack);
}
inline void MaxCliqueFinder::Cover(int e){
	int st_ind,last_uncover_edge;
	last_uncover_edge=pop(uncover_stack);
	st_ind=uncover_stack_ind[e];
	uncover_stack[st_ind]=last_uncover_edge;
	uncover_stack_ind[last_uncover_edge]=st_ind;
}
void MaxCliqueFinder::Add(int p){
	int i,e,v,deg_p=deg[p];
	
	in_cover[p]=true;
	++nc;
	dscore[p]=-dscore[p];
	
	remove_cand[++nr]=p;
	remove_cand_ind[p]=nr;
	
	for(i=1;i<=deg_p;++i){
		e=edge_ind[p][i];
		v=adj[p][i];
		
		if(in_cover[v]){
			dscore[v]+=edge_weight[e];
		}else{
			dscore[v]-=edge_weight[e];
			conf_change[v]=true;
			Cover(e);
		}
	}
}
void MaxCliqueFinder::Remove(int p){
	if(p==-1)return;
	
	int i,e,v,deg_p=deg[p];
	
	in_cover[p]=false;
	--nc;
	dscore[p]=-dscore[p];
	conf_change[p]=false;
	
	int cand_ind,last_remove_cand;
	last_remove_cand=remove_cand[nr--];
	cand_ind=remove_cand_ind[p];
	remove_cand[cand_ind]=last_remove_cand;
	remove_cand_ind[last_remove_cand]=cand_ind;
	remove_cand_ind[p]=0;
	
	for(i=1;i<=deg_p;++i){
		e=edge_ind[p][i];
		v=adj[p][i];
		
		if(in_cover[v]){
			dscore[v]-=edge_weight[e];
		}else{
			dscore[v]+=edge_weight[e];
			conf_change[v]=true;
			Uncover(e);
		}
	}
}
int MaxCliqueFinder::ChooseRemove_MinLoss(){
	if(!nr)return -1;
	
	int i,v,v_loss;
	int min_v,min_loss;
	min_v=remove_cand[1];
	min_loss=abs(dscore[min_v]);
	
	if(min_loss){
		for(i=2;i<=nr;++i){
			v=remove_cand[i];
			v_loss=abs(dscore[v]);
			if(v_loss<min_loss||
				v_loss==min_loss&&timestamp[v]<timestamp[min_v]){
				min_v=v;
				min_loss=v_loss;
			}
			if(!min_loss)break;
		}
	}
	
	return min_v;
}
int MaxCliqueFinder::ChooseRemove_BMS(int times){
	if(!nr)return -1;

	uniform_int_distribution<> rand(1, nr);
	
	int i,v,v_loss;
	int remove_v,remove_loss;
	remove_v=remove_cand[rand(Gn)];
	remove_loss=abs(dscore[remove_v]);
	
	for(;times--;){
		v=remove_cand[rand(Gn)];
		if(tabu[v])continue;
		v_loss=abs(dscore[v]);
		if(v_loss<remove_loss||
			v_loss==remove_loss&&timestamp[v]<timestamp[remove_v]){
			remove_v=v;
			remove_loss=v_loss;
		}
	}
	
	/* tabu[remove_v] is possibly true */
	return remove_v;
}
int MaxCliqueFinder::ChooseAddV(int remove_v1,int remove_v2){
	int i,v,v_gain;
	int max_v=-1,max_gain=0;
	int p_deg,*p_adj;
	p_deg=deg[remove_v1];
	p_adj=adj[remove_v1];
	for(i=1;i<=p_deg;++i){
		v=p_adj[i];
		if(in_cover[v])continue;
		if(!conf_change[v])continue;
		v_gain=dscore[v];
		if(v_gain>max_gain||
			v_gain==max_gain&&timestamp[v]<timestamp[max_v]){
			max_v=v;
			max_gain=v_gain;
		}
	}
	v=remove_v1;
	if(conf_change[v]&&!in_cover[v]){
		v_gain=dscore[v];
		if(v_gain>max_gain||
			v_gain==max_gain&&timestamp[v]<timestamp[max_v]){
			max_v=v;
			max_gain=v_gain;
		}
	}
	if(remove_v2==-1)return max_v;
	p_deg=deg[remove_v2];
	p_adj=adj[remove_v2];
	for(i=1;i<=p_deg;++i){
		v=p_adj[i];
		if(in_cover[v])continue;
		if(!conf_change[v])continue;
		v_gain=dscore[v];
		if(v_gain>max_gain||
			v_gain==max_gain&&timestamp[v]<timestamp[max_v]){
			max_v=v;
			max_gain=v_gain;
		}
	}
	v=remove_v2;
	if(conf_change[v]&&!in_cover[v]){
		v_gain=dscore[v];
		if(v_gain>max_gain||
			v_gain==max_gain&&timestamp[v]<timestamp[max_v]){
			max_v=v;
			max_gain=v_gain;
		}
	}
	return max_v;
}
inline void MaxCliqueFinder::UpdateBestSolution(){
	if(nc<best_nc){
		best_nc=nc;
		memcpy(best_in_cover,in_cover,sizeof(bool)*(n+1));
	}
}
void MaxCliqueFinder::RemoveRedundant(){
	int i,v;
	for(i=1;i<=nr;++i){
		v=remove_cand[i];
		if(in_cover[v]&&!dscore[v]){
			Remove(v);
			--i;
		}
	}
}
void MaxCliqueFinder::ConstructVC(int tries){
	int i;
	int u,v;
	
	uncover_stack_p=0;
	nc=0;
	
	for(i=1;i<=m;++i){
		edge_ind_seq[i]=i;
		u=be[i].u,v=be[i].v;
		if(in_cover[u]||in_cover[v])continue;
		in_cover[deg[u]>deg[v]?u:v]=true;
		++nc;
	}
	
	for(;tries--;){
		memset(tmp_in_cover,0,sizeof(bool)*(n+1));
		tmp_nc=0;
		shuffle(edge_ind_seq+1,edge_ind_seq+m+1,Gn);
		for(i=1;i<=m;++i){
			u=be[edge_ind_seq[i]].u,v=be[edge_ind_seq[i]].v;
			if(tmp_in_cover[u]||tmp_in_cover[v])continue;
			tmp_in_cover[deg[u]>deg[v]?u:v]=true;
			++tmp_nc;
		}
		if(tmp_nc<nc){
			nc=tmp_nc;
			memcpy(in_cover,tmp_in_cover,sizeof(bool)*(n+1));
		}
	}
	
	for(i=1;i<=m;++i){
		u=be[i].u,v=be[i].v;
		if(in_cover[u]&&in_cover[v])continue;
		dscore[in_cover[u]?u:v]-=edge_weight[i];
	}
	
	ResetRemoveCand();
	for(i=1;i<=n;++i){
		if(in_cover[i]&&!dscore[i])Remove(i);
	}
	
	best_nc=nc;
	memcpy(best_in_cover,in_cover,sizeof(bool)*(n+1));
}
void MaxCliqueFinder::UpdateEdgeWeight(){
	int i,e;
	for(i=0;i<uncover_stack_p;++i){
		e=uncover_stack[i];
		++edge_weight[e];
		++dscore[be[e].u],++dscore[be[e].v];
		conf_change[be[e].u]=conf_change[be[e].v]=true;
	}
	
}
void MaxCliqueFinder::LocalSearch(int iters){
	int add_v,remove_v1,remove_v2;
	
	step=0;
	
	for(;iters--;){
		++step;
		
		remove_v1=ChooseRemove_MinLoss();
		Remove(remove_v1);
		/* do not update the timestamp of remove_v1 */
		remove_v2=ChooseRemove_BMS(RemoveBMS_Max_Tries);
		Remove(remove_v2);
		if(remove_v2!=-1)timestamp[remove_v2]=step;
		
		memset(tabu,0,sizeof(bool)*(n+1));
		while(uncover_stack_p){
			add_v=ChooseAddV(remove_v1,remove_v2);
			Add(add_v);
			UpdateEdgeWeight();
			tabu[add_v]=true;
			timestamp[add_v]=step;
		}
		RemoveRedundant();
		
		UpdateBestSolution();
	}
}

void MaxCliqueFinder::InitComplimentGraph(){
	int i,u,v;
	
	n=on,m=0;
	
	for(u=1;u<=n;++u){
		for(v=u+1;v<=n;++v){
			if(oe[u][v])continue;
			++m;
			be[m]=(bEdge){u,v};
			++deg[u],++deg[v];
			adj[u][deg[u]]=v,adj[v][deg[v]]=u;
			edge_ind[u][deg[u]]=edge_ind[v][deg[v]]=m;
		}
	}
	
	memset(conf_change,1,sizeof(conf_change));
	fill_n(edge_weight+1,m,1);
}
void MaxCliqueFinder::FindMinVertexCover(){
	if(!m)return;
	ConstructVC(ConstructVC_Max_Tries);
	//LocalSearch(Local_Search_Iterations);
	//LocalSearch(n*4000);
	LocalSearch(n*n*10);
}
void MaxCliqueFinder::AddVertex(int u){
	if(!mp.count(u))mp[u]=++on,mp_num[on]=u;
}
void MaxCliqueFinder::AddEdge(int u,int v){
	u=mp[u],v=mp[v];
	oe[u][v]=oe[v][u]=true;
}
vector<int> MaxCliqueFinder::FindMaxClique(){
	int i;
	vector<int>max_clique;
	
	InitComplimentGraph();
	FindMinVertexCover();
	
	for(i=1;i<=n;++i){
		if(!best_in_cover[i])max_clique.push_back(i);
	}
	
	return max_clique;
}

//----------------------------------------------------------------

int n,m;

MaxCliqueFinder finder;
vector<int>max_clique;

void Initialize(){
	finder.Init();
	int i,u,v;
	for(i=1;i<=n;++i)finder.AddVertex(i);
	for(i=1;i<=m;++i){
		u=readnum(),v=readnum();
		finder.AddEdge(u,v);
	}
}
void Work(){
	max_clique=finder.FindMaxClique();
}
void PrintSolution(){
	printf("%d\n",max_clique.size());
	for(int i=0;i<max_clique.size();++i){
		printf("%d%c",max_clique[i]," \n"[i==max_clique.size()-1]);
	}
}

int main(){
	while((n = read_u()) && (m = read_u())){
		Initialize();
		Work();
		PrintSolution();
	}
	return 0;
}
