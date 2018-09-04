
/*########################################################################
#                                                                        #
#          HIGH PERFORMANCE MULTI-LAYER CHANNEL ROUTING					 #
#																		 #
# Objective : Crosstalk minimization in multi-layer channel routing      #
#																		 #
########################################################################*/


# include <stdio.h>
# include <conio.h>
# include <stdlib.h>
# include <math.h>
# include <graphics.h>

# define TOP    1
# define BOTTOM 0
# define MAX    50
# define MIN    30

//--------  Global Declarations  ------------
struct col
{
   int netno;
   int left;
   int top[MIN];
   int bottom[MIN];
   int right;
   int topmax;
   int bottommax;
   int count;
   int interval;
};

//--------Structure defn for HNCG
typedef struct nd
{
   int    netno;
   struct nd * next;
};


//-------Structure defn for VCG

//-- edge structure
typedef struct edge
{
  struct node *tonode;
  struct edge *adjnode;
}edgelist;

//-- node structure
typedef struct node
{
   int netno;
   struct node *nextnode;
   struct edge *edge;
}nodelist;


//-------Structure defn for RVCG

//--- node defn
typedef struct rvcgnd
{
   int interval;
   int status;
   nodelist  *member;
   struct rvcgedge *edge;
   struct rvcgnd   *nextnode;
}rvcg_node;

//--- edge defn
typedef struct rvcgedge
{
	rvcg_node *tonode;
	struct rvcgedge * nextedge;
}rvcg_edge;

//--------Allot list
typedef struct
{
	int layer;
	int track;
	int vlayer;
	rvcg_node *node;
}allot;

//-------Stack structure
typedef struct
{
	rvcg_node *node[MIN];
	int top;
}stack;


//--------- Global variables ----------------------

typedef  struct col nets;
typedef  struct nd nodes;
nodes    node   [MAX], dnode [MAX];
nets     net    [MAX];
int      top[MAX], bottom[MAX], sorttop[MAX], sortbottom[MAX];
int      net_count[MAX][2],lcs[MIN],rcs[MIN],lcs_count,rcs_count;
int      lcsrcspresent;
nodelist * vcg_start[MIN];
int      clique[MIN][MIN];
int      top_head,bottom_head;
int      layers,tracks,models;
allot    alloted[MIN][MIN];
stack    stk;
int      vlayert[7][MIN][MAX];
int      vlayerb[7][MIN][MAX];

//-------- Global Function Declarations  ----------

int  accept_top_bottom     (char *);
void show_top_bottom       (int);
int  calculate_interval    (int);
void mcc1			       (int);
int  cal_dmax              (int,int);
int  make_vcg   		   (int);
void show_vcg			   (int);
rvcg_node * make_rvcg	   (int,int,int);
void show_rvcg			   (rvcg_node *);
void allocate			   (rvcg_node **,int);
int  crosstalk             (int);
void vlayer_assign		   (int, int);
void draw_sol              (int, int);

//--------   Main   ----------
void main (int argc,char *argv[])
{

	int no_of_terminal,no_of_u_net,comp,dmaxval,ctval,vl=0;
	rvcg_node * rvcg_head;
	if(argc>2)
	{
	   printf("\nInvalid number of arguments!!");
	   return;
	}
	//-----------------Read top bottom vector from file
	no_of_terminal=accept_top_bottom(argv[1]);
	//-----------------Calculate interval of each net
	no_of_u_net=calculate_interval(no_of_terminal);
	//-----------------Determine Miinimum Clique Cover
	mcc1(no_of_u_net);
	//-----------------Calculate Maximum density
	dmaxval=cal_dmax(no_of_u_net,no_of_terminal);
	printf(" \nDmax is : %d\n",dmaxval);
	//-----------------Construct VCG
	comp=make_vcg(no_of_terminal);
	//-----------------Construct Reduced VCG
	rvcg_head=make_rvcg(comp,dmaxval,no_of_u_net);
	show_rvcg(rvcg_head);
	//-----------------Route the channel using the RVCG
	allocate(&rvcg_head,dmaxval);
	//-----------------Calculate Crosstalk
	ctval=crosstalk(layers-1);
	printf("\nCrosstalk= %d",ctval);
	getch();
	//-----------------Assign vertical layers to each net
	switch (models)
	{
		case 1:vl=layers+1;
			break;
		case 2:vl=layers;
			break;
		case 3:vl=layers-1;
			break;
	}
	vlayer_assign(vl,no_of_u_net);
	//-----------------Show solution graphically
	draw_sol(no_of_terminal, no_of_u_net);
	//-----------------------------------------------*/
}

//-----------------------------------------1accepts top-bottom vector/main
int accept_top_bottom(char *file)
{
	int n = 0,i = -1,topval;
	FILE *fp;
	if((fp=fopen(file,"r"))==NULL)
	{
	  printf(stderr,"\nCannot open file %s.",file);
	  exit(1);
	}
	//-----------------Read Top vector
	while(1)
	{
	   fscanf(fp,"%d",&topval);
	   if(topval < 0)
		  break;
	   else
		  top[++i]=topval;
	}
	n=i;
	//-----------------Read Bottom Vector
	for(i=0;i<=n;i++)
	   fscanf(fp,"%d",&bottom[i]);

	//----------------LCS/RCS present?
	fscanf(fp,"%d",&lcsrcspresent);
	if(lcsrcspresent == -2)
	{
		while(1)
		{
			fscanf(fp,"%d",&topval);
			if(topval < 0)
			   break;
			else
			   lcs[lcs_count++]=topval;
		}
		while(1)
		{
			fscanf(fp,"%d",&topval);
			if(topval < 0)
			   break;
			else
			   rcs[rcs_count++]=topval;
		}
	}
	fclose(fp);
	return(n);

}

//----------------------------------------1displays top-bottom vector/main
void show_top_bottom(int n)
{
	int i;
	for(i=0;i<=n;i++)
	   printf("%d\t%d\n",top[i],bottom[i]);
	return;
}

