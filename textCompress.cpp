#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <math.h>

using namespace std;

struct Show	
{
   char word;
   int f;
};

struct Node
{
   char label;
   int weight;
   Node* left;
   Node* right;
};

struct Code
{
   char word;
   char* code_word;
};

int COMPRESS();
int DECOMPRESS();
int HUFFMAN_COUNT(Show*,char*,int);
Node* HUFFMAN_BUILD_TREE(Show*,int,int&);
void BUILD_MIN_HEAP(Node**,int);
void MIN_HEAPIFY(Node**,int,int);
void HUFFMAN_BUILD_CODE(Show*,Code*,Node*,int,char*,int&);
void HUFFMAN_ENCODE_TREE(Node*,char*,int&);
void HUFFMAN_ENCODE_TEXT(map<char,char*>,char*,char*,int,int&);
void HUFFMAN_REBUILD_TREE(char*,Node*,char*,int&);
void HUFFMAN_DECODE_TEXT(string&,char*,Node*,int&);

int main()
{
	int select;

	cout << "1.Compress\n" << "2.Decompress\n" << "? ";
	cin >> select;

	switch(select){
		case 1:
			COMPRESS();
			break;
		case 2:
			DECOMPRESS();
			break;
		default:
			cout << "\n\n\n!!WRONG!!\n\n\n";
	}
	system("pause");
	return 0;
}
int DECOMPRESS(){
	//-----------------load file-------------------
	string file_name;
	cout<<"請輸入檔名：";
	cin>>file_name;

	ifstream file_in;
	file_in.open(file_name.c_str(),ios::binary);
	if(!file_in)
	{
		cout<<"找不到檔案\n";
		return 0;
	}

	int file_size;
	file_in.seekg(0,file_in.end);
	file_size=file_in.tellg();
	file_in.seekg(0,file_in.beg);

	char* read_file=new char[file_size];
	file_in.read(read_file,file_size);
	file_in.close();
	//---------------------------------------------
	
	//----------------ascii to binary--------------
	int i, ascii, k = 7;
	char *code = new char[file_size*8];

	for( i = 0; i < file_size; i++, k+=8 ){
		ascii = read_file[i];
		if( ascii < 0 )
			ascii = 128 - ascii;
		for( int j = 0; j < 8; j++, k-- ){
			code[k] = (ascii % 2) + 48;
			ascii /= 2;
		}
		k+=8;
	}
	//---------------------------------------------
	
	Node *tree=new Node;
	i = 0;
	HUFFMAN_REBUILD_TREE(read_file, tree, code, i);

	i++;
	string output;
	HUFFMAN_DECODE_TEXT( output, code, tree, i);

	//-------------------save-----------------------
	file_name = file_name + "_out.txt";
	ofstream file_out;
	file_out.open(file_name, ios::out | ios::binary);

	for( i=0; i < output.size(); i++ )
		file_out.write(reinterpret_cast<char*>(&output[i]),sizeof(char));
	file_out.close();
	//---------------------------------------------

	delete []code;
	delete tree;
	return 0;
}
int COMPRESS()
{
//-------------------------讀檔開始---------------------------------
	string file_name;
	cout<<"請輸入檔名 (NAME.txt)：";
	cin>>file_name;

	ifstream file_in;
	file_in.open(file_name.c_str(),ios::binary);
	if(!file_in)
	{
		cout<<"找不到檔案\n";
		return 0;
	}

	int file_size;
	file_in.seekg(0,file_in.end);
	file_size=file_in.tellg();
	file_in.seekg(0,file_in.beg);

	char* read_file=new char[file_size];
	file_in.read(read_file,file_size);
	file_in.close();
//-------------------------讀檔結束---------------------------------

	Show *word_f=new Show[file_size];
	int table_size=HUFFMAN_COUNT(word_f,read_file,file_size); //Counts the character frequencies.

	Node *tree=new Node;
	int tree_high=0;
	tree=HUFFMAN_BUILD_TREE(word_f,table_size,tree_high); //Builds the Huffman coding tree.

	Code *CodeWord=new Code[table_size];
	for(int i=0;i<table_size;i++)
		CodeWord[i].code_word=new char[tree_high+1];
	char *temp=new char[tree_high+1];
	int i=0;
	HUFFMAN_BUILD_CODE(word_f,CodeWord,tree,0,temp,i); //Builds character codewords from the coding tree.
	delete []temp;
	delete []word_f;

	i=-1;
	char *header=new char[(tree_high+9)*table_size+1];
	HUFFMAN_ENCODE_TREE(tree,header,i); //Stores the coding tree in the compressed file.
	int header_length=i+1;
	header[++i]='\0';

	map<char,char*>mapWord;
	for(int j=0;j<table_size;j++)
		mapWord[CodeWord[j].word]=CodeWord[j].code_word;

	i=0;
	char *tail=new char[tree->weight*tree_high+1];
	delete tree;
	HUFFMAN_ENCODE_TEXT(mapWord,tail,read_file,file_size,i); //Encodes the characters in the compressed file.
	delete []read_file;
	int tail_length=i;
	for(int i=0;i<table_size;i++)
		delete []CodeWord[i].code_word;
	delete []CodeWord; 

	int final_length=header_length+tail_length;
	int zero=8-( (final_length%8)==0 ? 8 : final_length%8 );
	final_length+=zero;
	char *temp_final=new char[final_length+1];
	strcpy(temp_final,header);
	strcat(temp_final,tail);
	for(int i=0;i<zero;i++)
		strcat(temp_final,"0");
	delete []header;

	char *final_code=new char[final_length/8+1];
	int p=0;
	for(int j=0;temp_final[j]!='\0';j+=8)
	{
		int count=0;
		int exp=0;
		int k;
		for(k=j+7;k>j;k--)
		{
			if(temp_final[k]=='1')
				count+=(int)pow(2.0,exp);
			exp++;
		}
		final_code[p]=( temp_final[k]=='1'? 0-count : count);
		if(temp_final[k]=='1'&&count==0)
			final_code[p]=128;
		p++;
	}

	delete []temp_final;

//-------------------------寫檔開始---------------------------------
	
	file_name = file_name.substr(0, file_name.find('.'));

	ofstream file_out;
	file_out.open(file_name,ios::binary);

	file_out.write((char*)final_code,final_length/8);
	file_out.close();
//-------------------------寫檔結束---------------------------------
	delete []final_code;

	float final_length_f = final_length, file_size_f = file_size*8;

	cout << "\n壓縮比：" << 100 * (final_length_f/file_size_f) << "%\n\n";

	return 0;
}
int HUFFMAN_COUNT(Show* word_f,char* read_file,int file_size)
{
	int table_size=0;
	word_f[table_size].word=read_file[0];
	word_f[table_size].f=0;
	table_size++;

	for(int i=0;i<file_size;i++)
	{
		bool same_word=false;
		for(int j=0;j<table_size;j++)
		{
			if(read_file[i]==word_f[j].word)
			{
				same_word=true;				
				word_f[j].f++;
				break;
			}
		}
		if(same_word==false)
		{
			word_f[table_size].word=read_file[i];
			word_f[table_size].f=1;
			table_size++;
		}
	}

	word_f[table_size].word='\0';
	word_f[table_size].f=1;
	table_size++;

	return table_size;
}
Node* HUFFMAN_BUILD_TREE(Show* word_f,int table_size,int& tree_high)
{
	Node **tree_node=new Node*[table_size];
	for(int i = 0; i < table_size; i++)
		tree_node[i] = new Node;

	for(int i=0;i<table_size;i++) //每個word都是leaf
	{
		tree_node[i]->label=word_f[i].word;
		tree_node[i]->weight=word_f[i].f;
		tree_node[i]->left=NULL;
		tree_node[i]->right=NULL;
	}

	BUILD_MIN_HEAP(tree_node,table_size); //weight最小的點==root

	Node *new_node;
	while(table_size>1)
	{
		tree_high++;
		new_node=new Node;
		new_node->left=tree_node[0];
		swap(tree_node[0],tree_node[table_size-1]);
		table_size--;
		MIN_HEAPIFY(tree_node,0,table_size);
		new_node->right=tree_node[0];
		new_node->weight=new_node->left->weight+new_node->right->weight;
		swap(tree_node[0],tree_node[table_size-1]);
		tree_node[table_size-1]=new_node;
		MIN_HEAPIFY(tree_node,0,table_size);
	}

	return new_node;
}
void BUILD_MIN_HEAP(Node** tree_node,int heap_size)
{
	for(int i=heap_size/2-1;i>=0;i--)
		MIN_HEAPIFY(tree_node,i,heap_size);
}
void MIN_HEAPIFY(Node** tree_node,int i,int heap_size)
{
	int l=i*2+1;
	int r=l+1;
	int smallest;

	if (l < heap_size && tree_node[l]->weight < tree_node[i]->weight)
		smallest = l;
	else	
		smallest = i;
	if (r < heap_size && tree_node[r]->weight < tree_node[smallest]->weight)
		smallest = r;
	if (smallest != i)
	{
		swap(tree_node[i],tree_node[smallest]);
		MIN_HEAPIFY(tree_node,smallest,heap_size);
	}
}
void HUFFMAN_BUILD_CODE(Show* word_f,Code *CodeWord,Node *tree,int length,char* temp,int& i)
{
	if(tree->left||tree->right)
	{
		temp[length]='0';
		HUFFMAN_BUILD_CODE(word_f,CodeWord,tree->left,length+1,temp,i);
		temp[length]='1';
		HUFFMAN_BUILD_CODE(word_f,CodeWord,tree->right,length+1,temp,i);
	}
	else
	{
		CodeWord[i].word=tree->label;
		for(int j=0;j<length;j++)
			CodeWord[i].code_word[j]=temp[j];
		CodeWord[i].code_word[length]='\0';
		i++;
	}
}
void HUFFMAN_ENCODE_TREE(Node *tree,char *header,int& i)
{
	if(tree->left||tree->right)
	{
		i++;
		header[i]='0';
		HUFFMAN_ENCODE_TREE(tree->left,header,i);
		HUFFMAN_ENCODE_TREE(tree->right,header,i);
	}
	else
	{
		i++;
		header[i]='1';

		if(tree->label=='\0')
		{
			i++;
			header[i]='1';
			i=i+1;
			for(int j=0;j<8;j++,i++)
				header[i]='0';
			i--;
		}
		else
		{
			bool neg=false;
			int a=(int)tree->label;
			if(a<0)
			{
				a=0-a;
				neg=true;
			}
			i=i+9;
			for(int j=0;j<8;j++,i--)
			{
				header[i]=(a%2)+48;//0's ascii
				a/=2;
			}
			if(neg==true)
				header[i]='1';
			else
				header[i]='0';
			i+=8;
		}
	}
}
void HUFFMAN_ENCODE_TEXT(map<char,char*>mapWord,char *tail,char* read_file,int file_size,int& i)
{
	for(int j=0;j<file_size&&read_file[j]!='\0';j++)
	{
		for(int k=0;mapWord[read_file[j]][k]!='\0';k++,i++)
			tail[i]=mapWord[read_file[j]][k];
	}
	for(int k=0;mapWord['\0'][k]!='\0';k++,i++)
		tail[i]=mapWord['\0'][k];
	tail[i]='\0';
}
void HUFFMAN_REBUILD_TREE(char* read_file,Node *tree,char *code,int& i)
{
	if(code[i]=='1')
	{
		tree->left=NULL;
		tree->right=NULL;

		int count=0;
		int exp=0;
		int k;
		for(k=i+9;k>i+1;k--)
		{
			if(code[k]=='1')
				count+=(int)pow(2.0,exp);
			exp++;
		}
		if(code[k]=='1'&&count==0)
			count=-128;
		else if(code[k]=='1')
			count=0-count;
		if(count==-128)
			tree->label='\0';
		else
			tree->label=count;
		i+=9;
	}
	else
	{
		Node* new_node=new Node;
		tree->left=new_node;
		i++;
		HUFFMAN_REBUILD_TREE(read_file,new_node,code,i);
		new_node=new Node;
		tree->right=new_node;
		i++;
		HUFFMAN_REBUILD_TREE(read_file,new_node,code,i);
	}
}
void HUFFMAN_DECODE_TEXT(string& output,char* code,Node *tree,int& i)
{
	Node* t=tree;
	while(t->label!='\0')
	{
		if(!(t->left||t->right))
		{
			output.push_back(t->label);
			t=tree;
		}
		else
		{
			if(code[i]=='1')
				t=t->right;
			else
				t=t->left;
			i++;
		}
	}
}
