#include "TestScene.h"
#include "D3DManager.h"
#include "MeshCreater.h"

#define PASS_NUMBER 2

UINT m_voxel_count = 0;
bool m_render_silhouette = false;

DirectX::BoundingSphere m_scene_sphere;

void TestScene::Enter(D3DManager* d3d_manager) {
	m_object_manager = std::make_unique<ObjectManager>();
	m_input_manager = std::make_unique<InputManager>(this, m_object_manager.get(), d3d_manager->Get_Client_Width(), d3d_manager->Get_Client_Height());                                   

	ID3D12Device* device = d3d_manager->Get_Device();
	ID3D12GraphicsCommandList* command_list = d3d_manager->Get_Cmd_List();

	Resize(d3d_manager);

	d3d_manager->Rst_Cmd_List();

	//
	m_shadow_map = std::make_unique<Shadowmap>(device, 8192, 8192);

	Load_Texture(device, command_list);
	Build_RS(device);
	Build_S_N_L();
	Build_Material();
	Build_Mesh(device, command_list);
	Build_O();
	Build_C(d3d_manager);
	Build_FR(device);
	Build_DH(device);
	Build_CBV(d3d_manager);
	Build_PSO(d3d_manager);

	Binding_Key();
	Pairing_Collision_Set();

	d3d_manager->Cls_Cmd_List();
	d3d_manager->Exct_Cmd_List();
	d3d_manager->Flush_Cmd_Q();

	//
	m_scene_sphere.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_scene_sphere.Radius = std::sqrtf(650.0f * 650.0f + 650.0f * 650.0f);
}

void TestScene::Exit(D3DManager* d3d_manager) {
	Flush_Cmd_Q(d3d_manager);
}