//----------------------------------------1Calculates interval of each net/main
int calculate_interval(int no_of_terminal)
{
	void sort              (int);
	int  net_occur_count   (int);
	void set_net_info      (int,int);
	void show_net          (int);

	int no_of_u_net;
	//-----------------Sort TOP BOTTOM Vector
	sort(no_of_terminal);
	//-------------Determine occurence of each net
	no_of_u_net=net_occur_count(no_of_terminal);
	//-------------Set start/end of each net
	set_net_info(no_of_u_net,no_of_terminal);
	show_net(no_of_u_net);
	getch();
	return(no_of_u_net);
}
//==============================2Sorts the top bottom vector/cal_interval
void sort(int n)
{
	int i,j,temp;

	for(i=0;i<=n;i++)
	{
		sorttop[i]=top[i];
		sortbottom[i]=bottom[i];
	}
	for (i=0;i<=n-1;i++)
		for(j=0;j<=(n-1-i);j++)
		{
			if(sorttop[j]>=sorttop[j+1])
			{
				temp=sorttop[j];
				sorttop[j]=sorttop[j+1];
				sorttop[j+1]=temp;
			}
			if(sortbottom[j]>=sortbottom[j+1])
			{
				temp=sortbottom[j];
				sortbottom[j]=sortbottom[j+1];
				sortbottom[j+1]=temp;
			}

		}
}
//==============================2Counts the occurence of each net/cal_interval
int net_occur_count(int n)
{
	int net_count_top[MAX][2],net_count_bottom[MAX][2],net_count[MAX][2];
	int i=0,j=0,cnt=0,k=0,l=0,index,topindex,bottomindex,www;
	while(i<=n)
	{
		j=i;
		cnt=0;
		net_count_top[k][0]=sorttop[i];
		while(sorttop[i]==sorttop[j])
		{
		   cnt++;
		   j++;
		}
		net_count_top[k][1]=cnt;
		i=j;
		k++;
	}
	i=0;
	l=0;
	while(i<=n)
	{
		j=i;
		cnt=0;
		net_count_bottom[l][0]=sortbottom[i];
		while(sortbottom[i]==sortbottom[j])
		{
		   cnt++;
		   j++;
		}
		net_count_bottom[l][1]=cnt;
		i=j;
		l++;
	}
	index=topindex=bottomindex=0;
	while (topindex!=k && bottomindex!=l)
	{
		if(net_count_top[topindex][0]<net_count_bottom[bottomindex][0])
		{
			net_count[index][0]=net_count_top[topindex][0];
			net_count[index][1]=net_count_top[topindex][1];
			topindex++;
			index++;
		}
		else if (net_count_top[topindex][0]>net_count_bottom[bottomindex][0])
		{
			net_count[index][0]=net_count_bottom[bottomindex][0];
			net_count[index][1]=net_count_bottom[bottomindex][1];
			bottomindex++;
			index++;
		}
		else
		{
			net_count[index][0]=net_count_bottom[bottomindex][0];
			net_count[index][1]=net_count_bottom[bottomindex][1];
			net_count[index][1]+=net_count_top[topindex][1];
			topindex++;
			bottomindex++;
			index++;
		}
	}
	if(topindex==k)
		while(bottomindex!=l)
		{
			net_count[index][0]=net_count_bottom[bottomindex][0];
			net_count[index][1]=net_count_bottom[bottomindex][1];
			bottomindex++;
			index++;
		}
	if(bottomindex==l)
		while(topindex!=k)
		{
		   net_count[index][0]=net_count_top[topindex][0];
		   net_count[index][1]=net_count_top[topindex][1];
		   topindex++;
		   index++;
		}
	for (i=0;i<index;i++)
		if(net_count[i][0]!=0)
		{
		   net[i].netno=net_count[i][0];
		   net[i].count=net_count[i][1];
		   net[i].left=-1;
		}

	return(index);
}
//===============================2Sets each Net Info/cal_interval
void set_net_info(int nou_net,int no_of_col)
{
   int  check_net_array(int,int);
   void set_net_begin  (int,int,int);
   int  dec_net_count  (int,int);
   void set_net_end    (int,int,int,int);
   void set_top        (int,int);
   void set_bottom     (int,int);
   void interval	   (int);
   void sort_startpos  (int);

   int col=0,i,ncount,netpos;

   while(col<=no_of_col)
   {

		if(top[col]!=0)
		{
			netpos=check_net_array(top[col],nou_net);
			ncount=dec_net_count(top[col],nou_net);
			if(net[netpos].left<0)
			{
				set_net_begin(col,TOP,netpos);
				ncount=dec_net_count(top[col],nou_net);
			}
			else
			{
				ncount=dec_net_count(top[col],nou_net);
				if(ncount<=0)
				   set_net_end(col,TOP,netpos,no_of_col);
				else
				   set_top(col,netpos);
			}
		}
		if(bottom[col]!=0)
		{
			netpos=check_net_array(bottom[col],nou_net);
			ncount=dec_net_count(bottom[col],nou_net);
			if(net[netpos].left<0)
			{
				set_net_begin(col,BOTTOM,netpos);
				ncount=dec_net_count(bottom[col],nou_net);
			}
			else
			{
				ncount=dec_net_count(bottom[col],nou_net);
				if(ncount<=0)
				  set_net_end(col,BOTTOM,netpos,no_of_col);
				else
				   set_bottom(col,netpos);
			}
		}
		col++;
   }
   for(i=0;i<nou_net;i++)
	if(net[i].right<net[i].left)
		net[i].right=no_of_col;
   interval(nou_net);
   sort_startpos(nou_net);
}
//*********************3 checks net structure for occurence of a given net/set_net_info
int check_net_array(int net_no,int nou_net)
{
   int i;
   for(i=0;i<nou_net;i++)
   {
	  if(net[i].netno==net_no)
		 return(i);
   }
   return(-1);
}
//*********************3 decrements net count/set_net_info
int dec_net_count(int net_no,int nou_net)
{

   int i;
   for(i=0;i<nou_net;i++)
   {
	 if(net[i].netno==net_no)
	 {
		net[i].count-=1;
		break;
	 }
   }
   return(net[i].count);
}
//*********************3 sets beginning of a net/set_net_info
void set_net_begin(int col,int tob,int netpos)
{
	int i=0;
	if(lcsrcspresent==-2)
	{
		for(i=0;i<lcs_count;i++)
		{
			if(net[netpos].netno==lcs[i])
			{       net[netpos].right=col;
				net[netpos].left=0;
				break;
			}
			else
				net[netpos].left=col;
		}
	}
	else
		net[netpos].left=col;
	if(tob==TOP)
		net[netpos].top[net[netpos].topmax++]=col;
	else
		net[netpos].bottom[net[netpos].bottommax++]=col;

	return;
}
//*********************3 marks end of a net/set_net_info
void set_net_end(int col,int tob,int netpos,int no_col)
{
	int i=0;
	if(lcsrcspresent==-2)
	{
		for(i=0;i<rcs_count;i++)
		{
			if(net[netpos].netno==rcs[i])
			{	net[netpos].right=no_col;
				break;
			}
			else
				net[netpos].right=col;
		}
	}
	else
		net[netpos].right=col;
	if(tob==TOP)
	   net[netpos].top[net[netpos].topmax++]=col;
	else
	   net[netpos].bottom[net[netpos].bottommax++]=col;
	return;
}
//*********************3 sets top connections of a net/set_net_info
void set_top(int col,int netpos)
{
	net[netpos].top[net[netpos].topmax++]=col;
	return;
}
//*********************3 sets bottom connections of a net/set_net_info
void set_bottom(int col,int netpos)
{
	net[netpos].bottom[net[netpos].bottommax++]=col;
	return;
}
//*********************3 calculates interval of each net/set_net_info
void interval (int nou_net)
{
	int i;
	for (i=0;i<nou_net;i++)
		net[i].interval=abs(net[i].right - net[i].left);
}
//*********************3 sorts the net info on its starting position/set_net_info
void sort_startpos(int nou_net)
{
	nets temp;
	int i,j;
	for (i=0;i<nou_net-1;i++)
		for(j=0;j<(nou_net-1-i);j++)
			if (net[j].left>net[j+1].left)
			{
				temp=net[j];
				net[j]=net[j+1];
				net[j+1]=temp;
			}
}
//===============================2 displays net information/cal_interval
void show_net(int nou_net)
{
   int i,j;
   printf("\nNet Info :\n");
   printf("Net\tLeft\tRight\tTopmax\tBmmax\tToplist\tBottomlist\tInterval\n\n");
   for(i=1;i<nou_net;i++)
   {
	  printf("%d\t%d\t%d\t%d\t%d\t",net[i].netno,net[i].left,net[i].right,net[i].topmax,net[i].bottommax);
	  for(j=0;j<net[i].topmax;j++)
		 printf("%d.",net[i].top[j]);
	  printf("\t");
	  for(j=0;j<net[i].bottommax;j++)
		 printf("%d.",net[i].bottom[j]);
	  printf("\t\t%d",net[i].interval);
	  printf("\n");
   }
}
//-----------------------------------------1 Minimum Clique Cover/main
void mcc1(int nou_net)
{
   void make_hncg         (int);
   void direct_hncg	      (int);
   int  identify_clique   (int);
   void show_clique       (int);

   int no_of_clique;
   make_hncg(nou_net);
   direct_hncg(nou_net);
   no_of_clique=identify_clique(nou_net);
   show_clique(no_of_clique);
   return;
}

