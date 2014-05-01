#ifndef _BLIB_GRAPH_SPARSE_DEF_
#define _BLIB_GRAPH_SPARSE_DEF_
#include "blib_error.h"

#define BLIB_GRAPH_SPARSE_MIN_ALLOC 5
#define BLIB_GRAPH_SPARSE_GROWTH_FACTOR 1.3
#define BLIB_GRAPH_SPARSE_NO_EDGE_VAL 0

/*Space efficency!!!*/
/*Store lists of vertices, no fancypants linked lists*/
typedef struct blib_graph_sparse_t{
	int size;
	int* alloced_edges;
	int* used_edges;
	int** adj;
	int* buffer;
	int buffer_size;
}blib_graph_sparse;

inline
int blib_graph_sparse_edge_deg(blib_graph_sparse* g, int x){
	return g->used_edges[x];
}

inline
int blib_graph_sparse_nth_edge(blib_graph_sparse*g, int x, int n){
#ifdef BLIB_DEBUG
	if(n>=g->used_edges[x])
		return -1;
#endif
	return g->adj[x][n];
}


inline
blib_graph_sparse* blib_graph_sparse_allocate(int size){
	int i;
	blib_graph_sparse* g;
	g=(blib_graph_sparse*)BLIB_MALLOC(sizeof(blib_graph_sparse));
	g->size=size;
	g->alloced_edges=(int*)BLIB_MALLOC(sizeof(int)*size);
	g->used_edges=(int*)BLIB_MALLOC(sizeof(int)*size);
	g->adj=(int**)BLIB_MALLOC(sizeof(int*)*size);
	for(i=0;i<size;i++){
		g->adj[i]=(int*)BLIB_MALLOC(sizeof(int)*BLIB_GRAPH_SPARSE_MIN_ALLOC);
		g->used_edges[i]=0;
		g->alloced_edges[i]=BLIB_GRAPH_SPARSE_MIN_ALLOC;
	}
	return g;
}

inline
void blib_graph_free(blib_graph_sparse* g){
	int i;
	for(i=0;i<g->size;i++){
		free(g->adj[i]);
	}
	free(g->used_edges);
	free(g->alloced_edges);
	free(g);
}


inline
int blib_graph_edge(blib_graph_sparse* g,int a, int b){
	int i;
#ifdef BLIB_DEBUG
	if(a >= g->size || b >= g->size){
		BLIB_ERROR("OUT OF BOUNDS graph access");
	}
#endif
	for(i=0;i<g->used_edges[a];i++){
		if(g->adj[a][i]==b){
			return 1;
		}
	}
	return BLIB_GRAPH_SPARSE_NO_EDGE_VAL;
}

inline
int blib_graph_size(blib_graph_sparse* g){
	return g->size;	
}

void blib_graph_set_dir_edge( blib_graph_sparse* g, int a, int b,int val){
	int i,j,new_size;
	int* new_list;
    #ifdef BLIB_DEBUG
	if(a >= g->size || b >= g->size){
		BLIB_ERROR("OUT OF BOUNDS");
	}
    #endif
	for(i=0;i<g->used_edges[a];i++){
		if(g->adj[a][i]==b){
			if(val!= BLIB_GRAPH_SPARSE_NO_EDGE_VAL)
				return;
			else{
				for(j=i;j<g->used_edges[a]-1;j++){
					g->adj[a][j]=g->adj[a][j+1];
				}
				g->used_edges[a]--;
				
				return;
			}
		}
		
	}
	if(val==BLIB_GRAPH_SPARSE_NO_EDGE_VAL)
		return;
	if(g->alloced_edges[a]>g->used_edges[a]){
		g->adj[a][g->used_edges[a]]=b;
		g->used_edges[a]++;
		return;
	}
	new_size=(g->alloced_edges[a]*BLIB_GRAPH_SPARSE_GROWTH_FACTOR+1);
	new_list=(int*)BLIB_MALLOC(sizeof(int)*new_size);
	for(i=0;i<g->used_edges[a];i++){
		new_list[i]=g->adj[a][i];
	}
	new_list[g->used_edges[a]]=b;
	free(g->adj[a]);
	g->adj[a]=new_list;
	g->used_edges[a]++;
	g->alloced_edges[a]=new_size;
}

