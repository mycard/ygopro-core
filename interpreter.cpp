/*
 * interpreter.cpp
 *
 *  Created on: 2010-4-28
 *      Author: Argon
 */

#include "duel.h"
#include "group.h"
#include "card.h"
#include "effect.h"
#include "scriptlib.h"
#include "ocgapi.h"
#include "interpreter.h"

interpreter::interpreter(duel* pd): coroutines(256) {
	lua_state = luaL_newstate();
	current_state = lua_state;
	pduel = pd;
	memcpy(lua_getextraspace(lua_state), &pd, LUA_EXTRASPACE); //set_duel_info
	no_action = 0;
	call_depth = 0;
	disable_action_check = 0;
	//Initial
#ifdef YGOPRO_NO_LUA_SAFE
	luaL_openlibs(lua_state);
#else
	luaL_requiref(lua_state, "base", luaopen_base, 0);
	lua_pop(lua_state, 1);
	luaL_requiref(lua_state, "string", luaopen_string, 1);
	lua_pop(lua_state, 1);
	luaL_requiref(lua_state, "utf8", luaopen_utf8, 1);
	lua_pop(lua_state, 1);
	luaL_requiref(lua_state, "table", luaopen_table, 1);
	lua_pop(lua_state, 1);
	luaL_requiref(lua_state, "math", luaopen_math, 1);
#endif
	lua_pop(lua_state, 1);

	//add bit lib back
	lua_getglobal(lua_state, "bit32");
	lua_setglobal(lua_state, "bit");
	//open all libs
	scriptlib::open_cardlib(lua_state);
	scriptlib::open_effectlib(lua_state);
	scriptlib::open_grouplib(lua_state);
	scriptlib::open_duellib(lua_state);
	scriptlib::open_debuglib(lua_state);
	//extra scripts
	load_script("./script/constant.lua");
	load_script("./script/utility.lua");
	load_script("./script/procedure.lua");
	//load kpro constant
	//card data constants
	lua_pushinteger(lua_state, CARDDATA_CODE);
	lua_setglobal(lua_state, "CARDDATA_CODE");
	lua_pushinteger(lua_state, CARDDATA_ALIAS);
	lua_setglobal(lua_state, "CARDDATA_ALIAS");
	lua_pushinteger(lua_state, CARDDATA_SETCODE);
	lua_setglobal(lua_state, "CARDDATA_SETCODE");
	lua_pushinteger(lua_state, CARDDATA_TYPE);
	lua_setglobal(lua_state, "CARDDATA_TYPE");
	lua_pushinteger(lua_state, CARDDATA_LEVEL);
	lua_setglobal(lua_state, "CARDDATA_LEVEL");
	lua_pushinteger(lua_state, CARDDATA_ATTRIBUTE);
	lua_setglobal(lua_state, "CARDDATA_ATTRIBUTE");
	lua_pushinteger(lua_state, CARDDATA_ATTRIBUTE);
	lua_setglobal(lua_state, "CARDDATA_ATTRIBUTE");
	lua_pushinteger(lua_state, CARDDATA_RACE);
	lua_setglobal(lua_state, "CARDDATA_RACE");
	lua_pushinteger(lua_state, CARDDATA_ATTACK);
	lua_setglobal(lua_state, "CARDDATA_ATTACK");
	lua_pushinteger(lua_state, CARDDATA_DEFENSE);
	lua_setglobal(lua_state, "CARDDATA_DEFENSE");
	lua_pushinteger(lua_state, CARDDATA_LSCALE);
	lua_setglobal(lua_state, "CARDDATA_LSCALE");
	lua_pushinteger(lua_state, CARDDATA_RSCALE);
	lua_setglobal(lua_state, "CARDDATA_RSCALE");
	lua_pushinteger(lua_state, CARDDATA_LINK_MARKER);
	lua_setglobal(lua_state, "CARDDATA_LINK_MARKER");
	//effect flag2s
	lua_pushinteger(lua_state, EFFECT_FLAG2_SPOSITCH);
	lua_setglobal(lua_state, "EFFECT_FLAG2_SPOSITCH");
	lua_pushinteger(lua_state, EFFECT_FLAG2_AVAILABLE_BD);
	lua_setglobal(lua_state, "EFFECT_FLAG2_AVAILABLE_BD");
	lua_pushinteger(lua_state, EFFECT_FLAG2_ACTIVATE_MONSTER_SZONE);
	lua_setglobal(lua_state, "EFFECT_FLAG2_ACTIVATE_MONSTER_SZONE");
	//effects
	lua_pushinteger(lua_state, EFFECT_CHANGE_LINK_MARKER_KOISHI);
	lua_setglobal(lua_state, "EFFECT_CHANGE_LINK_MARKER_KOISHI");
	lua_pushinteger(lua_state, EFFECT_ADD_LINK_MARKER_KOISHI);
	lua_setglobal(lua_state, "EFFECT_ADD_LINK_MARKER_KOISHI");
	lua_pushinteger(lua_state, EFFECT_REMOVE_LINK_MARKER_KOISHI);
	lua_setglobal(lua_state, "EFFECT_REMOVE_LINK_MARKER_KOISHI");
	lua_pushinteger(lua_state, EFFECT_CANNOT_LOSE_KOISHI);
	lua_setglobal(lua_state, "EFFECT_CANNOT_LOSE_KOISHI");
	lua_pushinteger(lua_state, EFFECT_EXTRA_TOMAIN_KOISHI);
	lua_setglobal(lua_state, "EFFECT_EXTRA_TOMAIN_KOISHI");
	lua_pushinteger(lua_state, EFFECT_OVERLAY_REMOVE_COST_CHANGE_KOISHI);
	lua_setglobal(lua_state, "EFFECT_OVERLAY_REMOVE_COST_CHANGE_KOISHI");
	lua_pushinteger(lua_state, EFFECT_ALLOW_SYNCHRO_KOISHI);
	lua_setglobal(lua_state, "EFFECT_ALLOW_SYNCHRO_KOISHI");
	lua_pushinteger(lua_state, EFFECT_MINIATURE_GARDEN_GIRL);
	lua_setglobal(lua_state, "EFFECT_MINIATURE_GARDEN_GIRL");
	lua_pushinteger(lua_state, EFFECT_ADD_SUMMON_TYPE_KOISHI);
	lua_setglobal(lua_state, "EFFECT_ADD_SUMMON_TYPE_KOISHI");
	lua_pushinteger(lua_state, EFFECT_REMOVE_SUMMON_TYPE_KOISHI);
	lua_setglobal(lua_state, "EFFECT_REMOVE_SUMMON_TYPE_KOISHI");
	lua_pushinteger(lua_state, EFFECT_CHANGE_SUMMON_TYPE_KOISHI);
	lua_setglobal(lua_state, "EFFECT_CHANGE_SUMMON_TYPE_KOISHI");
	lua_pushinteger(lua_state, EFFECT_CHANGE_SUMMON_LOCATION_KOISHI);
	lua_setglobal(lua_state, "EFFECT_CHANGE_SUMMON_LOCATION_KOISHI");
	lua_pushinteger(lua_state, EFFECT_LINK_SPELL_KOISHI);
	lua_setglobal(lua_state, "EFFECT_LINK_SPELL_KOISHI");
	lua_pushinteger(lua_state, EFFECT_SEA_PULSE);
	lua_setglobal(lua_state, "EFFECT_SEA_PULSE");
	lua_pushinteger(lua_state, EFFECT_MAP_OF_HEAVEN);
	lua_setglobal(lua_state, "EFFECT_MAP_OF_HEAVEN");

	//music hints
	lua_pushinteger(lua_state, HINT_MUSIC);
	lua_setglobal(lua_state, "HINT_MUSIC");
	lua_pushinteger(lua_state, HINT_SOUND);
	lua_setglobal(lua_state, "HINT_SOUND");
	lua_pushinteger(lua_state, HINT_MUSIC_OGG);
	lua_setglobal(lua_state, "HINT_MUSIC_OGG");
	//detect operating system
#ifdef _WIN32
	lua_pushboolean(lua_state, 1);
	lua_setglobal(lua_state, "_WIN32");
#endif
}
interpreter::~interpreter() {
	lua_close(lua_state);
}
int32 interpreter::register_card(card *pcard) {
	//create a card in by userdata
	luaL_checkstack(lua_state, 1, NULL);
	luaL_checkstack(current_state, 1, NULL);
	card ** ppcard = (card**) lua_newuserdata(lua_state, sizeof(card*));
	*ppcard = pcard;
	pcard->ref_handle = luaL_ref(lua_state, LUA_REGISTRYINDEX);
	//some userdata may be created in script like token so use current_state
	lua_rawgeti(current_state, LUA_REGISTRYINDEX, pcard->ref_handle);
	//load script
	if(pcard->data.alias && (pcard->data.alias < pcard->data.code + CARD_ARTWORK_VERSIONS_OFFSET) && (pcard->data.code < pcard->data.alias + CARD_ARTWORK_VERSIONS_OFFSET))
		load_card_script(pcard->data.alias);
	else
		load_card_script(pcard->data.code);
	//set metatable of pointer to base script
	lua_setmetatable(current_state, -2);
	lua_pop(current_state, 1);
	//Initial
	if(pcard->data.code && (!(pcard->data.type & TYPE_NORMAL) || (pcard->data.type & TYPE_PENDULUM))) {
		pcard->set_status(STATUS_INITIALIZING, TRUE);
		add_param(pcard, PARAM_TYPE_CARD);
		call_card_function(pcard, "initial_effect", 1, 0);
		pcard->set_status(STATUS_INITIALIZING, FALSE);
	}
	pcard->cardid = pduel->game_field->infos.card_id++;
	return OPERATION_SUCCESS;
}
void interpreter::register_effect(effect *peffect) {
	if (!peffect)
		return;
	//create a effect in by userdata
	luaL_checkstack(lua_state, 3, NULL);
	effect ** ppeffect = (effect**) lua_newuserdata(lua_state, sizeof(effect*));
	*ppeffect = peffect;
	peffect->ref_handle = luaL_ref(lua_state, LUA_REGISTRYINDEX);
	//set metatable of pointer to base script
	lua_rawgeti(lua_state, LUA_REGISTRYINDEX, peffect->ref_handle);
	lua_getglobal(lua_state, "Effect");
	lua_setmetatable(lua_state, -2);
	lua_pop(lua_state, 1);
}
void interpreter::unregister_effect(effect *peffect) {
	if (!peffect)
		return;
	if(peffect->condition)
		luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->condition);
	if(peffect->cost)
		luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->cost);
	if(peffect->target)
		luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->target);
	if(peffect->operation)
		luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->operation);
	if(peffect->value && peffect->is_flag(EFFECT_FLAG_FUNC_VALUE))
		luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->value);
	luaL_unref(lua_state, LUA_REGISTRYINDEX, peffect->ref_handle);
	peffect->ref_handle = 0;
}
void interpreter::register_group(group *pgroup) {
	if (!pgroup)
		return;
	//create a group in by userdata
	luaL_checkstack(lua_state, 3, NULL);
	group ** ppgroup = (group**) lua_newuserdata(lua_state, sizeof(group*));
	*ppgroup = pgroup;
	pgroup->ref_handle = luaL_ref(lua_state, LUA_REGISTRYINDEX);
	//set metatable of pointer to base script
	lua_rawgeti(lua_state, LUA_REGISTRYINDEX, pgroup->ref_handle);
	lua_getglobal(lua_state, "Group");
	lua_setmetatable(lua_state, -2);
	lua_pop(lua_state, 1);
}
void interpreter::unregister_group(group *pgroup) {
	if (!pgroup)
		return;
	luaL_unref(lua_state, LUA_REGISTRYINDEX, pgroup->ref_handle);
	pgroup->ref_handle = 0;
}
int32 interpreter::load_script(const char* script_name) {
	int32 len = 0;
	byte* buffer = read_script(script_name, &len);
	if (!buffer)
		return OPERATION_FAIL;
	no_action++;
	int32 error = luaL_loadbuffer(current_state, (char*)buffer, len, script_name) || lua_pcall(current_state, 0, 0, 0);
	if (error) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		return OPERATION_FAIL;
	}
	no_action--;
	return OPERATION_SUCCESS;
}
int32 interpreter::load_card_script(uint32 code) {
	char class_name[20];
	sprintf(class_name, "c%d", code);
	luaL_checkstack(current_state, 1, NULL);
	lua_getglobal(current_state, class_name);
	//if script is not loaded, create and load it
	if (lua_isnil(current_state, -1)) {
		luaL_checkstack(current_state, 5, NULL);
		lua_pop(current_state, 1);
		//create a table & set metatable
		lua_createtable(current_state, 0, 0);
		lua_setglobal(current_state, class_name);
		lua_getglobal(current_state, class_name);
		lua_getglobal(current_state, "Card");
		lua_setmetatable(current_state, -2);
		lua_pushstring(current_state, "__index");
		lua_pushvalue(current_state, -2);
		lua_rawset(current_state, -3);
		lua_getglobal(current_state, class_name);
		lua_setglobal(current_state, "self_table");
		lua_pushinteger(current_state, code);
		lua_setglobal(current_state, "self_code");
		char script_name[64];
		sprintf(script_name, "./script/c%d.lua", code);
		int32 res = load_script(script_name);
		lua_pushnil(current_state);
		lua_setglobal(current_state, "self_table");
		lua_pushnil(current_state);
		lua_setglobal(current_state, "self_code");
		if(!res) {
			return OPERATION_FAIL;
		}
	}
	return OPERATION_SUCCESS;
}
void interpreter::add_param(void *param, int32 type, bool front) {
	if(front)
		params.emplace_front(param, type);
	else
		params.emplace_back(param, type);
}
void interpreter::add_param(int32 param, int32 type, bool front) {
	if(front)
		params.emplace_front((void*)(intptr_t)param, type);
	else
		params.emplace_back((void*)(intptr_t)param, type);
}
void interpreter::push_param(lua_State* L, bool is_coroutine) {
	int32 pushed = 0;
	for (const auto& it : params) {
		luaL_checkstack(L, 1, NULL);
		uint32 type = it.second;
		switch(type) {
		case PARAM_TYPE_INT:
			lua_pushinteger(L, (lua_Integer)it.first);
			break;
		case PARAM_TYPE_STRING:
			lua_pushstring(L, (const char *)it.first);
			break;
		case PARAM_TYPE_BOOLEAN:
			lua_pushboolean(L, (int32)(intptr_t)it.first);
			break;
		case PARAM_TYPE_CARD: {
			if (it.first)
				lua_rawgeti(L, LUA_REGISTRYINDEX, ((card*)it.first)->ref_handle);
			else
				lua_pushnil(L);
			break;
		}
		case PARAM_TYPE_EFFECT: {
			if (it.first)
				lua_rawgeti(L, LUA_REGISTRYINDEX, ((effect*)it.first)->ref_handle);
			else
				lua_pushnil(L);
			break;
		}
		case PARAM_TYPE_GROUP: {
			if (it.first)
				lua_rawgeti(L, LUA_REGISTRYINDEX, ((group*)it.first)->ref_handle);
			else
				lua_pushnil(L);
			break;
		}
		case PARAM_TYPE_FUNCTION: {
			function2value(L, (int32)(intptr_t)it.first);
			break;
		}
		case PARAM_TYPE_INDEX: {
			int32 index = (int32)(intptr_t)it.first;
			if(index > 0)
				lua_pushvalue(L, index);
			else if(is_coroutine) {
				//copy value from current_state to new stack
				lua_pushvalue(current_state, index);
				int32 ref = luaL_ref(current_state, LUA_REGISTRYINDEX);
				lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
				luaL_unref(current_state, LUA_REGISTRYINDEX, ref);
			} else {
				//the calling function is pushed before the params, so the actual index is: index - pushed -1
				lua_pushvalue(L, index - pushed - 1);
			}
			break;
		}
		}
		pushed++;
	}
	params.clear();
}
int32 interpreter::call_function(int32 f, uint32 param_count, int32 ret_count) {
	if (!f) {
		sprintf(pduel->strbuffer, "\"CallFunction\": attempt to call a null function.");
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	if (param_count != params.size()) {
		sprintf(pduel->strbuffer, "\"CallFunction\": incorrect parameter count (%d expected, %zu pushed)", param_count, params.size());
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	function2value(current_state, f);
	if (!lua_isfunction(current_state, -1)) {
		sprintf(pduel->strbuffer, "\"CallFunction\": attempt to call an error function");
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	no_action++;
	call_depth++;
	push_param(current_state);
	if (lua_pcall(current_state, param_count, ret_count, 0)) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return OPERATION_FAIL;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return OPERATION_SUCCESS;
}
int32 interpreter::call_card_function(card* pcard, const char* f, uint32 param_count, int32 ret_count) {
	if (param_count != params.size()) {
		sprintf(pduel->strbuffer, "\"CallCardFunction\"(c%d.%s): incorrect parameter count", pcard->data.code, f);
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	card2value(current_state, pcard);
	luaL_checkstack(current_state, 1, NULL);
	lua_getfield(current_state, -1, f);
	if (!lua_isfunction(current_state, -1)) {
		sprintf(pduel->strbuffer, "\"CallCardFunction\"(c%d.%s): attempt to call an error function", pcard->data.code, f);
		handle_message(pduel, 1);
		lua_pop(current_state, 2);
		params.clear();
		return OPERATION_FAIL;
	}
	no_action++;
	call_depth++;
	lua_remove(current_state, -2);
	push_param(current_state);
	if (lua_pcall(current_state, param_count, ret_count, 0)) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return OPERATION_FAIL;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return OPERATION_SUCCESS;
}
int32 interpreter::call_code_function(uint32 code, const char* f, uint32 param_count, int32 ret_count) {
	if (param_count != params.size()) {
		sprintf(pduel->strbuffer, "\"CallCodeFunction\": incorrect parameter count");
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	if (code)
		load_card_script(code);
	else
		lua_getglobal(current_state, "Auxiliary");
	luaL_checkstack(current_state, 1, NULL);
	lua_getfield(current_state, -1, f);
	if (!lua_isfunction(current_state, -1)) {
		if(!code) {
			params.clear();
			return OPERATION_FAIL;
		}
		sprintf(pduel->strbuffer, "\"CallCodeFunction\": attempt to call an error function");
		handle_message(pduel, 1);
		lua_pop(current_state, 2);
		params.clear();
		return OPERATION_FAIL;
	}
	lua_remove(current_state, -2);
	no_action++;
	call_depth++;
	push_param(current_state);
	if (lua_pcall(current_state, param_count, ret_count, 0)) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return OPERATION_FAIL;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return OPERATION_SUCCESS;
}
int32 interpreter::check_condition(int32 f, uint32 param_count) {
	if(!f) {
		params.clear();
		return TRUE;
	}
	no_action++;
	call_depth++;
	if (call_function(f, param_count, 1)) {
		int32 result = lua_toboolean(current_state, -1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return result;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return OPERATION_FAIL;
}
int32 interpreter::check_matching(card* pcard, int32 findex, int32 extraargs) {
	if(!findex || lua_isnil(current_state, findex))
		return TRUE;
	no_action++;
	call_depth++;
	luaL_checkstack(current_state, 1 + extraargs, NULL);
	lua_pushvalue(current_state, findex);
	interpreter::card2value(current_state, pcard);
	for(int32 i = 0; i < extraargs; ++i)
		lua_pushvalue(current_state, (int32)(-extraargs - 2));
	if (lua_pcall(current_state, 1 + extraargs, 1, 0)) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return OPERATION_FAIL;
	}
	int32 result = lua_toboolean(current_state, -1);
	lua_pop(current_state, 1);
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return result;
}
int32 interpreter::get_operation_value(card* pcard, int32 findex, int32 extraargs) {
	if(!findex || lua_isnil(current_state, findex))
		return 0;
	no_action++;
	call_depth++;
	luaL_checkstack(current_state, 1 + extraargs, NULL);
	lua_pushvalue(current_state, findex);
	interpreter::card2value(current_state, pcard);
	for(int32 i = 0; i < extraargs; ++i)
		lua_pushvalue(current_state, (int32)(-extraargs - 2));
	if (lua_pcall(current_state, 1 + extraargs, 1, 0)) {
		sprintf(pduel->strbuffer, "%s", lua_tostring(current_state, -1));
		handle_message(pduel, 1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return OPERATION_FAIL;
	}
	int32 result = lua_isinteger(current_state, -1) ? (int32)lua_tointeger(current_state, -1) : (int32)lua_tonumber(current_state, -1);
	lua_pop(current_state, 1);
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return result;
}
int32 interpreter::get_function_value(int32 f, uint32 param_count) {
	if(!f) {
		params.clear();
		return 0;
	}
	no_action++;
	call_depth++;
	if (call_function(f, param_count, 1)) {
		int32 result = 0;
		if(lua_isboolean(current_state, -1))
			result = lua_toboolean(current_state, -1);
		else if(lua_isinteger(current_state, -1))
			result = (int32)lua_tointeger(current_state, -1);
		else
			result = (int32)lua_tonumber(current_state, -1);
		lua_pop(current_state, 1);
		no_action--;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return result;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return OPERATION_FAIL;
}
int32 interpreter::get_function_value(int32 f, uint32 param_count, std::vector<int32>* result) {
	int32 is_success = OPERATION_FAIL;
	if(!f) {
		params.clear();
		return is_success;
	}
	int32 stack_top = lua_gettop(current_state);
	no_action++;
	call_depth++;
	if (call_function(f, param_count, LUA_MULTRET)) {
		int32 stack_newtop = lua_gettop(current_state);
		for (int32 index = stack_top + 1; index <= stack_newtop; ++index) {
			int32 return_value = 0;
			if(lua_isboolean(current_state, index))
				return_value = lua_toboolean(current_state, index);
			else if(lua_isinteger(current_state, index))
				return_value = (int32)lua_tointeger(current_state, index);
			else
				return_value = (int32)lua_tonumber(current_state, index);
			result->push_back(return_value);
		}
		lua_settop(current_state, stack_top);
		is_success = OPERATION_SUCCESS;
	}
	no_action--;
	call_depth--;
	if(call_depth == 0) {
		pduel->release_script_group();
		pduel->restore_assumes();
	}
	return is_success;
}
int32 interpreter::call_coroutine(int32 f, uint32 param_count, uint32 * yield_value, uint16 step) {
	*yield_value = 0;
	if (!f) {
		sprintf(pduel->strbuffer, "\"CallCoroutine\": attempt to call a null function");
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	if (param_count != params.size()) {
		sprintf(pduel->strbuffer, "\"CallCoroutine\": incorrect parameter count");
		handle_message(pduel, 1);
		params.clear();
		return OPERATION_FAIL;
	}
	auto it = coroutines.find(f);
	lua_State* rthread;
	if (it == coroutines.end()) {
		rthread = lua_newthread(lua_state);
		function2value(rthread, f);
		if(!lua_isfunction(rthread, -1)) {
			sprintf(pduel->strbuffer, "\"CallCoroutine\": attempt to call an error function");
			handle_message(pduel, 1);
			params.clear();
			return OPERATION_FAIL;
		}
		call_depth++;
		coroutines.emplace(f, rthread);
	} else {
		rthread = it->second;
		if(step == 0) {
			sprintf(pduel->strbuffer, "recursive event trigger detected.");
			handle_message(pduel, 1);
			params.clear();
			call_depth--;
			if(call_depth == 0) {
				pduel->release_script_group();
				pduel->restore_assumes();
			}
			return OPERATION_FAIL;
		}
	}
	push_param(rthread, true);
	lua_State* prev_state = current_state;
	current_state = rthread;
#if (LUA_VERSION_NUM >= 504)
	int32 nresults;
	int32 result = lua_resume(rthread, prev_state, param_count, &nresults);
#else
	int32 result = lua_resume(rthread, 0, param_count);
	int32 nresults = lua_gettop(rthread);
#endif
	if (result == LUA_OK) {
		coroutines.erase(f);
		if(yield_value) {
			if(nresults == 0)
				*yield_value = 0;
			else if(lua_isboolean(rthread, -1))
				*yield_value = lua_toboolean(rthread, -1);
			else
				*yield_value = (uint32)lua_tointeger(rthread, -1);
		}
		current_state = lua_state;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return COROUTINE_FINISH;
	} else if (result == LUA_YIELD) {
		return COROUTINE_YIELD;
	} else {
		coroutines.erase(f);
		sprintf(pduel->strbuffer, "%s", lua_tostring(rthread, -1));
		handle_message(pduel, 1);
		lua_pop(rthread, 1);
		current_state = lua_state;
		call_depth--;
		if(call_depth == 0) {
			pduel->release_script_group();
			pduel->restore_assumes();
		}
		return COROUTINE_ERROR;
	}
}
int32 interpreter::clone_function_ref(int32 func_ref) {
	luaL_checkstack(current_state, 1, NULL);
	lua_rawgeti(current_state, LUA_REGISTRYINDEX, func_ref);
	int32 ref = luaL_ref(current_state, LUA_REGISTRYINDEX);
	return ref;
}
void* interpreter::get_ref_object(int32 ref_handler) {
	if(ref_handler == 0)
		return nullptr;
	luaL_checkstack(current_state, 1, NULL);
	lua_rawgeti(current_state, LUA_REGISTRYINDEX, ref_handler);
	void* p = *(void**)lua_touserdata(current_state, -1);
	lua_pop(current_state, 1);
	return p;
}
//Convert a pointer to a lua value, +1 -0
void interpreter::card2value(lua_State* L, card* pcard) {
	luaL_checkstack(L, 1, NULL);
	if (!pcard || pcard->ref_handle == 0)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, pcard->ref_handle);
}
void interpreter::group2value(lua_State* L, group* pgroup) {
	luaL_checkstack(L, 1, NULL);
	if (!pgroup || pgroup->ref_handle == 0)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, pgroup->ref_handle);
}
void interpreter::effect2value(lua_State* L, effect* peffect) {
	luaL_checkstack(L, 1, NULL);
	if (!peffect || peffect->ref_handle == 0)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, peffect->ref_handle);
}
void interpreter::function2value(lua_State* L, int32 func_ref) {
	luaL_checkstack(L, 1, NULL);
	if (!func_ref)
		lua_pushnil(L);
	else
		lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
}
int32 interpreter::get_function_handle(lua_State* L, int32 index) {
	luaL_checkstack(L, 1, NULL);
	lua_pushvalue(L, index);
	int32 ref = luaL_ref(L, LUA_REGISTRYINDEX);
	return ref;
}
duel* interpreter::get_duel_info(lua_State * L) {
	duel* pduel;
	memcpy(&pduel, lua_getextraspace(L), LUA_EXTRASPACE);
	return pduel;
}
