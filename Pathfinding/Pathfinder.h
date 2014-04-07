#pragma once
#include <queue>
#include <map>

using namespace std;

struct neighbour
{
	int nodeID;
	int f;
	int g;
	int h;
} ;

enum TERRAIN_TYPE {WATER, PLAIN, DUNE, FOREST = 4};

struct terrainType
{
	int nodeID;
	TERRAIN_TYPE value;
} ;

struct node
{
	int xPos;
	int yPos;	
	int iD;
	bool passable;
	neighbour connections[6];
} ;

const int GRID_X = 11;
const int GRID_Y = 14;

class CompareCost
{
public:
	// order in opposite direction so greatest f value is at the top
	bool operator()(neighbour& n1, neighbour& n2)
	{
		return n1.f > n2.f;
	}
};

class CPathfinder
{
public:
	CPathfinder();
	~CPathfinder() {};

	void InitGrid(); // create nodes
	void InitTerrain(); // load terrain values
	void InitConnections(); // determine node connections

	bool FindPath(int iD1, int iD2); // find a path using ids
	bool FindPath(int x1, int y1, int x2, int y2); // find a path using coor

	void drawRoute(map<int, int> map, int start, int end, int displayType); // draws a route from a map

	int CalcH(node n1, node n2); // calc distance between nodes

	terrainType& GetTerrainREF(int iD); // returns address of terrain (for writing)
	terrainType GetTerrain(int iD) const; // returns terrain (for reading)

	node GetNode(int iD) const; // returns a node (for reading)
	node& GetNodeREF(int iD); // returns a nodes address (for writing)
	int GetNodeID(int x, int y) const; // returns a node id from x,y coor
	
	void UpdateConCost(int previousG, neighbour& next, node end); // calculates new cost to travel
	
	void SetTerrain(int iD, TERRAIN_TYPE ter); // sets a node ids terrain value

	void printNode(node n); // prints node info (x,y,ID)

	bool findNode(priority_queue<neighbour, vector<neighbour>, CompareCost> que, node n); // true if node exists in priority que
	bool findNode(vector<node> que, node n); // true if node exists in vector

private:
	node nodeList[GRID_X][GRID_Y];
	terrainType terrainMap[GRID_X * GRID_Y];
};