//==============================2   Builds HNCG/mcc1
void make_hncg(int nou_net)
{
	int i,j;
	nodes *ptr;
	void add_to_list(int,int,int);

	for(i=1;i<nou_net;i++)
		node[i].netno=net[i].netno;
	for(i=1;i<nou_net-1;i++)
	{
		for(j=i+1;j<nou_net;j++)
			if(net[i].right<net[j].left)
				add_to_list(net[i].netno,net[j].netno,nou_net);
	}
}

//********************3 adds a node information in adjacency list of HNCG/make_hncg
void add_to_list(int net1,int net2,int nou_net)
{
	nodes * find(int,int);
	void add_node(nodes *,int);
	nodes *temp;
	temp=find(net1,nou_net);
	add_node(temp,net2);
	temp=find(net2,nou_net);
	add_node(temp,net1);
}

//~~~~~~~~~~~4 finds a node in HNCG/add_to_list
nodes * find(int net,int nou_net)
{
   int i;
   for(i=1;i<nou_net;i++)
   {
		if(node[i].netno==net)
			return(&node[i]);
   }
   return(NULL);
}
//~~~~~~~~~~~4 adds a node to HNCG/add_to_list
void add_node(nodes *head ,int net)
{
   nodes * newnode,*ptr;
   ptr=head;
   while(ptr->next!=NULL)
   {
	  if(ptr->netno==net)
		 return;
	  else
		ptr=ptr->next;
   }
   newnode=(nodes *)malloc(sizeof(nodes));
   newnode->netno=net;
   newnode->next=NULL;
   ptr->next=newnode;
   return;
}


//==============================2 Calculate the precedence in hncg
void direct_hncg (int nou_net)
{
	int i,j,flag=0;
	nodes *ptr;
	for (i=1;i<=nou_net;i++)
	{
		dnode[i].netno=node[i].netno;
		ptr=node[i].next;
		while (ptr!=NULL)
		{
			for(j=1;j<=i;j++)
				if (dnode[j].netno==ptr->netno)
					{
						flag=1;
						break;
					}
			if (flag==1)
			{
				flag=0;
				ptr=ptr->next;
				continue;
			}
			else
			{
				add_node(&dnode[i],ptr->netno);
				ptr=ptr->next;
			}

		}
	}
}
//==============================2  Identifies each clique
int identify_clique(int nou_net)
{
	int i=0,j=0,k=1;
	nodes * ptr, * head=&dnode[1];
	void del(nodes *,int); //  del function to delete the head
	do
	{
	   while(head->next!=NULL)
	   {
			clique[i][j++]=head->netno;
			del(head,nou_net-k+1);
			k++;

	   }
	   clique[i][j++]=head->netno;
	   i++;
	   j=0;
	   del(head,nou_net-k+1);
	   k++;

	}while(k<nou_net);
	return (i-1);
}
//********************3 deletes a node from HNCG and sets and rebulids HNCG/identify_clique
void del (nodes * head,int k)
{
	int tofind,i,j;
	nodes * ptr;
	void del_sink(int,int);
	ptr=head;
	ptr=head->next;
	//--  case where sink
	if(ptr==NULL)
	{
		del_sink(head->netno,k);
		for(j=1;j<k-1;j++)
			dnode[j]=dnode[j+1];

		return;
	}
	else
	{
		del_sink(head->netno,k);
		tofind=ptr->netno;

		for (i=1;i < k ;i++)
			if (dnode[i].netno==tofind)
				break;
		dnode[1]=dnode[i];
		for(j=i+1;j< k ; j++)
			dnode[j-1]=dnode[j];
		return;
	}
}

