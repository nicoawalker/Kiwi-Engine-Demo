#include "Projectile.h"
#include "VoxelTerrain.h"

#include <Core\Entity.h>
#include <Core\Scene.h>

#include <Physics\Rigidbody.h>
#include <Physics\SphereCollider.h>

Projectile::Projectile( Kiwi::Entity* source, double radius, const Kiwi::Vector3d& velocity, double mass )
{

	m_source = source;
	m_radius = radius;
	m_mass = max( 0.00001, mass);
	m_velocity = velocity;
	
	m_rigidbody = 0;
	m_transform = 0;
	m_terrain = 0;

}

void Projectile::_OnAttached()
{

	m_rigidbody = m_entity->AttachComponent( new Kiwi::Rigidbody() );
	m_rigidbody->SetMass( m_mass );
	Kiwi::Collider* collider = m_rigidbody->AttachComponent( new Kiwi::SphereCollider( m_radius ) );
	collider->SetTrigger( true );
	collider->AddListener( this );

	m_rigidbody->ApplyForce( m_velocity * m_mass );

	m_terrain = m_entity->GetScene()->GetTerrain<VoxelTerrain*>();
	assert( m_terrain != 0 );

	m_transform = m_entity->FindComponent<Kiwi::Transform>();
	m_transform->SetScale( m_radius );

}

void Projectile::OnCollisionEnter( const Kiwi::CollisionEvent& e )
{

	if( e.GetTarget() != 0 && e.GetTarget()->HasTag( L"enemy" ) )
	{
		m_entity->Shutdown();
	}

}

void Projectile::_OnFixedUpdate()
{

	if( m_transform->GetPosition().y <= 0.0 )
	{
		m_entity->Shutdown();

	}else if( m_terrain != 0 )
	{
		Voxel* voxel = m_terrain->FindSurfaceVoxelAtPosition( m_transform->GetPosition() );
		if( voxel != 0 )
		{
			m_entity->Shutdown();
		}
	}

}

void Projectile::SetVelocity( const Kiwi::Vector3d& velocity )
{

	m_velocity = velocity;

	if( m_entity != 0 )
	{
		m_rigidbody->ApplyForce( m_velocity * m_mass );
	}

}

double Projectile::GetForce()
{

	double force = 0.0;
	if( m_rigidbody != 0 )
	{
		force = m_rigidbody->GetExertedForce().Magnitude();
	}

	return force;

}