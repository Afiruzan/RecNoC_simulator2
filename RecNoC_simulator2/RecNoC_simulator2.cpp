// RecNoC_simulator2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
//########################################################################
//RecNoC simulator by Arash Firuzan (arash.firuzan@gmail.com) _ 2017
//Under supervision of Dr. Mehdi Modarressi (modarressi@ut.ac.ir)
//Arbitration of each router is priority-based of ports
//########################################################################

#include<stdio.h>
#include<iostream>
#include <new>
#include<fstream>
#include<string>
#include<typeinfo>
#include <vector>
using namespace std;
//---------------------------------------------------------------------------------------------------------------
//Inputs of code
//Please set the flit data at line 400 of this code
#define buffer_size 4
int cluster_size = 1;
int num_of_corridors = 0;
const int networkx = 2; //networkx=networky
const int networky = 2;
const int networkz = 1;
int simulation_time = 100;//simulation time by cycle unit
int number_of_elements_in_x_direction = networkx + (((networkx/cluster_size) - 1)*num_of_corridors);
int number_of_elements_in_y_direction = networky + (((networky/cluster_size) - 1)*num_of_corridors);
ofstream myfile("Result.txt");
//ofstream myfile2("Result2.txt");
//---------------------------------------------------------------------------------------------------------------
//class definitions is here
class flit
{
public:
	int number;
	int x_dest;
	int y_dest;
	int data;
	int time; //time which flit comes to cluster-based RecNoC and reached to its destination
};

class inlink
{
public:
	flit f;
	bool is_full;
};

class outport
{
public:
	flit f;
	bool is_full;
	int empty_buffer_slots_of_next_router=buffer_size;//Credit-based flow control ***************************************newly added
	bool empty_buffer_slots_of_next_router_is_received=0;
	bool arbiter_array[7];//arbiter array
	outport()// constructor for initializing arbiter_array;
	{
		for (int i = 0; i < 7; i++)
		{
			arbiter_array[i] = 0;
		}
	}
};

class Queue //buffer is a circular queue
{
public:
	bool credit;//for credit-based flowcontrol
	bool credit_is_received = 0;
	flit f[buffer_size];
	int front, rear;
	Queue(){
		front = -1;
		rear = -1;
	}

	bool isFull(){
		if (front == 0 && rear == buffer_size - 1){
			return true;
		}
		if (front == rear + 1) {
			return true;
		}
		return false;
	}

	bool isEmpty(){
		if (front == -1) return true;
		else return false;
	}

	void enQueue(flit f2){
		if (isFull()){
			cout << "Buffer is full\n";
		}
		else {
			if (front == -1) front = 0;
			rear = (rear + 1) % buffer_size;
			f[rear] = f2;
			//cout << endl << "Inserted Flit " << f2.number << endl;
		}
	}

	flit deQueue(){
		flit f2;
		//flit errorflit;
		//errorflit.number = -1;
		if (isEmpty()){
			//cout << "Queue is empty" << endl;
			//return(errorflit);
		}
		else {
			f2 = f[front];
			if (front == rear){
				front = -1;
				rear = -1;
			} /* Q has only one element, so we reset the queue after deleting it. */
			else {
				front = (front + 1) % buffer_size;
			}
			return(f2);
		}
	}
	void display()
	{
		myfile << "\n";
		/* Function to display status of Circular Queue */
		int i;
		if (isEmpty()) {
			myfile << endl << "Empty Queue" << endl;
		}
		else
		{
			myfile << " ++++++++++++++ Index of Front -> " << front;
			myfile << endl << " ++++++++++++++ Items -> ";
			for (i = front; i != rear; i = (i + 1) % buffer_size)
			{
				myfile << f[i].number<<" , ";
			}
			myfile << f[i].number;
			myfile << endl << " ++++++++++++++ Index of Rear -> " << rear<<"\n";
		}
	}
};

class inport
{
public:
	//flit f[buffer_size];//buffersize=10
	flit inport_of_recswitch; //because inport of recswitch does not have buffer
	bool is_full_for_recswitch;
	Queue buffer;
//	bool is_full;//if is_full=1 means that buffer is full
	//bool buffer_full, buffer_empty;//if buffer is full then buffer_full is 1 and if buffer is empty then buffer_empty is 1;
	//flit buffer_read(); //this function read a flit from buffer
	//int buffer_write(flit f2 );//this function write flit f2 into buffer
	flit buffer_display();
	//int write_pointer=-1;// equal to rear initializing write pointer;
	//int read_pointer=-1;// equal to front initializing read pointer
	void buffer_read_increase_time();
	bool grant;//for arbitration section
};

flit inport::buffer_display()//this function returns front flit of queue
{
	flit result;
	int front2;
	front2 = buffer.front;
	result = buffer.f[front2];
	return result;
}


void inport::buffer_read_increase_time()
{
	int front3;
	front3 = buffer.front;
	buffer.f[front3].time++;
}