//~~~~~~~~~~~4 deletes all occurence of a node in HNCG /del
void del_sink(int net_no,int k)
{
   nodes * ptr,* prevptr;
   int i;
   for(i=1;i<k;i++)
   {
	  ptr=&dnode[i];
	  prevptr=ptr;
	  ptr=ptr->next;
	  while (ptr!=NULL)
	  {
		 if(ptr->netno==net_no)
		 {
			prevptr->next=ptr->next;
			break;
		 }
		 prevptr=ptr;
		 ptr=ptr->next;
	  }
   }
}
//===============================3 displays the cliques generated /mcc1
void show_clique(int noc)
{
	int i=0,j=0;
	printf("\nMinimum Clique Cover 1 -> \n");
	printf("Track No\tNets\n");
	for(i=0;i<=noc;i++)
	{
	  printf("%d\t\t",i+1);
	  for(j=0;j<10;j++)
	  {
		 if(clique[i][j]!=0)
			 printf("%d .",clique[i][j]);
		 else
			continue;
	  }
	  printf("\n");
	}
	return;
}

//-----------------------------------------1 Calculates  dmax/main
int cal_dmax(int nou_net,int no_col)
{
	int i,j,greatest=0,count=0;
	for (i=0;i<=no_col;i++)
	{
		greatest=greatest<=count?count:greatest;
		count=0;
		for(j=0;j<nou_net;j++)
			if(net[j].left<=i && net[j].right>=i)
				count++;
	}
	return (greatest<=count?count:greatest);
}

//-----------------------------------------1 Make VCG /main
int make_vcg(int no_of_col)
{
	nodelist * find_in_vcg(int,int,int);
	void add_edge(nodelist *,nodelist *);
	int i,com=0;
	nodelist * ptr_top,* ptr_bottom,*newnodetop,*newnodebottom,*topn,*tmp;
	for (i=0;i<=no_of_col;i++)
	{
	  if(top[i]==0 && bottom[i]==0)  // both nets are 0
		continue;
	  ptr_top=ptr_bottom=NULL;
	  if (top[i]!=0 && bottom[i]==0)    //  only bottom 0
	  {
		ptr_top=find_in_vcg(top[i],com,0);    //find top[i]
		if(ptr_top==NULL)                     //if not found
		{
			//create new node for top[i]
			newnodetop=(nodelist *)malloc(sizeof(nodelist));
			newnodetop->netno=top[i];
			newnodetop->nextnode=NULL;
			newnodetop->edge=NULL;
			//add to end of head array
			vcg_start[com++]=newnodetop;
		}
	  }
	  else if (bottom[i]!=0 && top[i]==0) // only top 0
	  {
		ptr_bottom=find_in_vcg(bottom[i],com,1); //find bottom[i]
		if(ptr_bottom==NULL)                     //if not found
		{
			//create new node for bottom[i]
			newnodebottom=(nodelist *)malloc(sizeof(nodelist));
			newnodebottom->netno=bottom[i];
			newnodebottom->nextnode=NULL;
			newnodebottom->edge=NULL;
			//add to end of head array
			vcg_start[com++]=newnodebottom;
		}
	  }
	  else                              // both top and bottom !0
	  {
		//find top[i] and bottom[i]
		ptr_top=find_in_vcg(top[i],com,0);
		ptr_bottom=find_in_vcg(bottom[i],com,1);
		if((ptr_bottom==ptr_top)&& ptr_top!=NULL) continue;
		if(ptr_top!=NULL && ptr_bottom==NULL)     //top=f;bottom=nf
		{
			//create new node for bottom
			newnodebottom=(nodelist *)malloc(sizeof(nodelist));
			newnodebottom->netno=bottom[i];
			newnodebottom->nextnode=NULL;
			newnodebottom->edge=NULL;
			//add a edge from top to bottom
			add_edge(ptr_top,newnodebottom);
			//add bottom to nodelist
			topn=ptr_top;
			while(topn->nextnode!=NULL)
				topn=topn->nextnode;
			topn->nextnode=newnodebottom;
		}
		if(ptr_top==NULL && ptr_bottom!=NULL)   //top=nf;bottom=f
		{
			//create a new node for top
			newnodetop=(nodelist *)malloc(sizeof(nodelist));
			newnodetop->netno=top[i];
			newnodetop->nextnode=NULL;
			newnodetop->edge=NULL;
			//add a edge from top to bottom
			add_edge(newnodetop,ptr_bottom);

			if(vcg_start[bottom_head]==ptr_bottom) //if bottom[i] is the first node
			{
			   newnodetop->nextnode=ptr_bottom; //add top before bottom in nodelist
			   vcg_start[bottom_head]=newnodetop;  //add top to head list
			}
			else
			   vcg_start[com++]=newnodetop;                                //if bottom[i] is not first node]

		}
		if(ptr_top!=NULL && ptr_bottom!=NULL)   //top=f;bottom=f
		{	add_edge(ptr_top,ptr_bottom);	  //add edge from top to bottom
			if (vcg_start[bottom_head]==ptr_bottom && vcg_start[bottom_head]->nextnode==NULL)
			{

				topn=ptr_top;
				while(topn->nextnode!=NULL)
				   topn=topn->nextnode;
				topn->nextnode=vcg_start[bottom_head];
				vcg_start[bottom_head]=NULL ;
			}
		}

		if(ptr_top==NULL && ptr_bottom==NULL)   //top=nf;bottom=nf
		{
			//create new node for top
			newnodetop=(nodelist *)malloc(sizeof(nodelist));
			newnodetop->netno=top[i];
			newnodetop->nextnode=NULL;
			newnodetop->edge=NULL;
			if(top[i]==bottom[i])
			{
			   vcg_start[com++]=newnodetop;
			   continue;
			}
			//create new node for bottom
			newnodebottom=(nodelist *)malloc(sizeof(nodelist));
			newnodebottom->netno=bottom[i];
			newnodebottom->nextnode=NULL;
			newnodebottom->edge=NULL;
			//addedge from top to bottom
			add_edge(newnodetop,newnodebottom);
			//add top,bottom in nodelist
			newnodetop->nextnode=newnodebottom;
			//add a new component to head array
			vcg_start[com++]=newnodetop;
		}
	 }

   }
   return(com);
}

