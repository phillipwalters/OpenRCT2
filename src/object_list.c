#pragma region Copyright (c) 2014-2016 OpenRCT2 Developers
/*****************************************************************************
 * OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
 *
 * OpenRCT2 is the work of many authors, a full list can be found in contributors.md
 * For more information, visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * A full copy of the GNU General Public License can be found in licence.txt
 *****************************************************************************/
#pragma endregion

#include "addresses.h"
#include "config.h"
#include "game.h"
#include "localisation/localisation.h"
#include "object.h"
#include "object_list.h"
#include "object/ObjectRepository.h"
#include "platform/platform.h"
#include "rct1.h"
#include "ride/track.h"
#include "ride/track_design.h"
#include "util/sawyercoding.h"
#include "world/entrance.h"
#include "world/footpath.h"
#include "world/scenery.h"
#include "world/water.h"

// 98DA00
int object_entry_group_counts[] = {
	128,	// rides
	252,	// small scenery
	128,	// large scenery
	128,	// walls
	32,		// banners
	16,		// paths
	15,		// path bits
	19,		// scenery sets
	1,		// park entrance
	1,		// water
	1		// scenario text
};

// 98DA2C
int object_entry_group_encoding[] = {
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_RLE,
	CHUNK_ENCODING_ROTATE
};

#if NO_RCT2
	rct_ride_entry				*gRideEntries[128];
	rct_small_scenery_entry		*gSmallSceneryEntries[252];
	rct_large_scenery_entry		*gLargeSceneryEntries[128];
	rct_wall_scenery_entry		*gWallSceneryEntries[128];
	rct_banner					*gBannerSceneryEntries[32];
	rct_footpath_entry			*gFootpathEntries[16];
	rct_path_bit_scenery_entry	*gFootpathAdditionEntries[15];
	rct_scenery_set_entry		*gSceneryGroupEntries[19];
	rct_entrance_type			*gParkEntranceEntries[1];
	rct_water_type				*gWaterEntries[1];
	rct_stex_entry				*gStexEntries[1];
#endif


// 0x98D97C chunk address', 0x98D980 object_entries
const rct_object_entry_group object_entry_groups[] = {
	(void**)(gRideEntries				), (rct_object_entry_extended*)(0x00F3F03C             ),	// rides
	(void**)(gSmallSceneryEntries		), (rct_object_entry_extended*)(0x00F3F03C + (128 * 20)),	// small scenery	0x009AD1A4, 0xF2FA3C
	(void**)(gLargeSceneryEntries		), (rct_object_entry_extended*)(0x00F3F03C + (380 * 20)),	// large scenery	0x009AD594, 0xF40DEC
	(void**)(gWallSceneryEntries		), (rct_object_entry_extended*)(0x00F3F03C + (508 * 20)),	// walls			0x009AD794, 0xF417EC
	(void**)(gBannerSceneryEntries		), (rct_object_entry_extended*)(0x00F3F03C + (636 * 20)),	// banners			0x009AD994, 0xF421EC
	(void**)(gFootpathEntries			), (rct_object_entry_extended*)(0x00F3F03C + (668 * 20)),	// paths			0x009ADA14, 0xF4246C
	(void**)(gFootpathAdditionEntries	), (rct_object_entry_extended*)(0x00F3F03C + (684 * 20)),	// path bits		0x009ADA54, 0xF425AC
	(void**)(gSceneryGroupEntries		), (rct_object_entry_extended*)(0x00F3F03C + (699 * 20)),	// scenery sets		0x009ADA90, 0xF426D8
	(void**)(gParkEntranceEntries		), (rct_object_entry_extended*)(0x00F3F03C + (718 * 20)),	// park entrance	0x009ADADC, 0xF42854
	(void**)(gWaterEntries				), (rct_object_entry_extended*)(0x00F3F03C + (719 * 20)),	// water			0x009ADAE0, 0xF42868
	(void**)(gStexEntries				), (rct_object_entry_extended*)(0x00F3F03C + (720 * 20))	// scenario text	0x009ADAE4, 0xF4287C
};

int check_object_entry(rct_object_entry *entry)
{
	uint32 *dwords = (uint32*)entry;
	return (0xFFFFFFFF & dwords[0] & dwords[1] & dwords[2] & dwords[3]) + 1 != 0;
}

/**
 *
 *  rct2: 0x006AB344
 */
void object_create_identifier_name(char* string_buffer, const rct_object_entry* object)
{
	for (uint8 i = 0; i < 8; ++i){
		if (object->name[i] != ' '){
			*string_buffer++ = object->name[i];
		}
	}

	*string_buffer++ = '/';

	for (uint8 i = 0; i < 4; ++i){
		uint8 flag_part = (object->flags >> (i * 8)) & 0xFF;
		*string_buffer++ = RCT2_ADDRESS(0x0098DA64, char)[flag_part >> 4];
		*string_buffer++ = RCT2_ADDRESS(0x0098DA64, char)[flag_part & 0xF];
	}

	for (uint8 i = 0; i < 4; ++i){
		uint8 checksum_part = (object->checksum >> (i * 8)) & 0xFF;
		*string_buffer++ = RCT2_ADDRESS(0x0098DA64, char)[checksum_part >> 4];
		*string_buffer++ = RCT2_ADDRESS(0x0098DA64, char)[checksum_part & 0xF];
	}
	*string_buffer++ = '\0';
}

