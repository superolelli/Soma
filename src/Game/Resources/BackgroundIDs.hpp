#pragma once

#include <unordered_map>


enum MainRoomDoors {
	door_bang,
	door_kutschfahrt,
	door_tichu
};

static std::unordered_map<std::string, int> backgroundIdentifierMap[2] = {
	{
		{ "bang_gang_ende", 0 },
		{ "bang_gang_1", 1 },
		{ "bang_gang_2", 2 },
		{ "bang_gang_3", 3 },
		{ "bang_gang_4", 4 },
		{ "bang_gang_5", 5 },
		{ "bang_gang_6", 6 },
		{ "bang_gang_7", 7 },
		{ "bang_gang_8", 8 },
		{ "bang_tür_1", 9 },
		{ "bang_tür_2", 10 },
		{ "bang_raum_1", 11 },
		{ "bang_raum_2", 12 },
		{ "bang_raum_3", 13 }
	},
	{
		{ "kutschfahrt_gang_ende", 0 },
		{ "kutschfahrt_gang_1", 1 },
		{ "kutschfahrt_gang_2", 2 },
		{ "kutschfahrt_gang_3", 3 },
		{ "kutschfahrt_gang_4", 4 },
		{ "kutschfahrt_gang_5", 5 },
		{ "kutschfahrt_gang_6", 6 },
		{ "kutschfahrt_raum_1", 7 }

	}
};