//==============================2 Search for a given node in VCG /make_vcg
nodelist * find_in_vcg(int netno,int component,int topflag)
{
   nodelist * ptr,* nodeaddress;
   nodelist * dfs(int,nodelist *);
   int i;
   for(i=0;i<component;i++)
   {
	  ptr=vcg_start[i];
	  if(topflag==0)
		top_head=i;
	  else
		bottom_head=i;
	  if(ptr==NULL)
		return(NULL);
	  else
		while(ptr!=NULL)
		{
			if(ptr->netno==netno)
			  return(ptr);
			ptr=ptr->nextnode;
		}
   }

   return(NULL);
}
//==============================2 Adds a edge between to given nodes/make_vcg
void add_edge(nodelist * top, nodelist * bottom)
{
	edgelist * newedge, * ptr;
	newedge=(edgelist *)malloc(sizeof(edgelist));
	newedge->tonode=bottom;
	newedge->adjnode=NULL;
	if(top->edge==NULL)
	{
	   top->edge=newedge;
	   return;
	}
	ptr=top->edge;
	while(ptr->adjnode!=NULL)
	 ptr=ptr->adjnode;
	ptr->adjnode=newedge;
	return;
}

//------------------------------1 Displays VCG/main
void show_vcg(int comp)
{
	nodelist * ptr,*nodenext;
	edgelist * nextedge;
	int i,k=0;
	printf("The VCG ->\n");
	for(i=0;i<comp;i++)
	{

		ptr=vcg_start[i];
		if (ptr==NULL) continue;
		printf("Component %d\n",++k);
		while(ptr!=NULL)
		{
			printf("%d -->",ptr->netno);
			nextedge=ptr->edge;
			while(nextedge!=NULL)
			{
			  nodenext=nextedge->tonode;
			  printf("->%d",nodenext->netno);
			  nextedge=nextedge->adjnode;
			}
			ptr=ptr->nextnode;
			printf("\n");
		}
		printf("\n");
	}
}


//-------------------------------1 Make RVCG from VCG  /main
rvcg_node * make_rvcg(int nocinvcg,int nocinrvcg,int nou_net)
{
	void make_rvcg_edge(int,rvcg_node *);
	rvcg_node * head,*newhead,*temp;
	int i,j,l,totinterval;
	nodelist * newnode,*tempnode;

	for(i=0;i<nocinrvcg;i++)
	{
		totinterval=0;
		newhead=(rvcg_node *)malloc(sizeof(rvcg_node));
		newhead->member =NULL;
		newhead->edge =NULL;
		newhead->nextnode =NULL;

		if(i==0)
			head=newhead;
		else
		{
			temp=head ;
			head=newhead;
			newhead->nextnode =temp;
		}
		for(j=0;j<MIN;j++)
		{
		   if(clique[i][j]==0)
			 break;
		   for(l=0;l<nou_net;l++)
		   {
				if( net[l].netno==clique[i][j])
					totinterval+=net[l].interval;
		   }
		   newhead->interval =totinterval;

		   newnode=(nodelist *) malloc(sizeof(nodelist));
		   newnode->netno=clique[i][j];
		   newnode->nextnode =NULL;
		   newnode->edge =NULL;
		   if(newhead->member ==NULL)
				newhead->member =newnode;
		   else
		   {
				tempnode=newhead->member ;
				newhead->member =newnode;
				newnode->nextnode =tempnode;
		   }
		}


	}

	make_rvcg_edge(nocinvcg,head);
	return(head);
}



//=====================2 set the edges in rvcg /make_rvcg

void make_rvcg_edge(int nocinvcg,rvcg_node * head)
{
	rvcg_node * find_in_rvcg_head(int,rvcg_node *);
	int i,k,a[MIN][2],net1,net2,flag;
	nodelist * nodeptr;
	rvcg_edge *rvcg_newedge,*rvcg_prevedge,*rvcg_edgeptr;
	rvcg_node * index1, * index2;
	edgelist *edgeptr;
	k=0;
	for (i=0;i<nocinvcg;i++)
	{
	   nodeptr=vcg_start[i];
	   if( nodeptr==NULL)
		 continue;
	   while(nodeptr!=NULL)
	   {
		   edgeptr=nodeptr->edge ;
		   while(edgeptr!=NULL)
		   {
				a[k][0]=nodeptr->netno ;
				a[k++][1]=(edgeptr->tonode)->netno;
				edgeptr=edgeptr->adjnode ;
		   }
			nodeptr=nodeptr->nextnode ;
	   }
	}
	for(i=0;i<k;i++)
	{
		net1=a[i][0];
		net2=a[i][1];
		index1=find_in_rvcg_head(net1,head);
		index2=find_in_rvcg_head(net2,head);
		if(index1->edge !=NULL)
		{
			rvcg_edgeptr=index1->edge ;
			flag=0;
			while(rvcg_edgeptr !=NULL)
			{
				if(rvcg_edgeptr->tonode==index2)
				{
					flag=1;
					break;
				}
				else
				{
					rvcg_prevedge=rvcg_edgeptr;
					rvcg_edgeptr=rvcg_edgeptr->nextedge ;
				}
			}
			if(flag==0)
			{
				rvcg_newedge=(rvcg_edge *)malloc(sizeof(rvcg_edge));
				rvcg_newedge->tonode=index2;
				rvcg_newedge->nextedge =NULL;
				rvcg_prevedge->nextedge =rvcg_newedge;
			}
		}
		else
		{
			rvcg_newedge=(rvcg_edge *)malloc(sizeof(rvcg_edge));
			rvcg_newedge->tonode=index2;
			rvcg_newedge->nextedge=NULL;
			index1->edge  =rvcg_newedge;
		}
	}
}

//************3 Find occurence of a node in rvcg /make_rvcg_edge
rvcg_node * find_in_rvcg_head(int netnum,rvcg_node * head)
{

	rvcg_node * ptr;
	nodelist * nodeptr;
	ptr=head;
	while(ptr!=NULL)
	{
		nodeptr=ptr->member ;
		while(nodeptr!=NULL)
		{
			if(nodeptr->netno ==netnum)
				return(ptr);
			nodeptr=nodeptr->nextnode ;
		}
		ptr=ptr->nextnode ;
	}
	return(NULL);
}

//-------------------------------1 Display the RVCG /main