class element //an element can be router or reconfigurable switch
{
public:
	int id;
	int x;
	int y;
	int z;
	int router; //if element been router then router is 1
	inport inport_number[20]; // an array for easily usage of inports
	outport outport_number[20];  // an array for easily usage of outports
	inlink inlink_number[8];
	int crossbar_in_recswitch[4][4]; //which input of recswitch is connected to which output using a 4 * 4 matrix
	element(); //constructor for recswitch matrix
	/*
	port N_in; //1
	port W_in; //2
	port S_in; //3
	port E_in;  //4
	port N_out; //5
	port W_out; //6
	port S_out; //7
	port E_out; //8
	port PE_in; //9
	port PE_out; //10
	port TSV_UP_in; //11
	port TSV_UP_out; //12
	port TSV_DOWN_in;//13
	port TSV_DOWN_out;//14
	inlink PE_inlink; //1
	inlink N_inlink;  //2
	inlink W_inlink; //3
	inlink S_inlink; //4
	inlink E_inlink; //5
    */
	
	//routingfunction();
	//which routing function must we use for this porpuse;
};
element::element() //constructor for recswitch matrix, by default North connected to south, south connected to north, east is connected to west and west is connected to east.
{
	crossbar_in_recswitch[0][1] = 1;
	crossbar_in_recswitch[1][0] = 1;
	crossbar_in_recswitch[2][3] = 1;
	crossbar_in_recswitch[3][2] = 1;
	if (router == 0) ///////*****************************************************must be completed
	{
		
	}
}
//End of class definitions
//------------------------------------------------------------------------------------------------------------------------
//Required functions definition
void place_router(element(&net)[100][100][2], int &x, int y, int z, int &routercounter_in_x_dimension)
{
	net[x][y][z].router =1;
	//cout <<"&net=" <<&net<<"\n";
	x++;
	//cout << "x in place router="<<x<<" and y is "<<y<<"\n";
	routercounter_in_x_dimension++;
}
void place_recswitch(element(&net)[100][100][2], int &x, int y, int z)
{
	net[x][y][z].router = 0;
	x++;
}

void x_placement_router(element(&net)[100][100][2], int &x, int y, int z)
{
	x = 1;
	int routercounter_in_x_dimension = 0;
	here2:
	for (int i = 1; i < cluster_size + 1; i++)
	{
		place_router(net, x, y, z, routercounter_in_x_dimension);
		//cout << "x in x_placement_router=" << x<<" and y is "<<y<<"\n";
		
	}
	//cout << "routercounter_in_x_dimension in x_placement_router is" << routercounter_in_x_dimension << "\n";
	if (routercounter_in_x_dimension < networkx)
	{
		for (int j = 1; j < num_of_corridors+1; j++)
		{
			place_recswitch(net,x, y, z);
			//cout << "x after place recswitch = " << x<<"\n";
		}
	}
	else goto here3;
	goto here2;
here3:;
}

void x_placement_recswitch(element(&net)[100][100][2], int &x, int y, int z)
{
	x = 1;
	//cout <<"*x="<< x<<"\n";
	for (int i = 1; i < ((((networkx / cluster_size) - 1)*num_of_corridors) + networkx); i++)
	{
		place_recswitch(net, x, y, z);
	}
}

int xy_routing_function(flit f, int j, int k, int l, element(&net)[100][100][2])//**********************************must be completed
{
	int outputport;
	int x_offset,y_offset;
	x_offset = f.x_dest - j;
	y_offset = f.y_dest - k;
	if (x_offset < 0)
	{
		outputport = 2;
	}
	if (x_offset > 0)
	{
		outputport = 4;
	}
	if ((x_offset == 0) && (y_offset<0))
	{
		outputport = 3;
	}
	if ((x_offset == 0) && (y_offset>0))
	{
		outputport = 1;
	}
	if ((x_offset == 0) && (y_offset==0))
	{
		outputport = 5;
	}
	return outputport;
}

int j_at_next_router(int j, int k, int l, element(&net)[100][100][2], int u)//computes location of next router in x_dimension
{
	int result;
	if (u == 4)
	{
		result = j + 1;
		return result;
	}
	if (u == 2)
	{
		result = j - 1;
		return result;
	}
	else return j;
}

int k_at_next_router(int j, int k, int l, element(&net)[100][100][2], int u)//computes location of next router in y_dimension
{
	int result;
	if (u== 1)
	{
		result=k+1;
		return result;
	}
	if (u ==3)
	{
		result= k - 1;
		return result;
	}//************************************************************must be completed
	else return k;
}

int inlinknumber_computer_for_neighbor_element(int u)//converts out port of this router to inlink of neighbor router
{
	int result;
	if (u == 1)//out port of this router//******************************** 3D must be completed
	{
		result = 4;//inlink of neighbor router
	}
	if (u==2)//out port of this router
	{
		result = 5;//inlink of neighbor router
	}
	if (u==3)//out port of this router
	{
		result = 2;//inlink of neighbor router
	}
	if (u==4)//out port of this router
	{
		result = 3;//inlink of neighbor router
	}
	if (u==6)//out port of this router
	{
		result = 0;//inlink of neighbor router
	}
	return result;
}

int inlink_to_inport_computer(int i)
{
	int result;
	if (i == 1)
	{
		result = 5;
	}
	if (i == 2)
	{
		result = 1;
	}
	if (i == 3)
	{
		result = 2;
	}
	if (i == 4)
	{
		result = 3;
	}
	if (i == 5)
	{
		result = 4;
	}
	return result;
}

int y_computer(int i, int j)//crossbar matrix for inputs of recswitch
{
	int result[4][4] = {
		{ 1, 1, 1, 1 },
		{ 3, 3, 3, 3 },
		{ 4, 4, 4, 4 },
		{ 2, 2, 2, 2 }

	};
	return result[i][j];
}

