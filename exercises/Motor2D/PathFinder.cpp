#include "PathFinder.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Pathfinding.h"

PathFinder::PathFinder() : last_path(DEFAULT_PATH_LENGTH), pathCompleted(false),available(true)
{
	LOG("PathFinder created");
	
}

PathFinder::~PathFinder()
{
}



void PathFinder::PreparePath(const iPoint& origin, const iPoint& destination)
{
	// Add the origin tile to open
	if (open.GetNodeLowestScore() == NULL)
		open.list.add(PathNode(0, origin.DistanceTo(destination), origin, nullptr));

	this->origin = origin;
	this->destination = destination;
	available = false;
	
}

bool PathFinder::IteratePath()
{
	//TODO 2: This function won't need a loop inside anymore, we are controlling this loop outside
	bool ret = true;
	while (ret) {
		PathNode* node = new PathNode(open.GetNodeLowestScore()->data);
		close.list.add(*node);
		open.list.del(open.Find(node->pos));

		if (node->pos == destination) {
			const PathNode* iterator = node;

			last_path.Clear();
			// Backtrack to create the final path
			for (iterator; iterator->pos != origin; iterator = iterator->parent)
			{
				last_path.PushBack(iterator->pos);
			}

			last_path.PushBack(origin);

			last_path.Flip();
			pathCompleted = true;
			available = true;
			open.list.clear();
			close.list.clear();
			RELEASE(node);
			ret = false;
			return ret;
		}

		PathList adjacentNodes;
		uint numNodes = node->FindWalkableAdjacents(adjacentNodes);

		for (uint i = 0; i < numNodes; i++)
		{
			// ignore nodes in the closed list
			if (close.Find(adjacentNodes.list[i].pos) == NULL) {
				// If it is NOT found, calculate its F and add it to the open list
				if (open.Find(adjacentNodes.list[i].pos) == NULL) {
					adjacentNodes.list[i].CalculateF(destination);
					open.list.add(adjacentNodes.list[i]);
				}
				// If it is already in the open list, check if it is a better path (compare G)
				else {
					if (adjacentNodes.list[i].g < open.Find(adjacentNodes.list[i].pos)->data.g) {
						// If it is a better path, Update the parent
						adjacentNodes.list[i].CalculateF(destination);
						open.list.del(open.Find(adjacentNodes.list[i].pos));
						open.list.add(adjacentNodes.list[i]);
					}
				}
			}
		}
		return ret;
	}
}



// To request all tiles involved in the last generated path
const p2DynArray<iPoint>* PathFinder::GetLastPath() const
{
	return &last_path;
}

bool PathFinder::Update()
{
	//TODO 2: Make a loop to take control on how many times the function "IteratePath" should be called in one frame
	bool ret = true;

	if (ret)
		ret = IteratePath();
	
	return ret;
}


#pragma region PathList


// PathList ------------------------------------------------------------------------
// Looks for a node in this list and returns it's list node or NULL
// ---------------------------------------------------------------------------------
p2List_item<PathNode>* PathList::Find(const iPoint& point) const
{
	p2List_item<PathNode>* item = list.start;
	while (item)
	{
		if (item->data.pos == point)
			return item;
		item = item->next;
	}
	return NULL;
}

// PathList ------------------------------------------------------------------------
// Returns the Pathnode with lowest score in this list or NULL if empty
// ---------------------------------------------------------------------------------
p2List_item<PathNode>* PathList::GetNodeLowestScore() const
{
	p2List_item<PathNode>* ret = NULL;
	int min = 65535;

	p2List_item<PathNode>* item = list.end;
	while (item)
	{
		if (item->data.Score() < min)
		{
			min = item->data.Score();
			ret = item;
		}
		item = item->prev;
	}
	return ret;
}
#pragma endregion

#pragma region PathNode



// PathNode -------------------------------------------------------------------------
// Convenient constructors
// ----------------------------------------------------------------------------------
PathNode::PathNode() : g(-1), h(-1), pos(-1, -1), parent(NULL)
{}

PathNode::PathNode(int g, int h, const iPoint& pos, const PathNode* parent) : g(g), h(h), pos(pos), parent(parent)
{}

PathNode::PathNode(const PathNode& node) : g(node.g), h(node.h), pos(node.pos), parent(node.parent)
{}

// PathNode -------------------------------------------------------------------------
// Fills a list (PathList) of all valid adjacent pathnodes
// ----------------------------------------------------------------------------------
uint PathNode::FindWalkableAdjacents(PathList& list_to_fill) const
{
	iPoint cell;
	uint before = list_to_fill.list.count();

	// north
	cell.create(pos.x, pos.y + 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// south
	cell.create(pos.x, pos.y - 1);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// east
	cell.create(pos.x + 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	// west
	cell.create(pos.x - 1, pos.y);
	if (App->pathfinding->IsWalkable(cell))
		list_to_fill.list.add(PathNode(-1, -1, cell, this));

	return list_to_fill.list.count();
}

// PathNode -------------------------------------------------------------------------
// Calculates this tile score
// ----------------------------------------------------------------------------------
int PathNode::Score() const
{
	return g + h;
}

// PathNode -------------------------------------------------------------------------
// Calculate the F for a specific destination tile
// ----------------------------------------------------------------------------------
int PathNode::CalculateF(const iPoint& destination)
{
	g = parent->g + 1;
	h = pos.DistanceManhattan(destination);

	return g + h;
}
#pragma endregion

