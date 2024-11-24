#include "Scene.h"
#include "D3DManager.h"

void Scene::Flush_Cmd_Q(D3DManager* d3d_manager) {
	d3d_manager->Rst_Cmd_List();
	d3d_manager->Cls_Cmd_List();
	d3d_manager->Exct_Cmd_List();
	d3d_manager->Flush_Cmd_Q();
}