void show_rvcg(rvcg_node * head)
{
	int i=0;
	rvcg_node * ptr;
	nodelist *nodeptr;
	rvcg_edge * edgeptr;
	ptr=head;
	printf("\nThe reduced VCG -->");
	while(ptr!=NULL)
	{
		printf("\n\nSupernode %d : Interval % d",i+1,ptr->interval );
		printf("\nStatus %d",ptr->status);
		nodeptr= ptr->member  ;
		printf("\nMembers ->");
		while(nodeptr!=NULL)
		{
			printf("%d. ",nodeptr->netno );
			nodeptr=nodeptr->nextnode ;
		}
		printf("\nEdgelist ->");
		edgeptr=ptr->edge ;
		while(edgeptr!=NULL)
		{
			printf("%d - ",((edgeptr->tonode)->member)->netno) ;
			edgeptr=edgeptr->nextedge  ;
		}
		ptr=ptr->nextnode;
		i++;
	}
}

/*----------------------------------------------------------------
|  Allocate net in the channel for the given number of layer.    |
|    The allocation is done so as to minimize the crosstalk.     |
----------------------------------------------------------------*/

void allocate(rvcg_node **rvcg_head,int dmax)
{
	int i,model,track,layer,trk,sl,el,sel_idx=-1,j,k,beg,end,t;
	int x,gap,rem_layer,new_track,m,rem_node,copy_no,cnt;
	nodelist *node;
	rvcg_node *max,* sel[MIN],*temp;
	rvcg_node* rvcg_search(rvcg_node *);
	rvcg_node* rvcg_search2(rvcg_node *);

	rvcg_node** modify_rvcg(rvcg_node **,rvcg_node *);
	for(j=0;j<MIN;j++)
		sel[j]=NULL;
L1: printf("\nEnter the number of horizontal layers (2-6) -> ");
	scanf("%d",&i);
	if(i<2 || i>6)
	{
		printf("\nProvide layer within 2 and 6.");
		goto L1;
	}
	if(i>dmax)
		track=(int)ceil((double)(dmax+1)/i);
	else
		track=(int)ceil((double)(dmax/(double)i));

L2:	printf("\nEnter the model(1 for Vi+1Hi / 2 for ViHi / 3 for ViHi+1) : ");
	scanf("%d",&model);
	models=model;
	if(i==2 && model==3)
	{
		printf("\nInvalid model...Retry!!");
		goto L2;
	}
	switch(model)
	{
	   case 1 : sl=0;el=i-1;break;
	   case 2 : sl=1;el=i-1;break;
	   case 3 : sl=1;el=i-2;break;
	   default: goto L2;
	}
	cnt=0;
	//------- Select nets that are to be allocated in Hlayers
	//------- flanked by two V layer.
	for(layer=sl;layer<=el;layer++)
	{
	  for(trk=0;trk<track&&cnt<dmax;trk++)
	  {
		  max=rvcg_search(*rvcg_head);
		  max->status=-1;
		  rvcg_head=modify_rvcg(rvcg_head,max);
		  sel[++sel_idx]=max;
		  cnt++;
	  }
	}
	//---------print sel array
	printf("\nSelect array for middle layers (initial):\n");
	for(j=0;j<=sel_idx;j++)
	{
	   printf("%d -",(sel[j])->member->netno);
	}

	if(model!=1)
	{
	 //---- Identify nets that are to be allocated in the extreme layer 0
		t=-1;
		for(trk=0;trk<track&&*rvcg_head!=NULL;trk++)
		{
			max=rvcg_search2(*rvcg_head);
			max->status=-1;
			rvcg_head=modify_rvcg(rvcg_head,max);
			alloted[0][++t].layer=0;
			alloted[0][t].track=trk;
			alloted[0][t].node=max;
		}

	 //---- Identify nets that are to be allocated in the extreme layer n
		if (model==3)
		{
			t=-1;
			while(*rvcg_head!=NULL)
			{
				max=rvcg_search2(*rvcg_head);
				max->status=-1;
				rvcg_head=modify_rvcg(rvcg_head,max);
				alloted[i-1][++t].layer=i-1;
				alloted[i-1][t].track=t;
				alloted[i-1][t].node=max;
			}
		}
	}

	//-----------sort select list for middle layer on their interval
	for(j=0;j<=sel_idx-1;j++)
	 for(k=j+1;k<=sel_idx;k++)
	 {
		if((sel[j])->interval<=(sel[k])->interval)
		{
		   temp=sel[j];
		   sel[j]=sel[k];
		   sel[k]=temp;
		}
	 }

	/*---------print sel array-----------------*/
	printf("\nSelect array for middle layers(sorted) :\n");
	for(j=0;j<=sel_idx;j++)
	{
	   printf("%d -",(sel[j])->member->netno);
	}
	getch();

	/*----Allocate nets Max/Min alternate order for middle layer
		  to reduce crosstalk                          -------*/

	beg=0;end=sel_idx;
	j=sl;k=0;
	while(beg<=end)
	{
	  for(j=sl;j<=el;j++)
	  {
		   alloted[j][k].layer=j;
		   alloted[j][k].track=k;
		   alloted[j][k].node=sel[beg++];
		   if(beg>end) break;
	  }
	  k++;

	  if(beg<=end)
	  {
		  for(j=sl;j<=el;j++)
		  {
			alloted[j][k].layer=j;
			alloted[j][k].track=k;
			alloted[j][k].node=sel[end--];
			if(beg>end) break;
		  }
	   }
	   k++;
	}
	//-------------Display Alloted list
	printf("\nAllot List:\n");
	printf("Layer\tTrack\tNodelist\n");
	printf("-----\t-----\t---------\n");
	for(j=0;j<i;j++)
	{
	  for(k=0;k<track;k++)
	  {
		 if(alloted[j][k].node!=NULL)
		 {
			printf("%d\t%d\t",alloted[j][k].layer,alloted[j][k].track);
			node=alloted[j][k].node ->member;
			while(node!=NULL)
			{
				  printf("%d.",node->netno);
				node=node->nextnode ;
			}
			printf("\n");
		 }
	  }
	}
	getch();
	layers=i;
	tracks=track;

}

