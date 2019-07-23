#pragma once

#include <unordered_map>


enum BackgroundID { 
	end, 
	corridor_1,
	corridor_2,
	corridor_3,
	corridor_4,
	corridor_5,
	corridor_6,
	corridor_7,
	corridor_8,
	door_1, 
	door_2, 
	room_1 
};

enum MainRoomDoors {
	door_bang,
	door_kutschfahrt,
	door_tichu
};

static std::unordered_map<std::string, BackgroundID> backgroundIdentifierMap = {
	{ "bang_gang_ende", BackgroundID::end },
	{ "bang_gang_1", BackgroundID::corridor_1 },
	{ "bang_gang_2", BackgroundID::corridor_2 },
	{ "bang_gang_3", BackgroundID::corridor_3 },
	{ "bang_gang_4", BackgroundID::corridor_4 },
	{ "bang_gang_5", BackgroundID::corridor_5 },
	{ "bang_gang_6", BackgroundID::corridor_6 },
	{ "bang_gang_7", BackgroundID::corridor_7 },
	{ "bang_gang_8", BackgroundID::corridor_8 },
	{ "bang_tür_1", BackgroundID::door_1 },
	{ "bang_tür_2", BackgroundID::door_2 },
	{ "bang_raum_1", BackgroundID::room_1 }
};