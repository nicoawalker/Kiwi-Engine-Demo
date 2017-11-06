#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include <Core\Component.h>
#include <Physics\ICollisionEventListener.h>

#include <Core\Vector3d.h>

namespace Kiwi
{
	class Entity;
	class Rigidbody;
	class Transform;
}

class VoxelTerrain;

class Projectile :
	public Kiwi::Component,
	public Kiwi::ICollisionEventListener
{
protected:

	Kiwi::Entity* m_source;

	Kiwi::Rigidbody* m_rigidbody;
	Kiwi::Transform* m_transform;

	VoxelTerrain* m_terrain;

	Kiwi::Vector3d m_velocity;

	double m_radius;
	double m_mass;

protected:

	void _OnFixedUpdate();
	void _OnAttached();

	void OnCollisionEnter( const Kiwi::CollisionEvent& e );

public:

	Projectile( Kiwi::Entity* source, double radius, const Kiwi::Vector3d& velocity, double mass = 1.0 );

	void SetVelocity( const Kiwi::Vector3d& velocity );

	double GetForce();

	Kiwi::Entity* GetSource()const { return m_source; }

};

#endif