/**
 *
 *  rct2: 0x675827
 */
void set_load_objects_fail_reason()
{
	return;

	rct_object_entry *object;
	memcpy(&object, gCommonFormatArgs, sizeof(rct_object_entry*));
	
	int expansion = (object->flags & 0xFF) >> 4;
	if (expansion == 0 ||
		expansion == 8 ||
		RCT2_GLOBAL(RCT2_ADDRESS_EXPANSION_FLAGS, uint16) & (1 << expansion)
	) {
		char* string_buffer = RCT2_ADDRESS(0x9BC677, char);

		format_string(string_buffer, STR_MISSING_OBJECT_DATA_ID, 0);

		object_create_identifier_name(string_buffer, object);
		gErrorType = ERROR_TYPE_FILE_LOAD;
		gErrorStringId = STR_PLACEHOLDER;
		return;
	}

	rct_string_id expansionNameId;
	switch(expansion) {
		case 1: // Wacky Worlds
			expansionNameId = STR_OBJECT_FILTER_WW;
			break;
		case 2: // Time Twister
			expansionNameId = STR_OBJECT_FILTER_TT;
			break;
		default:
			gErrorType = ERROR_TYPE_FILE_LOAD;
			gErrorStringId = STR_REQUIRES_AN_ADDON_PACK;
			return;
	}

	char* string_buffer = RCT2_ADDRESS(0x9BC677, char);

	format_string(string_buffer, STR_REQUIRES_THE_FOLLOWING_ADDON_PACK, &expansionNameId);

	gErrorType = ERROR_TYPE_FILE_LOAD;
	gErrorStringId = STR_PLACEHOLDER;
}

/**
 *
 *  rct2: 0x006AA0C6
 */
bool object_read_and_load_entries(SDL_RWops* rw)
{
	// Read all the object entries
	rct_object_entry *entries = malloc(OBJECT_ENTRY_COUNT * sizeof(rct_object_entry));
	sawyercoding_read_chunk(rw, (uint8*)entries);
	bool result = object_load_entries(entries);
	free(entries);
	return result;
}

bool object_load_entries(rct_object_entry* entries)
{
	log_verbose("loading required objects");

	object_unload_all();

	bool loadFailed = false;
	// Load each object
	for (int i = 0; i < OBJECT_ENTRY_COUNT; i++) {
		if (!check_object_entry(&entries[i])) {
			continue;
		}

		// Get entry group index
		int entryGroupIndex = i;
		for (int j = 0; j < countof(object_entry_group_counts); j++) {
			if (entryGroupIndex < object_entry_group_counts[j])
				break;
			entryGroupIndex -= object_entry_group_counts[j];
		}

		// Load the obect
		if (!object_load_chunk(entryGroupIndex, &entries[i], NULL)) {
			// log_error("failed to load entry: %.8s", entries[i].name);
			// memcpy(gCommonFormatArgs, &entries[i], sizeof(rct_object_entry));
			loadFailed = true;
		}
	}

	if (loadFailed) {
		object_unload_all();
		return false;
	}

	log_verbose("finished loading required objects");
	return true;
}

/**
 *
 *  rct2: 0x006A9DA2
 * bl = entry_index
 * ecx = entry_type
 */
int find_object_in_entry_group(const rct_object_entry* entry, uint8* entry_type, uint8* entry_index){
	*entry_type = entry->flags & 0xF;

	rct_object_entry_group entry_group = object_entry_groups[*entry_type];
	for (*entry_index = 0;
		*entry_index < object_entry_group_counts[*entry_type];
		++(*entry_index),
		entry_group.chunks++,
		entry_group.entries++){

		if (*entry_group.chunks == (uint8*)-1) continue;

		if (object_entry_compare((rct_object_entry*)entry_group.entries, entry))break;
	}

	if (*entry_index == object_entry_group_counts[*entry_type])return 0;
	return 1;
}

void object_list_init()
{
	for (uint8 objectType = 0; objectType < OBJECT_ENTRY_GROUP_COUNT; objectType++) {
		for (size_t i = 0; i < (size_t)object_entry_group_counts[objectType]; i++) {
			object_entry_groups[objectType].chunks[i] = (void*)-1;
		}
	}
}

void get_type_entry_index(size_t index, uint8 * outObjectType, uint8 * outEntryIndex)
{
	uint8 objectType = OBJECT_TYPE_RIDE;
	for (size_t i = 0; i < OBJECT_ENTRY_GROUP_COUNT; i++) {
		size_t groupCount = object_entry_group_counts[i];
		if (index >= groupCount) {
			index -= groupCount;
			objectType++;
		} else {
			break;
		}
	}

	if (outObjectType != NULL) *outObjectType = objectType;
	if (outEntryIndex != NULL) *outEntryIndex = (uint8)index;
}

const rct_object_entry * get_loaded_object_entry(size_t index)
{
	uint8 objectType, entryIndex;
	get_type_entry_index(index, &objectType, &entryIndex);

	rct_object_entry * entry = (rct_object_entry *)&(object_entry_groups[objectType].entries[entryIndex]);
	return entry;
}

void * get_loaded_object_chunk(size_t index)
{
	uint8 objectType, entryIndex;
	get_type_entry_index(index, &objectType, &entryIndex);

	void *entry = object_entry_groups[objectType].chunks[entryIndex];
	return entry;
}
