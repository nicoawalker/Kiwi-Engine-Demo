#ifndef _PATHFINDER_H_
#define _PATHFINDER_H_

#include <Core\Component.h>
#include <Core\Vector2d.h>
#include <Core\Vector3d.h>

namespace Pathing
{
	struct Node
	{
		Kiwi::Vector3d position;
		double radius;

		Node() {}
		Node( const Kiwi::Vector3d& pos )
		{
			position = pos;
			radius = 1.0;
		}

	};

	struct NodePath
	{
		std::vector<Node> nodes;
	};
};

class NavMesh;

class Pathfinder :
	public Kiwi::Component
{
protected:

	struct VectorHash
	{
		size_t operator()( const Kiwi::Vector2d& v ) const
		{
			size_t h1 = std::hash<double>()(v.x);
			size_t h2 = std::hash<double>()(v.y);
			return (h1 ^ (h2 << 1));
		}
	};

	struct VectorEquals
	{
		bool operator()( const Kiwi::Vector2d& lhs, const Kiwi::Vector2d& rhs ) const
		{
			return (lhs.x == rhs.x) && (lhs.y == rhs.y);
		}
	};

	Pathing::NodePath* m_nodePath;

	Kiwi::Vector3d m_pathTarget; //target to path to
	Pathing::Node* m_nodeTarget; //next node in the node path to the path target

	double m_minNodeEqDist; //minimum distance at which the entity is considered to have arrived at a node

	double m_maxDistance;

	unsigned int m_maxJump;
	unsigned int m_maxFall;
	unsigned int m_minClearance;

	bool m_pathingEnabled;

protected:

	void _OnAttached();
	void _OnFixedUpdate();

	/*heuristic used by the AStar algorithm*/
	double _Heuristic_AStar( const Kiwi::Vector3d& a, const Kiwi::Vector3d& b );

	/*Generates a node path to the target using the AStar algorithm*/
	Pathing::NodePath* _GenerateNodePath_AStar( const Kiwi::Vector3d& origin, const Kiwi::Vector3d& target, NavMesh* navMesh );

public:

	Pathfinder();
	~Pathfinder();

	void SetMaxDistance( double maxDistance ) { m_maxDistance = maxDistance; }
	void SetMaxJump( unsigned int maxJump ) { m_maxJump = maxJump; }
	void SetMaxFall( unsigned int maxFall ) { m_maxFall = maxFall; }
	void SetMinClearance( unsigned int minClearance ) { m_minClearance = minClearance; }

	void GeneratePath( const Kiwi::Vector3d& targetPos );

	/*returns the end position that is being pathed to*/
	const Kiwi::Vector3d& GetPathTarget()const { return m_pathTarget; }

	/*returns the next node in the path*/
	Pathing::Node* GetTargetNode()const { return m_nodeTarget; }

};


#endif