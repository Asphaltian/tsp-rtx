#pragma once
#define CLIENT_ENTITY_INTERFACE_VERSION "VClientEntityList003"
#include "game/structs.hpp"

namespace sdk
{
	class entity_list {
	public:
		components::C_BaseEntity* get_client_entity(int i);
		components::C_BaseEntity* get_client_entity_from_handle(components::CBaseHandle hEnt);
		//int get_max_entity();
	};

}
