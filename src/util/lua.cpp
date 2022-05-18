
#include "util/misc.h"
#include "stub/misc.h"
#include "stub/baseentity.h"
#include "stub/econ.h"
#include "stub/tfweaponbase.h"
#include "mod/pop/common.h"
#include "util/iterate.h"
#include "util/lua.h"
#include "util/admin.h"
#include "util/clientmsg.h"
#include "mod/etc/mapentity_additions.h"
#include "mod.h"

namespace Util::Lua
{
    LuaState *cur_state = nullptr;

    int vector_meta = LUA_NOREF;
    const void *vector_meta_ptr = nullptr;
    
    int entity_meta = LUA_NOREF;
    const void *entity_meta_ptr = nullptr;

    int prop_meta = LUA_NOREF;
    int entity_table_store = LUA_NOREF;

    static void stackDump (lua_State *L) {
      int i;
      int top = lua_gettop(L);
      for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        printf("%d: ", i);
        switch (t) {
    
          case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, i));
            break;
    
          case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;
    
          case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, i));
            break;
    
          default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;
    
        }
        printf("\n");  /* put a separator */
      }
      printf("\n");  /* end the listing */
    }

    int LPrint(lua_State *l)
    {
        int args = lua_gettop(l);
        std::string str = "";
        for (int i = 1; i <= args; i++) {
            const char *msg = nullptr;
            auto type = lua_type(l, i);
            if (type == LUA_TSTRING || type == LUA_TNUMBER) {
                msg = lua_tostring(l, i);
            }
            else {
                lua_getglobal(l, "tostring");
                //lua_pushvalue(l, -1);
                lua_pushvalue(l,i);
                lua_pcall(l, 1, -2, 0);
                msg = lua_tostring(l, -1);
            }
            str += msg != nullptr ? msg : "";
            str += '\t';
        }
        SendConsoleMessageToAdmins("%s\n", str.c_str());
        Msg("%s\n", str.c_str());
        return 0;
    }

    inline QAngle *LAngleAlloc(lua_State *l)
    {
        QAngle *angles = (QAngle *)lua_newuserdata(l, sizeof(Vector));
        lua_rawgeti(l, LUA_REGISTRYINDEX, vector_meta);
        lua_setmetatable(l, -2);
        return angles;
    }

    inline Vector *LVectorAlloc(lua_State *l)
    {
        Vector *vec = (Vector *)lua_newuserdata(l, sizeof(Vector));
        lua_rawgeti(l, LUA_REGISTRYINDEX, vector_meta);
        lua_setmetatable(l, -2);
        return vec;
    }

    int LVectorNew(lua_State *l)
    {
        if (lua_type(l, 1) == LUA_TSTRING) {
            auto vec = LVectorAlloc(l);
            UTIL_StringToVectorAlt(*vec, lua_tostring(l, 1));
        }
        else {
            float x = luaL_checknumber(l, 1);
            float y = luaL_checknumber(l, 2);
            float z = luaL_checknumber(l, 3);

            LVectorAlloc(l)->Init(x, y, z);
        }
        return 1;
    }

    // Faster version of userdata check that compares metadata pointers
    inline void *LGetUserdata(lua_State *l, int index, const void *metatablePtr)
    {
        if (lua_getmetatable(l, index)) {
            auto ourMetatablePtr = lua_topointer(l, -1);
            lua_pop(l,1);
            if (metatablePtr == ourMetatablePtr) {
                void *ud = lua_touserdata(l, index);
                if (ud != nullptr) { 
                    return ud;
                }
            }
        }
        return nullptr;
    }

    // Faster version of userdata check that compares metadata pointers
    inline void *LCheckUserdata(lua_State *l, int index, const void *metatablePtr, const char *name)
    {
        auto ud = LGetUserdata(l, index, metatablePtr);
        if (ud == nullptr) {
            luaL_error(l, "Invalid argument %d: expected '%s'", index, name);
        }
        return ud;
    }

    inline Vector *LVectorGetCheck(lua_State *l, int index)
    {
        return (Vector *)LCheckUserdata(l, index, vector_meta_ptr, "vector");
    }

    inline Vector *LVectorGetNoCheck(lua_State *l, int index)
    {
        return (Vector *)LGetUserdata(l, index, vector_meta_ptr);
    }

    inline QAngle *LAngleGetCheck(lua_State *l, int index)
    {
        return (QAngle *)LCheckUserdata(l, index, vector_meta_ptr, "vector");
    }

    inline QAngle *LAngleGetNoCheck(lua_State *l, int index)
    {
        return (QAngle *)LGetUserdata(l, index, vector_meta_ptr);
    }

    int LVectorGet(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        int index = luaL_checkinteger(l, 2);

        luaL_argcheck(l, index >= 1 && index <= 3, 2, "index out of 1-3 range");

        lua_pushnumber(l, (*vec)[index - 1]);
        return 1;
    }

    int LVectorGetX(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        lua_pushnumber(l, vec->x);
        return 1;
    }

    int LVectorGetY(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        lua_pushnumber(l, vec->y);
        return 1;
    }

    int LVectorGetZ(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        lua_pushnumber(l, vec->z);
        return 1;
    }

    int LVectorSet(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        int index = luaL_checkinteger(l, 2);
        float value = luaL_checknumber(l, 3);

        luaL_argcheck(l, index >= 1 && index <= 3, 2, "index out of 1-3 range");

        (*vec)[index - 1] = value;
        return 0;
    }

    int LVectorSetX(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        float value = luaL_checknumber(l, 2);

        vec->x = value;
        return 0;
    }

    int LVectorSetY(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        float value = luaL_checknumber(l, 2);

        vec->x = value;
        return 0;
    }

    int LVectorSetZ(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        float value = luaL_checknumber(l, 2);

        vec->x = value;
        return 0;
    }

    int LVectorIndex(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        int index = -1;

        // Read index from offset
        if (lua_isnumber(l, 2)) {
            index = luaL_checkinteger(l, 2);
            luaL_argcheck(l, index >= 1 && index <= 3, 2, "index out of 1-3 range");
        }
        // Read index from x/y/z
        else {
            const char *str = luaL_checkstring(l, 2);
            if (str != nullptr && str[0] != '\0' && str[1] == '\0') {
                switch(str[0]) {
                    case 'x': index = 1; break;
                    case 'y': index = 2; break;
                    case 'z': index = 3; break;
                }
            }
        }
        if (index != -1) {
            lua_pushnumber(l, (*vec)[index - 1]);
        }
        // Get function
        else {
            lua_getmetatable(l, 1);
            lua_pushvalue(l, 2);
            lua_rawget(l, 3);
        }
        return 1;
    }

    int LVectorSetIndex(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        int index = -1;
        if (lua_isnumber(l, 2)) {
            index = luaL_checkinteger(l, 2);
            luaL_argcheck(l, index >= 1 && index <= 3, 2, "index out of 1-3 range");
        }
        else {
            const char *str = luaL_checkstring(l, 2);
            if (str != nullptr && str[0] != '\0' && str[1] == '\0') {
                switch(str[0]) {
                    case 'x': index = 1; break;
                    case 'y': index = 2; break;
                    case 'z': index = 3; break;
                }
            }
        }
        if (index != -1) {
            float value = luaL_checknumber(l, 3);

            (*vec)[index - 1] = value;
        }
        return 0;
    }

    int LVectorLength(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);

        lua_pushnumber(l, vec->Length());
        return 1;
    }

    int LVectorEquals(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *vec2 = LVectorGetCheck(l, 2);

        lua_pushboolean(l, vec == vec2);
        return 1;
    }

    int LVectorDistance(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *vec2 = LVectorGetCheck(l, 2);

        lua_pushnumber(l, vec->DistTo(*vec2));
        return 1;
    }

    int LVectorDot(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *vec2 = LVectorGetCheck(l, 2);

        lua_pushnumber(l, vec->Dot(*vec2));
        return 1;
    }

    int LVectorCross(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *vec2 = LVectorGetCheck(l, 2);

        Vector *out = LVectorAlloc(l);
        CrossProduct(*vec, *vec2, *out);
        return 1;
    }

    int LVectorRotate(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        QAngle *angle = LAngleGetCheck(l, 2);

        Vector *out = LVectorAlloc(l);
        VectorRotate(*vec, *angle, *out);
        return 1;
    }

    int LVectorToAngles(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);

        QAngle *out = LAngleAlloc(l);
        VectorAngles(*vec, *out);
        return 1;
    }
    
    int LVectorNormalize(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);

        Vector *out = LVectorAlloc(l);
        *out = *vec;
        VectorNormalize(*out);
        return 1;
    }
    
    int LVectorGetForward(lua_State *l)
    {
        QAngle *angle = LAngleGetCheck(l, 1);

        Vector *out1 = LVectorAlloc(l);
        AngleVectors(*angle, out1);
        return 1;
    }
    
    int LVectorGetAngleVectors(lua_State *l)
    {
        QAngle *angle = LAngleGetCheck(l, 1);

        Vector *out1 = LVectorAlloc(l);
        Vector *out2 = LVectorAlloc(l);
        Vector *out3 = LVectorAlloc(l);
        AngleVectors(*angle, out1, out2, out3);
        return 3;
    }
    
    int LVectorAdd(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *out = LVectorAlloc(l);
        if (lua_isnumber(l, 2)) {
            float num = lua_tonumber(l, 2);
            out->Init(vec->x+num,vec->y+num,vec->z+num);
        }
        else {
            Vector *vec2 = LVectorGetCheck(l, 2);
            *out = *vec + *vec2;
        }
        return 1;
    }

    int LVectorSubtract(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *out = LVectorAlloc(l);
        if (lua_isnumber(l, 2)) {
            float num = lua_tonumber(l, 2);
            out->Init(vec->x-num,vec->y-num,vec->z-num);
        }
        else {
            Vector *vec2 = LVectorGetCheck(l, 2);
            *out = *vec - *vec2;
        }
        return 1;
    }

    int LVectorMultiply(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *out = LVectorAlloc(l);
        if (lua_isnumber(l, 2)) {
            float num = lua_tonumber(l, 2);
            out->Init(vec->x*num,vec->y*num,vec->z*num);
        }
        else {
            Vector *vec2 = LVectorGetCheck(l, 2);
            *out = *vec * *vec2;
        }
        return 1;
    }

    int LVectorDivide(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *out = LVectorAlloc(l);
        if (lua_isnumber(l, 2)) {
            float num = lua_tonumber(l, 2);
            out->Init(vec->x/num,vec->y/num,vec->z/num);
        }
        else {
            Vector *vec2 = LVectorGetCheck(l, 2);
            *out = *vec / *vec2;
        }
        return 1;
    }

    int LVectorCopy(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        Vector *vec2 = LVectorGetCheck(l, 2);
        *vec = *vec2;
        return 0;
    }

    int LVectorCopyUnpacked(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);
        float x = luaL_checknumber(l, 2);
        float y = luaL_checknumber(l, 3);
        float z = luaL_checknumber(l, 4);

        LVectorAlloc(l)->Init(x, y, z);
        return 0;
    }

    int LVectorToString(lua_State *l)
    {
        Vector *vec = LVectorGetCheck(l, 1);

        lua_pushfstring(l, "%f %f %f", vec->x, vec->y, vec->z);
        return 1;
    }

    inline const EHANDLE *LEntityAlloc(lua_State *l, const EHANDLE oldhandle)
    {
        EHANDLE *handle = (EHANDLE *)lua_newuserdata(l, sizeof(EHANDLE));
        *handle = oldhandle;
        //luaL_getmetatable(l, "entity");
        lua_rawgeti(l, LUA_REGISTRYINDEX, entity_meta);
        lua_setmetatable(l, -2);
        return handle;
    }

    const EHANDLE invalid_handle;
    //!! Returns invalid handle ref if entity is null
    const EHANDLE *LEntityAlloc(lua_State *l, CBaseEntity *entity)
    {
        if (entity == nullptr) {
            lua_pushnil(l);
            return &invalid_handle;
        }
        EHANDLE *handle = (EHANDLE *)lua_newuserdata(l, sizeof(EHANDLE));
        handle->Set(entity);
        //luaL_getmetatable(l, "entity");
        lua_rawgeti(l, LUA_REGISTRYINDEX, entity_meta);
        lua_setmetatable(l, -2);
        return handle;
    }

    inline CBaseEntity *LEntityFindByIndex(int index)
    {
        if (index > 2048)
            return EHANDLE::FromIndex(index);
        else
            return UTIL_EntityByIndex(index);
    }

    int LEntityNew(lua_State *l)
    {
        if (lua_type(l, 1) == LUA_TNUMBER) {
            auto entity = LEntityFindByIndex(lua_tointeger(l, 1));
            auto handle = LEntityAlloc(l,entity);
        }
        else if (lua_isstring(l, 1)) {
            auto entity = CreateEntityByName(lua_tostring(l, 1));
            auto handle = LEntityAlloc(l,entity);
            if (entity != nullptr && (lua_gettop(l) < 2 || lua_toboolean(l,2)) ) {
                DispatchSpawn(entity);
            }
            if (entity != nullptr && (lua_gettop(l) < 3 || lua_toboolean(l,3)) ) {
                entity->Activate();
            }
        }
        else {
            luaL_error(l, "Expected classname string or entity index");
        }
        return 1;
    }    

    // Returns entity handle, throws an error if variable is not nil or entity
    inline const EHANDLE *LEntityGetCheck(lua_State *l, int index)
    {
        if (lua_type(l, index) == LUA_TNIL ) return &invalid_handle;

        return (const EHANDLE *)LCheckUserdata(l, index, entity_meta_ptr, "entity");
    }

    // Returns entity, or null if the variable is not a valid entity
    inline CBaseEntity *LEntityGetOptional(lua_State *l, int index)
    {
        auto *handle = (const EHANDLE *)LGetUserdata(l, index, entity_meta_ptr);

        return handle != nullptr ? handle->Get() : nullptr;
    }

    // Returns entity and throws an error if variable is not an entity or the entity is invalid
    inline CBaseEntity *LEntityGetNonNull(lua_State *l, int index)
    {
        auto ud = (const EHANDLE *)LCheckUserdata(l, index, entity_meta_ptr, "entity");
        auto entity = ud->Get();
        luaL_argcheck(l, entity != nullptr, index, "entity handle is invalid");
        return entity;
    }

    // Returns player entity and throws an error if variable is not an entity, the entity is invalid or not a player
    CTFPlayer *LPlayerGetNonNull(lua_State *l, int index)
    {
        auto ud = (const EHANDLE *)LCheckUserdata(l, index, entity_meta_ptr, "entity");
        auto entity = ud->Get();
        luaL_argcheck(l, entity != nullptr, index, "entity handle is invalid");
        auto player = ToTFPlayer(entity);
        luaL_argcheck(l, player != nullptr, index, "entity is not a player");
        return player;
    }


    int LEntityEquals(lua_State *l)
    {
        auto *handlel = LEntityGetCheck(l, 1);
        auto *handler = LEntityGetCheck(l, 2);

        lua_pushboolean(l, *handlel == *handler || handlel->Get() == handler->Get());
        return 1;
    }

    int LIsValid(lua_State *l)
    {
        lua_pushboolean(l, LEntityGetOptional(l, 1) != nullptr);
        return 1;
    }

    int LEntityIsValid(lua_State *l)
    {
        auto *handle = LEntityGetCheck(l, 1);

        lua_pushboolean(l, handle->Get() != nullptr);
        return 1;
    }

    int LEntityGetHandleIndex(lua_State *l)
    {
        auto *handle = LEntityGetCheck(l, 1);

        lua_pushinteger(l, handle->ToInt());
        return 1;
    }

    int LEntityGetNetworkIndex(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
    
        lua_pushinteger(l, entity->edict() != nullptr ? entity->entindex() : -1);
        return 1;
    }

    int LEntitySpawn(lua_State *l)
    {
        servertools->DispatchSpawn(LEntityGetNonNull(l, 1));
        return 0;
    }

    int LEntityActivate(lua_State *l)
    {
        LEntityGetNonNull(l, 1)->Activate();
        return 0;
    }

    int LEntityRemove(lua_State *l)
    {
        LEntityGetNonNull(l, 1)->Remove();
        return 0;
    }

    int LEntityGetName(lua_State *l)
    {
        lua_pushstring(l, STRING(LEntityGetNonNull(l, 1)->GetEntityName()));
        return 1;
    }

    int LEntitySetName(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        char nameLower[1024];
        StrLowerCopy(name, nameLower);

        entity->SetName(AllocPooledString(nameLower));
        return 0;
    }

    int LEntityGetPlayerName(lua_State *l)
    {
        auto entity = LPlayerGetNonNull(l, 1);
        lua_pushstring(l, entity->GetPlayerName());
        return 1;
    }

    int LEntityIsAlive(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->IsAlive());
        return 1;
    }

    int LEntityIsPlayer(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->IsPlayer());
        return 1;
    }

    int LEntityIsNPC(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && !entity->IsPlayer() && entity->MyNextBotPointer() != nullptr);
        return 1;
    }

    int LEntityIsBot(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, ToTFBot(entity) != nullptr);
        return 1;
    }

    int LEntityIsRealPlayer(lua_State *l)
    {
        auto entity = ToTFPlayer(LEntityGetOptional(l, 1));
        lua_pushboolean(l, entity != nullptr && !entity->IsFakeClient() && !entity->IsHLTV());
        return 1;
    }

    int LEntityIsWeapon(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->IsBaseCombatWeapon());
        return 1;
    }

    int LEntityIsObject(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->IsBaseObject());
        return 1;
    }

    int LEntityIsCombatCharacter(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->MyCombatCharacterPointer() != nullptr);
        return 1;
    }

    int LEntityIsWearable(lua_State *l)
    {
        auto entity = LEntityGetOptional(l, 1);
        lua_pushboolean(l, entity != nullptr && entity->IsWearable());
        return 1;
    }

    int LEntityGetClassname(lua_State *l)
    {
        auto *handle = LEntityGetCheck(l, 1);
        auto entity = handle->Get();
        lua_pushstring(l, entity->GetClassname());
        return 1;
    }

    enum CallbackType
    {
        ON_REMOVE,
        ON_SPAWN,
        ON_ACTIVATE,
        ON_DAMAGE_RECEIVED_PRE,
        ON_DAMAGE_RECEIVED_POST,
        ON_INPUT,
        ON_OUTPUT,
        ON_KEY_PRESSED,
        ON_KEY_RELEASED,
        ON_DEATH,
        CALLBACK_TYPE_COUNT
    };

    class EntityCallback 
    {
    public:
        EntityCallback(LuaState *state, int func) : state(state), func(func) {};

        ~EntityCallback() {
            if (state != nullptr) {
                luaL_unref(state->GetState(), LUA_REGISTRYINDEX, func);
            }
        };
        LuaState *state;
        int func;
    };

    struct EntityTableStorageEntry
    {
        LuaState *state;
        int value;
    };

    class LuaEntityModule : public EntityModule
    {
    public:
        LuaEntityModule(CBaseEntity *entity) : EntityModule(entity), owner(entity) {};

        virtual ~LuaEntityModule() {
            if (owner != nullptr && owner->IsMarkedForDeletion()) {
                FireCallback(ON_REMOVE);
            }
            for (auto state : states) {
                state->EntityDeleted(owner);
            }
            //for (auto &entry : tableStore) {
            //    luaL_unref(entry.state->GetState(), LUA_REGISTRYINDEX, entry.value);
            //}
        }

        // LuaState removed, remove associated callbacks
        void LuaStateRemoved(LuaState *state) {
            for (int callbackType = 0; callbackType < CALLBACK_TYPE_COUNT; callbackType++) {
                RemoveIf(callbacks[callbackType], [&](auto &pair) {
                    if (pair.state == state) {

                        // Set to null to prevent destructor trying to unref function
                        pair.state = nullptr;
                        return true;
                    }
                    return false;
                });
            }
            states.erase(state);
            //std::remove_if(tableStore.begin(), tableStore.end(), [&](auto &entry) {
            //    return entry.state == state;
            //});
        }

        void AddCallback(CallbackType type, LuaState *state, int func) {
            if (type < 0 && type >= CALLBACK_TYPE_COUNT) return;

            callbacks[type].emplace_back(state, func);
            states.insert(state);
            state->EntityCallbackAdded(owner);
        }

        void RemoveCallback(CallbackType type, LuaState *state, int func) {
            RemoveFirstElement(callbacks[type], [&](auto &pair) {
                return pair.state == state && func == pair.func;
            });
        }

        // bool RemoveTableStorageEntry(LuaState *state, int value) {
        //     if (tableStore.empty()) return false;

        //     if (RemoveFirstElement(tableStore, [&](auto &entry){
        //         return entry.state == state && entry.value == value;
        //     })) {
        //         luaL_unref(state->GetState(), LUA_REGISTRYINDEX, value);
        //     }

        //     return false;
        // }

        // void SetTableStorageEntry(LuaState *state, int value) {
        //     for(auto &entry : tableStore) {
        //         if (entry.state == state && entry.value == value) {
        //             return;
        //         }
        //     }
        //     tableStore.push_back({state, value});
        // }
        
        void FireCallback(CallbackType type, std::function<void(LuaState *)> *extrafunc = nullptr, int extraargs = 0, std::function<void(LuaState *)> *extraretfunc = nullptr, int ret = 0) {
            for (auto &pair : callbacks[type]) {
                auto l = pair.state->GetState();
                lua_rawgeti(l, LUA_REGISTRYINDEX, pair.func);
                LEntityAlloc(l, owner);
                if (extrafunc != nullptr)
                    (*extrafunc)(pair.state);
                pair.state->Call(1 + extraargs, ret);
                if (extraretfunc != nullptr)
                    (*extraretfunc)(pair.state);

                lua_settop(l, 0);
            }
        }

        std::vector<EntityCallback> callbacks[CALLBACK_TYPE_COUNT];
        std::unordered_set<LuaState *> states;
        CBaseEntity *owner;
        //std::vector<EntityTableStorageEntry> tableStore;
    };

    int LEntityAddCallback(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        int type = luaL_checkinteger(l, 2);
        luaL_argcheck(l, type >= 0 && type < CALLBACK_TYPE_COUNT, 2, "type out of range");
        luaL_checktype(l, 3, LUA_TFUNCTION);
        int func = luaL_ref(l, LUA_REGISTRYINDEX);

        entity->GetOrCreateEntityModule<LuaEntityModule>("luaentity")->AddCallback((CallbackType)type, cur_state, func);
        lua_pushinteger(l, func);
        return 1;
    }

    int LEntityRemoveCallback(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        int type = luaL_checkinteger(l, 2);
        luaL_argcheck(l, type >= 0 && type < CALLBACK_TYPE_COUNT, 2, "type out of range");
        int func = luaL_checkinteger(l, 3);

        auto module = entity->GetEntityModule<LuaEntityModule>("luaentity");
        if (module != nullptr) {
            module->RemoveCallback((CallbackType)type, cur_state, func);
        }
        return 0;
    }

    void LFromVariant(lua_State *l, variant_t &variant)
    {
        auto fieldType = variant.FieldType();
        switch (fieldType) {
            case FIELD_VOID: lua_pushnil(l); return;
            case FIELD_BOOLEAN: lua_pushboolean(l,variant.Bool()); return;
            case FIELD_CHARACTER: case FIELD_SHORT: case FIELD_INTEGER: variant.Convert(FIELD_INTEGER); lua_pushinteger(l,variant.Int()); return;
            case FIELD_FLOAT: lua_pushnumber(l,variant.Float()); return;
            case FIELD_STRING: lua_pushstring(l,variant.String()); return;
            /*case FIELD_CUSTOM: {
                int ref;
                variant.SetOther(&ref);
                lua_rawgeti(l, LUA_REGISTRYINDEX, ref);
                return;
            }*/
            case FIELD_POSITION_VECTOR: case FIELD_VECTOR: {
                variant.Convert(FIELD_VECTOR);
                auto vec = LVectorAlloc(l);
                variant.Vector3D(*vec);
                return;
            }
            case FIELD_EHANDLE: case FIELD_CLASSPTR: variant.Convert(FIELD_EHANDLE); LEntityAlloc(l, variant.Entity()); return;
            default: lua_pushnil(l); return;
        }
        
    }

    void LToVariant(lua_State *l, int index, variant_t &variant)
    {
        int type = lua_type(l, index);
        if (type == LUA_TNIL) {

        }
        else if (type == LUA_TBOOLEAN) {
            variant.SetBool(lua_toboolean(l, index));
        }
        else if (type == LUA_TNUMBER) {
            double number = lua_tonumber(l, index);
            int inumber = (int) number;
            if (number == inumber) {
                variant.SetInt(lua_tointeger(l, index));
            }
            else {
                variant.SetFloat(lua_tointeger(l, index));
            }
        }
        else if (type != LUA_TUSERDATA && type != LUA_TLIGHTUSERDATA) {
            variant.SetString(AllocPooledString(lua_tostring(l,index)));
        }
        else if (luaL_testudata(l, index, "vector")) {
            variant.SetVector3D(*(Vector*)lua_touserdata(l, index));
        }
        else if (luaL_testudata(l, index, "entity")) {
            variant.SetEntity(*(EHANDLE*)lua_touserdata(l, index));
        }
        /*else if (allowTables) {
            lua_pushvalue(l, index);
            int ref = luaL_ref(l, LUA_REGISTRYINDEX);
            variant.Set(FIELD_CUSTOM, &ref);
        }*/
    }

    int LEntityAcceptInput(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        const char *name = luaL_checkstring(l, 2);
        variant_t variant;
        if (lua_gettop(l) > 2)
            LToVariant(l, 3, variant);

        CBaseEntity *activator = lua_gettop(l) > 3 ? LEntityGetCheck(l, 4)->Get() : nullptr;
        CBaseEntity *caller = lua_gettop(l) > 4 ? LEntityGetCheck(l, 5)->Get() : nullptr;
        
        lua_pushboolean(l, entity->AcceptInput(name, activator, caller, variant, -1));
        return 1;
    }

    int LEntityGetPlayerItemBySlot(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        int slot = luaL_checkinteger(l, 2);
        if (player != nullptr) {
            LEntityAlloc(l, GetEconEntityAtLoadoutSlot(player, slot));
            return 1;
        }
        lua_pushnil(l);
        return 1;
    }

    int LEntityGetPlayerItemByName(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        const char *name = luaL_checkstring(l, 2);
        if (player != nullptr) {
            CBaseEntity *ret = nullptr;
            ForEachTFPlayerEconEntity(player, [&](CEconEntity *entity){
                if (entity->GetItem() != nullptr && FStrEq(GetItemName(entity->GetItem()), name)) {
                    ret = entity;
                    return false;
                }
                return true;
            });
            LEntityAlloc(l, ret);
            return 1;
        }
        lua_pushnil(l);
        return 1;
    }

    int LEntityGetItemAttribute(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);

        if (entity == nullptr) {
            luaL_error(l, "Entity in not valid");
            lua_pushnil(l);
            return 1;
        }

        CAttributeList *list = nullptr;
        if (entity->IsPlayer()) {
            list = ToTFPlayer(entity)->GetAttributeList();
        } 
        else {
            CEconEntity *econEntity = rtti_cast<CEconEntity *>(entity);
            if (econEntity != nullptr) {
                list = &econEntity->GetItem()->GetAttributeList();
            }
        }
        if (list == nullptr) {
            luaL_error(l, "Entity (%s) is not a player or item", entity->GetClassname());
            lua_pushnil(l);
            return 1;
        }
        CEconItemAttribute * attr = list->GetAttributeByName(name);
        if (attr != nullptr) {
            if (!attr->GetStaticData()->IsType<CSchemaAttributeType_String>()) {
                lua_pushnumber(l,attr->GetValuePtr()->m_Float);
            }
            else {
                char buf[256];
                attr->GetStaticData()->ConvertValueToString(*attr->GetValuePtr(), buf, sizeof(buf));
                lua_pushstring(l,buf);
            }
        }
        else {
            lua_pushnil(l);
        }
        return 1;
    }

    int LEntitySetItemAttribute(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        auto value = lua_tostring(l, 3);

        CAttributeList *list = nullptr;
        if (entity->IsPlayer()) {
            list = ToTFPlayer(entity)->GetAttributeList();
        } 
        else {
            CEconEntity *econEntity = rtti_cast<CEconEntity *>(entity);
            if (econEntity != nullptr) {
                list = &econEntity->GetItem()->GetAttributeList();
            }
        }
        if (list == nullptr) {
            luaL_error(l, "Entity (%s) is not a player or item", entity->GetClassname());
            return 0;
        }
        auto def = GetItemSchema()->GetAttributeDefinitionByName(name);
        if (def != nullptr) {
            if (value == nullptr) {
                list->RemoveAttribute(def);
            }
            else {
                list->AddStringAttribute(def, value);
            }
        }
        else {
            luaL_error(l, "Invalid attribute name: %s", name);
        }
        return 0;
    }

    int LEntityToString(lua_State *l)
    {
        auto *handle = LEntityGetCheck(l, 1);
        auto entity = handle->Get();

        if (entity != nullptr) {
            lua_pushfstring(l, "Entity handle id: %d , classname: %s , targetname: %s , net id: %d", handle->ToInt(), entity->GetClassname(), STRING(entity->GetEntityName()), entity->entindex());
        }
        else {
            lua_pushfstring(l, "Invalid entity handle id: %d", handle->ToInt());
        }
        
        return 1;
    }

    void FindSendProp(int& off, SendTable *s_table, std::function<void(SendProp *, int)> onfound)
    {
        for (int i = 0; i < s_table->GetNumProps(); ++i) {
            SendProp *s_prop = s_table->GetProp(i);
            //Msg("Prop %d %s %d %d %d\n", s_prop, s_prop->GetName(), s_prop->GetDataTable(), s_prop->GetDataTable() != nullptr && s_prop->GetDataTable()->GetNumProps() > 1, s_prop->GetDataTable() != nullptr && s_prop->GetDataTable()->GetNumProps() > 1 && strcmp(s_prop->GetDataTable()->GetProp(0)->GetName(), "000") == 0);
            if (s_prop->IsInsideArray())
                continue;

            if (s_prop->GetDataTable() == nullptr || (s_prop->GetDataTable() != nullptr && s_prop->GetDataTable()->GetNumProps() > 1 && strcmp(s_prop->GetDataTable()->GetProp(0)->GetName(), "000") == 0)) {
                onfound(s_prop, off + s_prop->GetOffset());
            }
            else if (s_prop->GetDataTable() != nullptr) {
                off += s_prop->GetOffset();
                FindSendProp(off, s_prop->GetDataTable(), onfound);
                off -= s_prop->GetOffset();
            }
        }
    }

    void *stringTSendProxy = nullptr;
    CStandardSendProxies* sendproxies = nullptr;
    int LEntityDumpProperties(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);

        int off = 0;
        
        lua_newtable(l);
        FindSendProp(off, entity->GetServerClass()->m_pTable, [&](SendProp *prop, int offset){
            Mod::Etc::Mapentity_Additions::PropCacheEntry entry;
            Mod::Etc::Mapentity_Additions::GetSendPropInfo(prop, entry, offset);
            if (entry.arraySize > 1) {
                lua_newtable(l);
            }
            for (int i = 0; i < entry.arraySize; i++) {
                variant_t variant;
                Mod::Etc::Mapentity_Additions::ReadProp(entity, entry, variant, i, -1);
                //Msg("Sendprop %s = %s %d %d\n", prop->GetName(), variant.String(), entry.offset, entry.arraySize);

                LFromVariant(l, variant);
                if (entry.arraySize == 1) {
                    lua_setfield(l, -2, prop->GetName());
                }
                else {
                    lua_rawseti(l, -2, i + 1);
                }
            }
            if (entry.arraySize > 1) {
                lua_setfield(l, -2, prop->GetName());
            }
        });

        for (datamap_t *dmap = entity->GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap) {
            // search through all the readable fields in the data description, looking for a match
            for (int i = 0; i < dmap->dataNumFields; i++) {
                if (dmap->dataDesc[i].fieldName != nullptr) {
                    Mod::Etc::Mapentity_Additions::PropCacheEntry entry;
                    Mod::Etc::Mapentity_Additions::GetDataMapInfo(dmap->dataDesc[i], entry);
                    if (entry.arraySize > 1) {
                        lua_newtable(l);
                    }
                    for (int j = 0; j < entry.arraySize; j++) {
                        variant_t variant;
                        Mod::Etc::Mapentity_Additions::ReadProp(entity, entry, variant, j, -1);
                        //Msg("Datamap %s = %s %d\n", dmap->dataDesc[i].fieldName, variant.String(), entry.offset);

                        LFromVariant(l, variant);
                        if (entry.arraySize == 1) {
                            lua_setfield(l, -2, dmap->dataDesc[i].fieldName);
                        }
                        else {
                            lua_rawseti(l, -2, j + 1);
                        }
                    }
                    if (entry.arraySize > 1) {
                        lua_setfield(l, -2, dmap->dataDesc[i].fieldName);
                    }
                }
            }
        }

        auto vars = GetCustomVariables(entity);
        for (auto var : vars) {
            LFromVariant(l, var.value);
            lua_setfield(l, -2, STRING(var.key));
        }

        return 1;
    }

    
    int LEntityDumpInputs(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        lua_newtable(l);
        int j = 0;
        for (datamap_t *dmap = entity->GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap) {
            // search through all the readable fields in the data description, looking for a match
            for (int i = 0; i < dmap->dataNumFields; i++) {
                if (dmap->dataDesc[i].externalName != nullptr && (dmap->dataDesc[i].flags & FTYPEDESC_INPUT)) {
                    lua_pushstring(l, dmap->dataDesc[i].externalName);
                    lua_rawseti(l, -2, ++j);
                }
            }
        }
        for (auto &filter : Mod::Etc::Mapentity_Additions::InputFilter::List()) {
            if (filter->Test(entity)) {
                for (auto &input : filter->inputs) {
                    lua_pushfstring(l, "$%d%s",input.name, input.prefix ? "<var>" : "");
                    lua_rawseti(l, -2, ++j);
                    //if ((!input.prefix && CompareCaseInsensitiveStringView(inputNameLower, input.name)) ||
                    //    (input.prefix && CompareCaseInsensitiveStringViewBeginsWith(inputNameLower, input.name))) {
                    //    return &input.func;
                    //}
                }
            }
        }
        return 1;
    }
    struct InputCacheEntry {
        inputfunc_t inputptr = nullptr;
        Mod::Etc::Mapentity_Additions::CustomInputFunction *custominputptr = nullptr;
    };

    /*bool CheckInput(CBaseEntity *entity) {

        auto datamap = entity->GetDataDescMap();
        for (datamap_t *dmap = datamap; dmap != NULL; dmap = dmap->baseMap) {
            // search through all the readable fields in the data description, looking for a match
            for (int i = 0; i < dmap->dataNumFields; i++) {
                if (dmap->dataDesc[i].fieldName != nullptr && strcmp(dmap->dataDesc[i].fieldName, nameNoArray.c_str()) == 0) {
                    fieldtype_t fieldType = isVecAxis ? FIELD_FLOAT : dmap->dataDesc[i].fieldType;
                    int offset = dmap->dataDesc[i].fieldOffset[ TD_OFFSET_NORMAL ] + extraOffset;
                    
                    offset += clamp(arrayPos, 0, (int)dmap->dataDesc[i].fieldSize) * (dmap->dataDesc[i].fieldSizeInBytes / dmap->dataDesc[i].fieldSize);

                    datamap_cache.push_back({datamap, name, offset, fieldType, dmap->dataDesc[i].fieldSize});
                    return datamap_cache.back();
                }
            }
        }
    }*/
    std::vector<datamap_t *> input_cache_classes;
    std::vector<std::pair<std::vector<std::string>, std::vector<InputCacheEntry>>> input_cache;

    struct ArrayProp
    {
        CBaseEntity *entity;
        Mod::Etc::Mapentity_Additions::PropCacheEntry entry;
    };

    ArrayProp *LPropAlloc(lua_State *l, CBaseEntity *entity, Mod::Etc::Mapentity_Additions::PropCacheEntry &entry)
    {
        ArrayProp *prop = (ArrayProp *)lua_newuserdata(l, sizeof(ArrayProp));
        prop->entity = entity;
        prop->entry = entry;
        //luaL_getmetatable(l, "entity");
        lua_rawgeti(l, LUA_REGISTRYINDEX, prop_meta);
        lua_setmetatable(l, -2);
        return prop;
    }

    int LPropGet(lua_State *l)
    {
        auto prop = (ArrayProp *) lua_touserdata(l, 1);
        int index = lua_tointeger(l, 2);
        luaL_argcheck(l, prop != nullptr, 1, "Userdata is null");
        luaL_argcheck(l, index > 0 && index <= prop->entry.arraySize, 2, "index out of range");

        variant_t variant;
        Mod::Etc::Mapentity_Additions::ReadProp(prop->entity, prop->entry, variant, index, -1);
        LFromVariant(l, variant);
        return 1;
    }

    int LPropGetN(lua_State *l)
    {
        auto prop = (ArrayProp *) lua_touserdata(l, 1);
        luaL_argcheck(l, prop != nullptr, 1, "Userdata is null");

        lua_pushinteger(l, prop->entry.arraySize);
        return 1;
    }

    int LPropSet(lua_State *l)
    {
        auto prop = (ArrayProp *) lua_touserdata(l, 1);
        int index = lua_tointeger(l, 2);
        luaL_argcheck(l, prop != nullptr, 1, "Userdata is null");
        luaL_argcheck(l, index > 0 && index <= prop->entry.arraySize, 2, "index out of range");

        variant_t variant;
        LToVariant(l, 3, variant);
        Mod::Etc::Mapentity_Additions::WriteProp(prop->entity, prop->entry, variant, index, -1);
        return 0;
    }

    inline InputCacheEntry &LGetInputCacheEntry(CBaseEntity *entity, std::string &name)
    {
        auto datamap = entity->GetDataDescMap();
        size_t classIndex = 0;
        for (; classIndex < input_cache_classes.size(); classIndex++) {
            if (input_cache_classes[classIndex] == datamap) {
                break;
            }
        }
        if (classIndex >= input_cache_classes.size()) {
            input_cache_classes.push_back(datamap);
            input_cache.emplace_back();
        }
        auto &pair = input_cache[classIndex];
        auto &names = pair.first;

        size_t nameCount = names.size();
        for (size_t i = 0; i < nameCount; i++ ) {
            if (names[i] == name) {
                return pair.second[i];
            }
        }
        for (datamap_t *dmap = datamap; dmap != NULL; dmap = dmap->baseMap) {
            // search through all the readable fields in the data description, looking for a match
            for (int i = 0; i < dmap->dataNumFields; i++) {
                if (dmap->dataDesc[i].fieldName != nullptr && (dmap->dataDesc[i].flags & FTYPEDESC_INPUT) && (strcmp(dmap->dataDesc[i].externalName, name.c_str()) == 0)) {
                    names.push_back(name);
                    pair.second.push_back({dmap->dataDesc[i].inputFunc, nullptr});
                    return pair.second.back();
                }
            }
        }
        auto func = Mod::Etc::Mapentity_Additions::GetCustomInput(entity, name.c_str());
        if (func != nullptr) {
            names.push_back(name);
            pair.second.push_back({nullptr, func});
            return pair.second.back();
        }
        
        names.push_back(name);
        pair.second.push_back({nullptr, nullptr});
        return pair.second.back();
    }

    int LEntityCallInputNormal(lua_State *l)
    {
        auto func = MakePtrToMemberFunc<CBaseEntity, void, inputdata_t &>((void *)(uintptr_t) lua_tointeger(l, lua_upvalueindex(1)));
        auto entity = LEntityGetNonNull(l, 1);
        inputdata_t data;
        LToVariant(l, 2, data.value);
        data.pActivator = LEntityGetOptional(l, 3);
        data.pCaller = LEntityGetOptional(l, 4);
        data.nOutputID = -1;
        (entity->*func)(data);

        return 0;
    }

    int LEntityCallInputCustom(lua_State *l)
    {
        auto func = (Mod::Etc::Mapentity_Additions::CustomInputFunction *)(uintptr_t) lua_tointeger(l, lua_upvalueindex(1));
        auto entity = LEntityGetNonNull(l, 1);
        variant_t value;
        LToVariant(l, 2, value);
        (*func)(entity, lua_tostring(l, lua_upvalueindex(2)), LEntityGetOptional(l,3), LEntityGetOptional(l,4), value);
        return 0;
    }

    int LEntityGetProp(lua_State *l)
    {
        // Get function
        lua_getmetatable(l, 1);
        lua_pushvalue(l, 2);
        lua_rawget(l, 3);
        if (!lua_isnil(l,4)) return 1;

        auto entity = LEntityGetNonNull(l, 1);
        
        // Table access first
        lua_rawgeti(l, LUA_REGISTRYINDEX, entity_table_store);
        if (lua_rawgetp(l, -1, entity) != LUA_TNIL) {
            lua_pushvalue(l, 2);
            if (lua_rawget(l, -2) != LUA_TNIL) {
                return 1;
            }
        }

        const char *varName = luaL_checkstring(l, 2);
        
        if (varName == nullptr) {
            lua_pushnil(l);
            return 1;
        }

        std::string varNameStr(varName);
        
        variant_t variant;
        
        if (entity->GetCustomVariableByText(varName, variant)) {
            LFromVariant(l, variant);
            return 1;
        }
        auto *entry = &Mod::Etc::Mapentity_Additions::GetDataMapOffset(entity->GetDataDescMap(), varNameStr);
        if (entry == nullptr && entry->offset <= 0) {
            entry = &Mod::Etc::Mapentity_Additions::GetSendPropOffset(entity->GetServerClass(), varNameStr);
        }
        if (entry != nullptr && entry->offset > 0) {
            if (entry->arraySize > 1) {
                LPropAlloc(l, entity, *entry);
            }
            else{
                Mod::Etc::Mapentity_Additions::ReadProp(entity, *entry, variant, 0, -1);
                LFromVariant(l, variant);
            }
            return 1;
        }

        auto &input = LGetInputCacheEntry(entity, varNameStr);
        if (input.inputptr != nullptr) {
            lua_pushinteger(l, (uintptr_t) GetAddrOfMemberFunc(input.inputptr));
            lua_pushcclosure(l, LEntityCallInputNormal,1);
            return 1;
        } 
        else if (input.custominputptr != nullptr) {
            lua_pushinteger(l, (uintptr_t) input.custominputptr);
            lua_pushstring(l, varNameStr.c_str());
            lua_pushcclosure(l, LEntityCallInputCustom,2);
            return 1;
        } 
        lua_pushnil(l);
        return 1;
    }

    int LEntitySetProp(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);

        // Tables must be saved in special way
        if (lua_type(l, 3) == LUA_TTABLE) {
            lua_rawgeti(l, LUA_REGISTRYINDEX, entity_table_store);
            if (lua_rawgetp(l, -1, entity) == LUA_TNIL) {
                auto mod = entity->GetOrCreateEntityModule<LuaEntityModule>("luaentity");
                lua_newtable(l);
                lua_pushvalue(l,2);
                lua_pushvalue(l,3);
                lua_rawset(l, -3);

                lua_rawsetp(l, -3, entity);
                return 0;
            }
            lua_pushvalue(l,2);
            lua_pushvalue(l,3);
            lua_rawset(l, -3);
            return 0;
        }
        else {
            lua_rawgeti(l, LUA_REGISTRYINDEX, entity_table_store);
            if (lua_rawgetp(l, -1, entity) != LUA_TNIL) {
                lua_pushvalue(l,2);
                lua_pushnil(l);
                lua_rawset(l, -3);
            }
        }

        const char *varName = luaL_checkstring(l, 2);

        if (varName == nullptr) {
            lua_pushnil(l);
            return 1;
        }
       
        std::string varNameStr(varName);
        /*auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");
        if (mod != nullptr) {
            variant_t variantPre;
            entity->GetCustomVariableByText(varName, variantPre);
            if (variantPre.FieldType() == FIELD_CUSTOM) {
                int ref;
                variantPre.SetOther(&ref);

            }
        }*/

        variant_t variant;
        LToVariant(l, 3, variant);
        SetEntityVariable(entity, Mod::Etc::Mapentity_Additions::ANY, varNameStr, variant, 0, -1);
        
        /*if (variant.FieldType() == FIELD_CUSTOM) {
            if (mod == nullptr) {
                entity->AddEntityModule("luaentity", mod = new LuaEntityModule(entity));
            }
            int ref;
            variant.SetOther(&ref);
            mod->SetTableStorageEntry(cur_state, ref);
        }*/

        return 0;
    }

    struct EntityCreateCallback
    {   
        LuaState *state;
        string_t classname;
        bool wildcard = false;
        int func;
    };
    std::vector<EntityCreateCallback> entity_create_callbacks;

    int LEntityAddCreateCallback(lua_State *l)
    {
        const char *name = luaL_checkstring(l, 1);
        luaL_argcheck(l, strlen(name) > 0, 1, "entity classname empty");
        luaL_checktype(l,2, LUA_TFUNCTION);
        int func = luaL_ref(l, LUA_REGISTRYINDEX);
        
        entity_create_callbacks.push_back({cur_state, AllocPooledString(name), name[strlen(name) - 1] == '*', func});
        lua_pushinteger(l, func);
        return 1;
    }

    int LEntityRemoveCreateCallback(lua_State *l)
    {
        int func = luaL_checkinteger(l, 1);
        RemoveIf(entity_create_callbacks, [&](auto &callback){ 
            if (callback.state == cur_state && callback.func == func) {
                luaL_unref(l, LUA_REGISTRYINDEX, func);
                return true ;
            }
            return false ;
        });

        return 0;
    }

    void DamageInfoToTable(lua_State *l, CTakeDamageInfo &info) {
        lua_newtable(l);
        LEntityAlloc(l, info.GetAttacker());
        lua_setfield(l, -2, "Attacker");
        LEntityAlloc(l, info.GetInflictor());
        lua_setfield(l, -2, "Inflictor");
        LEntityAlloc(l, info.GetWeapon());
        lua_setfield(l, -2, "Weapon");
        lua_pushnumber(l, info.GetDamage());
        lua_setfield(l, -2, "Damage");
        lua_pushinteger(l, info.GetDamageType());
        lua_setfield(l, -2, "DamageType");
        lua_pushinteger(l, info.GetDamageCustom());
        lua_setfield(l, -2, "DamageCustom");
        lua_pushinteger(l, info.GetCritType());
        lua_setfield(l, -2, "CritType");
    };

    void TableToDamageInfo(lua_State *l, int idx, CTakeDamageInfo &info) {
        lua_pushvalue(l, idx);
        lua_getfield(l, -1, "Attacker");
        info.SetAttacker(LEntityGetOptional(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "Inflictor");
        info.SetInflictor(LEntityGetOptional(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "Weapon");
        info.SetWeapon(LEntityGetOptional(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "Damage");
        info.SetDamage(lua_tonumber(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "DamageType");
        info.SetDamageType(lua_tointeger(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "DamageCustom");
        info.SetDamageCustom(lua_tointeger(l, -1));
        lua_pop(l, 1);

        lua_getfield(l, -1, "CritType");
        info.SetCritType((CTakeDamageInfo::ECritType)lua_tointeger(l, -1));
        lua_pop(l, 1);
    };

    int LEntityTakeDamage(lua_State *l)
    {
        CTakeDamageInfo info;
        auto entity = LEntityGetNonNull(l, 1);
        luaL_checktype(l, 2, LUA_TTABLE);
        TableToDamageInfo(l, 2, info);
        lua_pushinteger(l, entity->TakeDamage(info));
        return 1;
    }

    int LEntityAddCond(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        int condition = luaL_checkinteger(l, 2);
        float duration = luaL_optnumber(l, 3, -1);
        auto provider = lua_gettop(l) > 3 ? ToTFPlayer(LEntityGetOptional(l, 4)) : nullptr;
        player->m_Shared->AddCond((ETFCond)condition, duration, provider);
        return 0;
    }

    int LEntityRemoveCond(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        int condition = luaL_checkinteger(l, 2);
        player->m_Shared->RemoveCond((ETFCond)condition);
        return 0;
    }

    int LEntityGetCondProvider(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        int condition = luaL_checkinteger(l, 2);
        LEntityAlloc(l, player->m_Shared->GetConditionProvider((ETFCond)condition));
        return 1;
    }

    int LEntityInCond(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        int condition = luaL_checkinteger(l, 2);
        lua_pushinteger(l, player->m_Shared->InCond((ETFCond)condition));
        return 1;
    }

    int LEntityStunPlayer(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        float duration = luaL_checknumber(l, 2);
        float amount = luaL_checknumber(l, 3);
        int flags = luaL_optnumber(l, 4, 1);
        auto stunner = lua_gettop(l) > 4 ? ToTFPlayer(LEntityGetOptional(l, 5)) : nullptr;
        player->m_Shared->StunPlayer(duration, amount, flags, stunner);
        return 0;
    }

    int LEntityGetAbsOrigin(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto *vec = LVectorAlloc(l);
        *vec = entity->GetAbsOrigin();
        return 1;
    }

    int LEntitySetAbsOrigin(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto vec = LVectorGetCheck(l, 2);
        entity->SetAbsOrigin(*vec);
        return 0;
    }

    int LEntityGetAbsAngles(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto *vec = LAngleAlloc(l);
        *vec = entity->GetAbsAngles();
        return 1;
    }

    int LEntitySetAbsAngles(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto vec = LAngleGetCheck(l, 2);
        entity->SetAbsAngles(*vec);
        return 0;
    }

    int LEntityTeleport(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto pos = LVectorGetNoCheck(l, 2);
        auto ang = LAngleGetNoCheck(l, 3);
        auto vel = LVectorGetNoCheck(l, 4);
        entity->Teleport(pos, ang, vel);
        return 0;
    }

    int LEntityCreateItem(lua_State *l)
    {
        auto player = LPlayerGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        auto entity = CreateItemByName(player, name);
        auto noRemove = lua_toboolean(l, 4);
        auto forceGive = lua_gettop(l) < 5 || lua_toboolean(l, 5);
        if (entity != nullptr && lua_gettop(l) > 2) {
            auto view = entity->GetItem();
            lua_pushnil(l);
            while (lua_next(l, 3) != 0) {
                if (lua_type(l, -2) == LUA_TSTRING) {
                    auto attrdef = GetItemSchema()->GetAttributeDefinitionByName(lua_tostring(l, -2));
                    if (attrdef != nullptr) {
                        view->GetAttributeList().AddStringAttribute(attrdef, lua_tostring(l, -1));
                    }
                }
                lua_pop(l, 1);
            }
        } 
        if (entity != nullptr && !GiveItemToPlayer(player, entity, noRemove, forceGive, name)) {
            entity->Remove();
            entity = nullptr;
        }
        if (entity != nullptr) {
            LEntityAlloc(l, entity);
        }
        else {
            lua_pushnil(l);
        }
        return 1;
    }
    
    int LEntityFireOutput(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        variant_t value;
        LToVariant(l, 3, value);
        auto activator = LEntityGetOptional(l, 4);
        auto delay = luaL_optnumber(l, 5, 0);
        entity->FireNamedOutput(name,value, activator, entity, delay);
        return 0;
    }

    int LEntitySetFakeProp(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        variant_t value;
        LToVariant(l, 3, value);
        
        auto mod = entity->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::FakePropModule>("fakeprop");
        mod->props[name] = {value, value};
        return 0;
    }

    int LEntityResetFakeProp(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        variant_t value;
        LToVariant(l, 3, value);
        
        auto mod = entity->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::FakePropModule>("fakeprop");
        mod->props.erase(name);
        return 0;
    }

    int LEntityGetFakeProp(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        auto name = luaL_checkstring(l, 2);
        
        auto mod = entity->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::FakePropModule>("fakeprop");
        LFromVariant(l, mod->props[name].first);
        return 1;
    }

    int LEntityAddEffects(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        entity->AddEffects(luaL_checknumber(l, 2));
        return 0;
    }

    int LEntityRemoveEffects(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        entity->RemoveEffects(luaL_checknumber(l, 2));
        return 0;
    }

    int LEntityIsEffectActive(lua_State *l)
    {
        auto entity = LEntityGetNonNull(l, 1);
        lua_pushboolean(l, entity->IsEffectActive(luaL_checknumber(l, 2)));
        
        return 1;
    }

    int LFindEntityByName(lua_State *l)
    {
        const char *name = luaL_checkstring(l, 1);
        auto prevEntity = lua_gettop(l) < 2 ? nullptr : LEntityGetCheck(l, 2)->Get();
        LEntityAlloc(l, servertools->FindEntityByName(prevEntity, name));
        return 1;
    }

    int LFindEntityByClassname(lua_State *l)
    {
        const char *name = luaL_checkstring(l, 1);
        auto prevEntity = lua_gettop(l) < 2 ? nullptr : LEntityGetCheck(l, 2)->Get();
        LEntityAlloc(l, servertools->FindEntityByClassname(prevEntity, name));
        return 1;
    }

    int LFindAllEntityByName(lua_State *l)
    {
        const char *name = luaL_checkstring(l, 1);
        lua_newtable(l);
        int idx = 1;
        for(CBaseEntity *entity = nullptr ; (entity = servertools->FindEntityByName(entity, name)) != nullptr; ) {
            LEntityAlloc(l, entity);
            lua_rawseti (l, -2, idx);
            idx++;
        }
        return 1;
    }

    int LFindAllEntityByClassname(lua_State *l)
    {
        const char *name = luaL_checkstring(l, 1);
        lua_newtable(l);
        int idx = 1;
        for(CBaseEntity *entity = nullptr ; (entity = servertools->FindEntityByClassname(entity, name)) != nullptr; ) {
            LEntityAlloc(l, entity);
            lua_rawseti (l, -2, idx);
            idx++;
        }
        return 1;
    }

    int LFindAllEntity(lua_State *l)
    {
        lua_createtable(l, gEntList->m_iNumEnts, 0);
        int idx = 1;
        for (const CEntInfo *pInfo = gEntList->FirstEntInfo(); pInfo != nullptr; pInfo = pInfo->m_pNext) {
            CBaseEntity *entity = (CBaseEntity *) pInfo->m_pEntity;
            if (entity != nullptr) {
                LEntityAlloc(l, entity);
                lua_rawseti (l, -2, idx);
                idx++;
            }
        }
        return 1;
    }

    class CLuaEnumerator : public IPartitionEnumerator
    {
    public:
        CLuaEnumerator(lua_State *l) : l(l) {};

        // This gets called	by the enumeration methods with each element
        // that passes the test.
        virtual IterationRetval_t EnumElement( IHandleEntity *pHandleEntity ) {
            CBaseEntity *pEntity = static_cast<IServerUnknown *>(pHandleEntity)->GetBaseEntity();
            if ( pEntity )
            {
                LEntityAlloc(l, pEntity);
                lua_rawseti (l, -2, idx);
                idx++;
            }
            return ITERATION_CONTINUE;
        }
        lua_State *l;
        int idx = 1;
    };

    int LFindAllEntityInBox(lua_State *l)
    {
        auto min = LVectorGetCheck(l, 1);
        auto max = LVectorGetCheck(l, 2);

        CLuaEnumerator iter = CLuaEnumerator(l);

        lua_newtable(l);
		partition->EnumerateElementsInBox(PARTITION_ENGINE_NON_STATIC_EDICTS, *min, *max, false, &iter);
        return 1;
    }

    int LFindAllEntityInSphere(lua_State *l)
    {
        auto center = LVectorGetCheck(l, 1);
        auto radius = luaL_checknumber(l, 2);

        CLuaEnumerator iter = CLuaEnumerator(l);

        lua_newtable(l);
		partition->EnumerateElementsInSphere(PARTITION_ENGINE_NON_STATIC_EDICTS, *center, radius, false, &iter);
        return 1;
    }

    int LEntityFirst(lua_State *l)
    {
        LEntityAlloc(l, servertools->FirstEntity());
        return 1;
    }

    int LEntityNext(lua_State *l)
    {
        LEntityAlloc(l, servertools->NextEntity(LEntityGetCheck(l,1)->Get()));
        return 1;
    }

    void LCreateTimer(lua_State *l)
    {

        float delay = luaL_checknumber(l, 1);
        
        luaL_checktype(l, 2, LUA_TFUNCTION);
        lua_pushvalue(l,2);
        int func = luaL_ref(l, LUA_REGISTRYINDEX);
        int repeats = lua_gettop(l) >= 3 ? luaL_checknumber(l,3) : 1;

        lua_pushvalue(l,4);
        int param = lua_gettop(l) >= 4 ? luaL_ref(l, LUA_REGISTRYINDEX) : 0;

        lua_pushnumber(l, cur_state->AddTimer(delay, repeats, func, param));
    }
    
    int LTimerSimple(lua_State *l)
    {
        float delay = luaL_checknumber(l, 1);
        luaL_checktype(l, 2, LUA_TFUNCTION);
        lua_pushvalue(l,2);
        int func = luaL_ref(l, LUA_REGISTRYINDEX);

        lua_pushnumber(l, cur_state->AddTimer(delay, 1, func, 0));
        return 1;
    }

    int LTimerCreate(lua_State *l)
    {
        LCreateTimer(l);
        return 1;
    }

    int LTimerStop(lua_State *l)
    {
        int id = luaL_checkinteger(l, 1);
        if (!cur_state->StopTimer(id)) {
            luaL_error(l, "Timer with id %d is not valid\n", id);
        }
        return 0;
    }

    class LuaTraceFilter : public CTraceFilterSimple
    {
    public:
        LuaTraceFilter(lua_State *l, IHandleEntity *entity, int collisionGroup) : CTraceFilterSimple( entity, collisionGroup)
        {
            if (lua_istable(l, -1)) {
                int len = lua_rawlen(l, -1);
                for(int i = 1; i <= len; i++) {
                    lua_rawgeti(l, -1, i);
                    auto entity = LEntityGetOptional(l, -1);

                    if (entity != nullptr) {
                        list.push_back(entity);
                    }
                    lua_pop(l, 1);
                }
            }
            this->hasFunction = lua_isfunction(l, -1);
        }
        virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask) {
            if (CTraceFilterSimple::ShouldHitEntity(pHandleEntity, contentsMask)) {
                if (!list.empty() && std::find(list.begin(), list.end(), pHandleEntity) != list.end()) {
                    return false;
                }
                if (hasFunction) {
                    LEntityAlloc(l, EntityFromEntityHandle(pHandleEntity));
                    lua_call(l, 1, 1);
                    bool hit = lua_toboolean(l, -1);
                    lua_pop(l, 1);
                    return hit;
                }
            }
            return false;
        }
        std::vector<IHandleEntity *> list;
        lua_State *l;
        bool hasFunction;
    };

    int LTraceLine(lua_State *l)
    {
        lua_getfield(l, 1, "start");
        auto startptrentity = LEntityGetOptional(l, -1);
        auto startptrvector = LVectorGetNoCheck(l, -1);
        Vector start(0,0,0);
        if (startptrentity != nullptr) {
            start = startptrentity->EyePosition();
        }
        else if (startptrvector != nullptr) {
            start = *startptrvector;
        }

        lua_getfield(l, 1, "endpos");
        auto endptr = LVectorGetNoCheck(l, -1);
        Vector end(0,0,0);
        if (endptr != nullptr) {
            end = *endptr;
        }
        else {
            QAngle angles(0,0,0);
            lua_getfield(l, 1, "distance");
            float distance = lua_isnil(l, -1) ? 8192 : lua_tonumber(l, -1);
            
            lua_getfield(l, 1, "angles");
            auto anglesptr = LAngleGetNoCheck(l, -1);
            if (anglesptr != nullptr) {
                angles = *anglesptr;
            }
            Vector fwd;
            AngleVectors(angles, &fwd);
            end = start + fwd * distance;
        }

        lua_getfield(l, 1, "mask");
        int mask = lua_isnil(l, -1) ? MASK_SOLID : lua_tointeger(l, -1);
        lua_getfield(l, 1, "collisiongroup");
        int collisiongroup = lua_isnil(l, -1) ? COLLISION_GROUP_NONE : lua_tointeger(l, -1);

        //lua_getfield(l, 1, "filter");
        //
        //

        trace_t trace;
        lua_getfield(l, 1, "mins");
        auto *minsptr = LVectorGetNoCheck(l, -1);
        lua_getfield(l, 1, "maxs");
        auto *maxsptr = LVectorGetNoCheck(l, -1);
        
        
        lua_getfield(l, 1, "filter");
        CBaseEntity *filterEntity = lua_isnil(l, -1) ? startptrentity : LEntityGetOptional(l, -1);
        LuaTraceFilter filter(l, filterEntity, collisiongroup);
        if (minsptr == nullptr || maxsptr == nullptr) {
	        UTIL_TraceLine(start, end, mask, &filter, &trace);
        }
        else {
	        UTIL_TraceHull(start, end, *minsptr, *maxsptr, mask, &filter, &trace);
        }
            

        lua_newtable(l);
        LEntityAlloc(l, trace.m_pEnt);
        lua_setfield(l, -2, "Entity");
        lua_pushnumber(l, trace.fraction);
        lua_setfield(l, -2, "Fraction");
        lua_pushnumber(l, trace.fractionleftsolid);
        lua_setfield(l, -2, "FractionLeftSolid");
        lua_pushboolean(l, trace.DidHit());
        lua_setfield(l, -2, "Hit");
        lua_pushinteger(l, trace.hitbox);
        lua_setfield(l, -2, "HitBox");
        lua_pushinteger(l, trace.hitgroup);
        lua_setfield(l, -2, "HitGroup");
        lua_pushboolean(l, trace.surface.flags & SURF_NODRAW);
        lua_setfield(l, -2, "HitNoDraw");
        lua_pushboolean(l, trace.DidHitNonWorldEntity());
        lua_setfield(l, -2, "HitNonWorld");
        auto normal = LVectorAlloc(l);
        *normal = trace.plane.normal;
        lua_setfield(l, -2, "HitNormal");
        auto hitpos = LVectorAlloc(l);
        *hitpos = trace.endpos;
        lua_setfield(l, -2, "HitPos");
        lua_pushboolean(l, trace.surface.flags & SURF_SKY);
        lua_setfield(l, -2, "HitSky");
        lua_pushstring(l, trace.surface.name);
        lua_setfield(l, -2, "HitTexture");
        lua_pushboolean(l, trace.DidHitWorld());
        lua_setfield(l, -2, "HitWorld");
        auto normal2 = LVectorAlloc(l);
        *normal2 = (trace.endpos - trace.startpos).Normalized();
        lua_setfield(l, -2, "Normal");
        auto start2 = LVectorAlloc(l);
        *start2 = trace.startpos;
        lua_setfield(l, -2, "StartPos");
        lua_pushboolean(l, trace.startsolid);
        lua_setfield(l, -2, "StartSolid");
        lua_pushboolean(l, trace.allsolid);
        lua_setfield(l, -2, "AllSolid");
        lua_pushinteger(l, trace.surface.flags);
        lua_setfield(l, -2, "SurfaceFlags");
        lua_pushinteger(l, trace.dispFlags);
        lua_setfield(l, -2, "DispFlags");
        lua_pushinteger(l, trace.contents);
        lua_setfield(l, -2, "Contents");
        lua_pushinteger(l, trace.surface.surfaceProps);
        lua_setfield(l, -2, "SurfaceProps");
        lua_pushinteger(l, trace.physicsbone);
        lua_setfield(l, -2, "PhysicsBone");
        return 1;
    }

    int LUtilPrintToConsoleAll(lua_State *l)
    {
        int args = lua_gettop(l);
        std::string str;
        for (int i = 1; i <= args; i++) {
            const char *msg = lua_tostring(l, i);
            if (msg != nullptr) {
                str += msg;
                str += '\t';
            }
        }
        ClientMsgAll("%s\n", str.c_str());
        return 0;
    }

    int LUtilPrintToConsole(lua_State *l)
    {
        auto player = ToTFPlayer(LEntityGetCheck(l, 1)->Get());
        if (player != nullptr) {
            luaL_error(l, "Entity is not a valid player");
        }
        int args = lua_gettop(l);
        std::string str;
        for (int i = 2; i <= args; i++) {
            const char *msg = lua_tostring(l, i);
            if (msg != nullptr) {
                str += msg;
                str += '\t';
            }
        }
        ClientMsg(player,"%s\n", str.c_str());
        return 0;
    }

    int LUtilPrintToChatAll(lua_State *l)
    {
        int args = lua_gettop(l);
        std::string str;
        for (int i = 1; i <= args; i++) {
            const char *msg = lua_tostring(l, i);
            if (msg != nullptr) {
                str += msg;
                str += '\t';
            }
        }
        PrintToChatAll(str.c_str());
        return 0;
    }

    int LUtilPrintToChat(lua_State *l)
    {
        auto player = ToTFPlayer(LEntityGetCheck(l, 1)->Get());
        if (player != nullptr) {
            luaL_error(l, "Entity is not a valid player");
        }
        int args = lua_gettop(l);
        std::string str;
        for (int i = 1; i <= args; i++) {
            const char *msg = lua_tostring(l, i);
            if (msg != nullptr) {
                str += msg;
                str += '\t';
            }
        }
        PrintToChat(str.c_str(), player);
        return 0;
    }

    int LCurTime(lua_State *l)
    {
        lua_pushnumber(l, gpGlobals->curtime);
        return 1;
    }

    int LTickCount(lua_State *l)
    {
        lua_pushinteger(l, gpGlobals->tickcount);
        return 1;
    }

    int LGetMapName(lua_State *l)
    {
        lua_pushstring(l, STRING(gpGlobals->mapname));
        return 1;
    }

    static const struct luaL_Reg vectorlib_f [] = {
        {"__call", LVectorNew},
        {nullptr, nullptr},
    };

    static const struct luaL_Reg vectorlib_m [] = {
        {"Length", LVectorLength},
        {"Distance", LVectorDistance},
        {"Cross", LVectorCross},
        {"Dot", LVectorDot},
        {"ToAngles", LVectorToAngles},
        {"GetForward", LVectorGetForward},
        {"GetAngleVectors", LVectorGetAngleVectors},
        {"Normalize", LVectorNormalize},
        {"Rotate", LVectorRotate},
        {"Set", LVectorCopy},
        {"SetUnpacked", LVectorCopyUnpacked},
        {"__add", LVectorAdd},
        {"__sub", LVectorSubtract},
        {"__mul", LVectorMultiply},
        {"__div", LVectorDivide},
        {"__eq", LVectorEquals},
        {"__tostring", LVectorToString},
        {"__index", LVectorIndex},
        {"__newindex", LVectorSetIndex},
        {nullptr, nullptr},
    };

    static const struct luaL_Reg entitylib_f [] = {
        {"__call", LEntityNew},
        {"FindByName", LFindEntityByName},
        {"FindByClass", LFindEntityByClassname},
        {"FindAllByName", LFindAllEntityByName},
        {"FindAllByClass", LFindAllEntityByClassname},
        {"FindInBox", LFindAllEntityInBox},
        {"FindInSphere", LFindAllEntityInSphere},
        {"GetAll", LFindAllEntity},
        {"Create", LEntityNew},
        {"GetFirstEntity", LEntityFirst},
        {"GetNextEntity", LEntityNext},
        {"AddCreateCallback", LEntityAddCreateCallback},
        {"RemoveCreateCallback", LEntityRemoveCreateCallback},
        {nullptr, nullptr},
    };

    static const struct luaL_Reg entitylib_m [] = {
        {"IsValid", LEntityIsValid},
        {"DispatchSpawn", LEntitySpawn},
        {"Activate", LEntityActivate},
        {"Remove", LEntityRemove},
        {"AcceptInput", LEntityAcceptInput},
        {"GetNetIndex", LEntityGetNetworkIndex},
        {"GetHandleIndex", LEntityGetHandleIndex},
        {"GetPlayerItemBySlot", LEntityGetPlayerItemBySlot},
        {"GetPlayerItemByName", LEntityGetPlayerItemByName},
        {"GetAttributeValue", LEntityGetItemAttribute},
        {"SetAttributeValue", LEntitySetItemAttribute},
        {"PrintToChat", LUtilPrintToChat},
        {"PrintToConsole", LUtilPrintToConsole},
        {"GetName", LEntityGetName},
        {"SetName", LEntitySetName},
        {"GetPlayerName", LEntityGetPlayerName},
        {"IsAlive", LEntityIsAlive},
        {"IsPlayer", LEntityIsPlayer},
        {"IsNPC", LEntityIsNPC},
        {"IsBot", LEntityIsBot},
        {"IsRealPlayer", LEntityIsRealPlayer},
        {"IsObject", LEntityIsObject},
        {"IsWeapon", LEntityIsWeapon},
        {"IsCombatCharacter", LEntityIsCombatCharacter},
        {"IsWearable", LEntityIsWearable},
        {"AddCallback", LEntityAddCallback},
        {"RemoveCallback", LEntityRemoveCallback},
        {"DumpProperties", LEntityDumpProperties},
        {"DumpInputs", LEntityDumpInputs},
        {"TakeDamage", LEntityTakeDamage},
        {"AddCond", LEntityAddCond},
        {"RemoveCond", LEntityRemoveCond},
        {"InCond", LEntityInCond},
        {"GetConditionProvider", LEntityGetCondProvider},
        {"StunPlayer", LEntityStunPlayer},
        {"GetAbsOrigin", LEntityGetAbsOrigin},
        {"SetAbsOrigin", LEntitySetAbsOrigin},
        {"GetAbsAngles", LEntityGetAbsAngles},
        {"SetAbsAngles", LEntitySetAbsAngles},
        {"Teleport", LEntityTeleport},
        {"GiveItem", LEntityCreateItem},
        {"FireOutput", LEntityFireOutput},
        {"SetFakeSendProp", LEntitySetFakeProp},
        {"ResetFakeSendProp", LEntityResetFakeProp},
        {"GetFakeSendProp", LEntityGetFakeProp},
        {"AddEffects", LEntityAddEffects},
        {"RemoveEffects", LEntityRemoveEffects},
        {"IsEffectActive", LEntityIsEffectActive},
        {"__eq", LEntityEquals},
        {"__tostring", LEntityToString},
        {"__index", LEntityGetProp},
        {"__newindex", LEntitySetProp},
        
        {nullptr, nullptr},
    };

    static const struct luaL_Reg timerlib_f [] = {
        
        {"Create", LTimerCreate},
        {"Simple", LTimerSimple},
        {"Stop", LTimerStop},
        {nullptr, nullptr},
    };

    static const struct luaL_Reg utillib_f [] = {
        
        {"Trace", LTraceLine},
        {"PrintToConsoleAll", LUtilPrintToConsoleAll},
        {"PrintToConsole", LUtilPrintToConsole},
        {"PrintToChatAll", LUtilPrintToChatAll},
        {"PrintToChat", LUtilPrintToChat},
        {nullptr, nullptr},
    };

    static const struct luaL_Reg proplib_m [] = {
        {"__index", LPropGet},
        {"__newindex", LPropSet},
        {"__len", LPropGetN},
        {nullptr, nullptr},
    };

    THINK_FUNC_DECL(ScriptModuleTick)
    {
        
        this->SetNextThink(gpGlobals->curtime + 6.0f, "ScriptModuleTick");
    }

    LuaState::LuaState() 
    {
        
        l = luaL_newstate();
        luaL_openlibs(l);

        // Blacklist some stuff
        lua_pushnil(l);
        lua_setglobal(l, "io");
        lua_pushnil(l);
        lua_setglobal(l, "package");
        lua_pushnil(l);
        lua_setglobal(l, "loadfile");
        lua_pushnil(l);
        lua_setglobal(l, "dofile");
        lua_pushnil(l);
        lua_setglobal(l, "load");
        lua_pushnil(l);
        lua_setglobal(l, "require");

        lua_getglobal(l, "os");
        lua_pushnil(l);
        lua_setfield(l, -2, "execute");
        lua_pushnil(l);
        lua_setfield(l, -2, "getenv");
        lua_pushnil(l);
        lua_setfield(l, -2, "remove");
        lua_pushnil(l);
        lua_setfield(l, -2, "rename");
        lua_pushnil(l);
        lua_setfield(l, -2, "setlocale");
        lua_pushnil(l);
        lua_setfield(l, -2, "tmpname");
        lua_pushnil(l);
        lua_setfield(l, -2, "exit");
        
        // Override print
        lua_register(l, "print", LPrint);

        /* Vector */
        luaL_newmetatable(l, "vector");
        luaL_setfuncs(l, vectorlib_m, 0);
        m_pVectorMeta = vector_meta_ptr = lua_topointer(l, -1);
        m_iVectorMeta = vector_meta = luaL_ref(l, LUA_REGISTRYINDEX);

        luaL_newmetatable(l, "entity");
        luaL_setfuncs(l, entitylib_m, 0);
        m_pEntityMeta = entity_meta_ptr = lua_topointer(l, -1);
        m_iEntityMeta = entity_meta = luaL_ref(l, LUA_REGISTRYINDEX);

        luaL_newmetatable(l, "prop");
        luaL_setfuncs(l, proplib_m, 0);
        m_iPropMeta = prop_meta = luaL_ref(l, LUA_REGISTRYINDEX);
        //lua_rawsetp(l, LUA_REGISTRYINDEX, (void*)entity_meta);

        //lua_pushstring(l, "__index");
        //lua_pushvalue(l, -2);
        //lua_settable(l, -3);
        
        //lua_pushcfunction(l, LVectorSetIndex);
        //lua_settable(l, -3);

        //lua_pushstring(l, "");
        //lua_pushcfunction(l, LVectorIndex);
        //lua_settable(l, -3);
        
        lua_register(l, "Vector", LVectorNew);
        lua_register(l, "Entity", LEntityNew);
        lua_register(l, "Timer", LTimerCreate);
        lua_register(l, "StopTimer", LTimerStop);
        lua_register(l, "CurTime", LCurTime);
        lua_register(l, "GetMapName", LGetMapName);
        lua_register(l, "TickCount", LTickCount);
        lua_register(l, "IsValid", LIsValid);
        

        luaL_newlib(l, entitylib_f);
        //lua_newtable(l);
        //luaL_setfuncs(l, entitylib_f, 0);
        lua_setglobal(l, "ents");

        luaL_newlib(l, timerlib_f);
        //lua_newtable(l);
        //luaL_setfuncs(l, timerlib_f, 0);
        lua_setglobal(l, "timer");

        luaL_newlib(l, utillib_f);
        //lua_newtable(l);
        //luaL_setfuncs(l, utillib_f, 0);
        lua_setglobal(l, "util");

        lua_newtable(l);
        m_iEntityTableStorage = entity_table_store = luaL_ref(l, LUA_REGISTRYINDEX);

        if (filesystem != nullptr)
            DoFile("scripts/globals.lua", true);
    }

    LuaState::~LuaState() {
        for (auto entity : callbackEntities) {
            auto luaModule = entity->GetEntityModule<LuaEntityModule>("luaentity");
            if (luaModule != nullptr) {
                luaModule->LuaStateRemoved(this);
            }
        }
        RemoveIf(entity_create_callbacks, [&](auto &callback){
            return callback.state == this;
        });
        lua_close(l);
    }

    double script_exec_time = 0.0;
    double script_exec_time_tick = 0.0;
    bool script_exec_time_guard = false;
    void LuaState::DoString(const char *str, bool execute) {
        VPROF_BUDGET("LuaState::DoString", "Lua");
        CFastTimer timer;
        timer.Start();
        SwitchState();
        {
            int err = luaL_loadstring(l, str);
            if (err) {
                const char *errbuf = lua_tostring(l, -1);
                SendWarningConsoleMessageToAdmins("%s\n", errbuf);
            }
            else if (execute) {
                err = lua_pcall(l, 0, LUA_MULTRET, 0);
                
                if (err) {
                    const char *errbuf = lua_tostring(l, -1);
                    SendWarningConsoleMessageToAdmins("%s\n", errbuf);
                }
            }
        }
        lua_settop(l,0);
        timer.End();
        script_exec_time += timer.GetDuration().GetSeconds();
        script_exec_time_tick += timer.GetDuration().GetSeconds();
    }

    void LuaState::DoFile(const char *path, bool execute) {
        VPROF_BUDGET("LuaState::DoFile", "Lua");
        CFastTimer timer;
        timer.Start();
        SwitchState();

        FileHandle_t f = filesystem->Open(path, "rb", "GAME");
        if (f == nullptr) {
            SendWarningConsoleMessageToAdmins("Cannot find lua script file %s\n", path);
            return;
        }
        int fileSize = filesystem->Size(f);
        if (fileSize > 200000) {
            SendWarningConsoleMessageToAdmins("Lua script file %s is too large\n", path);
            return;
        }
        uint bufSize = ((IFileSystem *)filesystem)->GetOptimalReadSize(f, fileSize + 2);
        char *buffer = (char*)((IFileSystem *)filesystem)->AllocOptimalReadBuffer(f, bufSize);
        // read into local buffer
        bool bRetOK = (((IFileSystem *)filesystem)->ReadEx(buffer, bufSize, fileSize, f) != 0);
        
        if (bRetOK) {
            int err = luaL_loadbuffer(l, buffer, bufSize, path);
            if (err) {
                const char *errbuf = lua_tostring(l, -1);
                SendWarningConsoleMessageToAdmins("%s\n", errbuf);
            }
            else if (execute) {
                err = lua_pcall(l, 0, LUA_MULTRET, 0);
                
                if (err) {
                    const char *errbuf = lua_tostring(l, -1);
                    SendWarningConsoleMessageToAdmins("%s\n", errbuf);
                }
            }
        }
        else {
            SendWarningConsoleMessageToAdmins("Fail reading buffer of lua script file %s\n", path);
            return;
        }
        lua_settop(l,0);
        timer.End();
        script_exec_time += timer.GetDuration().GetSeconds();
        script_exec_time_tick += timer.GetDuration().GetSeconds();
    }

    void LuaState::SwitchState()
    {
        cur_state = this;
        vector_meta = m_iVectorMeta;
        vector_meta_ptr = m_pVectorMeta;
        entity_meta = m_iEntityMeta;
        entity_meta_ptr = m_pEntityMeta;
        entity_table_store = m_iEntityTableStorage;
        prop_meta = m_iPropMeta;
    }

    void LuaState::CallGlobal(const char *str, int numargs) {
        VPROF_BUDGET("LuaState::CallGlobal", "Lua");
        CFastTimer timer;
        timer.Start();
        
        SwitchState();
        {
            lua_getglobal(l, str);
            int err = lua_pcall(l, numargs, 0, 0);
            if (err) {
                const char *errstr = lua_tostring(l, -1);
                SendWarningConsoleMessageToAdmins("%s\n", errstr);
            }
        }
        stackDump(l);
        lua_settop(l,0);

        timer.End();
        script_exec_time += timer.GetDuration().GetSeconds();
        script_exec_time_tick += timer.GetDuration().GetSeconds();
    }


    int LuaState::Call(int numargs, int numret) {
        VPROF_BUDGET("LuaState::Call", "Lua");
        //TIME_SCOPE2(call);
        CFastTimer timer;
        timer.Start();
        SwitchState();

        int err = lua_pcall(l, numargs, numret, 0);
        if (err) {
            const char *errstr = lua_tostring(l, -1);
            SendWarningConsoleMessageToAdmins("%s\n", errstr);
        }

        timer.End();
        script_exec_time += timer.GetDuration().GetSeconds();
        script_exec_time_tick += timer.GetDuration().GetSeconds();
        return err;
    }

    bool LuaState::CheckGlobal(const char* name) {
        int type = lua_getglobal(l, name);
        if (type != LUA_TNIL) {
            return true;
        }
        lua_pop(l, -1);
        return false;
    }
    void LuaState::UpdateTimers() {

        VPROF_BUDGET("LuaState::UpdateTimers", "Lua");
        SwitchState();

        for (auto it = timers.begin(); it != timers.end();) {
            auto &timer = *it;
            
            if (timer.m_flNextCallTime < gpGlobals->curtime) {
                lua_rawgeti(l, LUA_REGISTRYINDEX, timer.m_iRefFunc);
                if (timer.m_iRefParam != 0)
                    lua_rawgeti(l, LUA_REGISTRYINDEX, timer.m_iRefParam);

                int err = Call(timer.m_iRefParam != 0 ? 1 : 0, 1);
                bool stop = false;
                if (timer.m_iRepeats > 0) {
                    if (--timer.m_iRepeats <= 0) {
                        stop = true;
                    }
                }
                if (!lua_isnil(l, -1) && !lua_toboolean(l, -1)) {
                    stop = true;
                }
                if (!stop) {
                    lua_settop(l,0);
                    timer.m_flNextCallTime = gpGlobals->curtime + timer.m_flDelay;
                }
                else {
                    timer.Destroy(l);
                    it = timers.erase(it);
                    lua_settop(l,0);

                    if (timers.empty())
                        AllTimersRemoved();
                    continue;
                }
            }
            it++;
        }
    }

    int LuaState::AddTimer(float delay, int repeats, int reffunc, int refparam) {
        m_iNextTimerID++;
        if (timers.empty())
            TimerAdded();
        timers.emplace_back(m_iNextTimerID, delay, repeats, reffunc, refparam);
        return m_iNextTimerID;
    }

    bool LuaState::StopTimer(int id) {
        for (auto it = timers.begin(); it != timers.end(); it++) {
            if (it->m_iID == id) {
                it->Destroy(l);
                timers.erase(it);
                
                if (timers.empty())
                    AllTimersRemoved();

                return true;
            }
        }
        return false;
    }

    void LuaState::EntityDeleted(CBaseEntity *entity) {
        callbackEntities.erase(entity);
        
        lua_rawgeti(l, LUA_REGISTRYINDEX, m_iEntityTableStorage);
        lua_pushnil(l);
        lua_rawsetp(l, -2, entity);
        lua_pop(l, 1);
    }

    void LuaState::EntityCallbackAdded(CBaseEntity *entity) {
        callbackEntities.insert(entity);
    }

    void LuaState::Activate() {
        // Tell scripts about every connected player so far
        ForEachTFPlayer([&](CTFPlayer *player){
            if (this->CheckGlobal("OnPlayerConnected")) {
                LEntityAlloc(l, player);
                this->Call(1, 0);
            }
        });
    }

    void LuaTimer::Destroy(lua_State *l) {
        luaL_unref(l, LUA_REGISTRYINDEX, m_iRefFunc);
        luaL_unref(l, LUA_REGISTRYINDEX, m_iRefParam);
    }

    LuaState *state = nullptr;
    CON_COMMAND_F(sig_lua_test, "", FCVAR_NONE)
    {
        if (state == nullptr) state = new LuaState();
        state->DoString(args[1], true);
    }
    CON_COMMAND_F(sig_lua_file, "", FCVAR_NONE)
    {
        if (state == nullptr) state = new LuaState();
        state->DoFile(args[1], true);
    }
    CON_COMMAND_F(sig_lua_call, "", FCVAR_NONE)
    {
        if (state == nullptr) state = new LuaState();
        state->CallGlobal(args[1], 0);
    }

    DETOUR_DECL_MEMBER(void, CBaseEntity_Activate)
	{
        auto entity = reinterpret_cast<CBaseEntity *>(this);
        DETOUR_MEMBER_CALL(CBaseEntity_Activate)();
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");
        if (mod != nullptr) {
            mod->FireCallback(ON_ACTIVATE);
        }
    }

	DETOUR_DECL_STATIC(void, DispatchSpawn, CBaseEntity *entity)
	{
        DETOUR_STATIC_CALL(DispatchSpawn)(entity);
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");
        if (mod != nullptr && !entity->IsPlayer()) {
            mod->FireCallback(ON_SPAWN);
        }
    }

	DETOUR_DECL_STATIC(CBaseEntity *, CreateEntityByName, const char *className, int iForceEdictIndex)
	{
		auto entity = DETOUR_STATIC_CALL(CreateEntityByName)(className, iForceEdictIndex);
        if (entity != nullptr && !entity_create_callbacks.empty()) {
            for(auto &callback : entity_create_callbacks) {
                if (entity->GetClassnameString() == callback.classname || (callback.wildcard && NamesMatchCaseSensitve(STRING(callback.classname), entity->GetClassnameString()))) {
                    auto l = callback.state->GetState();
                    lua_rawgeti(l, LUA_REGISTRYINDEX, callback.func);
                    LEntityAlloc(l, entity);
                    lua_pushstring(l, className);
                    callback.state->Call(2, 0);
                }
            }
        }
        return entity;
	}

    RefCount rc_CBaseEntity_TakeDamage;
    
    DETOUR_DECL_MEMBER(int, CBaseEntity_TakeDamage, CTakeDamageInfo &info)
	{
        SCOPED_INCREMENT(rc_CBaseEntity_TakeDamage);

        CBaseEntity *entity = reinterpret_cast<CBaseEntity *>(this);
        
        bool overriden = false;
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");
        int damage = 0;

        int health_pre = entity->GetHealth();
        if (mod != nullptr) {
            CTakeDamageInfo infooverride = info;
            for (auto &pair : mod->callbacks[ON_DAMAGE_RECEIVED_PRE]) {
                auto l = pair.state->GetState();
                DamageInfoToTable(l, infooverride);
                lua_pushvalue(l, -1);
                lua_rawgeti(l, LUA_REGISTRYINDEX, pair.func);
                LEntityAlloc(l, entity);
                lua_pushvalue(l, -3);
                pair.state->Call(2, 1);
                if (lua_toboolean(l, -1)) {
                    overriden = true;
                    TableToDamageInfo(l, -2, infooverride);
                }

                lua_settop(l, 0);
            }
            if(overriden) {
                damage = DETOUR_MEMBER_CALL(CBaseEntity_TakeDamage)(infooverride);
            }
        }

        if (!overriden) {
		    damage = DETOUR_MEMBER_CALL(CBaseEntity_TakeDamage)(info);
        }

        if (mod != nullptr) {
            for (auto &pair : mod->callbacks[ON_DAMAGE_RECEIVED_POST]) {
                auto l = pair.state->GetState();
                lua_rawgeti(l, LUA_REGISTRYINDEX, pair.func);
                LEntityAlloc(l, entity);
                DamageInfoToTable(l, info);
                lua_pushnumber(l, health_pre);
                pair.state->Call(3, 0);

                lua_settop(l, 0);
            }
        }

        return damage;
	}

    DETOUR_DECL_MEMBER(bool, CBaseEntity_AcceptInput, const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID)
    {
        CBaseEntity *entity = reinterpret_cast<CBaseEntity *>(this);
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");
        if (mod != nullptr) {
            std::function<void(LuaState *)> func = [&](LuaState *state){
                lua_pushstring(state->GetState(), szInputName);
                LFromVariant(state->GetState(), Value);
                LEntityAlloc(state->GetState(), pActivator);
                LEntityAlloc(state->GetState(), pCaller);
            };
            bool stop = false;
            std::function<void(LuaState *)> funcReturn = [&](LuaState *state){
                stop = lua_toboolean(state->GetState(), -1);
            };
            mod->FireCallback(ON_INPUT, &func, 4, &funcReturn, 1);
            if (stop) {
                return true;
            }
        }
        return DETOUR_MEMBER_CALL(CBaseEntity_AcceptInput)(szInputName, pActivator, pCaller, Value, outputID);
    }


    DETOUR_DECL_MEMBER(void, CBaseEntityOutput_FireOutput, variant_t Value, CBaseEntity *pActivator, CBaseEntity *pCaller, float fDelay)
    {

        CBaseEntityOutput *output = reinterpret_cast<CBaseEntityOutput *>(this);

        auto mod = pCaller != nullptr ? pCaller->GetEntityModule<LuaEntityModule>("luaentity") : nullptr;
        if (mod != nullptr) {
            
            auto datamap = pCaller->GetDataDescMap();
            const char *name = "";
            bool found = false;
            for (datamap_t *dmap = datamap; dmap != NULL; dmap = dmap->baseMap) {
                // search through all the readable fields in the data description, looking for a match
                for (int i = 0; i < dmap->dataNumFields; i++) {
                    if ((dmap->dataDesc[i].flags & FTYPEDESC_OUTPUT) && ((CBaseEntityOutput*)(((char*)pCaller) + dmap->dataDesc[i].fieldOffset[ TD_OFFSET_NORMAL ])) == output) {
                        name = dmap->dataDesc[i].externalName;
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            
            std::function<void(LuaState *)> func = [&](LuaState *state){
                lua_pushstring(state->GetState(), name);
                LFromVariant(state->GetState(), Value);
                LEntityAlloc(state->GetState(), pActivator);
            };
            bool stop = false;
            std::function<void(LuaState *)> funcReturn = [&](LuaState *state){
                stop = lua_toboolean(state->GetState(), -1);
            };
            mod->FireCallback(ON_OUTPUT, &func, 3, &funcReturn, 1);
            if (stop) {
                return;
            }
        }
        return DETOUR_MEMBER_CALL(CBaseEntityOutput_FireOutput)(Value, pActivator, pCaller, fDelay);
    }

	DETOUR_DECL_MEMBER(void, CServerGameClients_ClientPutInServer, edict_t *edict, const char *playername)
	{
        DETOUR_MEMBER_CALL(CServerGameClients_ClientPutInServer)(edict, playername);
        for(auto state : LuaState::List()) {
            
            if (state->CheckGlobal("OnPlayerConnected")) {
                LEntityAlloc(state->GetState(), GetContainingEntity(edict));
                state->Call(1, 0);
            }
        }
    }

    DETOUR_DECL_MEMBER(void, CTFPlayer_UpdateOnRemove)
	{
        auto player = reinterpret_cast<CTFPlayer *>(this);
        DETOUR_MEMBER_CALL(CTFPlayer_UpdateOnRemove)();
        for(auto state : LuaState::List()) {
            
            if (state->CheckGlobal("OnPlayerDisconnected")) {
                LEntityAlloc(state->GetState(), player);
                state->Call(1, 0);
            }
        }
    }

	DETOUR_DECL_MEMBER(void, CTFPlayer_PlayerRunCommand, CUserCmd* cmd, IMoveHelper* moveHelper)
	{
		CTFPlayer* player = reinterpret_cast<CTFPlayer*>(this);
        int prebuttons = player->m_nButtons;
        if (prebuttons != cmd->buttons) {
            auto mod = player->GetEntityModule<LuaEntityModule>("luaentity");
            if (mod != nullptr) {
                for (int i = 0; i < 32; i++) {
                    int mask = 1 << i;
                    if ((cmd->buttons & mask) != (prebuttons & mask)) {
                        std::function<void(LuaState *)> func = [&](LuaState *state){
                            lua_pushinteger(state->GetState(), mask);
                        };
                        //bool stop = false;
                        //std::function<void(LuaState *)> funcReturn = [&](LuaState *state){
                        //    stop = lua_toboolean(state->GetState(), -1);
                        //};
                        bool release = (cmd->buttons & mask) && !(prebuttons & mask);
                        mod->FireCallback(release ? ON_KEY_PRESSED : ON_KEY_RELEASED, &func, 1);
                       //if (stop) {
                        //    cmd->buttons = release ? cmd->buttons | mask : cmd->buttons & ~(mask);
                        //}
                    }
                }
            }
        }
		DETOUR_MEMBER_CALL(CBasePlayer_PlayerRunCommand)(cmd, moveHelper);
	}

    bool profile[34] = {};
    DETOUR_DECL_MEMBER(bool, CTFPlayer_ClientCommand, const CCommand& args)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (player != nullptr && ENTINDEX(player) < 34) {
            if (FStrEq(args[0], "sig_lua_prof_start")) {
                profile[ENTINDEX(player)] = 1;
                return true;
            }
            if (FStrEq(args[0], "sig_lua_prof_end")) {
                profile[ENTINDEX(player)] = 0;
                return true;
            }
		}
		
		return DETOUR_MEMBER_CALL(CTFPlayer_ClientCommand)(args);
	}
    
    DETOUR_DECL_MEMBER(void, CBaseEntity_Event_Killed, CTakeDamageInfo &info)
	{
        CBaseEntity *entity = reinterpret_cast<CBaseEntity *>(this);
        
        bool overriden = false;
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");

        if (mod != nullptr) {
            CTakeDamageInfo infooverride = info;
            for (auto &pair : mod->callbacks[ON_DEATH]) {
                auto l = pair.state->GetState();
                DamageInfoToTable(l, infooverride);
                lua_pushvalue(l, -1);
                lua_rawgeti(l, LUA_REGISTRYINDEX, pair.func);
                LEntityAlloc(l, entity);
                lua_pushvalue(l, -3);
                pair.state->Call(2, 1);
                if (lua_toboolean(l, -1)) {
                    overriden = true;
                    TableToDamageInfo(l, -2, infooverride);
                }

                lua_settop(l, 0);
            }
            if(overriden) {
                DETOUR_MEMBER_CALL(CBaseEntity_Event_Killed)(infooverride);
                return;
            }
        }
		DETOUR_MEMBER_CALL(CBaseEntity_Event_Killed)(info);
	}

    DETOUR_DECL_MEMBER(void, CBaseCombatCharacter_Event_Killed, CTakeDamageInfo &info)
	{
        CBaseEntity *entity = reinterpret_cast<CBaseEntity *>(this);
        
        bool overriden = false;
        auto mod = entity->GetEntityModule<LuaEntityModule>("luaentity");

        if (mod != nullptr) {
            CTakeDamageInfo infooverride = info;
            for (auto &pair : mod->callbacks[ON_DEATH]) {
                auto l = pair.state->GetState();
                DamageInfoToTable(l, infooverride);
                lua_pushvalue(l, -1);
                lua_rawgeti(l, LUA_REGISTRYINDEX, pair.func);
                LEntityAlloc(l, entity);
                lua_pushvalue(l, -3);
                pair.state->Call(2, 1);
                if (lua_toboolean(l, -1)) {
                    overriden = true;
                    TableToDamageInfo(l, -2, infooverride);
                }

                lua_settop(l, 0);
            }
            if(overriden) {
                DETOUR_MEMBER_CALL(CBaseCombatCharacter_Event_Killed)(infooverride);
                return;
            }
        }
		DETOUR_MEMBER_CALL(CBaseCombatCharacter_Event_Killed)(info);
	}

    DETOUR_DECL_MEMBER(void, CTFPlayer_Spawn)
	{
		DETOUR_MEMBER_CALL(CTFPlayer_Spawn)();
		CTFPlayer *player = reinterpret_cast<CTFPlayer *>(this); 
        auto mod = player->GetEntityModule<LuaEntityModule>("luaentity");
        if (mod != nullptr) {
            mod->FireCallback(ON_SPAWN);
        }

	}
    
    class CMod : public IMod, public IModCallbackListener, public IFrameUpdatePostEntityThinkListener
	{
	public:
		CMod() : IMod("Util:Lua") 
        {
			MOD_ADD_DETOUR_MEMBER_PRIORITY(CBaseEntity_AcceptInput, "CBaseEntity::AcceptInput", HIGH);
            MOD_ADD_DETOUR_MEMBER_PRIORITY(CBaseEntityOutput_FireOutput, "CBaseEntityOutput::FireOutput", HIGH);
            MOD_ADD_DETOUR_MEMBER_PRIORITY(CBaseEntity_TakeDamage, "CBaseEntity::TakeDamage", HIGH);
            MOD_ADD_DETOUR_MEMBER(CBaseEntity_Activate, "CBaseEntity::Activate");
            MOD_ADD_DETOUR_STATIC(DispatchSpawn, "DispatchSpawn");
            MOD_ADD_DETOUR_STATIC(CreateEntityByName, "CreateEntityByName");
            MOD_ADD_DETOUR_MEMBER(CServerGameClients_ClientPutInServer, "CServerGameClients::ClientPutInServer");
            MOD_ADD_DETOUR_MEMBER(CTFPlayer_UpdateOnRemove, "CTFPlayer::UpdateOnRemove");
            MOD_ADD_DETOUR_MEMBER(CTFPlayer_PlayerRunCommand, "CTFPlayer::PlayerRunCommand");
            MOD_ADD_DETOUR_MEMBER(CTFPlayer_ClientCommand, "CTFPlayer::ClientCommand");
            MOD_ADD_DETOUR_MEMBER(CBaseEntity_Event_Killed, "CBaseEntity::Event_Killed");
            MOD_ADD_DETOUR_MEMBER(CBaseCombatCharacter_Event_Killed, "CBaseCombatCharacter::Event_Killed");
            MOD_ADD_DETOUR_MEMBER(CTFPlayer_Spawn, "CTFPlayer::Spawn");
            
        }
		
        virtual void PreLoad() override
        {
            stringTSendProxy = AddrManager::GetAddr("SendProxy_StringT_To_String");
        }

		virtual bool ShouldReceiveCallbacks() const override { return this->IsEnabled(); }
		
	    virtual bool EnableByDefault() { return true; }

        virtual void OnEnable() override
		{
            sendproxies = gamedll->GetStandardSendProxies();
        }

        virtual void LevelInitPreEntity() override
        { 
            if (state != nullptr) { delete state; state = nullptr;}

            sendproxies = gamedll->GetStandardSendProxies();
        }

		virtual void FrameUpdatePostEntityThink() override
		{
            static double script_exec_time_tick_max = 0.0;
            if (script_exec_time_tick > script_exec_time_tick_max ) {
                script_exec_time_tick_max = script_exec_time_tick;
            }
            script_exec_time_tick = 0.0;
            if (gpGlobals->tickcount % 66 == 0) {
                for (int i = 0; i < 34; i++) {
                    if (profile[i]) {
                        auto player = UTIL_PlayerByIndex(i);
                        if (player != nullptr) {
                            ClientMsg(player, "Lua script execution time: [avg: %.9fs (%d%%)| max: %.9fs (%d%%)]\n", script_exec_time / 66, (int)(script_exec_time * 100), script_exec_time_tick_max, (int)((script_exec_time_tick_max / 0.015) * 100) );
                        }
                    }
                }
                script_exec_time = 0;
                script_exec_time_tick_max = 0;
            }

            if (state != nullptr)
			    state->UpdateTimers();

            if (!LuaState::List().empty())
            {
                for(auto state : LuaState::List()) {
                    
                    if (state->CheckGlobal("OnGameTick")) {
                        state->Call(0, 0);
                    }
                }
            }
		}
	};
	CMod s_Mod;
}