// RecNoC_simulator2.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}
//########################################################################
//3D Cluster-based Reconfigurable Network-on-Chip Simulator by Arash Firuzan (arash.firuzan@gmail.com) _ 2017
//Under supervision of Dr. Mehdi Modarressi (modarressi@ut.ac.ir)
//Arbitration of each router is priority-based of ports
//########################################################################

#include<stdio.h>
#include<iostream>
#include <new>
#include<fstream>
#include <stdlib.h>
#include<string>
#include<typeinfo>
#include <vector>
#include<time.h>
//#include "random_arbiter_function.h"
#include "priority_arbiter_function.h"
using namespace std;
//---------------------------------------------------------------------------------------------------------------
//Inputs of code
//Please set the flit data at line 400 of this code if you are not going to use synthetic traffic
#define buffer_size 8
#define NI_buffer_size 50
#define a_size 30000000 // estimation of number of generated flits
#define flit_trace_number 500 //trace an errori flit
int simulation_time = 50000; //10k cycle. simulation time by cycle unit
int traffic_generation_duration = (simulation_time - 200); //traffic_generation_duration by cycle unit
float injection_rate = 0.5; //if the number of this line is based on 0.0x we must chnage traffic generator line of this code to ran()*100 and aslo of statement
int const cluster_size = 1; //cluster size must be related with networkx and networky
int const num_of_corridors = 3;
const int networkx = 4; //networkx=networky
const int networky = 4;
const int networkz = 1;
int number_of_elements_in_x_direction = networkx + (((networkx / cluster_size) - 1)*num_of_corridors);
int number_of_elements_in_y_direction = networky + (((networky / cluster_size) - 1)*num_of_corridors);
int pl = 0; //pl is a global variable which will be used for counting number of flits
int pu = 0; //pl is a global variable which will be used for counting number of flits starting from PE_in Buffer
int pk = 0; //pk for debugging
int Minimum_Delay; //Minimum_Delay is a global variable which will be used for computing minimum delay
int credit_sends_counter = 0; //an integer for counting number of credit_send function calls
int b[a_size]; //a global variable for indexing elements of flit_path array
int buffer_full_counter = 0; ////a global variable for counting number of buffer_full errors
int arbiter_counter[networkx][networky][networkz][5];//TODO: 3D 
int number_of_errors = 0;
int number_of_flit_number_missed_errors = 0;
int number_of_flit_number_missed_errors_section1 = 0;
int number_of_flit_number_missed_errors_section2 = 0;
int number_of_flit_number_missed_errors_section3 = 0;
int last_cycle_which_flit_seen = 0; //last_cycle_which_flit_seen

ofstream myfile("Result.txt");
ofstream myfile2("path_of_flits.txt");
//ofstream myfile2("Result2.txt");
//---------------------------------------------------------------------------------------------------------------
//Begin of class definitions

class last_location_which_flit_seen // a class for tracing flits
{
public:
	string name = "N/A"; //flit location name
	int j, k,l; //coordinates of flit location
	string line_number = "N/A";
};
class flit
{
public:
	int number;
	int x_dest;
	int y_dest;
	int data;
	int time; //time which flit comes to cluster-based RecNoC and reached to its destination
	int injection_time;
	bool is_flit_reached_to_its_destination = 0; // if flit reaches to its destination then this flag being one
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
	int empty_buffer_slots_of_next_router = buffer_size; //Credit-based flow control. when network is empty, buffers of network is empty
	bool arbiter_array[7];//arbiter array. TODO: 3D
	outport()// constructor for initializing arbiter_array;
	{
		for (int i = 0; i < 7; i++)
		{
			arbiter_array[i] = 0;
		}
	}
	bool credit_out = 0;
	bool credit_recived = 0;
	int winner_inport_in_arbitration;
};

class NI_Queue //buffer of Network Interface (NI) is a circular queue
{
public:
	bool credit;//for credit-based flowcontrol
	bool credit_is_received = 0;
	flit f[NI_buffer_size];
	int front, rear;
	NI_Queue() {
		front = -1;
		rear = -1;
	}

	bool isFull() {
		if (front == 0 && rear == buffer_size - 1) {
			return true;
		}
		if (front == rear + 1) {
			return true;
		}
		return false;
	}

	bool isEmpty() {
		if (front == -1) return true;
		else return false;
	}

	void enQueue(flit f2) {
		if (isFull()) {
			//cout << "\n\nError: Buffer is full\n";
			//buffer_full_counter++;
		}
		else {
			if (front == -1) front = 0;
			rear = (rear + 1) % buffer_size;
			f[rear] = f2;
			//cout << endl << "Inserted Flit " << f2.number << endl;
		}
	}

	flit deQueue() {
		flit f2;
		//flit errorflit;
		//errorflit.number = -1;
		if (isEmpty()) {
			cout << "Queue is empty" << endl;
			//return(errorflit);
		}
		else {
			f2 = f[front];
			if (front == rear) {
				front = -1;
				rear = -1;
			} /* Q has only one element, so we reset the queue after deleting it. */
			else {
				front = (front + 1) % buffer_size;
			}
			return(f2);
		}
	}
	void display() /* Function to display inside status of Circular Queue */
	{
		myfile << "\n";
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
				myfile << f[i].number << " , ";
			}
			myfile << f[i].number;
			myfile << endl << " ++++++++++++++ Index of Rear -> " << rear << "\n";
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
	//Below line is Constructor for class Queue
	Queue() {
		front = -1;
		rear = -1;
	}

	bool isFull() {
		if (front == 0 && rear == buffer_size - 1) {
			return true;
		}
		if (front == rear + 1) {
			return true;
		}
		return false;
	}

	bool isEmpty() {
		if (front == -1) return true;
		else return false;
	}

	void enQueue(flit f2) {
		if (isFull()) {
			cout << "\n\nError: Buffer is full\n";
			//buffer_full_counter++;
		}
		else {
			if (front == -1) front = 0;
			rear = (rear + 1) % buffer_size;
			f[rear] = f2;
			//cout << endl << "Inserted Flit " << f2.number << endl;
		}
	}