int k_computer(int i, int j)//crossbar matrix for outputs of recswitch
{
	int result[4][4] ={
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 }

	};
	return result[i][j];
}

string u_to_inport_name(int u)//This function convert inport number to inport name for best readability
{
	string result[10] = { "Error occured", "North_in", "West_in", "South_in", "East_in",  "PE_in",  "TSV_UP_in",  "TSV_DOWN_in" };
	return result[u];
}

string u_to_outport_name(int u)//This function convert outport number to outport name for best readability
{
	string result[10] = { "Error occured", "North_out", "West_out", "Sout_out", "East_out", "PE_out", "TSV_UP_out", "TSV_DOWN_out" };
	return result[u];
}

int inport_to_neighbor_outport_number_computer_backpressure(int y1)//******************************must be completed for 3D
{
	int result;
	if (y1 == 1)
	{
		result = 3;//outport S-out
			return result;
	}
	if (y1 == 2)
	{
		result = 4;//outport E-out
		return result;
	}
	if (y1 ==3)
	{
		result = 1;//outport N-out
		return result;
	}
	if (y1 == 4)
	{
		result = 2;//outport W-out
		return result;
	}
}

int input_to_outport_computer(int u)
{
	int result[8] = { 100, 3, 4, 1, 2 };////*******************************3D must be completed
	return result[u];
}

int buffer_backpressure_outportnumber_computer(int u,element net[100][100][2],int j,int k, int l)//This function gives outport of this recswitch and then computes output port of neghibor element in credit_based back pressure buffer mechanism
{
	int y1, k1, i1, j1,result;
	//for (int u = 1; u < 8; u++)// for all output ports of recswitch//////*******************************3D must be completed
	//{
		for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
		{
			for (int j1 = 0; j1 < 4; j1++)
			{
				if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
				{
					int y1, k2;
					y1 = y_computer(i1, j1); //y= input port
					k2 = k_computer(i1, j1); //k=output port
					if (u == k2)
					{
						result = inport_to_neighbor_outport_number_computer_backpressure(y1);
					}
				}
			}
		}
	//}
	return result;
}

int x_of_neighbor_element_in_backpressure(int u,element net[100][100][2], int j, int k, int l)//This function gives outport of this recswitch as input, and then computes x of neighbor element in credit_based back pressure buffer mechanism
{
	//for (int u = 1; u < 8; u++)// for all output ports of recswitch//////*******************************3D must be completed
	//{
		for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
		{
			for (int j1 = 0; j1 < 4; j1++)
			{
				if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
				{

					int y1, k2;
					y1 = y_computer(i1, j1); //y= input port
					k2 = k_computer(i1, j1); //k=output port
					if (u == k2)
					{
						if (y1 == 1)
						{
							k++;
						}
						if (y1 == 2)
						{
							j--;
						}
						if (y1 == 3)
						{
							k--;
						}
						if (y1 == 4)
						{
							j++;
						}
					}/////*******************************3D must be completed
					
				}
			}
		}
//}
	return j;
			
}
int y_of_neighbor_element_in_backpressure(int u,element net[100][100][2], int j, int k, int l)//This function gives outport of this recswitch as input, and then computes y of neighbor element in credit_based back pressure buffer mechanism
{
	//for (int u = 1; u < 8; u++)// for all output ports of recswitch//////*******************************3D must be completed
	//{
		for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
		{
			for (int j1 = 0; j1 < 4; j1++)
			{
				if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
				{

					int y1, k2;
					y1 = y_computer(i1, j1); //y= input port
					k2 = k_computer(i1, j1); //k=output port
					if (u == k2)
					{
						if (y1 == 1)
						{
							k++;
						}
						if (y1 == 2)
						{
							j--;
						}
						if (y1 == 3)
						{
							k--;
						}
						if (y1 == 4)
						{
							j++;
						}
					}/////*******************************3D must be completed
				}
			}
		}
	//}
	return k;
}

int z_of_neighbor_element_in_backpressure(int u,element net[100][100][2], int j, int k, int l)//////*******************************3D must be completed
{
	
	return 1; ////*******************************3D must be completed
}

int x_of_neighbor_element(int u, element net[100][100][2], int j, int k, int l)
{
	int result;
	if ((u == 1) || (u==3))
	{
		result = j;
	}
	if (u==2)
	{
		result = j-1;
	}
	if (u == 4)
	{
		result = j + 1;
	}
	return result;///*******************************3D must be completed
}

int y_of_neighbor_element(int u, element net[100][100][2], int j, int k, int l)
{
	int result;
	if ((u == 2) || (u == 4))
	{
		result = k;
	}
	if (u == 1)
	{
		result = k + 1;
	}
	if (u == 3)
	{
		result = k + 1;
	}
	return result;///*******************************3D must be completed
}

int z_of_neighbor_element(int u, element net[100][100][2], int j, int k, int l)
{
	return 1;//*******************************3D must be completed

}

int buffer_backpressure_inportnumber_to_outport_number_of_neighbor_element_computer(int u, element net[100][100][2], int j, int k, int l)
{
	int result[5] = { 100, 3, 4, 1, 2 };/////*******************************3D must be completed
	return result[u];
}