void TestScene::Update(D3DManager* d3d_manager, float elapsed_time) {
	Object* cat_object = m_object_manager->Get_Obj(L"cat_test");
	cat_object->Set_Color_Alpha(
		MathHelper::Min(1.0f, 1.0f - MathHelper::Length(MathHelper::Subtract(cat_object->Get_Position_3f(), m_object_manager->Get_Obj(L"player")->Get_Position_3f())) / 500.0f));

	m_input_manager->Prcs_Input();
	m_object_manager->Update(elapsed_time);

	m_current_frameresource_index = (m_current_frameresource_index + 1) % FRAME_RESOURCES_NUMBER;
	m_current_frameresource = m_frameresources[m_current_frameresource_index].get();

	UINT64 value = d3d_manager->Get_Fence()->GetCompletedValue();
	if (m_current_frameresource->fence != 0 && d3d_manager->Get_Fence()->GetCompletedValue() < m_current_frameresource->fence) {
		HANDLE event_handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		Throw_If_Failed(d3d_manager->Get_Fence()->SetEventOnCompletion(m_current_frameresource->fence, event_handle));

		WaitForSingleObject(event_handle, INFINITE);
		CloseHandle(event_handle);
	}

	// object
	auto current_object_constant_buffer = m_current_frameresource->object_constant_buffer.get();
	auto current_animation_constant_buffer = m_current_frameresource->animation_constant_buffer.get();

	for (auto& object : m_object_manager->Get_Obj_Arr()) {
		if (object->Get_Dirty_Count()) {
			DirectX::XMMATRIX world_matrix = DirectX::XMLoadFloat4x4(&object->Get_WM_4x4f());

			ObjectConstants object_constants;
			DirectX::XMStoreFloat4x4(&object_constants.world_matrix, DirectX::XMMatrixTranspose(world_matrix));
			object_constants.color_multiplier = object->Get_Color_Mul();
			object_constants.animated = (UINT)object->Get_Animated();

			current_object_constant_buffer->Copy_Data(object->Get_CB_Index(), object_constants);

			if (object_constants.animated) {
				AnimationConstants animation_constants;
				animation_constants.animation_transform_matrix = object->Get_Animation_Matrix();

				current_animation_constant_buffer->Copy_Data(object->Get_CB_Index(), animation_constants);
			}

			object->Sub_Dirty_Count();
		}
	}

	// material
	if (m_object_manager->Get_Material_Manager().Get_Dirty_Count()) {
		auto current_material_constant_buffer = m_current_frameresource->material_constant_buffer.get();

		for (auto& m : m_object_manager->Get_Material_Manager().Get_Material_Map()) {
			Material* material = m.second.get();

			MaterialConstants material_constants;
			material_constants.material_array = material->Get_Material_Factors();

			current_material_constant_buffer->Copy_Data(material->constant_buffer_index, material_constants);
		}

		m_object_manager->Get_Material_Manager().Sub_Dirty_Count();
	}

	// shadow transform matrix
	{
		DirectX::XMVECTOR light_direction = DirectX::XMVectorSet(0.2f, -1.0f, 0.2f, 0.0f);
		DirectX::XMVECTOR light_position = DirectX::XMVectorScale(light_direction, -2.0f * m_scene_sphere.Radius);
		DirectX::XMVECTOR target_position = DirectX::XMLoadFloat3(&m_scene_sphere.Center);
		DirectX::XMVECTOR light_up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		DirectX::XMMATRIX light_view_matrix = DirectX::XMMatrixLookAtLH(light_position, target_position, light_up);

		DirectX::XMStoreFloat3(&m_light_position, light_position);

		DirectX::XMFLOAT3 sphere_center_lightspace;
		DirectX::XMStoreFloat3(&sphere_center_lightspace, DirectX::XMVector3TransformCoord(target_position, light_view_matrix));

		float left_x = sphere_center_lightspace.x - m_scene_sphere.Radius;
		float bottom_y = sphere_center_lightspace.y - m_scene_sphere.Radius;
		float near_z = sphere_center_lightspace.z - m_scene_sphere.Radius;
		float right_x = sphere_center_lightspace.x + m_scene_sphere.Radius;
		float top_y = sphere_center_lightspace.y + m_scene_sphere.Radius;
		float far_z = sphere_center_lightspace.z + m_scene_sphere.Radius;

		m_light_near_z = near_z;
		m_light_far_z = far_z;
		DirectX::XMMATRIX light_projection_matrix = DirectX::XMMatrixOrthographicOffCenterLH(left_x, right_x, bottom_y, top_y, near_z, far_z);

		DirectX::XMMATRIX texture_matrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);

		DirectX::XMMATRIX shadow_transform_matrix = light_view_matrix * light_projection_matrix * texture_matrix;
		DirectX::XMStoreFloat4x4(&m_light_view_matrix, light_view_matrix);
		DirectX::XMStoreFloat4x4(&m_light_projection_matrix, light_projection_matrix);
		DirectX::XMStoreFloat4x4(&m_shadow_transform_matrix, shadow_transform_matrix);
	}

	// main pass constants
	if (m_main_camera) {
		m_main_camera->Udt_VM();

		m_camera_position = m_main_camera->Get_Position_3f();

		DirectX::XMStoreFloat4x4(&m_view_matrix, m_main_camera->Get_VM_M());
		DirectX::XMStoreFloat4x4(&m_projection_matrix, m_main_camera->Get_PM_M());
	}
	else {
		m_camera_position = DirectX::XMFLOAT3(0.0f, 300.0f, -500.0f);
		DirectX::XMVECTOR pos = DirectX::XMVectorSet(m_camera_position.x, m_camera_position.y, m_camera_position.z, 1.0f);
		DirectX::XMVECTOR target = DirectX::XMVectorZero();
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
		DirectX::XMStoreFloat4x4(&m_view_matrix, view);
	}

	DirectX::XMMATRIX view_matrix = DirectX::XMLoadFloat4x4(&m_view_matrix);
	DirectX::XMMATRIX projection_matrix = DirectX::XMLoadFloat4x4(&m_projection_matrix);
	DirectX::XMMATRIX view_projection_matrix = DirectX::XMMatrixMultiply(view_matrix, projection_matrix);

	DirectX::XMMATRIX inverse_view_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_matrix), view_matrix);
	DirectX::XMMATRIX inverse_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projection_matrix), projection_matrix);
	DirectX::XMMATRIX inverse_view_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_projection_matrix), view_projection_matrix);

	DirectX::XMMATRIX shadow_transform_matrix = DirectX::XMLoadFloat4x4(&m_shadow_transform_matrix);

	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_matrix, DirectX::XMMatrixTranspose(view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_matrix, DirectX::XMMatrixTranspose(inverse_view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.projection_matrix, DirectX::XMMatrixTranspose(projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_projection_matrix, DirectX::XMMatrixTranspose(inverse_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_projection_matrix, DirectX::XMMatrixTranspose(view_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_projection_matrix, DirectX::XMMatrixTranspose(inverse_view_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.shadow_transform_matrix, DirectX::XMMatrixTranspose(shadow_transform_matrix));
	m_main_pass_constant_buffer.camera_position = m_camera_position;
	m_main_pass_constant_buffer.buffer_padding = 0.0f;
	m_main_pass_constant_buffer.render_target_size = DirectX::XMFLOAT2((float)d3d_manager->Get_Client_Width(), (float)d3d_manager->Get_Client_Height());
	m_main_pass_constant_buffer.inverse_render_target_size = DirectX::XMFLOAT2(1.0f / (float)d3d_manager->Get_Client_Width(), 1.0f / (float)d3d_manager->Get_Client_Height());
	m_main_pass_constant_buffer.near_z = 1.0f;
	m_main_pass_constant_buffer.far_z = 1000.0f;
	m_main_pass_constant_buffer.total_time = 0.0f;
	m_main_pass_constant_buffer.delta_time = 0.0f;
	m_main_pass_constant_buffer.ambient_light = { 0.25f, 0.25f, 0.35f, 1.0f };
	//
	//m_main_pass_constant_buffer.lights[0].direction = { 0.57735f, -0.57735f, 1.0f };
	m_main_pass_constant_buffer.lights[0].direction = { 0.2f, -1.0f, 0.2f };
	//m_main_pass_constant_buffer.lights[0].direction = { 0.5f, -1.0f, 0.5f };
	m_main_pass_constant_buffer.lights[0].strength = { 0.6f, 0.6f, 0.6f };
	//m_main_pass_constant_buffer.lights[0].strength = { 1.0f, 1.0f, 1.0f };
	//m_main_pass_constant_buffer.lights[0].strength = { 0.0f, 0.0f, 0.0f };
	//
	m_main_pass_constant_buffer.lights[1].position = { 0.0f, 100.0f, -300.0f };
	//m_main_pass_constant_buffer.lights[1].strength = { 0.6f, 0.6f, 0.6f };
	m_main_pass_constant_buffer.lights[1].strength = { 0.0f, 0.0f, 0.0f };
	m_main_pass_constant_buffer.lights[1].falloff_start = 500.0f;
	m_main_pass_constant_buffer.lights[1].falloff_end = 1000.0f;
	//
	m_main_pass_constant_buffer.lights[2].position = { 0.0f, 100.0f, -500.0f };
	m_main_pass_constant_buffer.lights[2].direction = { 0.0f, 0.0f, 1.0f };
	//m_main_pass_constant_buffer.lights[2].strength = { 0.6f, 0.6f, 0.6f };
	m_main_pass_constant_buffer.lights[2].strength = { 0.0f, 0.0f, 0.0f };
	m_main_pass_constant_buffer.lights[2].falloff_start = 500.0f;
	m_main_pass_constant_buffer.lights[2].falloff_end = 1000.0f;
	m_main_pass_constant_buffer.lights[2].spot_power = 256.0f;

	auto current_pass_constant_buffer = m_current_frameresource->pass_constant_buffer.get();
	current_pass_constant_buffer->Copy_Data(0, m_main_pass_constant_buffer);

	// shadow pass constants
	{
		DirectX::XMMATRIX view_matrix = DirectX::XMLoadFloat4x4(&m_light_view_matrix);
		DirectX::XMMATRIX projection_matrix = DirectX::XMLoadFloat4x4(&m_light_projection_matrix);
		DirectX::XMMATRIX view_projection_matrix = DirectX::XMMatrixMultiply(view_matrix, projection_matrix);

		DirectX::XMMATRIX inverse_view_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_matrix), view_matrix);
		DirectX::XMMATRIX inverse_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(projection_matrix), projection_matrix);
		DirectX::XMMATRIX inverse_view_projection_matrix = DirectX::XMMatrixInverse(&DirectX::XMMatrixDeterminant(view_projection_matrix), view_projection_matrix);

		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.view_matrix, DirectX::XMMatrixTranspose(view_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_view_matrix, DirectX::XMMatrixTranspose(inverse_view_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.projection_matrix, DirectX::XMMatrixTranspose(projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_projection_matrix, DirectX::XMMatrixTranspose(inverse_projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.view_projection_matrix, DirectX::XMMatrixTranspose(view_projection_matrix));
		DirectX::XMStoreFloat4x4(&m_shadow_pass_constant_buffer.inverse_view_projection_matrix, DirectX::XMMatrixTranspose(inverse_view_projection_matrix));
		m_shadow_pass_constant_buffer.camera_position = m_light_position;
		m_shadow_pass_constant_buffer.render_target_size = DirectX::XMFLOAT2((float)m_shadow_map->Get_Width(), (float)m_shadow_map->Get_Height());
		m_shadow_pass_constant_buffer.inverse_render_target_size = DirectX::XMFLOAT2(1.0f / (float)m_shadow_map->Get_Width(), 1.0f / (float)m_shadow_map->Get_Height());
		m_shadow_pass_constant_buffer.near_z = m_light_near_z;
		m_shadow_pass_constant_buffer.far_z = m_light_far_z;

		auto current_pass_constant_buffer = m_current_frameresource->pass_constant_buffer.get();
		current_pass_constant_buffer->Copy_Data(1, m_shadow_pass_constant_buffer);
	}
}

void TestScene::Resize(D3DManager* d3d_manager) {
	DirectX::XMMATRIX projection_view = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi(), d3d_manager->Get_Aspect_Ratio(), 1.0f, 1000.0f);
	DirectX::XMStoreFloat4x4(&m_projection_matrix, projection_view);
}

void TestScene::Draw(D3DManager* d3d_manager, ID3D12CommandList** command_lists) {
	//ID3D12Device* device = d3d_manager->Get_Device();
	ID3D12CommandAllocator* command_allocator = m_current_frameresource->command_allocator.Get();
	ID3D12GraphicsCommandList* command_list = m_current_frameresource->command_list.Get();

	Throw_If_Failed(command_allocator->Reset());
	Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"shadow"].Get()));

	ID3D12DescriptorHeap* descriptor_heaps[] = { m_CBV_heap.Get() };
	command_list->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);

	command_list->SetGraphicsRootSignature(m_root_signature.Get());
	
	// shadow map
	{
		command_list->RSSetViewports(1, &m_shadow_map->Get_Viewport());
		command_list->RSSetScissorRects(1, &m_shadow_map->Get_Scissor_Rect());

		command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(m_shadow_map->Get_Resource(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE));

		command_list->ClearDepthStencilView(m_shadow_map->Get_DSV_CPU_Handle(),
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

		command_list->OMSetRenderTargets(0, nullptr, false, &m_shadow_map->Get_DSV_CPU_Handle());

		UINT pass_CBV_index = m_pass_CBV_offset + m_current_frameresource_index * PASS_NUMBER + 1;	// main : 0 / shaadow : 1
		auto pass_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		pass_CBV_gpu_descriptor_handle.Get_By_Offset(pass_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
		command_list->SetGraphicsRootDescriptorTable(2, pass_CBV_gpu_descriptor_handle);

		//command_list->SetPipelineState(m_pipeline_state_map[L"shadow"].Get());

		//
		DirectX::BoundingFrustum frustum_OBB;
		DirectX::BoundingFrustum::CreateFromMatrix(frustum_OBB, XMFLOAT4X4_2_XMMATRIX(m_light_projection_matrix));
		frustum_OBB.Transform(frustum_OBB, MathHelper::Inverse(XMFLOAT4X4_2_XMMATRIX(m_light_view_matrix)));

		for (auto& object : m_object_manager->Get_Opaque_Obj_Arr()) {
			if (object->Get_Name() == L"Ceiling") {
				continue;
			}

			//if (!object->Get_Visiable()) {
			//	continue;
			//}

			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
			command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

			//
			DirectX::BoundingOrientedBox mesh_OBB;

			//UINT material_CBV_index;
			for (auto& m : object->Get_Mesh_Array()) {
				//
				m.mesh_info->Get_OBB().Transform(mesh_OBB, object->Get_WM_M());

				if (frustum_OBB.Intersects(mesh_OBB) == false) {
					continue;
				}

				//if (m.mesh_info->material == nullptr) {
				//	material_CBV_index = m_material_CBV_offset +
				//		m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
				//}
				//else {
				//	material_CBV_index = m_material_CBV_offset +
				//		m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
				//		m.mesh_info->material->constant_buffer_index;
				//}

				//auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
				//material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

				//command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

				m.mesh_info->Draw(command_list);
			}
		}

		command_list->ResourceBarrier(1, &D3D12_RESOURCE_BARRIER_EX::Transition(m_shadow_map->Get_Resource(),
			D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	}

	// 
	d3d_manager->Set_RTV_N_DSV(command_list);

	d3d_manager->Set_VP(command_list);
	d3d_manager->Set_SR(command_list);

	d3d_manager->Clr_RTV(command_list);
	d3d_manager->Clr_DSV(command_list);

	if (m_wireframe) {
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque_wireframe"].Get());
	}
	else {
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque"].Get());
	}

	//
	D3D12_GPU_DESCRIPTOR_HANDLE_EX texture_descriptor_handle(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	texture_descriptor_handle.Get_By_Offset(m_texture_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(5, texture_descriptor_handle);

	//
	D3D12_GPU_DESCRIPTOR_HANDLE_EX shadow_map_descriptor_handle(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	shadow_map_descriptor_handle.Get_By_Offset(m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(4, shadow_map_descriptor_handle);

	//
	DirectX::BoundingFrustum frustum_OBB;
	DirectX::BoundingFrustum::CreateFromMatrix(frustum_OBB, m_main_camera->Get_PM_M());
	frustum_OBB.Transform(frustum_OBB, MathHelper::Inverse(m_main_camera->Get_VM_M()));

	//
	UINT pass_CBV_index = m_pass_CBV_offset + m_current_frameresource_index * PASS_NUMBER;
	auto pass_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	pass_CBV_gpu_descriptor_handle.Get_By_Offset(pass_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(2, pass_CBV_gpu_descriptor_handle);

	//
	UINT object_constant_buffer_size = Calc_CB_Size(sizeof(ObjectConstants));

	auto object_constant_buffer = m_current_frameresource->object_constant_buffer->Get_Resource();

	{
		// draw cat mesh
		Object* cat_object = m_object_manager->Get_Obj(L"cat_test");
		command_list->OMSetStencilRef(1);

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		UINT material_CBV_index;
		for (auto& m : cat_object->Get_Mesh_Array()) {
			if (m.mesh_info->material == nullptr) {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
			}
			else {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
					m.mesh_info->material->constant_buffer_index;
			}

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			m.mesh_info->Draw(command_list);
		}
		//
	}

	// draw else
	command_list->OMSetStencilRef(2);

	for (auto& object : m_object_manager->Get_Opaque_Obj_Arr()) {
		if (object->Get_Name() == L"cat_test") {
			continue;
		}

		if (!object->Get_Visiable()) {
			continue;
		}

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		//
		DirectX::BoundingOrientedBox mesh_OBB;

		UINT material_CBV_index;
		for (auto& m : object->Get_Mesh_Array()) {
			//
			m.mesh_info->Get_OBB().Transform(mesh_OBB, object->Get_WM_M());

			if (frustum_OBB.Intersects(mesh_OBB) == false) {
				continue;
			}

			if (m.mesh_info->material == nullptr) {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
			}
			else {
				material_CBV_index = m_material_CBV_offset +
					m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
					m.mesh_info->material->constant_buffer_index;
			}

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			m.mesh_info->Draw(command_list);
		}
	}

	// draw transparent objects

	// draw siluet
	if (m_render_silhouette) {
		command_list->SetPipelineState(m_pipeline_state_map[L"silhouette"].Get());

		Object* cat_object = m_object_manager->Get_Obj(L"cat_test");

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + cat_object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		UINT material_CBV_index;
		for (auto& m : cat_object->Get_Mesh_Array()) {
			material_CBV_index = m_material_CBV_offset +
				m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;

			auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

			m.mesh_info->Draw(command_list);
		}
		//
	}

	// draw wireframe bounding box
	if (m_render_boundingbox) {
		d3d_manager->Clr_DSV(command_list);
		command_list->SetPipelineState(m_pipeline_state_map[L"opaque_wireframe"].Get());

		for (auto& object : m_object_manager->Get_Col_OBB_Obj_Arr()) {
			UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Obj_Count() + object->Get_CB_Index();
			auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
			object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			//
			command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);

			UINT material_CBV_index;
			for (auto& m : object->Get_Mesh_Array()) {
				if (m.mesh_info->material == nullptr) {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() + 0;
				}
				else {
					material_CBV_index = m_material_CBV_offset +
						m_current_frameresource_index * (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count() +
						m.mesh_info->material->constant_buffer_index;
				}

				auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
				material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

				command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);

				m.mesh_info->Draw(command_list);
			}
		}
	}

	// close
	Throw_If_Failed(command_list->Close());

	command_lists[0] = command_list;

	m_current_frameresource->fence = d3d_manager->Get_Curr_Fence() + 1;
}

void TestScene::Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	m_input_manager->Prcs_Input_Msg(hwnd, message, wparam, lparam);
}

void TestScene::Load_Texture(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
	auto ui_texture = std::make_unique<Texture_Info>();
	ui_texture->name = L"ui_texture";
	ui_texture->file_name = L"test_texture.dds";
	Throw_If_Failed(TextureLoader::Create_DDS_Texture_From_File(
		device, command_list,
		ui_texture->file_name.c_str(), ui_texture->resource, ui_texture->upload_heap));

	m_texture_map[ui_texture->name] = std::move(ui_texture);
}

void TestScene::Build_RS(ID3D12Device* device) {
	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_0;
	desriptor_range_0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_1;
	desriptor_range_1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_2;
	desriptor_range_2.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_3;
	desriptor_range_3.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_4;
	desriptor_range_4.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	D3D12_DESCRIPTOR_RANGE_EX desriptor_range_5;
	desriptor_range_5.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	D3D12_ROOT_PARAMETER_EX root_parameters[6];

	root_parameters[0].Init_As_DT(1, &desriptor_range_0);
	root_parameters[1].Init_As_DT(1, &desriptor_range_1);
	root_parameters[2].Init_As_DT(1, &desriptor_range_2);
	root_parameters[3].Init_As_DT(1, &desriptor_range_3);
	root_parameters[4].Init_As_DT(1, &desriptor_range_4, D3D12_SHADER_VISIBILITY_PIXEL);
	root_parameters[5].Init_As_DT(1, &desriptor_range_5, D3D12_SHADER_VISIBILITY_PIXEL);

	std::array<D3D12_STATIC_SAMPLER_DESC_EX, 2> sampler_desc;
	sampler_desc[0] = D3D12_STATIC_SAMPLER_DESC_EX(
		0,
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.0f,
		16,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);
	sampler_desc[1] = D3D12_STATIC_SAMPLER_DESC_EX(
		1,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	D3D12_ROOT_SIGNATURE_DESC_EX root_signature_desc(6, root_parameters,
		(UINT)sampler_desc.size(), sampler_desc.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_signature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> error_blob = nullptr;

	HRESULT hr = S_OK;

	hr = D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized_root_signature.GetAddressOf(), error_blob.GetAddressOf());

	if (error_blob != nullptr) {
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
	}

	Throw_If_Failed(hr);

	Throw_If_Failed(device->CreateRootSignature(0, serialized_root_signature->GetBufferPointer(),
		serialized_root_signature->GetBufferSize(), IID_PPV_ARGS(m_root_signature.GetAddressOf())));
}

void TestScene::Build_S_N_L() {
	m_shader_map[L"standard_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "VS", "vs_5_1");
	m_shader_map[L"opaque_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "PS", "ps_5_1");

	m_shader_map[L"silhouette_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Silhouette_PS", "ps_5_1");

	m_shader_map[L"shadow_VS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Shadow_VS", "vs_5_1");
	m_shader_map[L"shadow_PS"] = Compile_Shader(L"shaders\\shaders.hlsl", nullptr, "Shadow_PS", "ps_5_1");

	m_input_layouts = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONECOUNT", 0, DXGI_FORMAT_R32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "MATERIAL", 0, DXGI_FORMAT_R32_UINT, 0, 80, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void TestScene::Build_Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
	//
	Mesh_Info* mesh_info;
	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Box_Mesh(L"boundingbox");
	mesh_info->material = m_object_manager->Get_Material_Manager().Get_Material(L"boundingbox");

	mesh_info = m_object_manager->Get_Mesh_Manager().Crt_Box_Mesh(L"cheese");
	mesh_info->material = m_object_manager->Get_Material_Manager().Get_Material(L"cheese");

	m_object_manager->Ipt_From_FBX(L"cat_mesh_edit.fbx", true, false, true, MESH_INFO | SKELETON_INFO | MATERIAL_INFO);
	m_object_manager->Ipt_From_FBX(L"cat_walk.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_run.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_idle.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_bite.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_paw.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_start.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_idle.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"cat_jump_test_end.fbx", true, false, true, ANIMATION_INFO, L"cat_mesh_edit.fbx");

	m_object_manager->Ipt_From_FBX(L"mouse_mesh_edit.fbx", true, false, true, MESH_INFO | SKELETON_INFO | MATERIAL_INFO);
	m_object_manager->Ipt_From_FBX(L"mouse_death.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_hit.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	//m_object_manager->Ipt_From_FBX(L"mouse_jump.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_run.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_walk.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_idle.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_start.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_idle.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");
	m_object_manager->Ipt_From_FBX(L"mouse_jump_end.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh_edit.fbx");

	m_object_manager->Ipt_From_FBX(L"house.fbx", false, true, false, MESH_INFO | MATERIAL_INFO);

	m_object_manager->Build_BV(device, command_list);
}

void TestScene::Build_Material() {
	//
	m_object_manager->Get_Material_Manager().Add_Material(L"boundingbox", Material_Factor(DirectX::XMFLOAT4(DirectX::Colors::LightGreen)));
	m_object_manager->Get_Material_Manager().Add_Material(L"cheese", Material_Factor(DirectX::XMFLOAT4(DirectX::Colors::Yellow)));
}

void TestScene::Build_O() {
	m_object_manager->Add_Obj(L"player", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"player", L"mouse_mesh_edit.fbx");
	//m_object_manager->Add_Obj(L"player", L"cat_mesh_edit.fbx");
	//m_object_manager->Set_Sklt_2_Obj(L"player", L"cat_mesh_edit.fbx");

	m_object_manager->Add_Obj(L"cat_test", L"cat_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"cat_test", L"cat_mesh_edit.fbx");

	m_object_manager->Add_Obj(L"mouse_test", L"mouse_mesh_edit.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse_test", L"mouse_mesh_edit.fbx");

	m_object_manager->Get_Obj(L"player")->Set_Visiable(false);
	//m_object_manager->Get_Obj(L"cat_test")->Set_Visiable(false);
	m_object_manager->Get_Obj(L"mouse_test")->Set_Visiable(false);

	Object* object = m_object_manager->Get_Obj(L"player");
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"mouse_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"mouse_walk.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_START, Animation_Binding_Info(L"mouse_jump_start.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_JUMP_IDLE));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_IDLE, Animation_Binding_Info(L"mouse_jump_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_JUMP_END, Animation_Binding_Info(L"mouse_jump_end.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE));
	object->Bind_Anim_2_State(Object_State::STATE_ACTION_ONE, Animation_Binding_Info(L"mouse_hit.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE, NOT_MOVABLE));
	//object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"cat_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	//object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"cat_walk.fbx", 2.0f, 0.2f, LOOP_ANIMATION));
	//object->Bind_Anim_2_State(Object_State::STATE_JUMP_START, Animation_Binding_Info(L"cat_jump_test_start.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_JUMP_IDLE));
	//object->Bind_Anim_2_State(Object_State::STATE_JUMP_IDLE, Animation_Binding_Info(L"cat_jump_test_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	//object->Bind_Anim_2_State(Object_State::STATE_JUMP_END, Animation_Binding_Info(L"cat_jump_test_end.fbx", 1.0f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE));
	//object->Bind_Anim_2_State(Object_State::STATE_ACTION_ONE, Animation_Binding_Info(L"cat_paw.fbx", 0.5f, 0.2f, ONCE_ANIMATION, Object_State::STATE_IDLE, NOT_MOVABLE));
	object->Set_Animated(true);
	object->Set_Phys(true);
	//object->Set_Color_Mul(1.0f, 0.0f, 0.0f);

	object = m_object_manager->Get_Obj(L"mouse_test");
	object->TP_Down(61.592f);
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"mouse_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"mouse_walk.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Set_Animated(true);

	object = m_object_manager->Get_Obj(L"cat_test");
	object->TP_Down(61.592f);
	object->Bind_Anim_2_State(Object_State::STATE_IDLE, Animation_Binding_Info(L"cat_idle.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Bind_Anim_2_State(Object_State::STATE_MOVE, Animation_Binding_Info(L"cat_walk.fbx", 1.0f, 0.2f, LOOP_ANIMATION));
	object->Set_Animated(true);

	//Crt_Voxel_Cheese(DirectX::XMFLOAT3(169.475f, 10.049f, 230.732f), 1.0f, 0);
	//Crt_Voxel_Cheese(DirectX::XMFLOAT3(254.871f, 10.049f, 311.188f), 1.0f, 0);
	Crt_Voxel_Cheese(DirectX::XMFLOAT3(0.0f, -50.0f, 100.0f), 1.0f, 0);

	//
	DirectX::BoundingOrientedBox obj_obb;

	obj_obb.Center = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	obj_obb.Extents = DirectX::XMFLOAT3(1300.0, 300.0f, 1300.0f);
	m_object_manager->Add_Col_OBB_Obj(L"scene_obb", obj_obb);

	//int count = 0;
	//for (auto& o : m_object_manager->Get_Opaque_Obj_Arr()) {
	//	for (auto& m : o->Get_Mesh_Array()) {
	//		m.mesh_info->Get_OBB().Transform(obj_obb, o->Get_WM_M());

	//		m_object_manager->Add_Col_OBB_Obj(L"obb_" + std::to_wstring(count++), obj_obb);
	//	}
	//}
}

void TestScene::Build_C(D3DManager* d3d_manager) {
	//auto main_camera = reinterpret_cast<Camera*>(m_object_manager->Add_Cam(L"maincamera", L"camera", L"player",
	//	0.0f, 50.0f, 0.0f, 150.0f, ROTATE_SYNC_RPY));
	auto main_camera = reinterpret_cast<Camera*>(m_object_manager->Add_Cam(L"maincamera", L"camera", L"player",
		0.0f, 10.0f, 0.0f, 0.1f, ROTATE_SYNC_RPY));
	main_camera->Set_Frustum(0.25f * MathHelper::Pi(), d3d_manager->Get_Aspect_Ratio(), 1.0f, 2000.0f);
	main_camera->Set_Limit_Rotate_Right(true, -RIGHT_ANGLE_RADIAN + 0.01f, RIGHT_ANGLE_RADIAN - 0.01f);
	//main_camera->Set_Limit_Rotate_Right(true, DirectX::XMConvertToRadians(1.0f), DirectX::XMConvertToRadians(60.0f));
	main_camera->Set_Lagging_Degree(1.0f);

	m_main_camera = main_camera;
}

void TestScene::Build_FR(ID3D12Device* device) {
	for (int i = 0; i < FRAME_RESOURCES_NUMBER; ++i) {
		m_frameresources.emplace_back(std::make_unique<FrameResorce>(device, PASS_NUMBER,
			(UINT)m_object_manager->Get_Obj_Count(), (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count()));
	}
}

void TestScene::Build_DH(ID3D12Device* device) {
	UINT object_count = (UINT)m_object_manager->Get_Obj_Count();
	UINT material_count = (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count();
	UINT texture_count = (UINT)m_texture_map.size();

	UINT descriptors_number = (object_count * 2 + material_count + PASS_NUMBER) * FRAME_RESOURCES_NUMBER + texture_count + 1;	// object also has animation

	m_material_CBV_offset = object_count * FRAME_RESOURCES_NUMBER;
	m_pass_CBV_offset = m_material_CBV_offset + material_count * FRAME_RESOURCES_NUMBER;
	m_animation_CBV_offset = m_pass_CBV_offset + PASS_NUMBER * FRAME_RESOURCES_NUMBER;
	m_texture_SRV_offset = m_animation_CBV_offset + object_count * FRAME_RESOURCES_NUMBER;
	m_shadow_map_SRV_offset = m_texture_SRV_offset + texture_count;

	D3D12_DESCRIPTOR_HEAP_DESC CBV_heap_desc;
	CBV_heap_desc.NumDescriptors = descriptors_number;
	CBV_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	CBV_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	CBV_heap_desc.NodeMask = 0;

	Throw_If_Failed(device->CreateDescriptorHeap(&CBV_heap_desc, IID_PPV_ARGS(&m_CBV_heap)));
}

void TestScene::Build_CBV(D3DManager* d3d_manager) {
	ID3D12Device* device = d3d_manager->Get_Device();

	UINT object_constant_buffer_size = Calc_CB_Size(sizeof(ObjectConstants));
	UINT object_count = (UINT)m_object_manager->Get_Obj_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto object_constant_buffer = m_frameresources[frameresource_index]->object_constant_buffer->Get_Resource();

		for (UINT i = 0; i < object_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = object_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * object_constant_buffer_size;

			int descriptor_heap_index = frameresource_index * object_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = object_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT material_constant_buffer_size = Calc_CB_Size(sizeof(MaterialConstants));
	UINT material_count = (UINT)m_object_manager->Get_Material_Manager().Get_Material_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto material_constant_buffer = m_frameresources[frameresource_index]->material_constant_buffer->Get_Resource();

		for (UINT i = 0; i < material_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = material_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * material_constant_buffer_size;

			int descriptor_heap_index = m_material_CBV_offset + frameresource_index * material_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = material_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT pass_constant_buffer_size = Calc_CB_Size(sizeof(PassConstants));

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto pass_constant_buffer = m_frameresources[frameresource_index]->pass_constant_buffer->Get_Resource();

		for (UINT i = 0; i < PASS_NUMBER; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = pass_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * pass_constant_buffer_size;

			int descriptor_heap_index = m_pass_CBV_offset + frameresource_index * PASS_NUMBER + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = pass_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	UINT animation_constant_buffer_size = Calc_CB_Size(sizeof(AnimationConstants));
	object_count = (UINT)m_object_manager->Get_Obj_Count();

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto animation_constant_buffer = m_frameresources[frameresource_index]->animation_constant_buffer->Get_Resource();

		for (UINT i = 0; i < object_count; ++i) {
			D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = animation_constant_buffer->GetGPUVirtualAddress();

			constant_buffer_address += i * animation_constant_buffer_size;

			int descriptor_heap_index = m_animation_CBV_offset + frameresource_index * object_count + i;
			auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
			cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

			D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
			constant_buffer_view_desc.BufferLocation = constant_buffer_address;
			constant_buffer_view_desc.SizeInBytes = animation_constant_buffer_size;

			device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
		}
	}

	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> texture_list;

	for (auto& t : m_texture_map) {
		texture_list.emplace_back(t.second->resource);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC SRV_desc = {};
	SRV_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SRV_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SRV_desc.Texture2D.MostDetailedMip = 0;
	SRV_desc.Texture2D.ResourceMinLODClamp = 0.0f;

	for (UINT i = 0; i < (UINT)texture_list.size(); ++i) {
		D3D12_CPU_DESCRIPTOR_HANDLE_EX cpu_descriptor_handle(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
		cpu_descriptor_handle.Get_By_Offset(m_texture_SRV_offset + i, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		SRV_desc.Format = texture_list[i]->GetDesc().Format;
		SRV_desc.Texture2D.MipLevels = texture_list[i]->GetDesc().MipLevels;
		device->CreateShaderResourceView(texture_list[i].Get(), &SRV_desc, cpu_descriptor_handle);
	}

	//
	m_shadow_map->Build_Descriptors(d3d_manager->Get_Device(),
		D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart(), m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size()),
		D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart(), m_shadow_map_SRV_offset, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size()),
		D3D12_CPU_DESCRIPTOR_HANDLE_EX(d3d_manager->Get_DSV(), 1, d3d_manager->Get_DSV_Descritpor_Size()));
}

void TestScene::Build_PSO(D3DManager* d3d_manager) {
	ID3D12Device* device = d3d_manager->Get_Device();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaque_PSO_desc;
	ZeroMemory(&opaque_PSO_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaque_PSO_desc.InputLayout = { m_input_layouts.data(), (UINT)m_input_layouts.size() };
	opaque_PSO_desc.pRootSignature = m_root_signature.Get();
	opaque_PSO_desc.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"standard_VS"]->GetBufferPointer()), m_shader_map[L"standard_VS"]->GetBufferSize() };
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"opaque_PS"]->GetBufferPointer()), m_shader_map[L"opaque_PS"]->GetBufferSize() };
	opaque_PSO_desc.RasterizerState = D3D12_RASTERIZER_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	//opaque_PSO_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaque_PSO_desc.BlendState = D3D12_BLEND_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.DepthStencilState = D3D12_DEPTH_STENCIL_DESC_EX(
	TRUE, D3D12_DEPTH_WRITE_MASK_ALL, D3D12_COMPARISON_FUNC_LESS,
		TRUE, 0xFF, 0xFF, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE, D3D12_COMPARISON_FUNC_ALWAYS,
		D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS);
	opaque_PSO_desc.SampleMask = UINT_MAX;
	opaque_PSO_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaque_PSO_desc.NumRenderTargets = 1;
	opaque_PSO_desc.RTVFormats[0] = d3d_manager->Get_BB_Format();
	opaque_PSO_desc.SampleDesc.Count = d3d_manager->Is_4xMSAA() ? 4 : 1;
	opaque_PSO_desc.SampleDesc.Quality = d3d_manager->Is_4xMSAA() ? (d3d_manager->Get_4xMSAA_Qual() - 1) : 0;
	opaque_PSO_desc.DSVFormat = d3d_manager->Get_DS_Format();

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaque_PSO_desc_copy = opaque_PSO_desc;

	//
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque_wireframe"])));

	//
	D3D12_RENDER_TARGET_BLEND_DESC render_target_blend_desc;
	render_target_blend_desc.BlendEnable = true;
	render_target_blend_desc.LogicOpEnable = false;
	render_target_blend_desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	render_target_blend_desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	render_target_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
	render_target_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
	render_target_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
	render_target_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	render_target_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
	render_target_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	opaque_PSO_desc.BlendState.RenderTarget[0] = render_target_blend_desc;
	opaque_PSO_desc.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"silhouette_PS"]->GetBufferPointer()), m_shader_map[L"silhouette_PS"]->GetBufferSize() };
	opaque_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	opaque_PSO_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
	opaque_PSO_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	opaque_PSO_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
	//opaque_PSO_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"silhouette"])));

	//
	opaque_PSO_desc_copy.RasterizerState.DepthBias = 10000;
	opaque_PSO_desc_copy.RasterizerState.DepthBiasClamp = 0.0f;
	opaque_PSO_desc_copy.RasterizerState.SlopeScaledDepthBias = 1.0f;
	opaque_PSO_desc_copy.VS = { reinterpret_cast<BYTE*>(m_shader_map[L"shadow_VS"]->GetBufferPointer()), m_shader_map[L"shadow_VS"]->GetBufferSize() };
	opaque_PSO_desc_copy.PS = { reinterpret_cast<BYTE*>(m_shader_map[L"shadow_PS"]->GetBufferPointer()), m_shader_map[L"shadow_PS"]->GetBufferSize() };
	opaque_PSO_desc_copy.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	opaque_PSO_desc_copy.NumRenderTargets = 0;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc_copy, IID_PPV_ARGS(&m_pipeline_state_map[L"shadow"])));
}

void TestScene::Binding_Key() {

	m_input_manager->Bind_Key_Down(VK_W, BindingInfo(L"player", Action::MOVE_FORWARD, MOVE_ONLY_XZ));
	m_input_manager->Bind_Key_Down(VK_S, BindingInfo(L"player", Action::MOVE_BACK, MOVE_ONLY_XZ));
	m_input_manager->Bind_Key_Down(VK_A, BindingInfo(L"player", Action::MOVE_LEFT, MOVE_ONLY_XZ));
	m_input_manager->Bind_Key_Down(VK_D, BindingInfo(L"player", Action::MOVE_RIGHT, MOVE_ONLY_XZ));

	m_input_manager->Bind_Key_First_Down(VK_SPACE, BindingInfo(L"player", Action::ACTION_JUMP));
	m_input_manager->Bind_Key_First_Down(VK_LBUTTON, BindingInfo(L"player", Action::ACTION_ONE));

	m_input_manager->Bind_Mouse_Move(BindingInfo(L"maincamera", Action::ROTATE_PITCH, 0.01f),
		BindingInfo(L"maincamera", Action::ROTATE_RIGHT, 0.01f));

	m_input_manager->Bind_Key_First_Down(VK_F1, BindingInfo(L"", Action::CHANGE_WIREFRAME_FLAG));
	m_input_manager->Bind_Key_First_Down(VK_F2, BindingInfo(L"", Action::CHANGE_BOUNDINGBOX_FLAG));

	//
	m_input_manager->Bind_Key_First_Down(VK_F3, BindingInfo(L"", Action::CUSTOM_FUNCTION_ONE));

	//m_input_manager->Bind_Key_First_Down(VK_TAB, BindingInfo(L"", Action::FIX_CURSOR));

	//
	//m_input_manager->Set_Hide_Cursor(true);
	//m_input_manager->Set_Fix_Cursor(true);
}

void TestScene::Pairing_Collision_Set() {
	//m_object_manager->Add_Collision_Pair(L"")
}

void TestScene::Custom_Function_One() {
	m_render_silhouette = !m_render_silhouette;
}

void TestScene::Crt_Voxel(DirectX::XMFLOAT3 position, float scale, UINT detail_level) {
	if (detail_level == 0) {
		Object* object = m_object_manager->Add_Obj(L"voxel_" + std::to_wstring(m_voxel_count++), L"cheese", L"Object",
			DirectX::XMMATRIX(
				scale, 0.0f, 0.0f, 0.0f,
				0.0f, scale, 0.0f, 0.0f,
				0.0f, 0.0f, scale, 0.0f,
				position.x, position.y, position.z, 1.0f));
	}
	else {
		float half = scale / 2.0f;
		float quarter = scale / 4.0f;

		for (int i = -1; i < 2; ++++i) {
			for (int j = -1; j < 2; ++++j) {
				for (int k = -1; k < 2; ++++k) {
					Crt_Voxel(
						DirectX::XMFLOAT3(position.x + quarter * k, position.y + quarter * i, position.z + quarter * j),
						half, detail_level - 1);
				}
			}
		}
	}
}

void TestScene::Crt_Voxel_Cheese(DirectX::XMFLOAT3 position, float scale, UINT detail_level) {
	int m_random_value = 10;
	int y_value = 8;
	int z_value = 21;
	int x_value = z_value / 2;

	std::random_device rd;
	std::uniform_int_distribution<int> uid(1, m_random_value);

	DirectX::XMFLOAT3 pivot_position = position;

	position.y += scale / 2.0f;

	for (int i = 0; i < y_value; ++i) {
		position.z = pivot_position.z - scale * (float)(z_value / 2);

		for (int j = 1; j <= z_value; ++j) {
			position.x = pivot_position.x - scale * (float)(x_value / 2);

			for (int k = 0; k <= j / 2; ++k) {
				if ((i == y_value - 1 || j == z_value || k == 0 || k == j / 2) &&
					!(uid(rd) % m_random_value)) {
					position.x += scale;
					continue;
				}

				Crt_Voxel(position, scale, detail_level);
				position.x += scale;
			}

			position.z += scale;
		}

		position.y += scale;
	}
}