	flit deQueue()
	{
		flit f2;
		//flit errorflit;
		//errorflit.number = -1;
		if (isEmpty())
		{
			cout << "Error occured: Queue is empty" << endl;
		}
		else
		{
			f2 = f[front];
			if (front == rear)
			{
				front = -1;
				rear = -1;
			} /* Q has only one element, so we reset the queue after deleting it. */
			else
			{
				front = (front + 1) % buffer_size;
			}
			return(f2);
		}
	}
	void display() /* Function to display inside status of Circular Queue */
	{
		myfile << "\n";
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
				myfile << f[i].number << " , ";
			}
			myfile << f[i].number;
			myfile << endl << " ++++++++++++++ Index of Rear -> " << rear << "\n";
		}
	}
};

class inport
{
public:
	//flit f[buffer_size];//buffersize=10
	flit flit_of_inport_of_recswitch; //because inport of recswitch does not have buffer
	bool is_full_for_recswitch; //because input of router has buffer and does not need is_full, but inport of a reconfigurable switch have only a flit and needs this.
	Queue buffer; //buffer is a circular queue
				  //	bool is_full;//if is_full=1 means that buffer is full
				  //bool buffer_full, buffer_empty;//if buffer is full then buffer_full is 1 and if buffer is empty then buffer_empty is 1;
				  //flit buffer_read(); //this function read a flit from buffer
				  //int buffer_write(flit f2 );//this function write flit f2 into buffer
	flit buffer_display();
	//int write_pointer=-1;// equal to rear initializing write pointer;
	//int read_pointer=-1;// equal to front initializing read pointer
	void buffer_read_increase_time();
	bool grant = 0;//for arbitration section
	int outport_computed_by_routing_function; //This member is use for storing computed outport in routing step
};
flit inport::buffer_display() //this function returns front flit of queue
{
	flit result;
	int front2;
	front2 = buffer.front;
	result = buffer.f[front2];
	return result;
}
void inport::buffer_read_increase_time() //This function increases time for front flit inside buffer
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
		// do nothing
	}
}
class NI
{
public:
	NI_Queue queue[(networkx +((networkx-1)*num_of_corridors))+ 1][(networky + ((networky - 1)*num_of_corridors)) + 1];//NI_buffer for all routers. TODO: 3D must be completed
};
class trafficmanager
{
public:
	flit generate_flit(int j, int k, int l, int(&a)[6][a_size], int i, element net[100][100][2]);
	//void retire_flit();
};
flit trafficmanager::generate_flit(int j, int k, int l, int(&a)[6][a_size], int i, element net[100][100][2])//This function generates flit and i is clock cycle******************must be completd (int (&a))
{
	flit Temp_Flit;
	int x;
	int y;
LI:x = rand() % number_of_elements_in_x_direction + 1; //x in the range 1 to networkx
	y = rand() % number_of_elements_in_y_direction + 1; //y in the range 1 to networky. //TODO: 3D must be completed
	if ((x == j) && (y == k)) //Generated flit`s destination cannot be on itself //TODO: 3D must be completed
	{
		goto LI;
	}
	if (net[x][y][1].router != 1) //Generated flit`s destination cannot be on a reconfigurable switch. TODO: 3D must be completed
	{
		goto LI;
	}
	if (pl == -858993460)
	{
		cout << "\nerror occured: f.number= -858993460 at NI_buffer\n";
		number_of_flit_number_missed_errors++;
	}
	Temp_Flit.x_dest = x;
	Temp_Flit.y_dest = y;
	Temp_Flit.number = pl;
	Temp_Flit.injection_time = i;
	//myfile << "\n At cycle " << i << " Flit " << pl << " generated with X-dest = " << a[2][pl] << " and Y_dest = " << a[3][pl] << "\n\n";
	a[0][pl]++; //every time a flit generated a[0] array increments by one
	a[2][pl] = x; //stroing x_dest of flit for future statistics
	a[3][pl] = y; //stroing x_dest of flit for future statistics
	a[4][pl] = j;
	a[5][pl] = k; //TODO: 3D must be completed
				  //myfile << "\n At cycle " << i << " Flit " << pl << " generated with X-dest = " << a[2][pl] << " and Y_dest = " << a[3][pl] << "\n\n";
	pl++;
	if (Temp_Flit.number == -858993460)
	{
		cout << "\nerror occured: f.number= -858993460 at NI_buffer\n";
		number_of_flit_number_missed_errors++;
	}
	return Temp_Flit;
}
//End of class definitions
//------------------------------------------------------------------------------------------------------------------------
//Required functions definition
void place_router(element(&net)[100][100][2], int &x, int y, int z, int &routercounter_in_x_dimension)
{
	net[x][y][z].router = 1;
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
		for (int j = 1; j < num_of_corridors + 1; j++)
		{
			place_recswitch(net, x, y, z);
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

int xy_routing_function(flit f, int j, int k, int l, element(&net)[100][100][2])//This function computes routing for front element of buffer **********************************must be completed
{
	int outputport;
	int x_offset, y_offset;
	x_offset = f.x_dest - j;
	y_offset = f.y_dest - k;
	if (x_offset < 0)
	{
		outputport = 2; //outputport = W_out
	}
	if (x_offset > 0)
	{
		outputport = 4; //outputport = E_out
	}
	if ((x_offset == 0) && (y_offset<0))
	{
		outputport = 3; //outputport = S_out
	}
	if ((x_offset == 0) && (y_offset>0))
	{
		outputport = 1; //outputport = N_out
	}
	if ((x_offset == 0) && (y_offset == 0))
	{
		outputport = 5; //outputport = PE_out
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
	if (u == 1)
	{
		result = k + 1;
		return result;
	}
	if (u == 3)
	{
		result = k - 1;
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
	if (u == 2)//out port of this router
	{
		result = 5;//inlink of neighbor router
	}
	if (u == 3)//out port of this router
	{
		result = 2;//inlink of neighbor router
	}
	if (u == 4)//out port of this router
	{
		result = 3;//inlink of neighbor router
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
	int result[4][4] = {
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 },
		{ 1, 3, 4, 2 }

	};
	return result[i][j];
}

string u_to_inport_name(int u)//This function convert inport number to inport name for best readability
{
	string result[10] = { "Error occurred", "North_in", "West_in", "South_in", "East_in",  "PE_in",  "TSV_UP_in",  "TSV_DOWN_in" };
	return result[u];
}

string u_to_outport_name(int u)//This function convert outport number to outport name for best readability
{
	string result[10] = { "Error occured", "North_out", "West_out", "Sout_out", "East_out", "PE_out", "TSV_UP_out", "TSV_DOWN_out" };
	return result[u];
}

int inport_to_neighbor_outport_number_computer_backpressure(int y1)// y1 is inport ******************************must be completed for 3D
{
	int result;
	if (y1 == 1) //inport= N_in
	{
		result = 3;//outport S-out
	}
	if (y1 == 2)
	{
		result = 4;//outport E-out
	}
	if (y1 == 3)
	{
		result = 1;//outport N-out
	}
	if (y1 == 4)
	{
		result = 2;//outport W-out
	}
	return result;
}

int input_to_outport_computer(int u)//this function is another code of above function
{
	int result[8] = { 100, 3, 4, 1, 2 };////*******************************3D must be completed
	return result[u];
}

int buffer_backpressure_outportnumber_computer(int u, element net[100][100][2], int &j, int &k, int &l)//This function gives outport of this recswitch and then computes output port of neghibor element in credit_based back pressure buffer mechanism
{
	int y1, k1, i1, j1, result;
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
	if (result == 1)
		k++;
	if (result == 2)
		j--;
	if (result == 3)
		k--;
	if (result == 4)
		j++;
	return result;
}

int recswitch_outportnumber_to_inport_computer_without_change_of_j_k_l(int u, element net[100][100][2], int j, int k, int l)//This function gives outport of this recswitch and then computes output port of neghibor element in credit_based back pressure buffer mechanism
{
	int y1, k1, i1, j1, result;
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
					result = y1;
					goto awd;
				}
			}
		}
	}
	awd:return result;
}
int x_of_neighbor_element_in_backpressure(int u, element net[100][100][2], int j, int k, int l)//This function gives outport of this recswitch as input, and then computes x of neighbor element in credit_based back pressure buffer mechanism
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
int y_of_neighbor_element_in_backpressure(int u, element net[100][100][2], int j, int k, int l)//This function gives outport of this recswitch as input, and then computes y of neighbor element in credit_based back pressure buffer mechanism
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

