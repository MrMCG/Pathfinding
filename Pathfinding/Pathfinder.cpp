#include "Pathfinder.h"
#include <iostream>
#include <istream>
#include <fstream>
#include <math.h>

using namespace std;

CPathfinder::CPathfinder()
{
	InitGrid();
	InitTerrain();
	InitConnections();
}

// create grid of nodes
// follows "even-r" horozontal layout (offset)
// with origin at top left
/* eg: 4 x 4:

	  00  10  20  30 
	01  11  21  31
	  02  12  22  32
	03  13  23  33

	nodeID goes as follows:
	00 = 0
	10 = 1
	20 = 2
	30 = 3
	01 = 4 etc
*/
//  ! **************** A FULL GRID IS IN hexMap.jpg **************** !
void CPathfinder::InitGrid()
{
	for (int i = 0; i < GRID_X; i++)
	{
		for (int j = 0; j < GRID_Y; j++)
		{
			nodeList[i][j].xPos = i; 
			nodeList[i][j].yPos = j;
			nodeList[i][j].iD = (j * GRID_X) + i;
			nodeList[i][j].passable = false;	// all init nodes are NOT passable
		}
	}
}

void CPathfinder::InitTerrain()
{
	for (int i = 0; i < (GRID_X * GRID_Y); i++)
		{
			terrainMap[i].value = PLAIN; // all nodes are init to PLAIN
			terrainMap[i].nodeID = i;
		}

	// read in map file for terrain
	ifstream file("map.txt");
	if (file)
	{
		int x,y,t;
		while (file >> x >> y >> t)
		{
			switch (t)
			{
			case 0:
				SetTerrain(GetNodeID(x,y), WATER);
				break;
			case 1:
				GetNodeREF(GetNodeID(x,y)).passable = true;
				break;
			case 2:
				GetNodeREF(GetNodeID(x,y)).passable = true;
				SetTerrain(GetNodeID(x,y), DUNE);
				break;
			case 4:
				GetNodeREF(GetNodeID(x,y)).passable = true;
				SetTerrain(GetNodeID(x,y), FOREST);
				break;
			default:
				break;

			}
		}
	} else 
	{
		cout << " !! ERROR: FILE NOT FOUND: map.txt !!" << endl;
		cout << " !! ALL TERRAIN SET TO: PLAIN !!" << endl;
		system("pause");

		for (int i = 0; i < (GRID_X * GRID_Y); i++)
		{
			terrainMap[i].value = PLAIN;
			terrainMap[i].nodeID = i;
		}
	}
}

void CPathfinder::UpdateConCost(int previousG, neighbour& next, node end)
{
	// set travel cost to terrain value
	next.g = GetTerrain(next.nodeID).value + previousG;

	// get distance from conneced node to end
	next.h = CalcH(GetNode(next.nodeID), end); 

	// calc f value by g + h
	next.f = next.g + next.h;	
}

bool CPathfinder::FindPath(int x1, int y1, int x2, int y2)
{
	return FindPath(GetNodeID(x1, y1), GetNodeID(x2, y2));
}

