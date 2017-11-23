#include "stdafx.h"

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
#include <vector>
#include<time.h>
#include"random_arbiter_function.h"
using namespace std;

//----------------------------------------------------
int outport_arbiter_function(bool arbitration_array[5], int j, int k, int l, int t)//This function returns index of winner inport. priority Arbiter function: in this arbiter For an example, The port one have priority to port two
{
	//priority arbiter
	int checker = 0; // if arbiter_array does not have any 1 in it then 
	for (int i = 1; i < 6; i++)//For all input ports************************************ 3D must be completed
	{
		if (arbitration_array[i] == 1)//For an example, The port one have priority to port two
		{
			return i; //
			checker++;
		}
	}
	if (checker == 0)
	{
		cout << "\n" << "Error occurred at arbitration. Arbitration_array does not contains any one\n";
		return 7;
	}
}