#include "TestScene.h"
#include "D3DManager.h"
#include "MeshCreater.h"

void TestScene::Enter(D3DManager* d3d_manager) {
	m_object_manager = std::make_unique<ObjectManager>();
	m_input_manager = std::make_unique<InputManager>(this, m_object_manager.get());

	ID3D12Device* device = d3d_manager->Get_Device();
	ID3D12GraphicsCommandList* command_list = d3d_manager->Get_Cmd_List();

	Resize(d3d_manager);

	d3d_manager->Rst_Cmd_List();

	Build_RS(device);
	Build_S_N_L();
	Build_Mesh(device, command_list);
	Build_Material();
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
}

void TestScene::Exit(D3DManager* d3d_manager) {
	d3d_manager->Rst_Cmd_List();
	d3d_manager->Cls_Cmd_List();
	d3d_manager->Exct_Cmd_List();
	d3d_manager->Flush_Cmd_Q();


}

void TestScene::Update(D3DManager* d3d_manager, float elapsed_time) {
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

	//
	auto current_object_constant_buffer = m_current_frameresource->object_constant_buffer.get();
	auto current_animation_constant_buffer = m_current_frameresource->animation_constant_buffer.get();

	for (UINT i = 0; i < m_object_manager->Get_Obj_Count(); ++i) {
		Object* object = m_object_manager->Get_Obj(i);

		if (object->Get_Dirty_Count()) {
			DirectX::XMMATRIX world_matrix = DirectX::XMLoadFloat4x4(&object->Get_WM());

			ObjectConstants object_constants;
			DirectX::XMStoreFloat4x4(&object_constants.world_matrix, DirectX::XMMatrixTranspose(world_matrix));
			object_constants.animated = (UINT)object->Get_Animated();

			current_object_constant_buffer->Copy_Data(object->Get_CB_Index(), object_constants);

			AnimationConstants animation_constants;
			animation_constants.animation_transform_matrix = object->Get_Animation_Matrix();

			current_animation_constant_buffer->Copy_Data(object->Get_CB_Index(), animation_constants);

			object->Sub_Dirty_Count();
		}
	}

	auto current_material_constant_buffer = m_current_frameresource->material_constant_buffer.get();

	for (auto& m : m_material_map) {
		MaterialInfo* material = m.second.get();

		if (material->dirty_frame_count > 0) {
			MaterialConstants material_constants;
			material_constants.diffuse_albedo = material->diffuse_albedo;
			material_constants.fresnel = material->fresnel;
			material_constants.roughness = material->roughness;

			current_material_constant_buffer->Copy_Data(material->constant_buffer_index, material_constants);

			material->dirty_frame_count--;
		}
	}

	//
	//m_camera_position.x =100.0f * sinf(m_phi) * cosf(m_theta);
	//m_camera_position.y =100.0f * sinf(m_phi) * sinf(m_theta);
	//m_camera_position.z =100.0f * cosf(m_phi);
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

	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_matrix, DirectX::XMMatrixTranspose(view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_matrix, DirectX::XMMatrixTranspose(inverse_view_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.projection_matrix, DirectX::XMMatrixTranspose(projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_projection_matrix, DirectX::XMMatrixTranspose(inverse_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.view_projection_matrix, DirectX::XMMatrixTranspose(view_projection_matrix));
	DirectX::XMStoreFloat4x4(&m_main_pass_constant_buffer.inverse_view_projection_matrix, DirectX::XMMatrixTranspose(inverse_view_projection_matrix));
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
	m_main_pass_constant_buffer.lights[0].direction = { 0.5f, -1.0f, 0.5f };
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

	//if (m_wireframe) {
	//	Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"opaque_wireframe"].Get()));
	//}
	//else {
	//	Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"opaque"].Get()));
	//}

	//Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"opaque_wireframe"].Get()));
	Throw_If_Failed(command_list->Reset(command_allocator, m_pipeline_state_map[L"opaque"].Get()));

	d3d_manager->Clr_RTV(command_list);
	d3d_manager->Clr_DSV(command_list);

	d3d_manager->Set_VP(command_list);
	d3d_manager->Set_SR(command_list);

	d3d_manager->Set_RTV_N_DSV(command_list);

	ID3D12DescriptorHeap* descriptor_heaps[] = { m_CBV_heap.Get() };
	command_list->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);

	command_list->SetGraphicsRootSignature(m_root_signature.Get());

	//
	UINT pass_CBV_index = m_pass_CBV_offset + m_current_frameresource_index;
	auto pass_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
	pass_CBV_gpu_descriptor_handle.Get_By_Offset(pass_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());
	command_list->SetGraphicsRootDescriptorTable(2, pass_CBV_gpu_descriptor_handle);

	//
	UINT object_constant_buffer_size = Calc_CB_Size(sizeof(ObjectConstants));

	auto object_constant_buffer = m_current_frameresource->object_constant_buffer->Get_Resource();

	for (size_t i = 0; i < m_object_manager->Get_Opaque_Obj_Count(); ++i) {
		auto object = m_object_manager->Get_Opaque_Obj((UINT)i);

		if (!object->Get_Visiable()) {
			continue;
		}

		//command_list->IASetVertexBuffers(0, 1, &object->Get_Mesh_Info()->Get_VBV());
		//command_list->IASetIndexBuffer(&object->Get_Mesh_Info()->Get_IBV());
		//command_list->IASetPrimitiveTopology(object->Get_PT());

		UINT object_CBV_index = m_current_frameresource_index * (UINT)m_object_manager->Get_Opaque_Obj_Count() + object->Get_CB_Index();
		auto object_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		object_CBV_gpu_descriptor_handle.Get_By_Offset(object_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT material_CBV_index;
		if (object->Get_Material_Info()) {
			material_CBV_index = m_material_CBV_offset + m_current_frameresource_index * (UINT)m_material_map.size() + object->Get_Material_Info()->constant_buffer_index;
		}
		else {
			material_CBV_index = m_material_CBV_offset + m_current_frameresource_index * (UINT)m_material_map.size() + m_material_map[L"default"]->constant_buffer_index;
		}
		auto material_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		material_CBV_gpu_descriptor_handle.Get_By_Offset(material_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		UINT animation_CBV_index = m_animation_CBV_offset + m_current_frameresource_index * (UINT)m_object_manager->Get_Opaque_Obj_Count() + object->Get_CB_Index();
		auto animation_CBV_gpu_descriptor_handle = D3D12_GPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetGPUDescriptorHandleForHeapStart());
		animation_CBV_gpu_descriptor_handle.Get_By_Offset(animation_CBV_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		//
		command_list->SetGraphicsRootDescriptorTable(0, object_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(1, material_CBV_gpu_descriptor_handle);
		command_list->SetGraphicsRootDescriptorTable(3, animation_CBV_gpu_descriptor_handle);

		object->Draw(command_list);
		//for (UINT i = 0; i < object->Get_Index(); ++++++i) {
		//	command_list->DrawIndexedInstanced(3, 1, object->Get_Start_Index() + i, object->Get_Base_Vertex(), 0);
		//}
	}

	// draw transparent objects

	Throw_If_Failed(command_list->Close());

	command_lists[0] = command_list;

	m_current_frameresource->fence = d3d_manager->Get_Curr_Fence() + 1;
}

void TestScene::Prcs_Input_Msg(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
	m_input_manager->Prcs_Input_Msg(hwnd, message, wparam, lparam);
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

	D3D12_ROOT_PARAMETER_EX root_parameters[4];

	root_parameters[0].Init_As_DT(1, &desriptor_range_0);
	root_parameters[1].Init_As_DT(1, &desriptor_range_1);
	root_parameters[2].Init_As_DT(1, &desriptor_range_2);
	root_parameters[3].Init_As_DT(1, &desriptor_range_3);

	D3D12_ROOT_SIGNATURE_DESC_EX root_signature_desc(4, root_parameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

	m_input_layouts = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONECOUNT", 0, DXGI_FORMAT_R32_UINT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 64, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void TestScene::Build_Mesh(ID3D12Device* device, ID3D12GraphicsCommandList* command_list) {
	m_object_manager->Ipt_From_FBX(L"cat.fbx", true, false, true, MESH_INFO | SKELETON_INFO | ANIMATION_INFO);
	m_object_manager->Ipt_From_FBX(L"mouse.fbx", true, false, true, MESH_INFO | SKELETON_INFO | ANIMATION_INFO);
	//m_object_manager->Ipt_From_FBX(L"mouse_mesh.fbx", true, false, true, MESH_INFO | SKELETON_INFO);
	//m_object_manager->Ipt_From_FBX(L"mouse_walk.fbx", true, false, true, ANIMATION_INFO, L"mouse_mesh.fbx");
	//m_object_manager->Ipt_From_FBX(L"animationtest_0.fbx", true, false, true, MESH_INFO | SKELETON_INFO | ANIMATION_INFO);
	m_object_manager->Ipt_From_FBX(L"animationtest.fbx", true, false, true, MESH_INFO | SKELETON_INFO | ANIMATION_INFO);
	m_object_manager->Ipt_From_FBX(L"house.fbx", false, true, false, MESH_INFO);

	m_object_manager->Build_BV(device, command_list);
}

void TestScene::Build_Material() {
	auto default_material = std::make_unique<MaterialInfo>();
	default_material->name = L"default";
	default_material->constant_buffer_index = 0;
	default_material->diffuse_heap_index = 0;
	default_material->diffuse_albedo = DirectX::XMFLOAT4(DirectX::Colors::LightBlue);
	default_material->fresnel = DirectX::XMFLOAT3(0.01f, 0.01f, 0.01f);
	default_material->roughness = 0.1f;

	m_material_map[L"default"] = std::move(default_material);
}

void TestScene::Build_O() {
	m_object_manager->Add_Obj(L"player", L"cat.fbx");
	m_object_manager->Add_Obj(L"cat_test", L"cat.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"cat_test", L"cat.fbx");
	m_object_manager->Add_Obj(L"mouse_test", L"mouse.fbx");
	m_object_manager->Set_Sklt_2_Obj(L"mouse_test", L"mouse.fbx");

	m_object_manager->Get_Obj(L"player")->Set_Visiable(false);
	//m_object_manager->Get_Obj(L"cat_test")->Set_Visiable(false);

	m_object_manager->Get_Obj(L"mouse_test")->Set_Animation(L"mouse.fbx");
	m_object_manager->Get_Obj(L"mouse_test")->Set_Animated(true);
	m_object_manager->Get_Obj(L"cat_test")->Set_Animation(L"cat.fbx");
	m_object_manager->Get_Obj(L"cat_test")->Set_Animated(true);
}

void TestScene::Build_C(D3DManager* d3d_manager) {
	m_object_manager->Add_Obj(
		L"maincamera",
		nullptr,
		L"",
		nullptr,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		ObjectType::CAMERA_OBJECT,
		false,
		false,
		L"camera"
	);
	auto main_camera = reinterpret_cast<Camera*>(m_object_manager->Get_Obj(L"maincamera"));
	main_camera->Set_Frustum(0.25f * MathHelper::Pi(), d3d_manager->Get_Aspect_Ratio(), 1.0f, 2000.0f);

	m_object_manager->Bind_Cam_2_Obj(L"maincamera", L"player", 0.1f);

	m_main_camera = main_camera;
}

void TestScene::Build_FR(ID3D12Device* device) {
	for (int i = 0; i < FRAME_RESOURCES_NUMBER; ++i) {
		m_frameresources.emplace_back(std::make_unique<FrameResorce>(device, 1, (UINT)m_object_manager->Get_Obj_Count(), (UINT)m_material_map.size()));
	}
}

void TestScene::Build_DH(ID3D12Device* device) {
	UINT object_count = (UINT)m_object_manager->Get_Obj_Count();
	UINT material_count = (UINT)m_material_map.size();

	UINT descriptors_number = (object_count * 2 + material_count + 1) * FRAME_RESOURCES_NUMBER;	// object also has animation

	m_material_CBV_offset = object_count * FRAME_RESOURCES_NUMBER;
	m_pass_CBV_offset = m_material_CBV_offset + material_count * FRAME_RESOURCES_NUMBER;
	m_animation_CBV_offset = m_pass_CBV_offset + 1 * FRAME_RESOURCES_NUMBER;

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

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto material_constant_buffer = m_frameresources[frameresource_index]->material_constant_buffer->Get_Resource();
		D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = material_constant_buffer->GetGPUVirtualAddress();

		int descriptor_heap_index = m_material_CBV_offset + frameresource_index;
		auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
		cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
		constant_buffer_view_desc.BufferLocation = constant_buffer_address;
		constant_buffer_view_desc.SizeInBytes = material_constant_buffer_size;

		device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
	}

	UINT pass_constant_buffer_size = Calc_CB_Size(sizeof(PassConstants));

	for (int frameresource_index = 0; frameresource_index < FRAME_RESOURCES_NUMBER; ++frameresource_index) {
		auto pass_constant_buffer = m_frameresources[frameresource_index]->pass_constant_buffer->Get_Resource();
		D3D12_GPU_VIRTUAL_ADDRESS constant_buffer_address = pass_constant_buffer->GetGPUVirtualAddress();

		int descriptor_heap_index = m_pass_CBV_offset + frameresource_index;
		auto cpu_descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE_EX(m_CBV_heap->GetCPUDescriptorHandleForHeapStart());
		cpu_descriptor_handle.Get_By_Offset(descriptor_heap_index, d3d_manager->Get_CBV_SRV_UAV_Descritpor_Size());

		D3D12_CONSTANT_BUFFER_VIEW_DESC constant_buffer_view_desc;
		constant_buffer_view_desc.BufferLocation = constant_buffer_address;
		constant_buffer_view_desc.SizeInBytes = pass_constant_buffer_size;

		device->CreateConstantBufferView(&constant_buffer_view_desc, cpu_descriptor_handle);
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
	opaque_PSO_desc.DepthStencilState = D3D12_DEPTH_STENCIL_DESC_EX(D3D12_DEFAULT());
	opaque_PSO_desc.SampleMask = UINT_MAX;
	opaque_PSO_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaque_PSO_desc.NumRenderTargets = 1;
	opaque_PSO_desc.RTVFormats[0] = d3d_manager->Get_BB_Format();
	opaque_PSO_desc.SampleDesc.Count = d3d_manager->Is_4xMSAA() ? 4 : 1;
	opaque_PSO_desc.SampleDesc.Quality = d3d_manager->Is_4xMSAA() ? (d3d_manager->Get_4xMSAA_Qual() - 1) : 0;
	opaque_PSO_desc.DSVFormat = d3d_manager->Get_DS_Format();

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaque_wireframe_PSO_desc = opaque_PSO_desc;
	opaque_wireframe_PSO_desc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;

	Throw_If_Failed(device->CreateGraphicsPipelineState(&opaque_wireframe_PSO_desc, IID_PPV_ARGS(&m_pipeline_state_map[L"opaque_wireframe"])));
}

void TestScene::Binding_Key() {
	//m_input_manager->Bind_Key_Down(VK_W, BindingInfo(L"test", Action::MOVE_FORWARD));
	//m_input_manager->Bind_Key_Down(VK_S, BindingInfo(L"test", Action::MOVE_BACK));
	//m_input_manager->Bind_Key_Down(VK_A, BindingInfo(L"test", Action::MOVE_LEFT));
	//m_input_manager->Bind_Key_Down(VK_D, BindingInfo(L"test", Action::MOVE_RIGHT));

	//m_input_manager->Bind_Key_Down(VK_W, BindingInfo(L"test", Action::TELEPORT_FORWARD, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_S, BindingInfo(L"test", Action::TELEPORT_BACK, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_A, BindingInfo(L"test", Action::TELEPORT_LEFT, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_D, BindingInfo(L"test", Action::TELEPORT_RIGHT, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_SPACE, BindingInfo(L"test", Action::TELEPORT_UP, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_SHIFT, BindingInfo(L"test", Action::TELEPORT_DOWN, 1.0f));

	m_input_manager->Bind_Key_Down(VK_W, BindingInfo(L"player", Action::TELEPORT_FORWARD, 1.0f));
	m_input_manager->Bind_Key_Down(VK_S, BindingInfo(L"player", Action::TELEPORT_BACK, 1.0f));
	m_input_manager->Bind_Key_Down(VK_A, BindingInfo(L"player", Action::TELEPORT_LEFT, 1.0f));
	m_input_manager->Bind_Key_Down(VK_D, BindingInfo(L"player", Action::TELEPORT_RIGHT, 1.0f));
	m_input_manager->Bind_Key_Down(VK_SPACE, BindingInfo(L"player", Action::TELEPORT_UP, 1.0f));
	m_input_manager->Bind_Key_Down(VK_SHIFT, BindingInfo(L"player", Action::TELEPORT_DOWN, 1.0f));

	//m_input_manager->Bind_Key_Down(VK_SPACE, BindingInfo(L"test", Action::MOVE_UP, 1.0f));
	//m_input_manager->Bind_Key_Down(VK_SHIFT, BindingInfo(L"test", Action::MOVE_DOWN, 1.0f));

	m_input_manager->Bind_Key_Down(VK_Q, BindingInfo(L"maincamera", Action::ROTATE_PITCH, POINTF(-1.0f)));
	m_input_manager->Bind_Key_Down(VK_E, BindingInfo(L"maincamera", Action::ROTATE_PITCH, POINTF(1.0f)));

	m_input_manager->Bind_Mouse_Move(BindingInfo(L"maincamera", Action::ROTATE));
}

void TestScene::Pairing_Collision_Set() {
	//m_object_manager->Add_Collision_Pair(L"")
}