bool CPathfinder::FindPath(int iD1, int iD2)
{
	if (iD1 == iD2) // if theyre the same node
	{
		cout << "** Same Node! **" << endl;
		return true;
	}

	if ( (iD1 > GRID_Y*GRID_X || iD1 < 0) || // if node 1 is out of bounds
		 (iD2 > GRID_Y*GRID_X || iD2 < 0) ) // if node 2 is out of bounds
	{
		cout << "** Node Out Of Bounds! **" << endl;
		return false;
	}

	node start = GetNode(iD1); // get nodes by iD
	node end = GetNode(iD2);

	// if end node is not passable, cant place city here
	// could be water or out of map area
	if (!end.passable) 
	{
		cout << "** Cant Place City Here! **" << endl;
		return false;
	}

	cout << " -- Starting Search --" << endl;
	cout << "Beginning: \t";
	cout << start.xPos << "," << start.yPos << " | id: " << start.iD << endl;
	cout << "Ending: \t";
	cout << end.xPos << "," << end.yPos << " | id: " << end.iD << endl;

	priority_queue<neighbour, vector<neighbour>, CompareCost> openList;
	vector<node> closedList;
	map<int, int> parentMap;

	bool found = false;
	bool begin = true;
	node current = node();
	int g = 0;

	while (!found)
	{
		if (begin) // if this is the start
		{
			current = start; // use start node as first in openList to get connections
		} else
		{

			if (openList.empty()) // check if openList is empty
			{
				cout << "** No route possible **" << endl;
				break;
			}
			// set the lowest f node as current (to search connections)
			current = GetNode(openList.top().nodeID);

			// get current g value
			g = openList.top().g;

			// add the node itself to the closed list
			closedList.push_back(GetNode(openList.top().nodeID));
	
			// remove from the open list
			openList.pop();
		}
		
		// check all connections to that node
		for (int i = 0; i < 6; i++)
		{
			// check if adjacent node is end
			if (current.connections[i].nodeID == end.iD)
			{
				cout << "** Route Found! **" << endl;
				int parent = closedList.back().iD;
				parentMap[current.connections[i].nodeID] = parent; // set parent id of end node
				cout << "Cost of travel: " << g + GetTerrain(current.connections[i].nodeID).value << endl; // print cost of travel
				drawRoute(parentMap, iD1, current.connections[i].nodeID, 0); // draw route using node id
				drawRoute(parentMap, iD1, current.connections[i].nodeID, 1); // draw route using x,y coor
				found = true;
				break;
			}

			// add connected nodes (connections) from current node to open list
			// make sure its not already in open list
			if (current.connections[i].nodeID != -1 && // node exists
				GetNode(current.connections[i].nodeID).passable && // node is passable
				!findNode(closedList, GetNode(current.connections[i].nodeID)) && // node is not in closed list
				!findNode(openList, GetNode(current.connections[i].nodeID))) // node is not  in openList		 
			{
				UpdateConCost(g, current.connections[i], end); // update to get new cost
				openList.push(current.connections[i]); // add to open list
				parentMap[current.connections[i].nodeID] = current.iD; // set parent
			}
		}

		if (begin) // if this is the first loop, add the start node to the closed list
		{
			closedList.push_back(current);
			begin = false;
		}
	}

	// debug info
	bool debug = false;
	if (debug)
	{
		cout << " -DEBUG- Printing OpenList -DEBUG- " << endl;
		while (!openList.empty())
		{
			printNode(GetNode(openList.top().nodeID));
			cout << " : " << openList.top().f << endl;
			openList.pop();
		}
	
		cout << " -DEBUG- Printing ClosedList -DEBUG- " << endl;
		while (!closedList.empty())
		{
			printNode(GetNode(closedList.back().iD));
			cout << endl;
			closedList.pop_back();
		}

		cout << " -DEBUG- Printing Map -DEBUG- " << endl;
		for (map<int, int>::iterator it = parentMap.begin(); it != parentMap.end(); ++it)
		{
			cout << it->first << " -> " << it->second << endl;
		}
	}

	return found; // return if path has been found
}

void CPathfinder::drawRoute(map<int, int> map, int start, int end, int displayType)
{
	vector<int> route;

	int from = end;

	route.push_back(end);

	while (from != start)
	{
		route.push_back(map[from]);
		from = route.back();
	}
	 
	if (displayType == 0)
	{
		cout << " -- Printing Route ( ID )-- " << endl;
	} else
	{
		cout << " -- Printing Route ( x,y )-- " << endl;
	} 
	reverse(route.begin(), route.end());
	for (vector<int>::iterator it = route.begin(); it < route.end(); ++it)
		{
			if (displayType == 0)
			{
				cout << *it;
			} else
			{
				cout << GetNode(*it).xPos << "," << GetNode(*it).yPos;
			}

			if (it != route.end()-1)
			{
				cout << " -> ";
			}
		}
	cout << endl;
}

// returns true if a node is in the priority que
bool CPathfinder::findNode(priority_queue<neighbour, vector<neighbour>, CompareCost> que, node n)
{
	while (!que.empty())
	{
		if (que.top().nodeID == n.iD)
		{
			return true;
		}
		que.pop();
	}
	return false;
}