/*- Performs a DFS on RVCG to find the most visited node in RVCG -*/
rvcg_node* rvcg_search(rvcg_node *head)
{
	rvcg_node * start,*ptr,*max;
	rvcg_edge *ptr2,*ptr3;
	rvcg_node * unvisited(rvcg_node *);
	void push(rvcg_node *);
	rvcg_node * pop(void);
	int i;
	ptr=head;
	while(ptr!=NULL)
	{
	   ptr->status=0;
	   ptr=ptr->nextnode;
	}
	stk.top=-1;
	while(start=unvisited(head))
	{
	   while(start!=NULL)
	   {
			if(start->status!=0)
			{
			  start->status++;
			  start=pop();
			}
			else
			{
				start->status++;
				ptr2=start->edge ;
				while(ptr2 !=NULL)
				{
					push(ptr2->tonode);
					ptr2=ptr2->nextedge ;
				}
				start=pop();
			}

		}
	}
	ptr=head;
	max=ptr;
	while(ptr!=NULL)
	{
		if(ptr->status >=max->status)
		   max=ptr;
		ptr=ptr->nextnode;
	}
	return(max);
}

//----------Identify set of nodes for extreme layers
rvcg_node* rvcg_search2(rvcg_node *head)
{
	rvcg_node *ptr,*max;
	rvcg_edge *ptr2;
	int i=0,c=0;
	ptr=head;
	while(ptr!=NULL)
	{
		c=0;
		ptr2=ptr->edge;
		while(ptr2!=NULL)
		{
		   c++;
		   ptr2=ptr2->nextedge;
		}
		if(c>=i)
		{
		  i=c;
		  max=ptr;
		}
		ptr=ptr->nextnode;
	}
	return(max);
}

/*- Finds an Unvisited node in RVCG  -*/
rvcg_node * unvisited(rvcg_node * ptr)
{
	while(ptr!=NULL)
	{
		if(ptr->status==0)
			return(ptr);
		ptr=ptr->nextnode ;
	}
	return(ptr);
}

//----------Push to Stack
void push(rvcg_node * ptr)
{
	stk.node[++(stk.top)]=ptr;
	return;
}
//----------Pop from Stack
rvcg_node *pop(void)
{
	if(stk.top==-1)
		return(NULL);
	return(stk.node[(stk.top)--]);
}



/*- Modifies RVCG after deleting the nodes that are allocated in channel -*/

rvcg_node** modify_rvcg(rvcg_node **head,rvcg_node *max)
{
   int i,j;
   rvcg_node *ptr,*prevnode;
   rvcg_edge *edgehead,*prevedge,*nextedge;
   ptr=*head;
   while(ptr!=NULL)
   {
		edgehead= ptr->edge;
		prevedge=NULL;
		while(edgehead!=NULL)
		{
		  if(prevedge==NULL && (edgehead->tonode==max))
		  {
			 ptr->edge=edgehead->nextedge;
			 edgehead=edgehead->nextedge;
			 break;
		  }
		  else if(edgehead->tonode==max)
			 prevedge->nextedge=edgehead->nextedge;
		  else
			 prevedge=edgehead;
		  edgehead=edgehead->nextedge;
		}
		ptr=ptr->nextnode;
   }
   ptr=*head;
   prevnode=NULL;
   while(ptr!=NULL)
   {
	  if(prevnode==NULL && ptr->status==-1)
		 *head=ptr->nextnode;
	  else if(ptr->status==-1)
		 prevnode->nextnode=ptr->nextnode;
	  else
	  {
		 ptr->status=0;
		 prevnode=ptr;
	  }
	  ptr=ptr->nextnode;
   }
   return(head);
}

//---------------------------Calculation of Crosstalk
int crosstalk(int l)
{
	int j,k;
	long int ct=0;
	if(l>layers)
	   return(-1);
	if (tracks<=1)
		ct=0;
	else
	{
		for (j=0;j<=l;j++)
		{
			k=1;
			while(k<tracks)
			{
				if(alloted[j][k].node!=NULL)
				{
					if(alloted[j][k+1].node!=NULL && alloted[j][k-1].node!=NULL)
					{
						ct+= 2*(alloted[j][k].node)->interval;
						k+=2;
					}
					else
					{
						ct+= alloted[j][k].node->interval;
						k++;
					}
				}
				else
				  break;
			}
		}
	}
	return(ct);
}


//--------------------------- Vertical layer assignment /main
void vlayer_assign( int no_vlayer,int no_unet)
{
	int i,j,lidx=0,tidx=0,colidx=0,memnetno,hlayer;
	int columns_taken[MIN],temp,k;
	rvcg_node *snode;
	nodelist *memnet;

	hlayer=0;
	for(lidx=0;lidx<no_vlayer;lidx++)
	{
		for (j=0;j<MIN;j++)
			columns_taken[j]=0;

		for(tidx=0;tidx<tracks;tidx++)
		{
			snode=alloted[hlayer][tidx].node;
			memnet=snode->member;
			while (memnet!= NULL)
			{
				memnetno=memnet->netno;
				i=check_net_array(memnetno,no_unet);
				for(j=0;j<net[i].topmax;j++)
				{
					colidx=net[i].top[j];
					if (vlayert[lidx][tidx][colidx]==0)
					{
						vlayert[lidx][tidx][colidx]=memnetno;
					}
					else
						vlayert[lidx+1][tidx][colidx]=memnetno;

				}
				for (j=0;j<net[i].bottommax;j++)
				{
					colidx=net[i].bottom[j];
					if (vlayerb[lidx][tidx][colidx]==0)
					{
						vlayerb[lidx][tidx][colidx]=memnetno;
						for(k=tidx;k<tracks-1;k++)
						  vlayert[lidx][k+1][colidx]=-memnetno;
					}
					else
					{
						vlayerb[lidx+1][tidx][colidx]=memnetno;
					}
				}
				memnet=memnet->nextnode;
			}
		}
		hlayer++;
		if (hlayer == layers)
			break;
	}
	if (models==3)
	{
		lidx=layers-1;
		for(tidx=0;tidx<tracks;tidx++)
		{
			snode=alloted[hlayer][tidx].node;
			memnet=snode->member;
			while (memnet!= NULL)
			{
				memnetno=memnet->netno;
				i=check_net_array(memnetno,no_unet);
				for(j=0;j<net[i].topmax;j++)
				{
					colidx=net[i].top[j];
					if (vlayert[lidx][tidx][colidx]==0)
						vlayert[lidx][tidx][colidx]=memnetno;

				}
				for (j=0;j<net[i].bottommax;j++)
				{
					colidx=net[i].bottom[j];
					if (vlayerb[lidx][tidx][colidx]==0)
						vlayerb[lidx][tidx][colidx]=memnetno;
				}
				memnet=memnet->nextnode;
			}
		}
	}
}