int what_is_inport(int u, element net[100][100][2], int j, int k, int l)//This function gives outputport of a recswitch and then computes related inport of that recswitch in Credit_based backpressure mechanism
{
	int result;
	for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
	{
		for (int j1 = 0; j1 < 4; j1++)
		{
			if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
			{

				int y1, k2;
				y1 = y_computer(i1, j1); //y= input port
				k2 = k_computer(i1, j1); //k=output port
				if (k2 == u)
				{
					result = y1;
					return result;
				}
			}
		}
	}
}

int buffer_backpressure_inportnumber_computer(int u, element net[100][100][2], int j, int k, int l)//This function gives input of a recswitch and then computes the input of neighbor recswitch in backpressure Credit_based mechanism
{
	int result;
	result=what_is_inport(buffer_backpressure_inportnumber_to_outport_number_of_neighbor_element_computer(u,net,j,k,l),net,j,k,l);
	return result;
}

/*void send(int in, int out, int j, int k, int l, element(&net)[100][100][100])
{
	net[j][k][l].port_number[in].is_full = 0; //send internal to an output port
	net[j][k][l].port_number[10].is_full = 1;
	net[j][k][l].port_number[10].f = net[j][k][l].port_number[4].f;
}*/

int outport_arbiter_function(bool arbitration_array[5])//priority Arbiter function: in this arbiter For an example, The port one have priority to port two
{
	//priority arbiter
	int checker=0;
	for (int i = 1; i < 6; i++)//For all input ports************************************ 3D must be completed
	{
		if (arbitration_array[i] == 1)//For an example, The port one have priority to port two
		{
			return i;
			checker++;
		}
	}
	if (checker == 0)
		return 7;
}