// returns true if a node is in the vector
bool CPathfinder::findNode(vector<node> que, node n)
{
	while (!que.empty())
	{
		if (que.back().iD == n.iD)
		{
			return true;
		}
		que.pop_back();
	}
	return false;
}

// prints a given nodes info
void CPathfinder::printNode(node n)
{
	cout << n.xPos << "," << n.yPos << " : " << n.iD;
}

// calculates the heuristic
int CPathfinder::CalcH(node n1, node n2)
{
	// manhattan distance
	int dx = n2.xPos - n1.xPos;
	int dy = n2.yPos - n1.yPos;

	// int truncation should not give an overestimation
	// however an underestimation is possible
	return (int) sqrt((double)(dx*dx) + (dy*dy));
}

// returns the address of a terrain (for editing)
terrainType& CPathfinder::GetTerrainREF(int iD)
{
	if (terrainMap[iD].nodeID == iD)
	{
		return terrainMap[iD];
	} else
	{
		for (int i = 0; i < (GRID_X * GRID_Y); i++)
		{	
			if (terrainMap[i].nodeID == iD)
			{
				return terrainMap[i];
			}
		} 

		cout << "ERROR: TERRAIN by ID NOT FOUND!: " << iD << endl;
		system("pause");
	}
}

// returns a terrain (for accessing)
terrainType CPathfinder::GetTerrain(int iD) const
{
	if (terrainMap[iD].nodeID == iD)
	{
		return terrainMap[iD];
	} else
	{
		for (int i = 0; i < (GRID_X * GRID_Y); i++)
		{	
			if (terrainMap[i].nodeID == iD)
			{
				return terrainMap[i];
			}
		} 

		cout << "ERROR: TERRAIN by ID NOT FOUND!: " << iD << endl;
		system("pause");
	}
}

// sets a terrain (allows for chainging a nodes terrain value on the fly)
void CPathfinder::SetTerrain(int iD, TERRAIN_TYPE ter)
{
	GetTerrainREF(iD).value = ter;
}

// returns a node (for reading)
node CPathfinder::GetNode(int iD) const
{
	if (nodeList[iD%GRID_X][iD%GRID_X].iD == iD)
	{
		return nodeList[iD/GRID_X][iD%GRID_X];
	} else
	{
		for (int i = 0; i < GRID_X; i++)
		{
			for (int j = 0; j < GRID_Y; j++)
			{
				if (nodeList[i][j].iD == iD)
				{
					return nodeList[i][j];
				}
			}
		} 

		cout << "ERROR: ID NOT FOUND!: " << iD << endl;
		system("pause");
	}
}

// returns a nodes address (for writing)
node& CPathfinder::GetNodeREF(int iD)
{
	if (nodeList[iD%GRID_X][iD%GRID_X].iD == iD)
	{
		return nodeList[iD/GRID_X][iD%GRID_X];
	} else
	{
		for (int i = 0; i < GRID_X; i++)
		{
			for (int j = 0; j < GRID_Y; j++)
			{
				if (nodeList[i][j].iD == iD)
				{
					return nodeList[i][j];
				}
			}
		} 

		cout << "ERROR: ID NOT FOUND!: " << iD << endl;
		system("pause");
	}
}

// returns node ID from x,y coor
int CPathfinder::GetNodeID(int x, int y) const
{
	if (nodeList[x][y].xPos == x && nodeList[x][y].yPos == y)
	{
		return nodeList[x][y].iD;
	} else
	{
		for (int i = 0; i < GRID_X; i++)
		{
			for (int j = 0; j < GRID_Y; j++)
			{
				if (nodeList[i][j].xPos == x && nodeList[i][j].yPos == y)
				{
					return nodeList[i][j].iD;
				}
			}
		} 

		cout << "ERROR: NODE NOT FOUND!: " << x << "," << y << endl;
		system("pause");
	}
}