//----------------------------------Displays solution graphically
void draw_sol( int no_terminal,int no_unet)
{
	int gd=DETECT,gm,errorcode;
	int maxy,colgap,rowgap,col,l,t,c,row,rowt,rowb;
	int blinet,blineb,up,down,ctval;
	int pos,startx,endx,netnum,i,x=5,y=0;
	char *net_num,*c_val,*ly,*tr;
	int nets_present[MIN][2],idx=-1;
	rvcg_node *snode;
	nodelist *node;
	initgraph(&gd,&gm,"c:\\tc\\bgi");
	/* read result of initialization */
	errorcode = graphresult();

	if (errorcode != grOk)  /* an error occurred */
	{
		 printf("Graphics error: %s\n", grapherrormsg(errorcode));
		 printf("Press any key to halt:");
		 getch();
		 exit(1);             /* return with error code */
	}
	//---------------Initialization and scaling
	maxy=getmaxy();
	blinet=40;
	colgap=maxy/no_terminal;
	rowgap=40;
	blineb=rowgap*(tracks+2);

	//---------------Draw net by each layer
	for(l=0;l<layers;l++)
	{
	  itoa(l,ly,10);
	  outtextxy(0,460,"Layer=");
	  outtextxy(50,460,ly);

	  ctval=crosstalk(l)-crosstalk(l-1);
	  itoa(ctval,c_val,10);
	  outtextxy(100,460,";Crosstalk=");
	  outtextxy(200,460,c_val);


	  settextstyle(2, 1, 0);
	  row=blinet+rowgap;
	  idx=-1;
	  for (i=0;i<MIN;i++)
		for(c=0;c<2;c++)
			nets_present[i][c]=0;

	  line(0,blinet,maxy,blinet);
	  line(0,blinet-20,0,blinet);
	  line(maxy,blinet-20,maxy,blinet);
	  up=blinet-3;
	  down=blinet+3;
	  i=0;
	  for(col=5;col<maxy;col+=colgap)
	  {
		 line(col,up,col,down);
		 itoa(top[i++],net_num,10);
		 outtextxy(col-3,up-15 ,net_num);
		 if(i>no_terminal) break;
	  }
	  line(0,blineb,maxy,blineb);
	  line(0,blineb,0,blineb+20);
	  line(maxy,blineb,maxy,blineb+20);
	  up=blineb-3;
	  down=blineb+3;
	  i=0;
	  for(col=5;col<maxy;col+=colgap)
	  {
		 line(col,up,col,down);
		 itoa(bottom[i++],net_num,10);
		 outtextxy(col-3,up+10,net_num);
		 if(i>no_terminal) break;
	  }
	  //---------------- Draw horizontal lines
	  for(t=0;t<tracks;t++)
	  {
		  if(alloted[l][t].node==NULL) continue;
		  node=(alloted[l][t].node)->member;
		  while(node!=NULL)
		  {
			 netnum=node->netno;
			 if(lcsrcspresent=-2)
			 {
				for(i=0;i<lcs_count;i++)
				{
				  if(lcs[i]==netnum)
				  {
					x=0;
					break;
				  }
				  else
					x=5;
				}
				for(i=0;i<10;i++)
				{
				  if(rcs[i]==netnum)
				  {
					y=5;
					break;
				  }
				  else
					y=0;
				}
			 }

			 if(netnum!=0)
				nets_present[++idx][0]=netnum;
			 pos=check_net_array(netnum,no_unet);
			 startx=(net[pos].left*colgap)+x;
			 endx=(net[pos].right*colgap)+5+y;
			 line(startx,row,endx,row);
			 node=node->nextnode;
		  }
		  row=row+rowgap;
	  }
	  //--------------- Draw vertical lines
	  if (layers != 3)
	  {
		for(t=0;t<tracks;t++)
		{
			for(c=0;c<=no_terminal;c++)
			{
				for(i=0;i<=idx;i++)
				{
					if (vlayert[l][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet;
						rowb=blinet+((t+1)*rowgap);
						line(startx,rowt,endx,rowb);
					}
				}
				for(i=0;i<=idx;i++)
				{
					if (vlayert[l+1][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet;
						rowb=blinet+((t+1)*rowgap);
						setlinestyle(2,1, 1);
						line(startx-2,rowt,endx-2,rowb);
						setlinestyle(0,1,1);
					}
				}
				for(i=0;i<=idx;i++)
				{
					if (vlayerb[l][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet+((t+1)*rowgap);
						rowb=blineb;
						line(startx,rowt,endx,rowb);
					}
				}
				for(i=0;i<=idx;i++)
				{
					if (vlayerb[l+1][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowb=blinet+((t+1)*rowgap);
						rowt=blineb;
						setlinestyle(2,1, 1);
						line(startx-2,rowb,endx-2,rowt);
						setlinestyle(0,1,1);
					}
				}
			}
			settextstyle(0,0,0);

		}
	  }
	  else
	  {

		for(t=0;t<tracks;t++)
		{
			for(c=0;c<=no_terminal;c++)
			{
				for(i=0;i<=idx;i++)
				{
					if (vlayert[l][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet;
						rowb=blinet+((t+1)*rowgap);
						line(startx,rowt,endx,rowb);
					}
				}
				for(i=0;i<=idx;i++)
				{
					if (vlayert[l+1][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet;
						rowb=blinet+((t+1)*rowgap);
						setlinestyle(2,1, 1);
						line(startx-2,rowt,endx-2,rowb);
						setlinestyle(0,1,1);
					}
				}
				for(i=0;i<=idx;i++)
				{
					if (vlayerb[l][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowt=blinet+((t+1)*rowgap);
						rowb=blineb;
						line(startx,rowt,endx,rowb);
					}
				}
				if (l==layers-1) continue;
				for(i=0;i<=idx;i++)
				{
					if (vlayerb[l+1][t][c]==nets_present[i][0])
					{
						nets_present[i][1]=1;
						startx=endx=(c*colgap)+5;
						rowb=blineb;
						rowt=((t+1)*rowgap)+blinet;
						setlinestyle(2,1, 1);
						line(startx-2,rowt,endx-2,rowb);
						setlinestyle(0,1,1);
					}
				}
			}
			settextstyle(0,0,0);

		}
	  }
	  getch();
	  cleardevice();
	  row=10;
	}
}

//--------------------------------------------------------------------END