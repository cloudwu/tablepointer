#include <lua.h>
#include <lauxlib.h>
#include "lapi.h"
#include "ltable.h"

/*
	table
	return pointer
 */
static int
ltopointer(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	const void * t = lua_topointer(L, 1);
	lua_pushlightuserdata(L, (void *)t);
	return 1;
}

/*
	pointer
	return table
 */
static int
lcreatetable(lua_State *L) {
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	Table *t = lua_touserdata(L, 1);
	int narr = t->sizearray;
	int nrec = 1 << t->lsizenode;
	lua_createtable(L, narr, nrec);
	return 1;
}

static int
array_next(const Table *t, int index) {
	while(t->sizearray > index) {
		if (!ttisnil(&t->array[index++])) {
			return index;
		}
	}
	return 0;
}

static int
hash_next(const Table *t, int index) {
	index = -index;
	int nrec = 1 << t->lsizenode;
	while(nrec > index) {
		if (!ttisnil(gval(gnode(t, index++)))) {
			return -index;
		}
	}
	return 0;
}

static void
pushvalue(lua_State *L, const TValue *v) {
	int t = ttnov(v);
	if (!iscollectable(v)) {
		if (t == LUA_TLIGHTUSERDATA) {
			luaL_error(L, "Don't support lightuserdata");
		}
		lua_lock(L);
		setobj2s(L, L->top, v);
		api_incr_top(L);
		lua_unlock(L);
		return;
	}
	switch (t) {
	case LUA_TTABLE:
		lua_pushlightuserdata(L, (void *)v);
		break;
	case LUA_TSTRING:
		lua_pushlstring(L, svalue(v), vslen(v));
		break;
	default:
		luaL_error(L, "Don't support %s", lua_typename(L, t));
	}
}

/*
	pointer
	index 0:start 1:array part -1:hash part

	return index, key, value
 */
static int
literator(lua_State *L) {
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	Table *t = lua_touserdata(L, 1);
	int index = luaL_checkinteger(L, 2);
	if (index >= 0) {
		if (t->sizearray > 0) {
			index = array_next(t, index);
			if (index != 0) {
				lua_pushinteger(L, index);
				lua_pushinteger(L, index);
				pushvalue(L, &t->array[index-1]);
				return 3;
			}
		}
	}
	if (t->lsizenode > 0) {
		index = hash_next(t, index);
		if (index != 0) {
			Node *n = gnode(t, -index-1);
			lua_pushinteger(L, index);
			pushvalue(L, gkey(n));			
			pushvalue(L, gval(n));
			return 3;
		}
	}
	return 0;
}

/*
	pointer table
	return function iterator, pointer, 0 
 */
static int
lpairs(lua_State *L) {
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	lua_pushcfunction(L, literator);
	lua_pushvalue(L,1);
	lua_pushinteger(L, 0);

	return 3;
}

int
luaopen_tablepointer(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "topointer", ltopointer },
		{ "createtable", lcreatetable },
		{ "pairs", lpairs },
		{ NULL, NULL },
	};
	luaL_newlib(L,l);
	return 1;
}
