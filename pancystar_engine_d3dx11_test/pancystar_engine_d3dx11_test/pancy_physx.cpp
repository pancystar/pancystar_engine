#include"pancy_physx.h"
pancy_physx::pancy_physx(ID3D11Device *device_need, ID3D11DeviceContext *contex_need)
{
	physic_device = NULL;
	device_pancy = device_need;
	contex_pancy = contex_need;
	now_scene = NULL;
	foundation_need = NULL;
}
HRESULT pancy_physx::create()
{
	physx::PxDefaultErrorCallback error_message;
	foundation_need = PxCreateFoundation(PX_PHYSICS_VERSION, allocator_defaule, error_message);
	physic_device = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation_need, scale);
	if (physic_device == NULL)
	{
		MessageBox(0, L"create physx device error", L"tip", MB_OK);
		return E_FAIL;
	}
	if (physic_device->getPvdConnectionManager())
	{
		const char* pvd_host_ip = "127.0.0.1";
		int port = 5425;
		const int timeout = 100;
		physx::PxVisualDebuggerConnectionFlags connectflag = physx::PxVisualDebuggerExt::getAllConnectionFlags();
		physx::debugger::comm::PvdConnection *theconnection = physx::PxVisualDebuggerExt::createConnection(physic_device->getPvdConnectionManager(), pvd_host_ip, port, timeout, connectflag);
		if (theconnection)
		{
			MessageBox(0, L"debug connection success", L"tip", MB_OK);
		}
	}
//	mat_force = physic_device->createMaterial(0.5, 0.5, 0.5);
	//plan_pos = new physx::PxTransform(physx::PxVec3(0.0f, 0.0f, 0.0f), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0.0f, 1.0f)));
	

	physx::PxSimulationEventCallback *callback_scene = new pancy_collision_callback();
	physx::PxSceneDesc scene_desc(physic_device->getTolerancesScale());
	scene_desc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	scene_desc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	scene_desc.filterShader = physx::PxDefaultSimulationFilterShader;
	scene_desc.simulationEventCallback = callback_scene;

	now_scene = physic_device->createScene(scene_desc);
	/*
	plane = physic_device->createRigidStatic(*plan_pos);
	plane->createShape(physx::PxPlaneGeometry(), *mat_force);
	//now_scene->addActor(*plane);
	*/
	//physx::PxShape *trigger;
	//plane->getShapes(&trigger, 1);
	//trigger->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	//trigger->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	/*
	box = physx::PxCreateDynamic(*physic_device, *box_pos, *box_geo, *mat_force, 1);
	now_scene->addActor(*box);
	//box->setMass(0.2f);
	physx::PxReal rec = box->getMass();
	physx::PxVec3 dir_force(19.0f, 0.0f, 0.0f);
	box->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	*/
	//physx::PxShape *trigger1;
	//box->getShapes(&trigger1, 1);
	//trigger1->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
	//trigger1->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);

	return S_OK;
}
HRESULT pancy_physx::create_terrain(physx::PxHeightFieldDesc height_data, physx::PxVec3 scal_data, physx::PxTransform terrain_trans, physx::PxMaterial *mat_force)
{
	physx::PxHeightField* aHeightField = physic_device->createHeightField(height_data);
	physx::PxHeightFieldGeometry hfGeom(aHeightField, physx::PxMeshGeometryFlags(), scal_data.x, scal_data.y, scal_data.z);
	physx::PxRigidStatic* aHeightFieldActor = physic_device->createRigidStatic(terrain_trans);
	if (aHeightFieldActor == NULL) 
	{
		return E_FAIL;
	}
	physx::PxShape* aHeightFieldShape = aHeightFieldActor->createShape(hfGeom, *mat_force);
	if (aHeightFieldShape == NULL)
	{
		return E_FAIL;
	}
	now_scene->addActor(*aHeightFieldActor);
	return S_OK;
}
HRESULT pancy_physx::create_dynamic_box(physx::PxTransform position_st, physx::PxBoxGeometry geometry_need, physx::PxMaterial *mat_need,physx::PxRigidDynamic **physic_out)
{
	physx::PxRigidDynamic *box_need;
	box_need = physx::PxCreateDynamic(*physic_device, position_st, geometry_need, *mat_need, 1);
	if (box_need == NULL) 
	{
		return E_FAIL;
	}
	now_scene->addActor(*box_need);
	//box->setMass(0.2f);
	physx::PxReal rec = box_need->getMass();
	physx::PxVec3 dir_force(19.0f, 0.0f, 0.0f);
	box_need->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	*physic_out = box_need;
	return S_OK;
}
HRESULT pancy_physx::add_actor(physx::PxRigidDynamic *box)
{
	now_scene->addActor(*box);
	return S_OK;
}
void pancy_physx::get_rotation_data(physx::PxRigidDynamic *physic_body_in,float &angle, XMFLOAT3 &vector)
{
	physx::PxReal angle_need;
	physx::PxVec3 vector_need;
	physic_body_in->getGlobalPose().q.toRadiansAndUnitAxis(angle_need, vector_need);
	angle = static_cast<float>(angle_need);
	vector.x = static_cast<float>(vector_need.x);
	vector.y = static_cast<float>(vector_need.y);
	vector.z = static_cast<float>(vector_need.z);
}
void pancy_physx::update(float delta_time)
{
	//physx::PxReal rec = box->getMass();
	//physx::PxVec3 dir_force(0.0f, 9.8f, 0.0f);
	//box->addForce(dir_force, physx::PxForceMode::eIMPULSE);
	now_scene->simulate(static_cast<physx::PxReal>(delta_time));
	now_scene->fetchResults(true);
}
void pancy_physx::release()
{
	now_scene->release();
	physic_device->release();
	foundation_need->release();
}
XMFLOAT3 pancy_physx::get_position(physx::PxRigidDynamic *physic_body_in)
{
	XMFLOAT3 rec_pos;
	rec_pos.x = static_cast<float>(physic_body_in->getGlobalPose().p.x);
	rec_pos.y = static_cast<float>(physic_body_in->getGlobalPose().p.y);
	rec_pos.z = static_cast<float>(physic_body_in->getGlobalPose().p.z);
	return rec_pos;
}