//End of required functions definitions
//##########################################################################################################################
void main()
{
	cout << "number_of_elements_in_x_direction= " << number_of_elements_in_x_direction << "\n";
	cout << "number_of_elements_in_y_direction= " << number_of_elements_in_y_direction << "\n";

	element net[100][100][2]; //creates a RecNoC
	for (int i = 0; i < number_of_elements_in_x_direction + 1; i++)
	{
		for (int j = 0; j < number_of_elements_in_y_direction + 1; j++)
		{
			for (int k = 0; k < (networkz); k++)
			{
				net[i][j][k].router = 0;
			}
		}
	}
	//###################################################################################################################################
	//Cluster-based RecNoC constructor. This section of code initialize net[j][k][l] array as a recswitch.
	//cout <<"&net"<< &net << "\n";
	for (int z = 1; z < (networkz + 1); z++)
	{
		int routercounter_in_y_dimension = 0;
		//for (int y = 1; y < (networkx + (((networkx / cluster_size) - 1)* num_of_corridors)) + 1; y++)
		//{
		int x = 1;
		int y = 1;
	here4:
		for (int i = 1; i < cluster_size + 1; i++)
		{
			x_placement_router(net, x, y, z); //place router in this line
			routercounter_in_y_dimension++;
			y++;
			//cout << net[x-1][y][z].router << "  ok\n";
			//cout << ">>>> x after running x_placement_router =" << x<<" when y is "<<y<<"\n";
			//cout << "routercounter_in_y_dimension after x_placement_router is " << routercounter_in_y_dimension << "\n";
		}
		if (routercounter_in_y_dimension < networky)
		{
			for (int i = 1; i < (num_of_corridors + 1); i++)
			{
				x_placement_recswitch(net, x, y, z);
				y++;// this line contains all recswitch

			}
		}
		else goto here;
		goto here4;
	here:;
		//}
	}
	//cout << net[1][1][1].router<<"\n";
	//End of Cluster-based RecNoC constructor
	//###################################################################################################################################
	//for test the program and writing result in output txt file
	myfile << "Cluster-based RecNoC result:\n\n";
	for (int i = 1; i < (networkz + 1); i++)
	{
		for (int j = 1; j < number_of_elements_in_y_direction + 1; j++)
		{
			for (int k = 1; k < number_of_elements_in_x_direction + 1; k++)
			{
				//cout << &net << " final2\n\n";
				if (net[k][j][i].router == 1)
				{
					myfile << "R=1 (z, y, x)= " << i << "	" << j << "	  " << k << "      \n";
					//cout << "(i, j, k)=" << i << "	" << j << "	  " << k << "      \n";
				}
				else
				{
					myfile << "R=0 (z, y, x)= " << i << "	" << j << "	  " << k << "      \n";
					//cout << "(i, j, k)=" << i << "	" << j << "	  " << k << "      \n";
				}
			}
		}
	}
	//Display shape of net on output by * and . ( *=router && .=recswitch )
	myfile << "\nshape of RecNoC:\n\n";
	char net2[20][20];
	for (int i = 1; i < number_of_elements_in_y_direction + 1; i++)
	{
		for (int j = 1; j < number_of_elements_in_x_direction + 1; j++)
		{
			if (net[j][i][1].router == 1)
			{
				cout << " * ";
				myfile << " * ";
			}
			else
			{
				cout << " . ";
				myfile << " . ";
			}
		}
		cout << "\n";
		myfile << "\n";
	}
	//End of shape of net
	//##################################################################################################################################
	// traffic manager
	myfile << "\n\n#################################################################\n\ntraffic manager result:\n\n";
	//information of input flits is in below:
	flit f1, f2,f3,f4;
	int number_of_flits =4;
	f1.number = 1;
	f2.number = 2;
	f3.number = 3;
	f4.number = 4;
	
	f1.time = 0;//the time which flit is injected into network
	f2.time = 0;//the time which flit is injected into network
	f3.time = 0;
	f4.time = 0;

	//Destination of flits
	f1.x_dest = 2;
	f1.y_dest = 1;

	f2.x_dest = 2;
	f2.y_dest = 1;
	
	f3.x_dest = 2;
	f3.y_dest = 1;

	f4.x_dest = 2;
	f4.y_dest = 1;
	//injection location of flits into PE input ports
	net[2][2][1].inport_number[5].buffer.enQueue(f1);
	net[2][2][1].inport_number[4].buffer.enQueue(f2);
	net[2][2][1].inport_number[3].buffer.enQueue(f3);
	net[2][2][1].inport_number[2].buffer.enQueue(f4);
	
	//modified recswitch matrix
	/*net[2][3][1].crossbar_in_recswitch[3][2] = 0;
	net[2][3][1].crossbar_in_recswitch[3][1] = 1;
	net[2][3][1].crossbar_in_recswitch[0][1] = 0;

	net[2][1][1].crossbar_in_recswitch[0][1] = 0;
	net[2][1][1].crossbar_in_recswitch[0][2] = 1;
	net[2][1][1].crossbar_in_recswitch[2][3] = 1;
	net[2][1][1].crossbar_in_recswitch[3][2] = 1;*/
	//--------------------------------------------------------------------------------
	cout << "\n" << "simulation duration = " << simulation_time << "\n";
	for (int i = 1; i < simulation_time; i++) // i does not show clock cycle, it is only a loop variable
	{
		//net[3][1][1].inport_number[2].buffer.enQueue(f2);//??????????????????????????????????????????????????????????????????????????????
		for (int j = 1; j < number_of_elements_in_x_direction + 1; j++) ///////////////
		{
			for (int k = 1; k < number_of_elements_in_y_direction + 1; k++) /////////// for all routers
			{
				for (int l = 1; l < (networkz + 1); l++) //////////////////////////////
				{
					if (net[j][k][l].router == 1)//////////////////////////////////////
					{
						//--------------------------------------------------------------------------------
						//initialization of arbiter_array of all outports to zero
						for (int u = 1; u < 6; u++)//for all output ports
						{
							for (int y = 1; y < 6; y++)//for all arbiter_array elemnets
							{
								net[j][k][l].outport_number[u].arbiter_array[y] = 0;//set all arbitration_arrary of all outports to zero
							}
						}
						//End of initialization of arbiter_array of all outportd to zero
						//--------------------------------------------------------------------------------
						//--------------------------------------------------------------------------------
						for (int u = 1; u < 8; u++)//for all out ports if there is a flit send outport flit to in-link of neighbor element
						{
							if ((u == 5) && (net[j][k][l].outport_number[u].is_full == 1))/// if flit reached to its destination
							{
								myfile << ">>>>>>>>> flit " << net[j][k][l].outport_number[u].f.number << " reached to its destination after cycle " << net[j][k][l].outport_number[u].f.time << "\n\n";
								//net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router--;//Credit_based flow control
								goto thisplace;//do nothing and go to next output port
							}
							if (net[j][k][l].outport_number[u].is_full == 1) //for all out ports if there is a flit send outport flit to in-link of neighbor element
							{
								int j1, k1, l1, inlinknumber;
								j1 = j_at_next_router(j, k, l, net, u);//x of neighbor element ************************************ 3D must be completed
								k1 = k_at_next_router(j, k, l, net, u);//y of neighbor element
								inlinknumber = inlinknumber_computer_for_neighbor_element(u);
								//net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router++;//Credit_based flow control
								net[j][k][l].outport_number[u].f.time++; //This operation requires one cycle
								net[j][k][l].outport_number[u].is_full = 0; //send outport flit to in-link of neighbor element
								net[j1][k1][l].inlink_number[inlinknumber].is_full = 1;/////*************************************************************
								net[j1][k1][l].inlink_number[inlinknumber].f = net[j][k][l].outport_number[u].f;
								net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router--;//Credit_based flow control
								cout << "&&&&&&& " << "i= " << i << "j= " << j << " " << k << " " << l << " " << u << " \n";
								cout << "empty_buffer_slots_of_next_router = " << net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router << "\n";
								for (int t = 1; t < number_of_flits + 1; t++)///////*****************************************************must be corrected
								{
									if (net[j][k][l].outport_number[u].f.number == t)
									{
										myfile << "\n At cycle " << net[j][k][l].outport_number[u].f.time << " flit " << net[j][k][l].outport_number[u].f.number << " is in   router ( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
									}
								}
							}
						thisplace:;
						}
						//--------------------------------------------------------------------------------
						//routing function implemented here
						// for all input ports we must done routing
						for (int u = 1; u < 6; u++)// for all input ports send flit of input port to an output port of own which decided by routing function************************************ 3D must be completed
						{
							if (net[j][k][l].inport_number[u].buffer.isEmpty() != 1) //If buffer of this input port is not empty
							{
								int outport;
								outport = xy_routing_function(net[j][k][l].inport_number[u].buffer_display(), j, k, l, net);//outport is computed by routing function
								int j1, k1, l1, inlinknumber;
								j1 = j_at_next_router(j, k, l, net, outport);//x of forward neighbor element which computed by routing. we use j1 & k1 for credit ************************************ 3D must be completed
								k1 = k_at_next_router(j, k, l, net, outport);//y of forward neighbor element which computed by routing. we use j1 & k1 for credit ************************************ 3D must be completed
								net[j][k][l].outport_number[outport].arbiter_array[u] = 1;//Arbitration Request sent to outport
								if (outport == 5)//if flit must send to PE_out do nothing
								{
									goto thislocation;
									net[j][k][l].inport_number[u].buffer.credit = 1;//because buffer is now have at least one empty place
								}
								inlinknumber = inlinknumber_computer_for_neighbor_element(outport);//this function computes inlink number in neighbor element
								if (((net[j1][k1][l].router == 0) && ((net[j][k][l].outport_number[outport].empty_buffer_slots_of_next_router_is_received == 1) || (net[j1][k1][l].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit_is_received == 1))) || (net[j1][k1][l].router != 0))//******************************* 3D must be completed
								{
									if ((net[j1][k1][l].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit == 1) || (net[j][k][l].outport_number[outport].empty_buffer_slots_of_next_router != 0))//we can send flit to outport//Credit-based flow control
									{
										net[j][k][l].inport_number[u].buffer_read_increase_time();//This operation requires one cycle and flit time must be added by one
										//net[j][k][l].inport_number[u].is_full = 0; //send flit of input port to an output port of own which decided by routing function
										//myfile << "\n At cycle"
										myfile << "\n\n ++++++++++++++ Buffer of input port " << u_to_inport_name(u) << " in   router(" << j << " " << k << " " << l << ")\n";
										net[j][k][l].inport_number[u].buffer.display();
										myfile << " \n\n";
										if (net[j][k][l].inport_number[u].grant == 1)//if grant=1 then send input to output which computed by routing function 
										{
											net[j][k][l].outport_number[outport].f = net[j][k][l].inport_number[u].buffer.deQueue();
											net[j][k][l].outport_number[outport].is_full = 1;
										}
										//cout << "\nbefore" << net[j][k][l].inport_number[u].buffer_display().number<<"\n";
										//++++++++++++++++++++++++++++++++++++++++++++++
										//Below lines are arbitration:
										if (u == 5)//if we reach to last outport then we can run arbitraton function
										{
											int fz;
											for (int t = 1; t < 6; t++)//For all outports we have arbitration
											{
												//if (net[j][k][l].outport_number[t].arbiter_array[d]==1)
												fz=outport_arbiter_function(net[j][k][l].outport_number[t].arbiter_array);//fz is index of winner inport. winner inport will send to outport
												myfile << "\nfz= "<<fz<<"\n";
												if (fz == 7)//if fz=7 it means that the arbitration array does not have a one in its elements
													goto thisplace2;
												if (fz == 6)//for Debugging
													goto thisloc;
												//increasing the time for those who losed in arbitration phase
												for (int m = 1; m < 6; m++)// For all elements of arbiter_array
												{
													if ((net[j][k][l].outport_number[t].arbiter_array[m] == 1) && (fz != m))//if inport sent request for outport arbiter and lose the aritraion then increadse flit time by one
													{
														net[j][k][l].inport_number[m].buffer_read_increase_time();//increase time for losers
														cout << " \n at cycle " << net[j][k][l].inport_number[m].buffer_display().time << " flit " << net[j][k][l].inport_number[m].buffer_display().number << " losed in arbitration " << " at router router ( " << j << " " << k << " " << l << " )\n";
													}
												}
												net[j][k][l].outport_number[t].f = net[j][k][l].inport_number[fz].buffer.deQueue();//put winner in outport of this router
											thisplace2:;
											}
										}
										thisloc:
										//End of arbitration
										//cout << "\nafter" << net[j][k][l].inport_number[u].buffer_display().number<<"\n";
										//net[j][k][l].outport_number[outport].empty_buffer_slots_of_next_router--;
										//Below lines for sending credit to backward router
										int jj, kk, ll, f;
										jj = x_of_neighbor_element_in_backpressure(u, net, j, k, l); //x_of_neighbor_element_in_backpressure
										kk = y_of_neighbor_element_in_backpressure(u, net, j, k, l); //y_of_neighbor_element_in_backpressure
										ll = z_of_neighbor_element_in_backpressure(u, net, j, k, l); //z_of_neighbor_element_in_backpressure
										f = inport_to_neighbor_outport_number_computer_backpressure(u);
										if (u == 5)//if u==PE_in we does not need to reduce empty_buffer_slots_of_next_router by one
										{
											goto thislocation;
										}
										net[jj][kk][ll].outport_number[f].empty_buffer_slots_of_next_router--;//
										if (net[j1][k1][l].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit == 1)//If credit of next router input port is one, credit must change to zero, it means that we receive that credit and do related works, because if credit of next router input port remains one it means buffer is not full.
										{
											net[j1][k1][l].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit = 0;
											/*if (net[j1][k1][l].router == 1)
											{
											net[j1][k1][l].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit_is_received = 1;
											}*/
										}
										myfile << "\n routing: At cycle " << net[j][k][l].outport_number[outport].f.time << " flit " << net[j][k][l].outport_number[outport].f.number << " is in   router ( " << j << " " << k << " " << l << " ) at port[" << u_to_inport_name(u) << "]\n\n";
									}
									else //cannot send flit
									{
										net[j][k][l].inport_number[u].buffer_read_increase_time();//even if we cannot send flit we must increase time
										myfile << "\n At cycle " << net[j][k][l].inport_number[u].buffer_display().time << " cannot send flit " << net[j][k][l].inport_number[u].buffer_display().number << " into router ( " << j << " " << k << " " << l << " ) at port[" << u_to_inport_name(u) << "]\nBecause there are " << net[j][k][l].outport_number[outport].empty_buffer_slots_of_next_router << " empty buffer slot at next router\n";;
									}
								}
								if (net[j][k][l].inport_number[u].buffer.isFull() != 1)//If there is at least one empty buffer slot at this router send a credit to backpressure router //Credit-based flow control
								{
									int jj, kk, ll, f;
									jj = x_of_neighbor_element_in_backpressure(u, net, j, k, l);
									kk = y_of_neighbor_element_in_backpressure(u, net, j, k, l);
									ll = z_of_neighbor_element_in_backpressure(u, net, j, k, l);
									net[j][k][l].inport_number[u].buffer.credit = 1;//send a credit to backpressure router
									if (net[jj][kk][ll].router == 1)// if backpressure element is a router, then change credit_is_received to one.
									{
										net[jj][kk][ll].inport_number[inlink_to_inport_computer(inlinknumber)].buffer.credit_is_received = 1;
									}
								}
								/*for (int i = net[j][k][l].inport_number[u].buffer.rear; i < (net[j][k][l].inport_number[u].buffer.front)+1; i++)
								{
								myfile << "\n At cycle " << net[j][k][l].inport_number[u].buffer.f[i].time << " flit " << net[j][k][l].inport_number[u].buffer.f[i].number << " is in   router ( " << j << " " << k << " " << l << " ) at buffer[" << i << "] " << "at port[" << u_to_inport_name(u) << "]\n\n";
								}*/
							}
						thislocation:;
						}
						////--------------------------------------------------------------------------------
						//arbitration



						////--------------------------------------------------------------------------------
						for (int i = 1; i < 6; i++)// for all inlinks copy inlink to inport of that element
						{
							if (net[j][k][l].inlink_number[i].is_full == 1)
							{
								int inport_number;
								inport_number = inlink_to_inport_computer(i);
								//below lines are vc allocation


								net[j][k][l].inport_number[inport_number].buffer.enQueue(net[j][k][l].inlink_number[i].f);
								net[j][k][l].inlink_number[i].is_full = 0;
								myfile << "\n\n ++++++++++++++ Buffer of input port " << u_to_inport_name(inport_number) << " in   router(" << j << " " << k << " " << l << ")\n";
								net[j][k][l].inport_number[inport_number].buffer.display();
								myfile << " \n\n";
							}
						}
					}
					//--------------------------------------------------------------------------------
					//--------------------------------------------------------------------------------
					//code for recswitches					
					else
					{
						for (int z = 1; z < simulation_time; z++)//flit traversal on recswitches is real-time vs flit traversal in routers
						{
							// ******************************************************
							//// for all inlinks of recswitch copy inlink to inport of that recswitch
							for (int i = 1; i < 6; i++)//for all inlinks
							{
								if (net[j][k][l].inlink_number[i].is_full == 1)
								{
									int inport_number;
									inport_number = inlink_to_inport_computer(i);
									net[j][k][l].inport_number[inport_number].inport_of_recswitch = net[j][k][l].inlink_number[i].f;//????????????????????????????????
									net[j][k][l].inlink_number[i].is_full = 0;
									net[j][k][l].inport_number[inport_number].is_full_for_recswitch = 1;
								}
							}
							// *********************************************************************************
							// copy inport of recswitch to outport of that recswitch
							for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
							{
								for (int j1 = 0; j1 < 4; j1++)
								{
									if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
									{
										int y1, k2;
										y1 = y_computer(i1, j1); //y= input port
										k2 = k_computer(i1, j1); //k=output port
										if (net[j][k][l].inport_number[y1].is_full_for_recswitch == 1)
										{
											net[j][k][l].outport_number[k2].f = net[j][k][l].inport_number[y1].inport_of_recswitch;
											net[j][k][l].inport_number[y1].is_full_for_recswitch = 0;
											net[j][k][l].outport_number[k2].is_full = 1;
										}
									}
								}
							}
							//// ******************************************************
							// for all out ports if there is a flit send out to in-link of neighbor element
							for (int u = 1; u < 5; u++)// for all output ports//////*******************************
							{
								if (net[j][k][l].outport_number[u].is_full == 1) //for all out ports if there is a flit
								{
									int j1, k1, l1, inlinknumber;
									j1 = j_at_next_router(j, k, l, net, u);
									k1 = k_at_next_router(j, k, l, net, u);
									inlinknumber = inlinknumber_computer_for_neighbor_element(u);
									net[j][k][l].outport_number[u].is_full = 0; //send out to in-link of neighbor element
									net[j1][k1][l].inlink_number[inlinknumber].is_full = 1;/////*************************************************************
									net[j1][k1][l].inlink_number[inlinknumber].f = net[j][k][l].outport_number[u].f;
									/*if (net[j1][k1][l].router == 1)
									{
										goto here7;
									}*/
									for (int t = 1; t < number_of_flits + 1; t++)///////*****************************************************must be corrected
									{
										if (net[j][k][l].outport_number[u].f.number == t)
										{
											myfile << "\n At cycle " << net[j][k][l].outport_number[u].f.time << " flit " << net[j][k][l].outport_number[u].f.number << " is in recswitch( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
										}
									}
								}
							}
						//}
//					here7:;
						//for (int i = 1; i < simulation_time; i++)
						//{
							//// ******************************************************
							//credit_based buffer backpressure on recswitch output ports for transmiiting empty_buffer_slots_of_next_router
							for (int u = 1; u < 5; u++)// for all output ports of a recswitch copy empty_buffer_slots_of_next_router of that recswitch to empty_buffer_slots_of_next_router of backpressure recswitch output port//************************************ 3D must be completed
							{
								for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
								{
									for (int j1 = 0; j1 < 4; j1++)
									{
										if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
										{
											int y1, k2;
											y1 = y_computer(i1, j1); //y= input port
											k2 = k_computer(i1, j1); //k=output port
											if (u == k2)
											{
												int jj, kk, ll;
												jj = x_of_neighbor_element_in_backpressure(u, net, j, k, l);//x_of_neighbor_element_in_backpressure
												kk = y_of_neighbor_element_in_backpressure(u, net, j, k, l);//y_of_neighbor_element_in_backpressure
												ll = z_of_neighbor_element_in_backpressure(u, net, j, k, l);//z_of_neighbor_element_in_backpressure
												//Below lines contain error _ it does not change empty_buffer_slots_of_next_router when it is zero
												int b;
												b = buffer_backpressure_outportnumber_computer(u, net, j, k, l);
												if (net[jj][kk][ll].outport_number[b].empty_buffer_slots_of_next_router_is_received != 1)
												{

													net[jj][kk][ll].outport_number[buffer_backpressure_outportnumber_computer(u, net, j, k, l)].empty_buffer_slots_of_next_router = net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router; //for all out ports if there is a flit
												}
												else goto mk;
												if (net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router == 0)
												{
													cout << "&&&&&&&" << j << " " << k << " " << l << " " << u << " ";
												}
												if (net[jj][kk][ll].router == 1)
												{

													net[jj][kk][ll].outport_number[b].empty_buffer_slots_of_next_router_is_received = 1;
													//goto here3;
												}
											mk:;
											}
										}
									}
								}
								//net[jj][kk][ll].inport_number[u].buffer.credit = net[j][k][l].inport_number[]
							}
							//// ******************************************************
							//credit_based buffer backpressure on recswitch input ports for transmitting credit
							for (int u = 1; u < 5; u++)// for all input ports//************************************ 3D must be completed (5 must change to 8)
							{
								if (u == 5)//if u = PE_in do nothing
								{
									goto ghj;
								}
								for (int i1 = 0; i1 < 4; i1++) //crossbar in recswitch matrix
								{
									for (int j1 = 0; j1 < 4; j1++)
									{
										if (net[j][k][l].crossbar_in_recswitch[i1][j1] == 1)
										{
											int y1, k2;
											y1 = y_computer(i1, j1); //y= input port
											k2 = k_computer(i1, j1); //k=output port
											if (u == k2)
											{
												int jj, kk, ll, pz, sz;
												pz = input_to_outport_computer(u);//pz is outport of neighbore element and u is inport number
												jj = x_of_neighbor_element(pz, net, j, k, l);//x_of_neighbor_element_in_backpressure
												kk = y_of_neighbor_element(pz, net, j, k, l);//y_of_neighbor_element_in_backpressure
												ll = z_of_neighbor_element(pz, net, j, k, l);//z_of_neighbor_element_in_backpressure
												sz = what_is_inport(pz, net, j, k, l);
												//if (net[jj][kk][ll].router == 0)//if neighbor element is a recswitch
												//{
												if (net[j][k][l].inport_number[u].buffer.credit == 1)
												{
													net[jj][kk][ll].inport_number[sz].buffer.credit = net[j][k][l].inport_number[u].buffer.credit; //for all out ports if there is a flit
													net[j][k][l].inport_number[u].buffer.credit = 0;
												}
												if (net[jj][kk][ll].router == 1)
												{
													net[jj][kk][ll].inport_number[sz].buffer.credit_is_received = 1;
													goto here3;
												}
											}
										}
									}
								}
								//net[jj][kk][ll].inport_number[u].buffer.credit = net[j][k][l].inport_number[]
								//}
							ghj:;
							}
						here3:;
						}
					}
					// End of recswitch code section
					//--------------------------------------------------------------------------------
					//--------------------------------------------------------------------------------
					//three brackets				
				}
			}
		}
		//--------------------------------------------------------------------------------
		// one bracket
	}
	//Average delay computer //*****************************************must be corrected
	/*int avg_delay = 0;
	avg_delay = (f1.time + f2.time) / 2;
	myfile << "\n\nAverage delay=" << avg_delay;
	for (int t = 1; t < 3; t++)
	{
	avg_delay+=
	}
	*/
	//int avg_delay = 0;
	//avg_delay = (f1.time + f2.time + f3.time + f4.time)/4;
	//cout << "\nAvg delay = " << avg_delay;
	//myfile << "\nAvg delay = " << avg_delay;
	cout << "Simulation finished press enter to exit\nResults are written in result.txt file in code directory";
	getchar();
}