int z_of_neighbor_element_in_backpressure(int u, element net[100][100][2], int j, int k, int l)//////*******************************3D must be completed
{

	return 1; ////*******************************3D must be completed
}

int x_of_neighbor_element(int u, element net[100][100][2], int j, int k, int l) //x_of_neighbor_element in back pressure , u is inport
{
	int result;
	if ((u == 1) || (u == 3))
	{
		result = j;
	}
	if (u == 2)
	{
		result = j - 1;
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
		result = k - 1;
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
	result = what_is_inport(buffer_backpressure_inportnumber_to_outport_number_of_neighbor_element_computer(u, net, j, k, l), net, j, k, l);
	return result;
}

/*void send(int in, int out, int j, int k, int l, element(&net)[100][100][100])
{
net[j][k][l].port_number[in].is_full = 0; //send internal to an output port
net[j][k][l].port_number[10].is_full = 1;
net[j][k][l].port_number[10].f = net[j][k][l].port_number[4].f;
}*/


int is_backpressure_element_router(element net[100][100][2], int j, int k, int l, int u) //This function gives inport of this router and if backpressure element was router it returns 1
{
	if ((u == 1) && (net[j][k + 1][l].router == 1))
	{
		return 1;
	}
	if ((u == 2) && (net[j - 1][k][l].router == 1))
	{
		return 1;
	}
	if ((u == 3) && (net[j][k - 1][l].router == 1))
	{
		return 1;
	}
	if ((u == 4) && (net[j + 1][k][l].router == 1))
	{
		return 1;
	}
}
void send_credit(element(&net)[100][100][2], int j, int k, int l, int u) //This function sends a credit to the first backpressure router. u is inport number.
{
	if (is_backpressure_element_router(net, j, k, l, u) == 1) // if backpressure element was router
	{
		net[x_of_neighbor_element(u, net, j, k, l)][y_of_neighbor_element(u, net, j, k, l)][z_of_neighbor_element(u, net, j, k, l)].outport_number[inport_to_neighbor_outport_number_computer_backpressure(u)].credit_recived = 1; // In this line we send credit to backpressure router. in other words we place one at credit_receive section in outport of backpressure router
	}
	else // in case of (num_of_corridors>0)
	{
		int hj,j1,k1;
		j1 = j; //storing value of j
		k1 = k;
		hj = inport_to_neighbor_outport_number_computer_backpressure(u);
		j = x_of_neighbor_element_in_backpressure(hj, net, j1, k1, l);
		k = y_of_neighbor_element_in_backpressure(hj, net, j1, k1, l); //TODO: 3D must be complete
		goto asd;
		while (net[j][k][l].router == 0) // TODO: while we reach to the first router we must gone back untill reach router
		{
			asd:
			int yh,j2,k2;
			j2 = j; k2 = k;
			hj = recswitch_outportnumber_to_inport_computer_without_change_of_j_k_l(hj, net, j, k, l);
			yh=inport_to_neighbor_outport_number_computer_backpressure(hj);
			j=x_of_neighbor_element(hj, net, j2, k2, l);
			k=y_of_neighbor_element(hj, net, j2, k2, l); //TODO: 3D must be complete
			hj = yh;
		}
		net[j][k][l].outport_number[hj].credit_recived = 1;
	}
}
//End of required functions definitions
//##########################################################################################################################
void main()
{
	last_location_which_flit_seen last_place_which_flit_seen;
	cout << "number_of_elements_in_x_direction= " << number_of_elements_in_x_direction << "\n";
	cout << "number_of_elements_in_y_direction= " << number_of_elements_in_y_direction << "\n";
	int a[6][a_size]; //An array for storing the specification of flits such as when it reaches its destination and producing statistics //must be completed
					  //store flit_path[a_size][500]; // An array for storing path of flits;
	for (int i = 0; i < a_size; i++) //initialization of b[] array (b[]=a global variable for indexing elements of flit_path array)
	{
		b[i] = 1;
	}
	for (int i = 0; i < 6; i++) //initialization of a[] array
	{
		for (int j = 0; j < a_size; j++)
		{
			a[i][j] = 0;
		}
	}
	element net[100][100][2]; //creates a 3D cluster-based Reconfigurable NoC
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
	//Cluster-based Reconfigurable Network-on-Chip constructor. This section of code initialize net[j][k][l] array as a recswitch.
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
	}
	//End of Cluster-based RecNoC constructor
	//###################################################################################################################################
	//for testing location of routers the program and writing result in output txt file
	myfile << "Cluster-based RecNoC result:\n\n";
	cout << "\nnetwork x = "<<networkx<<"\n"; //TODO: 3D must be completed
	myfile << "\nnetwork x = " << networkx << "\n";
	cout << "\nnetwork y = " << networky << "\n";
	myfile << "\nnetwork y = " << networky << "\n";
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
	cout << "\nbuffer_size= " << buffer_size;
	myfile << "\nbuffer_size= " << buffer_size;
	// traffic manager
	myfile << "\n\n#################################################################\n\ntraffic manager result:\n\n";
	//information of input flits is in below:
	//number_of_flits = networkx*networky;
	flit f1;
	int number_of_flits = pl - 1;
	//f1.number = 1;
	//f2.number = 2;
	//f3.number = 3;
	//f4.number = 4;

	//f1.time = 0;//the time which flit is injected into network
	//f2.time = 0;//the time which flit is injected into network
	//f3.time = 0;
	//f4.time = 0;

	//Destination of flits
	//f1.x_dest =3;
	//f1.y_dest = 3;

	/*f2.x_dest = 2;
	f2.y_dest = 1;

	f3.x_dest = 2;
	f3.y_dest = 1;

	f4.x_dest = 2;
	f4.y_dest = 1;*/

	//injection location of flits into PE input ports
	//net[1][1][1].inport_number[5].buffer.enQueue(f1);
	/*net[2][2][1].inport_number[4].buffer.enQueue(f2);
	net[2][2][1].inport_number[3].buffer.enQueue(f3);
	net[2][2][1].inport_number[2].buffer.enQueue(f4);*/

	//modified recswitch matrix must be placed below
	/*net[2][3][1].crossbar_in_recswitch[3][2] = 0;
	net[2][3][1].crossbar_in_recswitch[2][3] = 0;
	net[2][3][1].crossbar_in_recswitch[0][1] = 0;
	net[2][3][1].crossbar_in_recswitch[1][0] = 0;
	net[2][3][1].crossbar_in_recswitch[3][1] = 1;
	net[2][3][1].crossbar_in_recswitch[1][3] = 1;
	
	net[2][1][1].crossbar_in_recswitch[0][1] = 0;
	net[2][1][1].crossbar_in_recswitch[1][0] = 0;
	net[2][1][1].crossbar_in_recswitch[0][2] = 1;
	net[2][1][1].crossbar_in_recswitch[2][0] = 1;*/
	
	//--------------------------------------------------------------------------------

	NI NI_1;
	trafficmanager TF1;
	//--------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------
	cout << "\n" << "simulation duration = " << simulation_time << "\n";

	for (int ii = 1; ii < simulation_time; ii++) // ii shows clock cycle. In other words it shows time. This simulator is a cycle-accurate simulator and this line is the main line of simulator
	{
		//--------------------------------------------------------------------------------
		//Generating Traffic
		if (ii < traffic_generation_duration)
		{
			for (int j = 1; j < number_of_elements_in_x_direction + 1; j++) ///////////////
			{
				for (int k = 1; k < number_of_elements_in_y_direction + 1; k++) /////////// for all routers
				{
					for (int l = 1; l < (networkz + 1); l++) //////////////////////////////
					{
						if (net[j][k][l].router == 1)//////////////////////////////////////
						{
							int R;
							R = rand() % 10+ 1; //R in the range 1 to 10. In rand() function For example, probability of generation number 3 is equal to probability of producing number 4
							flit temp_flit;
							if (R <= (injection_rate * 10))
							{
								temp_flit = TF1.generate_flit(j, k, l, a, ii, net); //This line generate a flit
								if (temp_flit.number == -858993460)
								{
									cout << "\nerror occured: f.number= -858993460 at generate_flit function\n";
									number_of_flit_number_missed_errors++;
								}
								if (temp_flit.number == flit_trace_number) // for debugging
								{
									cout << "\nerror occured: f.number= " << flit_trace_number << " at generate_flit function\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = "generate_flit function";
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1110";
									}
								}
								if (NI_1.queue[j][k].isFull() == 0) //if buffer of NI is not full and have at least one empty slot
								{
									NI_1.queue[j][k].enQueue(temp_flit); //put generated flit at NI buffer
									if (temp_flit.number == -858993460) // for debugging
									{
										cout << "\nerror occured: f.number= -858993460 at NI_buffer\n";
										number_of_flit_number_missed_errors++;
									}
									if (temp_flit.number == flit_trace_number)// for debugging
									{
										cout << "\nerror occured: f.number= "<< flit_trace_number<<" at NI_buffer\n";
										if (ii >= last_cycle_which_flit_seen)
										{
											last_cycle_which_flit_seen = ii;
											last_place_which_flit_seen.name = "NI_buffer";
											last_place_which_flit_seen.j = j;
											last_place_which_flit_seen.k = k;
											last_place_which_flit_seen.l = l;
											last_place_which_flit_seen.line_number = "1130";
										}
									}
								}
							}
							//Below lines: if there is an empty slot in buffer then Dequeue from NI buffer and then Enqueue to PE_in port of router

							if (net[j][k][l].inport_number[5].buffer.isFull() == 0) //if buffer of PE_in is not full and have at least one empty slot
							{
								pk++; //Debugging
								if (NI_1.queue[j][k].isEmpty() == 0) //if Network Interface buffer is not empty
								{
									flit temp4;
									temp4 = NI_1.queue[j][k].deQueue(); //deQueue from NI router
									net[j][k][l].inport_number[5].buffer.enQueue(temp4); //inject flit from NI to inport buffer
									pu++; //increasing number of generated flits in PE_in buffer
								}
							}
						}
					}
				}
			}
		}
		//End of traffic generation
		//--------------------------------------------------------------------------------

		for (int j = 1; j < number_of_elements_in_x_direction + 1; j++) ///////////////
		{
			for (int k = 1; k < number_of_elements_in_y_direction + 1; k++) /////////// for all routers
			{
				for (int l = 1; l < (networkz + 1); l++) //////////////////////////////
				{
					if (net[j][k][l].router == 1)//////////////////////////////////////
					{
						//initialization of arbiter_array of all outports to zero
						for (int u = 1; u < 6; u++)//for all output ports
						{
							for (int y = 1; y < 6; y++)//for all arbiter_array elements
							{
								net[j][k][l].outport_number[u].arbiter_array[y] = 0;//set all arbitration_arrary of all outports to zero
							}
						}
						//End of initialization of arbiter_array of all outportd to zero
						//------------------------------------------------------------------------------------------------------------------------------------




















						//------------------------------------------------------------------------------------------------------------------------------------
						//for all out ports if there is a flit send outport flit to in-link of neighbor element
						for (int u = 1; u < 8; u++)//for all out ports if there is a flit send outport flit to in-link of neighbor element
						{
							if (net[j][k][l].outport_number[u].is_full == 1)
							{
								//----------
								if (u == 5) /// u==5 means PE_out. So this line means flit reached to its destination
								{
									if (net[j][k][l].outport_number[u].f.number == -858993460)//for debugging
									{
										cout << "\nerror occured: f.number= -858993460\n";
										number_of_flit_number_missed_errors++;
										number_of_flit_number_missed_errors_section1++;
									}
									if (net[j][k][l].outport_number[u].f.number == flit_trace_number)//for debugging
									{
										cout << "\nerror occured: f.number= " << flit_trace_number << " \n";
										if (ii >= last_cycle_which_flit_seen)
										{
											last_cycle_which_flit_seen = ii;
											last_place_which_flit_seen.name = "PE_out";
											last_place_which_flit_seen.j = j;
											last_place_which_flit_seen.k = k;
											last_place_which_flit_seen.l = l;
											last_place_which_flit_seen.line_number = "1220";
										}
									}
									//myfile << ">>>>>>>>> flit " << net[j][k][l].outport_number[u].f.number << " reached to its destination in cycle " << i << "\n\n";
									if (net[j][k][l].outport_number[u].f.is_flit_reached_to_its_destination == 0)
									{
										int temp;
										temp = net[j][k][l].outport_number[u].f.number;
										//myfile << ">>>>>>>>> flit " << temp << " reached to its destination in cycle " << i << "\n\n";
										a[1][net[j][k][l].outport_number[u].f.number] = (ii - net[j][k][l].outport_number[u].f.injection_time); //storing flit latency
										a[0][net[j][k][l].outport_number[u].f.number]++; //increment by one, we use this for evaluating the when reached to its destination
																						 // TODO: flit must be deleted once reached to destination
										net[j][k][l].outport_number[u].is_full = 0;
									}
									net[j][k][l].outport_number[u].f.is_flit_reached_to_its_destination = 1; // For not to double cout myfile 
								}
								//----------
								else //for all out ports if there is a flit send outport flit to in-link of neighbor element
								{
									int j1, k1, l1, inlinknumber; //computing in-link of neighbor element
									j1 = j_at_next_router(j, k, l, net, u);//x of neighbor element ************************************ 3D must be completed
									k1 = k_at_next_router(j, k, l, net, u);//y of neighbor element
									inlinknumber = inlinknumber_computer_for_neighbor_element(u);
									net[j][k][l].outport_number[u].f.time++; //This operation requires one cycle
									net[j][k][l].outport_number[u].is_full = 0; //send outport flit to in-link of neighbor element
																				/////*************************************************************
									net[j1][k1][l].inlink_number[inlinknumber].is_full = 1;
									net[j1][k1][l].inlink_number[inlinknumber].f = net[j][k][l].outport_number[u].f;
									net[j][k][l].outport_number[u].empty_buffer_slots_of_next_router--; //credit-based flow control
									if (net[j][k][l].outport_number[u].f.number == -858993460) //for debugging
									{
										cout << "\nError occured: f.number == -858993460\n";
										number_of_flit_number_missed_errors++;
										number_of_flit_number_missed_errors_section2++;
									}
									if (net[j][k][l].outport_number[u].f.number ==flit_trace_number) //for debugging
									{
										cout << "\nError occured: f.number == " << flit_trace_number << "\n";
										if (ii >= last_cycle_which_flit_seen)
										{
											last_cycle_which_flit_seen = ii;
											last_place_which_flit_seen.name = u_to_outport_name(u);
											last_place_which_flit_seen.j = j;
											last_place_which_flit_seen.k = k;
											last_place_which_flit_seen.l = l;
											last_place_which_flit_seen.line_number = "1265";
										}
									}
									if (net[j][k][l].outport_number[u].f.number == 3413) //for debugging
										cout << "\n1024 &&&&&&&&&&&&&&\n";
									/*flit_path[net[j][k][l].outport_number[u].f.number][b[net[j][k][l].outport_number[u].f.number]].j = j; //storing path of flit
									flit_path[net[j][k][l].outport_number[u].f.number][b[net[j][k][l].outport_number[u].f.number]].k = k; //storing path of flit
									flit_path[net[j][k][l].outport_number[u].f.number][b[net[j][k][l].outport_number[u].f.number]].l = l; //storing path of flit
									flit_path[net[j][k][l].outport_number[u].f.number][b[net[j][k][l].outport_number[u].f.number]].step = 1; //storing path of flit
									b[net[j][k][l].outport_number[u].f.number]++;
									*/
									for (int t = 1; t < number_of_flits + 1; t++) ////*****************************************************must be corrected
									{
										if (net[j][k][l].outport_number[u].f.number == t)
										{
											//myfile << "\n At cycle " << net[j][k][l].outport_number[u].f.time << " flit " << net[j][k][l].outport_number[u].f.number << " is in   router ( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
											myfile << "\n At cycle " << ii << " flit " << net[j][k][l].outport_number[u].f.number << " is in   router ( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
										}
									}
								}
							}
						}
						//------------------------------------------------------------------------------------------------------------------------------------
						//------------------------------------------------------------------------------------------------------------------------------------


























						for (int u = 1; u < 6; u++) //for all inports if grant=1 then traverse to outport which computed by routing function in previous section
						{
							if (net[j][k][l].inport_number[u].grant == 1) //if grant=1 then send input to output which computed by routing function 
							{
								int temp;
								temp = net[j][k][l].inport_number[u].outport_computed_by_routing_function;// we use outport_computed by_routing_function for storing route which computed in prevoius cycle
																										  //if (net[j][k][l].inport_number[u].buffer.isEmpty() == 0) //if buffer of inport is not empty
																										  //{
								flit temp2;
								temp2 = net[j][k][l].inport_number[u].buffer_display();
								if (net[j][k][l].inport_number[u].buffer_display().number == -858993460) //for debugging
								{
									cout << "\nError occured: f.number == -858993460\n";
									number_of_flit_number_missed_errors++;
									number_of_flit_number_missed_errors_section3++;
								}
								if (net[j][k][l].inport_number[u].buffer_display().number == flit_trace_number) //for debugging
								{
									cout << "\nError occured: f.number == " << flit_trace_number << "\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = u_to_inport_name(u);
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1341";
									}
								}
								/*flit_path[temp2.number][b[temp2.number]].j = j; //storing path of flit
								flit_path[temp2.number][b[temp2.number]].k = k; //storing path of flit
								flit_path[temp2.number][b[temp2.number]].l = l; //storing path of flit
								flit_path[temp2.number][b[temp2.number]].step = 2; //storing path of flit
								b[net[j][k][l].outport_number[u].f.number]++;*/
								flit temp3;
								temp3 = net[j][k][l].inport_number[u].buffer.deQueue();
								net[j][k][l].outport_number[temp].f = temp3;//put flit of winner inport into outport
								net[j][k][l].outport_number[temp].is_full = 1;
								//net[j][k][l].outport_number[temp].empty_buffer_slots_of_next_router--;
								if (u == 5) //if inport is PE_in we do not need credit to be send
									goto kl;
								send_credit(net, j, k, l, u); //This function send a one to credit_recived of backpressure router
								credit_sends_counter++;
							kl: net[j][k][l].inport_number[u].grant = 0; //If do not set grant to zero, in next cycle next flit will send to 
								if (net[j][k][l].inport_number[u].buffer_display().number == 3413) //for debugging
									cout << "\n\nDebug\n";
								//}
							}
						}


						for (int u = 1; u < 6; u++) // for all inports compute routing and store result in outport_computed_by_routing_function member of that inport  ************************************ 3D must be completed
						{
							if (net[j][k][l].inport_number[u].buffer.isEmpty() == 0) //If buffer of this input port is not empty
							{
								int outport;
								flit temp, temp2;
								temp = net[j][k][l].inport_number[u].buffer_display(); //front element of buffer
								outport = xy_routing_function(temp, j, k, l, net); //outport is computed by routing function
								net[j][k][l].inport_number[u].outport_computed_by_routing_function = outport; //result of routing unit must be store for next use
								temp2 = net[j][k][l].inport_number[u].buffer_display(); //storing path of flit
																						/*flit_path[temp2.number][b[temp2.number]].j = j; //storing path of flit
																						flit_path[temp2.number][b[temp2.number]].k = k; //storing path of flit
																						flit_path[temp2.number][b[temp2.number]].l = l; //storing path of flit
																						flit_path[temp2.number][b[temp2.number]].step = 3; //storing path of flit
																						b[temp2.number]++;*/
								net[j][k][l].outport_number[outport].arbiter_array[u] = 1; //Arbitration Request sent to outport
								net[j][k][l].inport_number[u].buffer_read_increase_time();//This operation requires one cycle and flit time must be added by one
							}
						}








						//++++++++++++++++++++++++++++++++++++++++++++++
						//Below lines are arbitration: For all outports we have arbitration
						for (int t = 1; t < 6; t++)//For all outports we have arbitration. index of winner will be stored in an integer member of outport function
						{
							int counter = 0;
							//Below loop computes number of ones in arbiter_array
							for (int i = 1; i < 6; i++) //for all inports TODO:3D
							{
								if (net[j][k][l].outport_number[t].arbiter_array[i] == 1)
									counter++;
							}
							if ((counter > 0) && (net[j][k][l].outport_number[t].empty_buffer_slots_of_next_router>0)) //if there is at least one arbitration request and credit of this port is greater than one
							{
								if (net[j][k][l].outport_number[t].empty_buffer_slots_of_next_router > buffer_size) //for debugging
								{
									//cout << "\n\nError occurred in arbitration phase: empty_buffer_slots_of_next_router > buffer_size\n\n";
									number_of_errors++;
								}
								//net[j][k][l].outport_number[t] .winner_inport_in_arbitration= outport_arbiter_function(net[j][k][l].outport_number[t].arbiter_array,j,k,l,t,counter);// srand random arbiter function. Right side of this equation, is index of winner inport. winner inport will send to outport
								net[j][k][l].outport_number[t].winner_inport_in_arbitration = outport_arbiter_function(net[j][k][l].outport_number[t].arbiter_array, j, k, l, t);//Right side of this equation, is index of winner inport. winner inport will send to outport
								net[j][k][l].inport_number[net[j][k][l].outport_number[t].winner_inport_in_arbitration].grant = 1;
								if (net[j][k][l].inport_number[net[j][k][l].outport_number[t].winner_inport_in_arbitration].buffer.isEmpty() == 1)
								{
									cout << "\nError occured: Buffer Empty in line 1299\n";
								}
								if (net[j][k][l].inport_number[net[j][k][l].outport_number[t].winner_inport_in_arbitration].buffer_display().number == -858993460)
								{
									cout << "\nError occured: f.number == -858993460 in arbitration\n";
									number_of_flit_number_missed_errors++;
								}
								if (net[j][k][l].inport_number[net[j][k][l].outport_number[t].winner_inport_in_arbitration].buffer_display().number == flit_trace_number)
								{
									cout << "\nError occured: f.number == " << flit_trace_number << " in arbitration\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = u_to_outport_name(net[j][k][l].outport_number[t].winner_inport_in_arbitration);
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1434";
									}
								}
								//net[j][k][l].outport_number[t].arbiter_array[outport_arbiter_function(net[j][k][l].outport_number[t].arbiter_array,j,k,l,t,counter)] = 0; 
								net[j][k][l].outport_number[t].arbiter_array[outport_arbiter_function(net[j][k][l].outport_number[t].arbiter_array, j, k, l, t)] = 0; //This line is not necessery
																																									  //net[j][k][l].outport_number[t].empty_buffer_slots_of_next_router--;
							}
						}
						//End of arbitration
						//------------------------------------------------------------------------------------------------------------------------------------------------





						for (int i = 1; i < 6; i++)// for all inlinks copy inlink to inport of that element
						{
							if (net[j][k][l].inlink_number[i].is_full == 1)  //if there is a flit in inlink
							{
								int inport_number;
								inport_number = inlink_to_inport_computer(i);
								//TODO: below lines are vc allocation
								flit temp6;
								temp6 = net[j][k][l].inlink_number[i].f;
								net[j][k][l].inport_number[inport_number].buffer.enQueue(temp6);
								//cout << "\n" << net[j][k][l].inport_number[inport_number].buffer.f[7].number << "\n";
								net[j][k][l].inlink_number[i].is_full = 0;
								int temp;
								temp = temp6.number;
								if (temp == flit_trace_number)
								{
									cout << "\nError occured: f.number == " << flit_trace_number << " in copy inlink to inport of that element\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = u_to_inport_name(inport_number);
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1472";
									}
								}
								/*flit_path[temp][b[temp]].j = j; //storing path of flit
								flit_path[temp][b[temp]].k = k; //storing path of flit
								flit_path[temp][b[temp]].l = l; //storing path of flit
								flit_path[temp][b[temp]].step = 4; //storing path of flit
								b[temp]++;*/
								//send_credit(net, j, k, l, inport_number); //This function send a one to credit_recived of backpressure router
								//credit_sends_counter++;
							}
						}



						//-------------------------------------------------------------------------------------------
						//for all outports copy credit_recived to credit_out
						for (int t = 1; t < 6; t++)//For all outports TODO: 3D
						{
							if (net[j][k][l].outport_number[t].credit_recived == 1)
							{
								if (net[j][k][l].outport_number[t].credit_out == 1) //for debugging and error control
								{
									//cout << "\n\nError occurred: credit_out==1 before copying credit_recived to credit_out\n\n";
									number_of_errors++;
								}
								net[j][k][l].outport_number[t].credit_out = net[j][k][l].outport_number[t].credit_recived;
								net[j][k][l].outport_number[t].credit_recived = 0;
								net[j][k][l].outport_number[t].empty_buffer_slots_of_next_router++;
								net[j][k][l].outport_number[t].credit_out = 0;
								if (net[j][k][l].outport_number[t].empty_buffer_slots_of_next_router > buffer_size)
								{
									//cout << "\nError occurred:( empty_buffer_slots_of_next_router > buffer_size ) when copying copy credit_recived to credit_out\n\n";
									number_of_errors++;
								}
							}
						}
						//-------------------------------------------------------------------------------------------
					}




























					//----------------------------------------------------------------------------------------------------------------
					//----------------------------------------------------------------------------------------------------------------
					//code for recswitches					
					else
					{
						// ******************************************************
						//// for all inlinks of recswitch copy inlink to inport of that recswitch
						for (int i = 1; i < 6; i++)//for all inlinks
						{
							if (net[j][k][l].inlink_number[i].is_full == 1)
							{
								int inport_number;
								inport_number = inlink_to_inport_computer(i);
								net[j][k][l].inport_number[inport_number].flit_of_inport_of_recswitch = net[j][k][l].inlink_number[i].f;//????????????????????????????????
								net[j][k][l].inlink_number[i].is_full = 0;
								net[j][k][l].inport_number[inport_number].is_full_for_recswitch = 1;
								if (net[j][k][l].inlink_number[i].f.number == flit_trace_number) //for debugging
								{
									cout << "\nError occured: f.number == " << flit_trace_number << "\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = to_string(i);
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1555";
									}
								}
							}
						}
						// *********************************************************************************
						// copy inport of recswitch to outport of that recswitch by using crossbar in recswitch matrix
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
										net[j][k][l].outport_number[k2].f = net[j][k][l].inport_number[y1].flit_of_inport_of_recswitch;
										net[j][k][l].inport_number[y1].is_full_for_recswitch = 0;
										net[j][k][l].outport_number[k2].is_full = 1;
									}
									if (net[j][k][l].inport_number[y1].flit_of_inport_of_recswitch.number == flit_trace_number) //for debugging
									{
										cout << "\nError occured: f.number == " << flit_trace_number << "\n";
										if (ii >= last_cycle_which_flit_seen)
										{
											last_cycle_which_flit_seen = ii;
											last_place_which_flit_seen.name = u_to_inport_name(y1);
											last_place_which_flit_seen.j = j;
											last_place_which_flit_seen.k = k;
											last_place_which_flit_seen.l = l;
											last_place_which_flit_seen.line_number = "1586";
										}
									}
								}
							}
						}
						//// ******************************************************
						// for all out ports if there is a flit send out to in-link of neighbor element
						for (int u = 1; u < 5; u++)// for all output ports//////*******************************
						{
							if (net[j][k][l].outport_number[u].is_full == 1) //if there is a flit in this outport
							{
								int j1, k1, l1, inlinknumber;
								j1 = j_at_next_router(j, k, l, net, u);
								k1 = k_at_next_router(j, k, l, net, u);
								inlinknumber = inlinknumber_computer_for_neighbor_element(u); //inlinknumber computer for neighbor element
								net[j][k][l].outport_number[u].is_full = 0; //send out to in-link of neighbor element
								net[j1][k1][l].inlink_number[inlinknumber].is_full = 1;
								net[j1][k1][l].inlink_number[inlinknumber].f = net[j][k][l].outport_number[u].f; //send out to in-link of neighbor element
								if (net[j][k][l].outport_number[u].f.number == flit_trace_number) //for debugging
								{
									cout << "\nError occured: f.number == " << flit_trace_number << "\n";
									if (ii >= last_cycle_which_flit_seen)
									{
										last_cycle_which_flit_seen = ii;
										last_place_which_flit_seen.name = u_to_outport_name(u);
										last_place_which_flit_seen.j = j;
										last_place_which_flit_seen.k = k;
										last_place_which_flit_seen.l = l;
										last_place_which_flit_seen.line_number = "1619";
									}
								}
								for (int t = 1; t < number_of_flits + 1; t++)// for debugging /////*****************************************************must be corrected
								{
									if (net[j][k][l].outport_number[u].f.number == t)
									{
										//myfile << "\n At cycle " << net[j][k][l].outport_number[u].f.time << " flit " << net[j][k][l].outport_number[u].f.number << " is in recswitch( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
										myfile << "\n At cycle " << ii << " flit " << net[j][k][l].outport_number[u].f.number << " is in recswitch( " << j << " " << k << " " << l << " ) at port[" << u_to_outport_name(u) << "]\n\n";
									}
								}
							}
						}
					}
					//End of code for recswitches
				}
			}
		}
		//--------------------------------------------------------------------------------
		// one bracket
	}
	//--------------------------------------------------------------------------------
	int r_counter = 0;//does not reach to destination
	int y_counter = 0;//reach to destination
	int E_counter = 0; // for debug
					   //^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	for (int i = 1; i < (pl - 1); i++)
	{
		if (a[0][i] == 1)
		{
			r_counter++;
			/*myfile << "\n\nflit " << i << " does not reached to its destination\n";
			myfile << "\n\nX_source " << a[4][i] << " \n";
			myfile << "\n\nY-source " << a[5][i] << " \n";
			myfile << "\n\nX_destination " << a[2][i] << " \n";
			myfile << "\n\nY-destination " << a[3][i] << " \n";*/

		}
		if (a[0][i] == 2)
		{
			y_counter++;
			//myfile << "\n\nflit " << i << " reached to its destination\n";
		}
		if ((a[0][i] > 2) || (a[0][i]<1))
		{
			cout << "\n\n Error occurred in a[i] array\n";
			E_counter++;
		}
	}
	myfile << "\n ----------------------------------------------\nStatistics of 3D ReCNoC: \n";
	cout << "\n\n Number of send_credit function calls = " << credit_sends_counter << "\n";
	myfile << "\n\n Number of send_credit function calls = " << credit_sends_counter << "\n";
	myfile << "\n\n " << y_counter << " flits reached to destionation\n";
	myfile << "\n\n " << r_counter << " flits not reached to destination\n";
	myfile << "\n\n percent of flits which reached to destination = " << ((float)y_counter / (y_counter + r_counter)) * 100 << "\n";
	cout << "\n\n Injection Rate= " << injection_rate << "\n";
	cout << "\n\n percent of flits which reached to destination = " << ((float)y_counter / (y_counter + r_counter)) * 100 << "\n";
	//cout << "\n\n percent of flits which reached to destination (based on Genereated Flits in PE_In buffers) = " << ((float)y_counter / (y_counter + r_counter)) * 100 << "\n";
	cout << "\n\nnumber of generated flits = " << pl - 1 << "\n\n";
	cout << "\n\nnumber of generated flits in PE_in buffers = " << pu - 1 << "\n\n";
	int sum_of_flit_latencies = 0;
	int maximum_delay;
	for (int i = 0; i < (pl - 1); i++)
	{
		if (a[1][i]>0) //Latency must be greater than zero
			sum_of_flit_latencies += a[1][i];
	}
	Minimum_Delay = 1000000000; //the most biggest integer possible we need for computing minimum
	maximum_delay = 0;
	for (int i = 0; i < (pl - 1); i++)
	{
		if ((a[1][i] < Minimum_Delay) && (a[1][i]>0))
		{
			Minimum_Delay = a[1][i];
		}
	}
	for (int i = 0; i < (pl - 1); i++)
	{
		if (a[1][i] > maximum_delay)
			maximum_delay = a[1][i];
	}
	cout << "\n\nnumber of Buffer_is_Full errors = " << buffer_full_counter << "\n\n";
	cout << "\n\nnumber_of_flit_number_missed_errors = " << number_of_flit_number_missed_errors << "\n\n";
	cout << "\n\nnumber_of_flit_number_missed_errors_section1 = " << number_of_flit_number_missed_errors_section1 << "\n\n";
	cout << "\n\nnumber_of_flit_number_missed_errors_section2 = " << number_of_flit_number_missed_errors_section2 << "\n\n";
	cout << "\n\nnumber_of_flit_number_missed_errors_section3 = " << number_of_flit_number_missed_errors_section3 << "\n\n";
	cout << "\n\nnumber_of_errors = " << number_of_errors << "\n\n";
	//myfile << "\n\n flit_path = " << flit_path << " \n"; //for evaluating path of flits by using a breakpoint
	myfile << "\n\n traffic generation duration = " << traffic_generation_duration << " \n";
	myfile << "\n\n Injection_Rate = " << injection_rate << " \n";
	myfile << "\n\n Network_X = " << networkx << " \n";
	myfile << " Network_Y = " << networky << " \n";
	myfile << " Network_Z = " << networkz << " \n";
	myfile << "\n\n Buffer size = " << buffer_size << " \n";
	myfile << "\n\n simulation time = " << simulation_time << " cycle\n";
	myfile << "\n\n minimum delay = " << Minimum_Delay << " cycle\n";
	myfile << "\n\n maximum delay = " << maximum_delay << " cycle\n";
	myfile << "\n\n number of generated flits = " << pl - 1 << "\n\n";
	myfile << "\n\n number of generated flits in PE_in buffers = " << pu - 1 << "\n\n";
	myfile << "\n\n Sum of all flit latencies = " << sum_of_flit_latencies << " cycle\n";
	float temp7;
	temp7 = ((float)sum_of_flit_latencies / (y_counter));
	myfile << "\n\n Average Flit latency = " << temp7 << " cycle\n";
	myfile2 << "\n Path of all flits:\n\n" << " flit [flit number][step number]\n\n";
	/*for (int i = 1; i < a_size; i++) //Storing path of flits in a file (myflie2)
	{
	for (int j = 1; j < 500; j++)
	{
	if ((flit_path[i][j].j != -858993460) || (flit_path[i][j].k != -858993460) || (flit_path[i][j].l != -858993460))
	{
	myfile2 << "\n\n Flit[" << i << "][" << j << "] : (" << flit_path[i][j].j << "," << flit_path[i][j].k << "," << flit_path[i][j].l << ")" << "--> step "<< flit_path[i][j].step<<"\n";
	}
	if ((flit_path[i][j].j == a[2][i]) && (flit_path[i][j].k == a[3][i]))
	{
	myfile2 << "\n\n flit " << i << " reached to its destination";
	}
	}
	}*/
	cout << "\n\n Last cycle which flit " << flit_trace_number << " seen was in cycle " << last_cycle_which_flit_seen << " in "<<last_place_which_flit_seen.name<<" of element "<< last_place_which_flit_seen.j<<" "<< last_place_which_flit_seen.k<<" "<< last_place_which_flit_seen.l<<" in line number "<<last_place_which_flit_seen.line_number<<"\n";
	cout << "\n\nSimulation finished press enter to exit\nResults are written in result.txt file in code directory";
	cout << "\n\n pk= " << pk << "\n"; //pk is for debugging
	getchar();
	//End of simulator code
}