// determines all node connections
void CPathfinder::InitConnections()
{
	// create connections for those nodes
	// use -1 to indicate out of bounds node
	// starts at north east node and goes clockwise
	for (int i = 0; i < GRID_X; i++)
	{
		for (int j = 0; j < GRID_Y; j++)
		{
			int x = nodeList[i][j].xPos;
			int y = nodeList[i][j].yPos;

			// if y pos is even
			if (y % 2 == 0)
			{
				// if x is +ve out of bounds
				if (x+1 >= GRID_X)
				{
					nodeList[i][j].connections[0].nodeID = -1;
					nodeList[i][j].connections[1].nodeID = -1;
					nodeList[i][j].connections[2].nodeID = -1;

					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{
						nodeList[i][j].connections[3].nodeID = -1;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;

					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = -1;
					} else 
					{
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;
					}

				// if x is -ve out of bounds
				} else if (x-1 < 0)
				{
					nodeList[i][j].connections[4].nodeID = -1;

					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{				
						nodeList[i][j].connections[0].nodeID = nodeList[x + 1][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = -1;
						nodeList[i][j].connections[3].nodeID = -1;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;

					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						
						nodeList[i][j].connections[0].nodeID = -1;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x + 1][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[5].nodeID = -1;
					} else 
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x + 1][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x + 1][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;
					}

				} else
				{
					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{				
						nodeList[i][j].connections[0].nodeID = nodeList[x + 1][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = -1;
						nodeList[i][j].connections[3].nodeID = -1;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;

					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						
						nodeList[i][j].connections[0].nodeID = -1;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x + 1][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = -1;
					} else 
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x + 1][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x + 1][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD;
					}
				}

				/* used for even y nodes to determine connections
				nodeList[i][j].connections[0].nodeID = nodeList[x + 1][y - 1].iD; // north east
				nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD; // east
				nodeList[i][j].connections[2].nodeID = nodeList[x + 1][y + 1].iD; // south east
				nodeList[i][j].connections[3].nodeID = nodeList[x    ][y + 1].iD; // south west
				nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD; // west
				nodeList[i][j].connections[5].nodeID = nodeList[x    ][y - 1].iD; // north west
				*/

			} else
			{

				// if x is +ve out of bounds
				if (x+1 >= GRID_X)
				{
					
					nodeList[i][j].connections[1].nodeID = -1;
				

					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[2].nodeID = -1;
						nodeList[i][j].connections[3].nodeID = -1;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x - 1][y - 1].iD;

					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						nodeList[i][j].connections[0].nodeID = -1;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x - 1][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = -1;
					} else 
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x - 1][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x - 1][y - 1].iD;
					}

				// if x is -ve out of bounds
				} else if (x-1 < 0)
				{
					nodeList[i][j].connections[3].nodeID = -1;
					nodeList[i][j].connections[4].nodeID = -1;
					nodeList[i][j].connections[5].nodeID = -1;

					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{				
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = -1;


					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						
						nodeList[i][j].connections[0].nodeID = -1;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;

					} else 
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;

					}

				} else
				{
					// if y is +ve out of bounds
					if (y+1 >= GRID_Y)
					{				
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = -1;
						nodeList[i][j].connections[3].nodeID = -1;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x - 1][y - 1].iD;

					// if y is -ve out of bounds
					} else if (y-1 < 0)
					{
						
						nodeList[i][j].connections[0].nodeID = -1;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x - 1][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = -1;
					} else 
					{
						nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD;
						nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD;
						nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD;
						nodeList[i][j].connections[3].nodeID = nodeList[x - 1][y + 1].iD;
						nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD;
						nodeList[i][j].connections[5].nodeID = nodeList[x - 1][y - 1].iD;
					}
				}

				/* used for odd y nodes to determine connections
				nodeList[i][j].connections[0].nodeID = nodeList[x    ][y - 1].iD; // north east
				nodeList[i][j].connections[1].nodeID = nodeList[x + 1][y    ].iD; // east
				nodeList[i][j].connections[2].nodeID = nodeList[x    ][y + 1].iD; // south east
				nodeList[i][j].connections[3].nodeID = nodeList[x - 1][y + 1].iD; // south west
				nodeList[i][j].connections[4].nodeID = nodeList[x - 1][y    ].iD; // west
				nodeList[i][j].connections[5].nodeID = nodeList[x - 1][y - 1].iD; // north west
				*/
			}
		}
	}
}