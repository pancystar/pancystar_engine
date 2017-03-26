#pragma once
#include <PxPhysicsAPI.h>
#include<vehicle/PxVehicleSDK.h>
#include<DDSTextureLoader.h>
#include"pancy_d3d11_basic.h"
#ifdef _DEBUG
#pragma comment(lib,"PhysX3DEBUG_x86.lib")
#pragma comment(lib,"PhysX3CommonDEBUG_x86.lib")
#pragma comment(lib,"PhysX3ExtensionsDEBUG.lib")
#pragma comment(lib,"PhysXVisualDebuggerSDKDEBUG.lib")
#pragma comment(lib,"PhysX3CharacterKinematicDEBUG_x86.lib")
#else
#pragma comment(lib,"PhysX3_x86.lib")
#pragma comment(lib,"PhysX3Common_x86.lib")
#pragma comment(lib,"PhysX3Extensions.lib")
#pragma comment(lib,"PhysXVisualDebuggerSDK.lib")
#pragma comment(lib,"PhysX3CharacterKinematic_x86.lib")
#endif
class pancy_collision_callback : public physx::PxSimulationEventCallback
{
public:
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {};
	void onWake(physx::PxActor** actors, physx::PxU32 count) {};
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}
	void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		int a = 0;
	}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		int a = 0;
	};
};
class pancy_physx
{
	physx::PxPhysics* physic_device;
	physx::PxScene *now_scene;
	physx::PxFoundation *foundation_need;
	physx::PxDefaultAllocator allocator_defaule;
	physx::PxTolerancesScale scale;

	physx::PxControllerManager *controller_manager;
	//physx::PxController *player;
	//physx::PxMaterial *mat_force;
	ID3D11Device                *device_pancy;
	ID3D11DeviceContext         *contex_pancy;
public:
	pancy_physx(ID3D11Device *device_need, ID3D11DeviceContext *contex_need);
	HRESULT create_terrain(physx::PxHeightFieldDesc height_data, physx::PxVec3 scal_data, physx::PxTransform terrain_trans, physx::PxMaterial *mat_force);
	HRESULT create_dynamic_box(physx::PxTransform position_st, physx::PxBoxGeometry geometry_need, physx::PxMaterial *mat_need, physx::PxRigidDynamic **physic_out);
	HRESULT create_charactor(physx::PxCapsuleControllerDesc charactor_desc, physx::PxController **charactor_out);
	HRESULT create_charactor(physx::PxBoxControllerDesc charactor_desc, physx::PxController **charactor_out);
	HRESULT create();
	HRESULT add_actor(physx::PxRigidDynamic *box);
	HRESULT remove_actor(physx::PxRigidDynamic *box) {};
	physx::PxMaterial *create_material(physx::PxReal staticFriction, physx::PxReal dynamicFriction, physx::PxReal restitution) {return physic_device->createMaterial(staticFriction, dynamicFriction, restitution);};
	void update(float delta_time);
	XMFLOAT3 get_position(physx::PxRigidDynamic *physic_body_in);
	void get_rotation_data(physx::PxRigidDynamic *physic_body_in,float &angle, XMFLOAT3 &vector);
	void release();
};