inline
void blib_graph_set_edge(blib_graph_sparse* g, int a, int b, int val){
	blib_graph_set_dir_edge(g,a,b,val);
	blib_graph_set_dir_edge(g,b,a,val);
}

inline
int blib_graph_is_edge(blib_graph_sparse* g, int a, int b){
    #ifdef BLIB_DEBUG
	if( a> g->size || b>g->size){
		BLIB_ERROR("Edge out of bounds (%d,%d)",a,b);	
	}
    #endif
	if(blib_graph_edge(g,a,b)!=BLIB_GRAPH_SPARSE_NO_EDGE_VAL)
		return 1;
	return 0;
}

/*Returns the address of the copy*/
blib_graph_sparse* blib_graph_copy(blib_graph_sparse* a, blib_graph_sparse* b){
	int i,j;
	if(b==NULL)
		b=blib_graph_sparse_allocate(a->size);
	if(b->size != a->size){
		blib_graph_free(b);
		b=blib_graph_sparse_allocate(a->size);
	}
	for(i=0;i<a->size;i++){
		for(j=0;j<a->size;j++)
			blib_graph_set_edge(b,i,j, blib_graph_edge(a,i,j));
	}
	return b;
}


void blib_graph_print_dot(blib_graph_sparse* g,FILE* stream,char** names){
	int i,j;
	fprintf(stream,"digraph g {\n");
	if(names!=NULL){
		for(i=0;i<blib_graph_size(g);i++){
			if(names[i]!=NULL)
				fprintf(stream,"v%d [label=\"%s\"];\n",i,names[i]);
		}
	}
	for(i=0;i<blib_graph_size(g);i++){
		for(j=0;j<blib_graph_size(g);j++){
			if(blib_graph_is_edge(g,i,j)){
				fprintf(stream,"v%d -> v%d;\n",i,j);
			}
		}
	}
	fprintf(stream,"\n}\n");
}



/*Labels each vertex with the lex smallest vertex in it's connected component*/ 
/*I'm lazy so here is the O(n^3) version*/
/*Pre-color con_set with a list if you want to partition with it*/
void blib_graph_sparse_connected_component_list(blib_graph_sparse* g, int* con_set){
	int i,j,k,dest,dirty,size;
	size=blib_graph_size(g);
	for(i=0;i<size;i++){
		dirty=0;
		for(j=0;j<size;j++){
			if(con_set[j]<0)
				continue;
			for(k=0;k<blib_graph_sparse_edge_deg(g,j);k++){
				dest=blib_graph_sparse_nth_edge(g,j,k);
				if(j==dest)
					continue;
				if(con_set[dest]<0)
					continue;
				if(con_set[j]!=con_set[dest]){
					if(con_set[j]<con_set[dest])
						con_set[dest]=con_set[j];
					else
						con_set[j]=con_set[dest];
					dirty =1;
				}
			}
		}
		if(!dirty)
			break;
	}
}







#ifdef BLIB_UNIT_TEST

int blib_graph_sparse_unit(void){
	blib_graph_sparse* g;
	int i,j,fail;
	BLIB_ERROR("Starting Sparse Unit");
	fail=0;
	g=blib_graph_sparse_allocate(5);
	if(g==NULL){
		BLIB_ERROR("Graph didn't allocate");
		fail=1;
	}
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			if(blib_graph_is_edge(g,i,j)){
				BLIB_ERROR("Graph not set to empty during init");
				fail=1;
			}
		}
	}
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			blib_graph_set_dir_edge(g,i,j,1);
			if(!blib_graph_is_edge(g,i,j)){
				BLIB_ERROR("Graph didn't allocate edge");
				fail=1;
			}
		}
	}
	for(i=0;i<5;i++){
		for(j=0;j<5;j++){
			blib_graph_set_dir_edge(g,i,j,0);
			if(blib_graph_is_edge(g,i,j)){
				BLIB_ERROR("Graph didn't delete edge");
				fail=1;
			}
		}
	}
	
	
	blib_graph_free(g);
	/* Question how do we test to see if something is free?
        if(g!=NULL){
			BLIB_ERROR("Hey it didn't free the graph");
        }*/
	
	BLIB_ERROR("Ending sparse unit");
	return fail;
}
#endif



#endif /*_BLIB_GRAPH_SPARSE_DEF_*/