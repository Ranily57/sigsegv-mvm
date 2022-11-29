#include "mod.h"
#include "stub/entities.h"
#include "stub/extraentitydata.h"
#include "stub/projectiles.h"
#include "stub/player_util.h"
#include "stub/tfbot.h"
#include "stub/gamerules.h"
#include "stub/populators.h"
#include "stub/server.h"
#include "stub/misc.h"
#include "stub/team.h"
#include "util/iterate.h"
#include "util/scope.h"
#include "mod/pop/kv_conditional.h"
#include "mod/pop/pointtemplate.h"
#include "mod/mvm/extended_upgrades.h"
#include "mod/pop/common.h"
#include "stub/strings.h"
#include "stub/usermessages_sv.h"
#include "stub/objects.h"
#include "stub/tf_objective_resource.h"
#include "stub/upgrades.h"
#include "stub/nextbot_cc.h"
#include "util/clientmsg.h"
#include "util/admin.h"
#include "mod/etc/mapentity_additions.h"
#include <fmt/core.h>

WARN_IGNORE__REORDER()
#include <../server/vote_controller.h>
WARN_RESTORE()


#define UNW_LOCAL_ONLY
#include <cxxabi.h>

#define PLAYER_ANIM_WEARABLE_ITEM_ID 12138

enum SpawnResult
{
	SPAWN_FAIL     = 0,
	SPAWN_NORMAL   = 1,
	SPAWN_TELEPORT = 2,
};

namespace Mod::Pop::Wave_Extensions
{
	std::vector<std::string> *GetWaveExplanation(int wave);
}

namespace Mod::Pop::PopMgr_Extensions
{
    bool IsMannVsMachineMode();
}

namespace Mod {
    bool IsMannVsMachineMode(){
        return Mod::Pop::PopMgr_Extensions::IsMannVsMachineMode();
    }
}

namespace Mod::Etc::Mapentity_Additions
{
    void ClearFakeProp();
}

namespace Mod::Pop::PopMgr_Extensions
{
	int iGetTeamAssignmentOverride = 6;
	#if defined _LINUX

	static constexpr uint8_t s_Buf_GetTeamAssignmentOverride[] = {
		0x8D, 0xB6, 0x00, 0x00, 0x00, 0x00, // +0000
		0xB8, 0x06, 0x00, 0x00, 0x00, // +0006
		0x2B, 0x45, 0xC4, // +000B
		0x29, 0xF0, // +000E
		0x85, 0xC0, // +0010

		//0xc7, 0x84, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // +0000  mov dword ptr [ebx+eax*4+m_Visuals],0x00000000
		//0x8b, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,                         // +000B  mov eax,g_TeamVisualSections[eax*4]
	};

	struct CPatch_GetTeamAssignmentOverride : public CPatch
	{
		CPatch_GetTeamAssignmentOverride() : CPatch(sizeof(s_Buf_GetTeamAssignmentOverride)) {}
		
		virtual bool GetVerifyInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf.CopyFrom(s_Buf_GetTeamAssignmentOverride);
			
			mask.SetRange(0x06 + 1, 4, 0x00);
			
			return true;
		}
		
		virtual bool GetPatchInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf .SetRange(0x06+1, 1, iGetTeamAssignmentOverride);
			mask.SetRange(0x06+1, 4, 0xff);
			
			return true;
		}
		virtual bool AdjustPatchInfo(ByteBuf& buf) const override
		{
			buf .SetRange(0x06+1, 1, iGetTeamAssignmentOverride);
			
			return true;
		}
		virtual const char *GetFuncName() const override   { return "CTFGameRules::GetTeamAssignmentOverride"; }
		virtual uint32_t GetFuncOffMin() const override    { return 0x0100; }
		virtual uint32_t GetFuncOffMax() const override    { return 0x0300; }
	};

	#elif defined _WINDOWS

	using CExtract_GetTeamAssignmentOverride = IExtractStub;

	#endif

	int iClientConnected = 9;
	#if defined _LINUX

	static constexpr uint8_t s_Buf_CTFGameRules_ClientConnected[] = {
		0x31, 0xC0, // +0000
		0x83, 0xFE, 0x09, // +0002
	};

	struct CPatch_CTFGameRules_ClientConnected : public CPatch
	{
		CPatch_CTFGameRules_ClientConnected() : CPatch(sizeof(s_Buf_CTFGameRules_ClientConnected)) {}
		
		virtual bool GetVerifyInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf.CopyFrom(s_Buf_CTFGameRules_ClientConnected);
			
			mask.SetRange(0x02 + 2, 1, 0x00);
			
			return true;
		}
		
		virtual bool GetPatchInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf .SetRange(0x02+2, 1, iClientConnected);
			mask.SetRange(0x02+2, 1, 0xff);
			
			return true;
		}
		virtual bool AdjustPatchInfo(ByteBuf& buf) const override
		{
			buf .SetRange(0x02+2, 1, iClientConnected);
			
			return true;
		}
		virtual const char *GetFuncName() const override   { return "CTFGameRules::ClientConnected"; }
		virtual uint32_t GetFuncOffMin() const override    { return 0x0030; }
		virtual uint32_t GetFuncOffMax() const override    { return 0x0100; }
	};

	#elif defined _WINDOWS

	using CExtract_GetTeamAssignmentOverride = IExtractStub;

	#endif

	int iPreClientUpdate = 6;
	#if defined _LINUX

	static constexpr uint8_t s_Buf_CTFGCServerSystem_PreClientUpdate[] = {
		0x8B, 0x5D, 0xB0, // +0000
		0x83, 0xC3, 0x06, // +0003
	};

	struct CPatch_CTFGCServerSystem_PreClientUpdate : public CPatch
	{
		CPatch_CTFGCServerSystem_PreClientUpdate() : CPatch(sizeof(s_Buf_CTFGCServerSystem_PreClientUpdate)) {}
		
		virtual bool GetVerifyInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf.CopyFrom(s_Buf_CTFGCServerSystem_PreClientUpdate);
			
			mask.SetRange(0x03 + 2, 1, 0x00);
			
			return true;
		}
		
		virtual bool GetPatchInfo(ByteBuf& buf, ByteBuf& mask) const override
		{
			buf .SetRange(0x03+2, 1, iPreClientUpdate);
			mask.SetRange(0x03+2, 1, 0xff);
			
			return true;
		}
		virtual bool AdjustPatchInfo(ByteBuf& buf) const override
		{
			buf .SetRange(0x03+2, 1, iPreClientUpdate);
			
			return true;
		}
		virtual const char *GetFuncName() const override   { return "CTFGCServerSystem::PreClientUpdate"; }
		virtual uint32_t GetFuncOffMin() const override    { return 0x0350; }
		virtual uint32_t GetFuncOffMax() const override    { return 0x0600; }
	};

	#elif defined _WINDOWS

	using CPatch_CTFGCServerSystem_PreClientUpdate = IExtractStub;

	#endif
	
	int GetVisibleMaxPlayers();
	void SetVisibleMaxPlayers();
	void ResendUpgradeFile(bool force);
	void ResetMaxRedTeamPlayers(int resetTo);
	void ResetMaxTotalPlayers(int resetTo);
	void SetMaxSpectators(int resetTo);

	void TogglePatches();

	ConVar cvar_custom_upgrades_file("sig_mvm_custom_upgrades_file", "", FCVAR_NONE,
		"Set upgrades file to specified one",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			ResendUpgradeFile(true);
		});	
	ConVar cvar_bots_are_humans("sig_mvm_bots_are_humans", "0", FCVAR_NONE,
		"Bots should use human models");	

	ConVar cvar_human_bots_robot_voice("sig_mvm_human_bots_robot_voice", "0", FCVAR_NONE,
		"Bots should use robot voice if they are humans");	

	ConVar cvar_vanilla_mode("sig_vanilla_mode", "0", FCVAR_NONE,	
		"Disable most mods");	
	
	ConVar cvar_use_teleport("sig_mvm_bots_use_teleporters", "1", FCVAR_NOTIFY,
		"Blue humans in MvM: bots use player teleporters");

	ConVar cvar_bots_bleed("sig_mvm_bots_bleed", "0", FCVAR_NOTIFY,
		"Bots should bleed");

	ConVar cvar_max_red_players("sig_mvm_red_team_max_players", "0", FCVAR_NOTIFY,
		"Set max red team count",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			if (cvar_max_red_players.GetInt() > 0)
				ResetMaxRedTeamPlayers(cvar_max_red_players.GetInt());
			else
				ResetMaxRedTeamPlayers(6);
				
			SetVisibleMaxPlayers();
		});	

    ConVar cvar_modded_pvp{"sig_modded_pvp", "0", FCVAR_NOTIFY,
        "Allow more mods to work in non-MvM gamemodes"};

	ConVar cvar_send_bots_to_spectator_immediately("sig_send_bots_to_spectator_immediately", "0", FCVAR_NOTIFY,
		"Bots should be send to spectator immediately after dying");

    bool IsMannVsMachineMode(){
        return cvar_modded_pvp.GetBool() 
            ? true 
            : TFGameRules()->IsMannVsMachineMode();
    }
		
	std::string last_custom_upgrades = "";
	bool received_message_tick = false;
	void ResendUpgradeFile(bool force) {
		
		std::string convalue = cvar_custom_upgrades_file.GetString();
			
			if (convalue == "")
				convalue = "scripts/items/mvm_upgrades.txt";

			if (convalue == "scripts/items/mvm_upgrades.txt") {
				if(force) {
					/*ForEachTFPlayer([&](CTFPlayer *player){
						if (!player->IsFakeClient()) {
							DevMsg("Refunded money\n");
							player->m_Shared->m_bInUpgradeZone = true;
							auto kv = new KeyValues("MVM_Respec");
							serverGameClients->ClientCommandKeyValues(player->GetNetworkable()->GetEdict(), kv);
							player->m_Shared->m_bInUpgradeZone = false;
						}
					});*/
					TFGameRules()->SetCustomUpgradesFile(convalue.c_str());
				}
			}
			else {
				/*ForEachTFPlayer([&](CTFPlayer *player){
					if (!player->IsFakeClient()) {
						DevMsg("Refunded money\n");
						player->m_Shared->m_bInUpgradeZone = true;
						auto kv = new KeyValues("MVM_Respec");
						serverGameClients->ClientCommandKeyValues(player->GetNetworkable()->GetEdict(), kv);
						player->m_Shared->m_bInUpgradeZone = false;
					}
				});*/
				TFGameRules()->SetCustomUpgradesFile("scripts/items/mvm_upgrades.txt");
				convalue = "scripts/items/"+convalue;
				TFGameRules()->SetCustomUpgradesFile(convalue.c_str());
				convalue = "download/"+convalue;
				TFGameRules()->SetCustomUpgradesFile(convalue.c_str());
				if (force && last_custom_upgrades != convalue) {
					received_message_tick = true;
					PrintToChatAll("\x07""ffb200This server uses custom upgrades. Make sure you have enabled downloads in options (Download all files or Don't download sound files)");
				}
				last_custom_upgrades = convalue;
			}
	}

	const char g_sSounds[10][24] = {"", "Scout.No03",   "Sniper.No04", "Soldier.No01",
		"Demoman.No03", "Medic.No03",  "heavy.No02",
		"Pyro.No01",    "Spy.No02",    "Engineer.No03"};

	template<typename T>
	class IPopOverride
	{
	public:
		virtual ~IPopOverride() {}
		
		bool IsOverridden() const { return this->m_bOverridden; }
		
		void Reset()
		{
			if (this->m_bOverridden) {
				this->Restore();
				this->m_bOverridden = false;
			}
		}
		void Set(const T& val)
		{
			if (!this->m_bOverridden) {
				this->Backup();
				this->m_bOverridden = true;
			}
			this->SetValue(val);
		}
		T Get() { return this->GetValue(); }
		
	protected:
		virtual T GetValue() = 0;
		virtual void SetValue(const T& val) = 0;
		
	private:
		void Backup()
		{
			this->m_Backup = this->GetValue();
		}
		void Restore()
		{
			this->SetValue(this->m_Backup);
		}
		
		bool m_bOverridden = false;
		T m_Backup;
	};
	
	
	// TODO: consider rewriting this in terms of IConVarOverride from util/misc.h
	template<typename T>
	class CPopOverride_ConVar : public IPopOverride<T>
	{
	public:
		CPopOverride_ConVar(const char *name) :
			m_pszConVarName(name) {}
		
		virtual T GetValue() override { return ConVar_GetValue<T>(MyConVar()); }
		
		virtual void SetValue(const T& val) override
		{
			/* set the ConVar value in a manner that circumvents:
			 * - FCVAR_NOTIFY notifications
			 * - minimum value limits
			 * - maximum value limits
			 */
			
			int old_flags   = MyConVar().Ref_Flags();
			bool old_hasmin = MyConVar().Ref_HasMin();
			bool old_hasmax = MyConVar().Ref_HasMax();
			
			MyConVar().Ref_Flags() &= ~FCVAR_NOTIFY;
			MyConVar().Ref_HasMin() = false;
			MyConVar().Ref_HasMax() = false;
			
			ConVar_SetValue<T>(MyConVar(), val);
			
			MyConVar().Ref_Flags()  = old_flags;
			MyConVar().Ref_HasMin() = old_hasmin;
			MyConVar().Ref_HasMax() = old_hasmax;
		}
		
	private:
		ConVarRef& MyConVar()
		{
			if (this->m_pConVar == nullptr) {
				this->m_pConVar = std::unique_ptr<ConVarRef>(new ConVarRef(this->m_pszConVarName));
			}
			return *(this->m_pConVar);
		}
		
		const char *m_pszConVarName;
		std::unique_ptr<ConVarRef> m_pConVar;
	};
	
	

	// TODO: fix problems related to client-side convar tf_medieval_thirdperson:
	// - players start out in first person, until they taunt or respawn or whatever
	// - players do not get forced back into first person upon popfile switch if the new pop doesn't have this enabled
	// see: firstperson/thirdperson client-side concommands are FCVAR_SERVER_CAN_EXECUTE in TF2; might be a solution
	//      static ConCommand thirdperson( "thirdperson", ::CAM_ToThirdPerson, "Switch to thirdperson camera.", FCVAR_CHEAT | FCVAR_SERVER_CAN_EXECUTE );
	//      static ConCommand firstperson( "firstperson", ::CAM_ToFirstPerson, "Switch to firstperson camera.", FCVAR_SERVER_CAN_EXECUTE );
	// also: just generally look at SetAppropriateCamera in the client DLL
	struct CPopOverride_MedievalMode : public IPopOverride<bool>
	{
		virtual bool GetValue() override                { return TFGameRules()->IsInMedievalMode(); }
		virtual void SetValue(const bool& val) override { TFGameRules()->Set_m_bPlayingMedieval(val); }
	};
	
	
	class CPopOverride_CustomUpgradesFile : public IPopOverride<std::string>
	{
	public:
		virtual std::string GetValue() override
		{
			std::string val = TFGameRules()->GetCustomUpgradesFile();
			
			if (val == "") {
				val = "scripts/items/mvm_upgrades.txt";
			}
			
			return val;
		}
		
		virtual void SetValue(const std::string& val) override
		{
			std::string real_val = val;
			
			if (real_val == "") {
				real_val = "scripts/items/mvm_upgrades.txt";
			}
			
			if (this->GetValue() != real_val) {
				ConColorMsg(Color(0x00, 0xff, 0xff, 0xff), "CPopOverride_CustomUpgradesFile: SetCustomUpgradesFile(\"%s\")\n", real_val.c_str());
				TFGameRules()->SetCustomUpgradesFile(real_val.c_str());
			}
		}
	};
	
	
	class ExtraTankPath
	{
	public:
		ExtraTankPath(const char *name) : m_strName(name) {}
		
		~ExtraTankPath()
		{
			//Msg("Delete extra tank path %s and its %d\n", this->m_strName.c_str(),  this->m_PathNodes.size());
			for (CPathTrack *node : this->m_PathNodes) {
				if (node != nullptr) {
					//node->Remove();
				}
			}
		}
		
		bool AddNode(const Vector& origin)
		{
			assert(!this->m_bSpawned);
			
			auto node = rtti_cast<CPathTrack *>(CreateEntityByName("path_track"));
			if (node == nullptr) return false;
			
			CFmtStr name("%s_%zu", this->m_strName.c_str(), this->m_PathNodes.size() + 1);
			node->SetName(AllocPooledString(name));
			
			node->SetAbsOrigin(origin);
			node->SetAbsAngles(vec3_angle);
			
			node->m_eOrientationType = 1;
			
			this->m_PathNodes.emplace_back(node);
			
			DevMsg("ExtraTankPath: AddNode [ % 7.1f % 7.1f % 7.1f ] \"%s\"\n", VectorExpand(origin), name.Get());
			return true;
		}
		
		void SpawnNodes()
		{
			assert(!this->m_bSpawned);
			
			/* connect up the nodes' target links */
			for (auto it = this->m_PathNodes.begin(); (it + 1) != this->m_PathNodes.end(); ++it) {
				CPathTrack *curr = *it;
				CPathTrack *next = *(it + 1);
				
				DevMsg("ExtraTankPath: Link \"%s\" -> \"%s\"\n", STRING(curr->GetEntityName()), STRING(next->GetEntityName()));
				curr->m_target = next->GetEntityName();
			}
			
			/* now call Spawn() on each node */
			for (CPathTrack *node : this->m_PathNodes) {
				DevMsg("ExtraTankPath: Spawn \"%s\"\n", STRING(node->GetEntityName()));
				node->Spawn();
			}
			
			/* now call Activate() on each node */
			for (CPathTrack *node : this->m_PathNodes) {
				DevMsg("ExtraTankPath: Activate \"%s\"\n", STRING(node->GetEntityName()));
				node->Activate();
			}
			
			this->m_bSpawned = true;
		}
		
	private:
		std::string m_strName;
		std::vector<CHandle<CPathTrack>> m_PathNodes;
		bool m_bSpawned = false;
	};

	struct CustomWeapon
	{
		std::string name = "";
		CTFItemDefinition *originalId = nullptr;
		std::map<CEconItemAttributeDefinition *, std::string> attributes;
	};

	struct SprayDecal
	{
		CRC32_t texture;
		Vector pos;
		QAngle angle;
	};

	struct PlayerPointTemplateInfo
	{
		PointTemplateInfo info;
		int class_index = 0;
	};

	struct WeaponPointTemplateInfo
	{
		PointTemplateInfo info;
		std::vector<std::unique_ptr<ItemListEntry>> weapons;

	};

	struct ItemReplace
	{
		CEconItemDefinition *item_def;
		std::unique_ptr<ItemListEntry> entry;
		CEconItemView *item;
		std::string name;
	};

	struct ExtraLoadoutItem
	{
		int class_index;
		int loadout_slot;
		CEconItemView *item;
		std::string name;
		std::string definitionName;
		int cost = 0;
		int min_wave = 0;
		int max_wave = 9999;
		bool hidden = false;
		bool allow_refund = false;
	};

	struct BuildingPointTemplateInfo
	{
		PointTemplateInfo info;
		int building_type = OBJ_SENTRYGUN;
		int building_mode = 0;
		bool allow_bots = false;
		std::unique_ptr<ItemListEntry> entry;
	};

	struct DisallowUpgradeEntry
	{
		std::string name;
		int max = 0;
		std::vector<std::unique_ptr<ItemListEntry>> entries;
		bool checkAllSlots = false;
		std::vector<std::pair<std::string, int>> ifUpgradePresent;
	};

	enum SpawnLocationRelative
	{
		ANYWHERE,
		BEHIND,
		AHEAD
	};

	class SpawnLocationData
	{
	public:
		std::string name;
		float minDistance = 0;
		float maxDistance = 1500;
		bool advanced = false;
		SpawnLocationRelative relative = ANYWHERE;
	};

	struct PopState
	{
		PopState() :
			m_SpellsEnabled           ("tf_spells_enabled"),
			m_GrapplingHook           ("tf_grapplinghook_enable"),
			m_RespecEnabled           ("tf_mvm_respec_enabled"),
			m_RespecLimit             ("tf_mvm_respec_limit"),
			m_BonusRatioHalf          ("tf_mvm_currency_bonus_ratio_min"),
			m_BonusRatioFull          ("tf_mvm_currency_bonus_ratio_max"),
			m_FixedBuybacks           ("tf_mvm_buybacks_method"),
			m_BuybacksPerWave         ("tf_mvm_buybacks_per_wave"),
			m_DeathPenalty            ("tf_mvm_death_penalty"),
			m_SentryBusterFriendlyFire("tf_bot_suicide_bomb_friendly_fire"),
			m_BotPushaway             ("tf_avoidteammates_pushaway"),
			m_HumansMustJoinTeam      ("sig_mvm_jointeam_blue_allow_force"),
			m_AllowJoinTeamBlue       ("sig_mvm_jointeam_blue_allow"),
			m_AllowJoinTeamBlueMax    ("sig_mvm_jointeam_blue_allow_max"),
			m_BluHumanFlagPickup      ("sig_mvm_bluhuman_flag_pickup"),
			m_BluHumanFlagCapture     ("sig_mvm_bluhuman_flag_capture"),
			m_BluHumanInfiniteCloak   ("sig_mvm_bluhuman_infinite_cloak"),
			m_BluHumanInfiniteAmmo    ("sig_mvm_bluhuman_infinite_ammo"),
			m_SetCreditTeam           ("sig_mvm_set_credit_team"),
			m_EnableDominations       ("sig_mvm_dominations"),
			m_RobotLimit              ("sig_mvm_robot_limit_override"),
			m_BotsHumans              ("sig_mvm_bots_are_humans"),
			m_ForceHoliday            ("tf_forced_holiday"),
			m_VanillaMode             ("sig_vanilla_mode"),
			m_BodyPartScaleSpeed      ("sig_mvm_body_scale_speed"),
			m_SandmanStuns      	  ("sig_mvm_stunball_stun"),
			m_MedigunShieldDamage     ("sig_mvm_medigunshield_damage"),
			m_StandableHeads      	  ("sig_robot_standable_heads"),
			m_NoRomevisionCosmetics   ("sig_no_romevision_cosmetics"),
			m_CreditsBetterRadiusCollection   ("sig_credits_better_radius_collection"),
			m_AimTrackingIntervalMultiplier   ("sig_head_tracking_interval_multiplier"),
			m_ImprovedAirblast                ("sig_bot_improved_airblast"),
			m_FlagCarrierMovementPenalty      ("tf_mvm_bot_flag_carrier_movement_penalty"),
			m_TextPrintSpeed                  ("sig_text_print_speed"),
			m_AllowSpectators                 ("mp_allowspectators"),
			m_TeleporterUberDuration          ("tf_mvm_engineer_teleporter_uber_duration"),
			m_BluHumanTeleport                ("sig_mvm_bluhuman_teleport"),
			m_BluHumanTeleportPlayer          ("sig_mvm_bluhuman_teleport_player"),
			m_BotsUsePlayerTeleporters        ("sig_mvm_bots_use_teleporters"),
			m_RedTeamMaxPlayers               ("sig_mvm_red_team_max_players"),
			m_MaxSpeedEnable                  ("sig_etc_override_speed_limit"),
			m_MaxSpeedLimit                   ("sig_etc_override_speed_limit_value"),
			m_MaxVelocity                     ("sv_maxvelocity"),
			m_BotRunFast                      ("sig_bot_runfast"),
			m_BotRunFastJump                  ("sig_bot_runfast_allowjump"),
			m_BotRunFastUpdate                ("sig_bot_runfast_fast_update"),
			m_ConchHealthOnHitRegen           ("tf_dev_health_on_damage_recover_percentage"),
			m_MarkOnDeathLifetime             ("tf_dev_marked_for_death_lifetime"),
			m_VacNumCharges                   ("weapon_medigun_resist_num_chunks"),
			m_DoubleDonkWindow                ("tf_double_donk_window"),
			m_ConchSpeedBoost                 ("tf_whip_speed_increase"),
			m_StealthDamageReduction          ("tf_stealth_damage_reduction"),
			m_AllowFlagCarrierToFight         ("tf_mvm_bot_allow_flag_carrier_to_fight"),
			m_HealOnKillOverhealMelee         ("sig_attr_healonkill_overheal_melee"),
			m_MaxActiveZombie                 ("tf_max_active_zombie"),
			m_HHHAttackRange                  ("tf_halloween_bot_attack_range"),
			m_HHHChaseRange                   ("tf_halloween_bot_chase_range"),
			m_HHHChaseDuration                ("tf_halloween_bot_chase_range"),
			m_HHHQuitRange                    ("tf_halloween_bot_quit_range"),
			m_HHHTerrifyRange                 ("tf_halloween_bot_terrify_radius"),
			m_HHHHealthBase                   ("tf_halloween_bot_health_base"),
			m_HHHHealthPerPlayer              ("tf_halloween_bot_health_per_player"),
			m_ForceRobotBleed                 ("sig_mvm_bots_bleed"),
			m_BotHumansHaveRobotVoice         ("sig_mvm_human_bots_robot_voice"),
			m_BotHumansHaveEyeGlow            ("sig_mvm_human_eye_particle"),
			m_EyeParticle                     ("sig_mvm_eye_particle"),
			m_BotEscortCount                  ("tf_bot_flag_escort_max_count"),
			m_CustomAttrDisplay               ("sig_attr_display"),
			m_Accelerate                      ("sv_accelerate"),
			m_AirAccelerate                   ("sv_airaccelerate"),
			m_TurboPhysics                    ("sv_turbophysics"),
			m_UpgradeStationRegenCreators     ("sig_mvm_upgradestation_creators"),
			m_UpgradeStationRegen             ("sig_mvm_upgradestation_regen_improved"),
			m_AllowBluePlayerReanimators      ("sig_mvm_jointeam_blue_allow_revive"),
			m_BluVelocityRemoveLimit          ("sig_mvm_blu_velocity_limit_remove"),
			m_FastEntityNameLookup            ("sig_etc_fast_entity_name_lookup"),
			m_AllowMultipleSappers            ("sig_mvm_sapper_allow_multiple_active"),
			m_EngineerPushRange               ("sig_ai_engiebot_pushrange"),
			m_FixHuntsmanDamageBonus          ("sig_etc_huntsman_damage_fix"),
			m_DefaultBossScale                ("tf_mvm_miniboss_scale"),
			m_FastWholeMapTriggers            ("sig_pop_pointtemplate_fast_whole_map_trigger"),
			m_BluHumanSpawnNoShoot            ("sig_mvm_bluhuman_spawn_noshoot"),
			m_BluHumanSpawnProtection         ("sig_mvm_bluhuman_spawn_protection"),
			m_TvEnable                        ("tv_enable"),
			m_AllowBotsExtraSlots             ("sig_etc_extra_player_slots_allow_bots"),
			m_AutoWeaponStrip                 ("sig_auto_weapon_strip"),
			m_RemoveOffhandViewmodel          ("sig_etc_entity_limit_manager_viewmodel"),
			m_RemoveBotExpressions            ("sig_etc_entity_limit_manager_remove_expressions"),
			m_ExtraBotSlotsNoDeathcam         ("sig_etc_extra_player_slots_no_death_cam"),
			m_NoRobotFootsteps                ("sig_mvm_jointeam_blue_no_footsteps"),
			m_SentryHintBombForwardRange      ("tf_bot_engineer_mvm_sentry_hint_bomb_forward_range"),
			m_SentryHintBombBackwardRange     ("tf_bot_engineer_mvm_sentry_hint_bomb_backward_range"),
			m_SentryHintMinDistanceFromBomb   ("tf_bot_engineer_mvm_hint_min_distance_from_bomb"),
			m_SendBotsToSpectatorImmediately  ("sig_send_bots_to_spectator_immediately"),
			m_PathTrackIsServerEntity         ("sig_etc_path_track_is_server_entity"),
			m_FastWholeMapTriggersAll         ("sig_pop_pointtemplate_fast_whole_map_trigger_all"),

			m_CustomUpgradesFile              ("sig_mvm_custom_upgrades_file")
			
		{
			this->Reset();
		}
		
		void Reset()
		{
			this->m_bGiantsDropRareSpells   = false;
			this->m_flSpellDropRateCommon   = 1.00f;
			this->m_flSpellDropRateGiant    = 1.00f;
			this->m_iSpellDropForTeam       = 0;
			this->m_bNoReanimators          = false;
			this->m_bNoMvMDeathTune         = false;
			this->m_bSniperHideLasers       = false;
			this->m_bSniperAllowHeadshots   = false;
			this->m_bRedSniperNoHeadshots   = false;
			this->m_bDisableUpgradeStations = false;
			this->m_bRedPlayersRobots       = false;
			this->m_bBluPlayersRobots       = false;
			this->m_flRemoveGrapplingHooks  = -1.0f;
			this->m_bReverseWinConditions   = false;
			this->m_bDeclaredClassAttrib    = false;
			this->m_bFixSetCustomModelInput = false;
			this->m_bBotRandomCrits = false;
			this->m_bRedBotNoRandomCrits = false;
			this->m_bSingleClassAllowed = -1;
			this->m_bNoHolidayHealthPack = false;
			this->m_bSpyNoSapUnownedBuildings = false;
			this->m_bFixedRespawnWaveTimeBlue = false;
			this->m_bDisplayRobotDeathNotice = false;
			this->m_flRespawnWaveTimeBlue = -1.0f;
			//this->m_iRedTeamMaxPlayers = 0;
			this->m_iTotalMaxPlayers = 0;
			this->m_iTotalSpectators = -1;
			this->m_bPlayerMinigiant = false;
			this->m_fPlayerScale = -1.0f;
			this->m_iPlayerMiniBossMinRespawnTime = -1;
			this->m_bPlayerRobotUsePlayerAnimation = false;
			this->m_iEscortBotCountOffset = 0;
			this->m_bNoThrillerTaunt = false;
			this->m_bNoCritPumpkin = false;
			this->m_bNoMissionInfo = false;
			this->m_flRestartRoundTime = -1.0f;
			this->m_bNoCountdownSounds = false;
			this->m_bNoCritNoWin = false;
			this->m_bZombiesNoWave666 = false;
			this->m_bFastNPCUpdate = false;
			this->m_bNoRespawnMidwave = false;
			this->m_bHHHNoControlPointLogic = false;
			this->m_bMinibossSentrySingleKill = false;
			this->m_bExtendedUpgradesOnly = false;
			this->m_bExtendedUpgradesNoUndo = false;
			this->m_bHHHNonSolidToPlayers = false;
			this->m_iBunnyHop = 0;
			this->m_bNoSkeletonSplit = false;
			this->m_fStuckTimeMult = 1.0f;
			this->m_bNoCreditsVelocity = false;
			this->m_bRestoreNegativeDamageHealing = true;
			this->m_bRestoreNegativeDamageOverheal = true;
			this->m_bExtraLoadoutItemsAllowEquipOutsideSpawn = false;
            this->m_bNoWranglerShield = false;
			this->m_bForceRedMoney = false;
			this->m_bSpellbookRateSet = false;
			this->m_iEnemyTeamForReverse = TF_TEAM_RED;
			this->m_iRobotLimitSetByMission = 22;
			
			this->m_MedievalMode            .Reset();
			this->m_SpellsEnabled           .Reset();
			this->m_GrapplingHook           .Reset();
			this->m_RespecEnabled           .Reset();
			this->m_RespecLimit             .Reset();
			this->m_BonusRatioHalf          .Reset();
			this->m_BonusRatioFull          .Reset();
			this->m_FixedBuybacks           .Reset();
			this->m_BuybacksPerWave         .Reset();
			this->m_DeathPenalty            .Reset();
			this->m_SentryBusterFriendlyFire.Reset();
			this->m_BotPushaway             .Reset();
			this->m_HumansMustJoinTeam      .Reset();
			this->m_BotsHumans      .Reset();
			this->m_ForceHoliday      .Reset();
			
			this->m_AllowJoinTeamBlue   .Reset();
			this->m_AllowJoinTeamBlueMax.Reset();
			this->m_BluHumanFlagPickup  .Reset();
			this->m_BluHumanFlagCapture .Reset();
			this->m_BluHumanInfiniteCloak.Reset();
			this->m_BluHumanInfiniteAmmo.Reset();
			this->m_SetCreditTeam       .Reset();
			this->m_EnableDominations   .Reset();
			this->m_RobotLimit          .Reset();
			this->m_VanillaMode          .Reset();
			this->m_BodyPartScaleSpeed   .Reset();
			this->m_SandmanStuns        .Reset();
			this->m_StandableHeads      .Reset();
			this->m_MedigunShieldDamage .Reset();
			this->m_NoRomevisionCosmetics.Reset();
			this->m_CreditsBetterRadiusCollection.Reset();
			this->m_AimTrackingIntervalMultiplier.Reset();
			this->m_ImprovedAirblast   .Reset();
			this->m_FlagCarrierMovementPenalty   .Reset();
			this->m_AllowSpectators.Reset();
			this->m_TeleporterUberDuration.Reset();
			this->m_BluHumanTeleport.Reset();
			this->m_BluHumanTeleportPlayer.Reset();
			this->m_BotsUsePlayerTeleporters.Reset();
			this->m_RedTeamMaxPlayers.Reset();
			this->m_MaxSpeedEnable.Reset();
			this->m_MaxSpeedLimit.Reset();
			this->m_BotRunFast.Reset();
			this->m_BotRunFastJump.Reset();
			this->m_BotRunFastUpdate.Reset();
			this->m_MaxVelocity.Reset();
			this->m_ConchHealthOnHitRegen.Reset();
			this->m_MarkOnDeathLifetime.Reset();
			this->m_VacNumCharges.Reset();
			this->m_DoubleDonkWindow.Reset();
			this->m_ConchSpeedBoost.Reset();
			this->m_StealthDamageReduction.Reset();
			this->m_AllowFlagCarrierToFight.Reset();
			this->m_HealOnKillOverhealMelee.Reset();
			this->m_MaxActiveZombie.Reset();
			this->m_HHHChaseDuration.Reset();
			this->m_HHHChaseRange.Reset();
			this->m_HHHHealthBase.Reset();
			this->m_HHHQuitRange.Reset();
			this->m_HHHAttackRange.Reset();
			this->m_HHHHealthPerPlayer.Reset();
			this->m_HHHTerrifyRange.Reset();
			this->m_ForceRobotBleed.Reset();
			this->m_BotHumansHaveRobotVoice.Reset();
			this->m_BotHumansHaveEyeGlow.Reset();
			this->m_EyeParticle.Reset();
			this->m_BotEscortCount.Reset();
			this->m_CustomAttrDisplay.Reset();
			this->m_Accelerate.Reset();
			this->m_AirAccelerate.Reset();
			this->m_TurboPhysics.Reset();
			this->m_UpgradeStationRegenCreators.Reset();
			this->m_UpgradeStationRegen.Reset();
			this->m_AllowBluePlayerReanimators.Reset();
			this->m_BluVelocityRemoveLimit.Reset();
			this->m_FastEntityNameLookup.Reset();
			this->m_AllowMultipleSappers.Reset();
			this->m_EngineerPushRange.Reset();
			this->m_FixHuntsmanDamageBonus.Reset();
			this->m_DefaultBossScale.Reset();
			this->m_FastWholeMapTriggers.Reset();
			this->m_BluHumanSpawnNoShoot.Reset();
			this->m_BluHumanSpawnProtection.Reset();
			this->m_TvEnable.Reset();
			this->m_AllowBotsExtraSlots.Reset();
			this->m_AutoWeaponStrip.Reset();
			this->m_RemoveOffhandViewmodel.Reset();
			this->m_RemoveBotExpressions.Reset();
			this->m_ExtraBotSlotsNoDeathcam.Reset();
			this->m_NoRobotFootsteps.Reset();
			this->m_SentryHintBombForwardRange.Reset();
			this->m_SentryHintBombBackwardRange.Reset();
			this->m_SentryHintMinDistanceFromBomb.Reset();
			this->m_SendBotsToSpectatorImmediately.Reset();
			this->m_PathTrackIsServerEntity.Reset();
			this->m_FastWholeMapTriggersAll.Reset();
			
			this->m_CustomUpgradesFile.Reset();
			this->m_TextPrintSpeed.Reset();
			
			this->m_DisableSounds   .clear();
			this->m_OverrideSounds  .clear();
			this->m_ItemWhitelist   .clear();
			this->m_ItemBlacklist   .clear();
			this->m_ItemAttributes  .clear();
			this->m_PlayerAttributes.clear();
			this->m_PlayerAddCond   .clear();
			this->m_CustomWeapons   .clear();
			this->m_SprayDecals     .clear();
			this->m_Player_anim_cosmetics.clear();

			for (int i=0; i < 10; i++)
				this->m_DisallowedClasses[i] = -1;

			for (int i=0; i < 10; i++)
				this->m_PlayerAttributesClass[i].clear();

			for (int i=0; i < 10; i++)
				this->m_PlayerAddCondClass[i].clear();

			this->m_ForceItems.Clear();
		//	this->m_DisallowedItems.clear();
			
			this->m_FlagResetTimes.clear();
			
			for (CTFTeamSpawn *spawn : this->m_ExtraSpawnPoints) {
				if (spawn != nullptr) {
					spawn->Remove();
				}
			}
			this->m_ExtraSpawnPoints.clear();
			
			this->m_ExtraTankPaths.clear();

			this->m_PlayerSpawnTemplates.clear();
			this->m_PlayerSpawnOnceTemplates.clear();
			this->m_PlayerSpawnOnceTemplatesAppliedTo.clear();
			this->m_ShootTemplates.clear();
			this->m_WeaponSpawnTemplates.clear();
			this->m_ItemEquipTemplates.clear();

			this->m_SpellBookNormalRoll.clear();
			this->m_SpellBookRareRoll.clear();
			this->m_SpellBookNextRollTier.clear();

			this->m_DisallowedUpgrades.clear();
			this->m_DisallowedUpgradesExtra.clear();

			for (auto &replace : this->m_ItemReplace) {
				CEconItemView::Destroy(replace.item);
			}
			this->m_ItemReplace.clear();
			this->m_Description.clear();
			
			for (auto &item : this->m_ExtraLoadoutItems) {
				CEconItemView::Destroy(item.item);
			}
			this->m_ExtraLoadoutItems.clear();
			this->m_ExtraLoadoutItemsNotify = false;

			this->m_PlayerMissionInfoSend.clear();
			
			Clear_Point_Templates();

			this->m_CustomNavFile = "";
			this->m_LastMissionName = "";

			for (auto &val : this->m_HandModelOverride) {
				val = "";
			}

			this->m_ParticleOverride.clear();
			this->m_BuildingPointTemplates.clear();
			this->m_Scripts.clear();
			this->m_ScriptFiles.clear();
			this->m_SpawnLocations.clear();
		}
		
		bool  m_bGiantsDropRareSpells;
		float m_flSpellDropRateCommon;
		float m_flSpellDropRateGiant;
		int   m_iSpellDropForTeam;
		bool  m_bNoReanimators;
		bool  m_bNoMvMDeathTune;
		bool  m_bSniperHideLasers;
		bool  m_bSniperAllowHeadshots;
		bool  m_bDisableUpgradeStations;
		bool  m_bRedPlayersRobots;
		bool  m_bBluPlayersRobots;
		float m_flRemoveGrapplingHooks;
		bool  m_bReverseWinConditions;
		bool m_bDeclaredClassAttrib;
		bool m_bFixSetCustomModelInput;
		bool m_bBotRandomCrits;
		int m_bSingleClassAllowed;
		bool m_bNoHolidayHealthPack;
		bool m_bSpyNoSapUnownedBuildings;
		bool m_bDisplayRobotDeathNotice;
		float m_flRespawnWaveTimeBlue;
		float m_bFixedRespawnWaveTimeBlue;
		//float m_iRedTeamMaxPlayers;
		float m_iTotalMaxPlayers;
		bool m_bPlayerMinigiant;
		float m_fPlayerScale;
		int m_iTotalSpectators;
		int m_iPlayerMiniBossMinRespawnTime;
		bool m_bPlayerRobotUsePlayerAnimation;
		int m_iEscortBotCountOffset;
		bool m_bNoThrillerTaunt;
		bool m_bNoCritPumpkin;
		bool m_bNoMissionInfo;
		float m_flRestartRoundTime;
		bool m_bNoCountdownSounds;
		bool m_bNoCritNoWin;
		bool m_bZombiesNoWave666;
		bool m_bFastNPCUpdate;
		bool m_bNoRespawnMidwave;
		bool m_bHHHNoControlPointLogic;
		bool m_bForceRobotBleed;
		bool m_bMinibossSentrySingleKill;
		bool m_bExtendedUpgradesOnly;
		bool m_bExtendedUpgradesNoUndo;
		bool m_bHHHNonSolidToPlayers;
		int m_iBunnyHop;
		bool m_bNoSkeletonSplit;
		float m_fStuckTimeMult;
		bool m_bNoCreditsVelocity;
		bool m_bRestoreNegativeDamageHealing;
		bool m_bRestoreNegativeDamageOverheal;
		bool m_bExtraLoadoutItemsAllowEquipOutsideSpawn;
        bool m_bNoWranglerShield;
		bool m_bRemoveOffhandViewmodel;
		bool m_bRedBotNoRandomCrits;
		bool m_bRedSniperNoHeadshots;
		bool m_bForceRedMoney;
		bool m_bSpellbookRateSet;
		int m_iEnemyTeamForReverse;
		int m_iRobotLimitSetByMission;
		
		CPopOverride_MedievalMode        m_MedievalMode;
		CPopOverride_ConVar<bool>        m_SpellsEnabled;
		CPopOverride_ConVar<bool>        m_GrapplingHook;
		CPopOverride_ConVar<bool>        m_RespecEnabled;
		CPopOverride_ConVar<int>         m_RespecLimit;
		CPopOverride_ConVar<float>       m_BonusRatioHalf;
		CPopOverride_ConVar<float>       m_BonusRatioFull;
		CPopOverride_ConVar<bool>        m_FixedBuybacks;
		CPopOverride_ConVar<int>         m_BuybacksPerWave;
		CPopOverride_ConVar<int>         m_DeathPenalty;
		CPopOverride_ConVar<bool>        m_SentryBusterFriendlyFire;
		CPopOverride_ConVar<bool>        m_BotPushaway;
		CPopOverride_ConVar<bool> m_HumansMustJoinTeam;
		
		CPopOverride_ConVar<bool> m_AllowJoinTeamBlue;
		CPopOverride_ConVar<int>  m_AllowJoinTeamBlueMax;
		CPopOverride_ConVar<bool> m_BluHumanFlagPickup;
		CPopOverride_ConVar<bool> m_BluHumanFlagCapture;
		CPopOverride_ConVar<bool> m_BluHumanInfiniteCloak;
		CPopOverride_ConVar<bool> m_BluHumanInfiniteAmmo;
		CPopOverride_ConVar<int>  m_SetCreditTeam;
		CPopOverride_ConVar<bool> m_EnableDominations;
		CPopOverride_ConVar<int>  m_RobotLimit;
		CPopOverride_ConVar<int> m_BotsHumans;
		CPopOverride_ConVar<int> m_ForceHoliday;
		CPopOverride_ConVar<bool> m_VanillaMode;
		CPopOverride_ConVar<float> m_BodyPartScaleSpeed;
		CPopOverride_ConVar<bool> m_SandmanStuns;
		CPopOverride_ConVar<bool> m_MedigunShieldDamage;
		CPopOverride_ConVar<bool> m_StandableHeads;
		CPopOverride_ConVar<bool> m_NoRomevisionCosmetics;
		CPopOverride_ConVar<bool> m_CreditsBetterRadiusCollection;
		CPopOverride_ConVar<float> m_AimTrackingIntervalMultiplier;
		CPopOverride_ConVar<bool> m_ImprovedAirblast;
		CPopOverride_ConVar<float> m_FlagCarrierMovementPenalty;
		CPopOverride_ConVar<float> m_TextPrintSpeed;
		CPopOverride_ConVar<bool> m_AllowSpectators;
		CPopOverride_ConVar<float> m_TeleporterUberDuration;
		CPopOverride_ConVar<bool> m_BluHumanTeleport;
		CPopOverride_ConVar<bool> m_BluHumanTeleportPlayer;
		CPopOverride_ConVar<bool> m_BotsUsePlayerTeleporters;
		CPopOverride_ConVar<int> m_RedTeamMaxPlayers;
		CPopOverride_ConVar<bool> m_MaxSpeedEnable;
		CPopOverride_ConVar<float> m_MaxSpeedLimit;
		CPopOverride_ConVar<float> m_MaxVelocity;
		CPopOverride_ConVar<bool> m_BotRunFast;
		CPopOverride_ConVar<bool> m_BotRunFastJump;
		CPopOverride_ConVar<bool> m_BotRunFastUpdate;
		CPopOverride_ConVar<float> m_ConchHealthOnHitRegen;
		CPopOverride_ConVar<float> m_MarkOnDeathLifetime;
		CPopOverride_ConVar<int> m_VacNumCharges;
		CPopOverride_ConVar<float> m_DoubleDonkWindow;
		CPopOverride_ConVar<float> m_ConchSpeedBoost;
		CPopOverride_ConVar<float> m_StealthDamageReduction;
		CPopOverride_ConVar<bool> m_AllowFlagCarrierToFight;
		CPopOverride_ConVar<bool> m_HealOnKillOverhealMelee;
		CPopOverride_ConVar<int> m_MaxActiveZombie;
		CPopOverride_ConVar<float> m_HHHAttackRange;
		CPopOverride_ConVar<float> m_HHHChaseRange;
		CPopOverride_ConVar<float> m_HHHChaseDuration;
		CPopOverride_ConVar<float> m_HHHQuitRange;
		CPopOverride_ConVar<float> m_HHHTerrifyRange;
		CPopOverride_ConVar<int> m_HHHHealthBase;
		CPopOverride_ConVar<int> m_HHHHealthPerPlayer;
		CPopOverride_ConVar<bool> m_ForceRobotBleed;
		CPopOverride_ConVar<bool> m_BotHumansHaveRobotVoice;
		CPopOverride_ConVar<bool> m_BotHumansHaveEyeGlow;
		CPopOverride_ConVar<std::string> m_EyeParticle;
		CPopOverride_ConVar<int> m_BotEscortCount;
		CPopOverride_ConVar<bool> m_CustomAttrDisplay;
		CPopOverride_ConVar<float> m_Accelerate;
		CPopOverride_ConVar<float> m_AirAccelerate;
		CPopOverride_ConVar<bool> m_TurboPhysics;
		CPopOverride_ConVar<bool> m_UpgradeStationRegenCreators;
		CPopOverride_ConVar<bool> m_UpgradeStationRegen;
		CPopOverride_ConVar<bool> m_AllowBluePlayerReanimators;
		CPopOverride_ConVar<bool> m_BluVelocityRemoveLimit;
		CPopOverride_ConVar<bool> m_FastEntityNameLookup;
		CPopOverride_ConVar<bool> m_AllowMultipleSappers;
		CPopOverride_ConVar<float> m_EngineerPushRange;
		CPopOverride_ConVar<bool> m_FixHuntsmanDamageBonus;
		CPopOverride_ConVar<float> m_DefaultBossScale;
		CPopOverride_ConVar<bool> m_FastWholeMapTriggers;
		CPopOverride_ConVar<bool> m_BluHumanSpawnNoShoot;
		CPopOverride_ConVar<bool> m_BluHumanSpawnProtection;
		CPopOverride_ConVar<bool> m_TvEnable;
		CPopOverride_ConVar<bool> m_AllowBotsExtraSlots;
		CPopOverride_ConVar<bool> m_AutoWeaponStrip;
		CPopOverride_ConVar<bool> m_RemoveOffhandViewmodel;
		CPopOverride_ConVar<bool> m_RemoveBotExpressions;
		CPopOverride_ConVar<bool> m_ExtraBotSlotsNoDeathcam;
		CPopOverride_ConVar<bool> m_NoRobotFootsteps;
		CPopOverride_ConVar<float> m_SentryHintBombForwardRange;
		CPopOverride_ConVar<float> m_SentryHintBombBackwardRange;
		CPopOverride_ConVar<float> m_SentryHintMinDistanceFromBomb;
		CPopOverride_ConVar<bool> m_SendBotsToSpectatorImmediately;
		CPopOverride_ConVar<bool> m_PathTrackIsServerEntity;
		CPopOverride_ConVar<float> m_FastWholeMapTriggersAll;		
		
		
		//CPopOverride_CustomUpgradesFile m_CustomUpgradesFile;
		CPopOverride_ConVar<std::string> m_CustomUpgradesFile;
		
		std::vector<PointTemplateInfo>				m_SpawnTemplates;
		std::vector<PlayerPointTemplateInfo>        m_PlayerSpawnTemplates;
		std::vector<PointTemplateInfo>              m_PlayerSpawnOnceTemplates;
		std::set<CHandle<CTFPlayer>>                m_PlayerSpawnOnceTemplatesAppliedTo;
		std::vector<ShootTemplateData>              m_ShootTemplates;
		std::vector<WeaponPointTemplateInfo>        m_WeaponSpawnTemplates;

		std::set<std::string>                       m_DisableSounds;
		std::map<std::string,std::string>           m_OverrideSounds;
		std::vector<std::unique_ptr<ItemListEntry>> m_ItemWhitelist;
		std::vector<std::unique_ptr<ItemListEntry>> m_ItemBlacklist;
		std::vector<ItemAttributes>                 m_ItemAttributes;
		std::vector<SprayDecal>                     m_SprayDecals;
	//	std::set<int>                               m_DisallowedItems;
		
		std::map<std::string, int> m_FlagResetTimes;
		
		std::vector<CHandle<CTFTeamSpawn>> m_ExtraSpawnPoints;
		
		std::vector<ExtraTankPath> m_ExtraTankPaths;

		std::unordered_set<CTFPlayer*> m_PlayerUpgradeSend;

		int m_DisallowedClasses[11];

		std::map<std::string,float> m_PlayerAttributes;
		std::map<std::string,float> m_PlayerAttributesClass[11] = {};
		std::vector<ETFCond> m_PlayerAddCond;
		std::vector<ETFCond> m_PlayerAddCondClass[11] = {};
		ForceItems m_ForceItems;


		std::map<std::string, CustomWeapon> m_CustomWeapons;

		std::map<CHandle<CTFPlayer>, CHandle<CEconWearable>> m_Player_anim_cosmetics;
		std::unordered_map<CBaseEntity *, std::shared_ptr<PointTemplateInstance>> m_ItemEquipTemplates;

		std::unordered_set<std::string> m_MissingRobotBones[11];
		string_t m_CachedRobotModelIndex[22];

		std::vector<std::pair<int, int>> m_SpellBookNormalRoll;
		std::vector<std::pair<int, int>> m_SpellBookRareRoll;
		std::unordered_map<CBaseEntity *, int> m_SpellBookNextRollTier;

		std::vector<std::string> m_DisallowedUpgrades;
		std::vector<DisallowUpgradeEntry> m_DisallowedUpgradesExtra;
		std::vector<ItemReplace> m_ItemReplace;
		std::vector<std::string> m_Description;
		std::vector<ExtraLoadoutItem> m_ExtraLoadoutItems;
		bool m_ExtraLoadoutItemsNotify;
		std::map<CSteamID, std::set<int>> m_SelectedLoadoutItems;
		std::map<CSteamID, std::set<int>> m_BoughtLoadoutItems;
		std::map<CSteamID, std::set<int>> m_BoughtLoadoutItemsCheckpoint;

		std::unordered_set<CTFPlayer*> m_PlayerMissionInfoSend;

		std::unordered_set<CTFPlayer*> m_PlayersByWaveStart;

		std::string m_CustomNavFile;
		std::string m_LastMissionName;
		
		std::string m_HandModelOverride[11];

		std::unordered_map<std::string, std::string> m_ParticleOverride;

		std::vector<BuildingPointTemplateInfo> m_BuildingPointTemplates;

		std::vector<std::string> m_Scripts;
		std::vector<std::string> m_ScriptFiles;
		CHandle<CBaseEntity> m_ScriptManager;
		std::unordered_map<CSpawnLocation *, SpawnLocationData> m_SpawnLocations;

	};
	PopState state{};
	
	bool ExtendedUpgradesNoUndo(){ // this is very maintainable yes
		return Mod::Pop::PopMgr_Extensions::state.m_bExtendedUpgradesNoUndo;
	}
	
	bool PlayerUsesRobotModel(CTFPlayer *player)
	{
		auto class_shared = player->GetPlayerClass();
		int class_index = class_shared->GetClassIndex();
		string_t player_model = MAKE_STRING(class_shared->GetCustomModel());
		return player_model == state.m_CachedRobotModelIndex[class_index] || player_model == state.m_CachedRobotModelIndex[class_index + 10];
	}

	std::list<CHandle<CBaseAnimating>> item_loading_queue;

	bool IsItemApplicableToRobotModel(CTFPlayer *parent, CBaseAnimating *model_entity, bool check_load)
	{
		if (check_load) {
			int numbones_entity = model_entity->GetNumBones();
			if (modelinfo->IsDynamicModelLoading(model_entity->GetModelIndex()) || numbones_entity == 0) {
				item_loading_queue.push_front(model_entity);
				//THINK_FUNC_SET(model_entity, ItemApplicableCheckDelay, gpGlobals->curtime);
				//model_load_callback.AddCallback(model_entity);
				return true;
			}
		}
		int class_index = parent->GetPlayerClass()->GetClassIndex();

		std::unordered_set<std::string> &bones = state.m_MissingRobotBones[class_index];

		bool is_missing_bone = false;

		for (auto &str : bones) {
			if (model_entity->LookupBone(str.c_str()) != -1) {
				is_missing_bone = true;
				break;
			}
		}

		return !is_missing_bone;
	}
	
	/*bool IsItemApplicableToRobotModel(CTFPlayer *parent, CBaseAnimating *entity)
	{
		if (entity->GetModel() == nullptr)
			return true;

		studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( entity->GetModel() );

		if (pStudioHdr == nullptr) {
			DevMsg("studiohdrnull\n");
			return true;
		}

		return IsItemApplicableToRobotModel(parent, mdlcache->GetStudioHdr(modelinfo->GetCacheHandle(entity->GetModel())));
	}*/
	

	/* HACK: allow MvM:JoinTeam_Blue_Allow to force-off its admin-only functionality if the pop file explicitly set
	 * 'AllowJoinTeamBlue 1' (rather than someone manually setting 'sig_mvm_jointeam_blue_allow 1' via rcon) */
	bool PopFileIsOverridingJoinTeamBlueConVarOn()
	{
		return (state.m_AllowJoinTeamBlue.IsOverridden() && state.m_AllowJoinTeamBlue.Get());
	}
	
	const char *GetCustomWeaponNameOverride(const char *name)
	{
		if (!state.m_CustomWeapons.empty() && name != nullptr) {
			std::string namestr = name;
			auto entry = state.m_CustomWeapons.find(namestr);
			if (entry != state.m_CustomWeapons.end()) {
				return entry->second.originalId->GetName(name);
			}
		}
		return name;
	}

	CTFItemDefinition *GetCustomWeaponItemDef(std::string name)
	{
		auto entry = state.m_CustomWeapons.find(name);
		if (entry != state.m_CustomWeapons.end()) {
			return entry->second.originalId;
		}
		return nullptr;
	}

	bool AddCustomWeaponAttributes(std::string name, CEconItemView *view)
	{
		auto entrywep = state.m_CustomWeapons.find(name);
		if (entrywep != state.m_CustomWeapons.end()) {
			for (auto& entry : entrywep->second.attributes) {
				view->GetAttributeList().AddStringAttribute(entry.first, entry.second);
			}
		}
		return false;
	}

	void SaveStateInfoBetweenMissions()
	{
		Mod::Etc::Mapentity_Additions::change_level_info.selectedLoadoutItems = state.m_SelectedLoadoutItems;
		Mod::Etc::Mapentity_Additions::change_level_info.boughtLoadoutItems = state.m_BoughtLoadoutItems;
		Mod::Etc::Mapentity_Additions::change_level_info.loadoutItemsCount = state.m_ExtraLoadoutItems.size();
	}

	void RestoreStateInfoBetweenMissions()
	{
		// Only restore selected loadout items if there are enough loadout items in the target mission
		if (state.m_ExtraLoadoutItems.size() >= Mod::Etc::Mapentity_Additions::change_level_info.loadoutItemsCount) {
			state.m_SelectedLoadoutItems = Mod::Etc::Mapentity_Additions::change_level_info.selectedLoadoutItems;
			state.m_BoughtLoadoutItems = Mod::Etc::Mapentity_Additions::change_level_info.boughtLoadoutItems;
		}
	}

	bool bot_killed_check = false;
	RefCount rc_CTFGameRules_PlayerKilled;
	CBasePlayer *killed = nullptr;
	DETOUR_DECL_MEMBER(void, CTFGameRules_PlayerKilled, CBasePlayer *pVictim, const CTakeDamageInfo& info)
	{
	//	DevMsg("CTFGameRules::PlayerKilled\n");
		killed = pVictim;
		bot_killed_check = cvar_send_bots_to_spectator_immediately.GetBool();
		SCOPED_INCREMENT(rc_CTFGameRules_PlayerKilled);
		DETOUR_MEMBER_CALL(CTFGameRules_PlayerKilled)(pVictim, info);
		if (state.m_bMinibossSentrySingleKill) {
			auto object = ToBaseObject(info.GetAttacker());
			if (object == nullptr)
				object = ToBaseObject(info.GetInflictor());

			if (object != nullptr && IsMannVsMachineMode() && pVictim != nullptr && ToTFPlayer(pVictim)->IsMiniBoss()) {
				object->m_iKills -= 4;
			}
		}
	}
	
	DETOUR_DECL_MEMBER(bool, CTFGameRules_ShouldDropSpellPickup)
	{
	//	DevMsg("CTFGameRules::ShouldDropSpellPickup\n");
		
		if (IsMannVsMachineMode() && rc_CTFGameRules_PlayerKilled > 0) {
			CTFBot *bot = ToTFBot(killed);
			if (bot == nullptr) return false;
			if (!state.m_SpellsEnabled.Get()) return false;
			if (state.m_iSpellDropForTeam != 0 && bot->GetTeamNumber() != state.m_iSpellDropForTeam) return false;

			float rnd  = RandomFloat(0.0f, 1.0f);
			float rate = (bot->IsMiniBoss() ? state.m_flSpellDropRateGiant : state.m_flSpellDropRateCommon);
			
			if (rnd > rate) {
	//			DevMsg("  %.3f > %.3f, returning false\n", rnd, rate);
				return false;
			}
			
	//		DevMsg("  %.3f <= %.3f, returning true\n", rnd, rate);
			return true;
		}
		
		return DETOUR_MEMBER_CALL(CTFGameRules_ShouldDropSpellPickup)();
	}
	
	DETOUR_DECL_MEMBER(void, CTFGameRules_DropSpellPickup, const Vector& where, int tier)
	{
	//	DevMsg("CTFGameRules::DropSpellPickup\n");
		
		if (IsMannVsMachineMode() && rc_CTFGameRules_PlayerKilled > 0) {
			CTFBot *bot = ToTFBot(killed);
			if (bot != nullptr) {
	//			DevMsg("  is a bot\n");
				if (state.m_bGiantsDropRareSpells && bot->IsMiniBoss()) {
	//				DevMsg("  dropping rare spell for miniboss\n");
					tier = 1;
				}
			}
		}
		
		DETOUR_MEMBER_CALL(CTFGameRules_DropSpellPickup)(where, tier);
	}
	
	DETOUR_DECL_MEMBER(bool, CTFGameRules_IsUsingSpells)
	{
	//	DevMsg("CTFGameRules::IsUsingSpells\n");
		
		if (IsMannVsMachineMode() && rc_CTFGameRules_PlayerKilled > 0) {
	//		DevMsg("  returning true\n");
			return true;
		}
		
		return DETOUR_MEMBER_CALL(CTFGameRules_IsUsingSpells)();
	}
	
	DETOUR_DECL_STATIC(CTFReviveMarker *, CTFReviveMarker_Create, CTFPlayer *player)
	{
		if (state.m_bNoReanimators || TFGameRules()->IsInMedievalMode()) {
			return nullptr;
		}
		
		return DETOUR_STATIC_CALL(CTFReviveMarker_Create)(player);
	}
	
	DETOUR_DECL_MEMBER(void, CTFSniperRifle_CreateSniperDot)
	{
		auto rifle = reinterpret_cast<CTFSniperRifle *>(this);
		
		if (state.m_bSniperHideLasers && IsMannVsMachineMode()) {
			CTFPlayer *owner = rifle->GetTFPlayerOwner();
			if (owner != nullptr && owner->GetTeamNumber() == TF_TEAM_BLUE) {
				return;
			}
		}
		
		DETOUR_MEMBER_CALL(CTFSniperRifle_CreateSniperDot)();
	}
	
	RefCount rc_CTFSniperRifle_CanFireCriticalShot;
	DETOUR_DECL_MEMBER(bool, CTFSniperRifle_CanFireCriticalShot, bool bIsHeadshot, CBaseEntity *ent1)
	{
		SCOPED_INCREMENT(rc_CTFSniperRifle_CanFireCriticalShot);
		return DETOUR_MEMBER_CALL(CTFSniperRifle_CanFireCriticalShot)(bIsHeadshot, ent1);
	}
	
	RefCount rc_CTFRevolver_CanFireCriticalShot;
	DETOUR_DECL_MEMBER(bool, CTFRevolver_CanFireCriticalShot, bool bIsHeadshot, CBaseEntity *ent1)
	{
		SCOPED_INCREMENT(rc_CTFRevolver_CanFireCriticalShot);
		return DETOUR_MEMBER_CALL(CTFRevolver_CanFireCriticalShot)(bIsHeadshot, ent1);
	}
	
	RefCount rc_CTFWeaponBase_CanFireCriticalShot;
	DETOUR_DECL_MEMBER(bool, CTFWeaponBase_CanFireCriticalShot, bool bIsHeadshot, CBaseEntity *ent1)
	{
		SCOPED_INCREMENT_IF(rc_CTFWeaponBase_CanFireCriticalShot, bIsHeadshot);
		
		return DETOUR_MEMBER_CALL(CTFWeaponBase_CanFireCriticalShot)(bIsHeadshot, ent1);
	}
	
	RefCount rc_CTFProjectile_Arrow_StrikeTarget;
	DETOUR_DECL_MEMBER(bool, CTFProjectile_Arrow_StrikeTarget, mstudiobbox_t *bbox, CBaseEntity *ent)
	{
		SCOPED_INCREMENT(rc_CTFProjectile_Arrow_StrikeTarget);
		return DETOUR_MEMBER_CALL(CTFProjectile_Arrow_StrikeTarget)(bbox, ent);
	}
	
	RefCount rc_CTFWeaponBase_CalcIsAttackCritical;
	DETOUR_DECL_MEMBER(void, CTFWeaponBase_CalcIsAttackCritical)
	{
		SCOPED_INCREMENT(rc_CTFWeaponBase_CalcIsAttackCritical);
		DETOUR_MEMBER_CALL(CTFWeaponBase_CalcIsAttackCritical)();
	}

	DETOUR_DECL_MEMBER(bool, CTFGameRules_IsPVEModeControlled, CBaseEntity *ent)
	{
		if ((rc_CTFProjectile_Arrow_StrikeTarget || rc_CTFWeaponBase_CanFireCriticalShot) && state.m_bRedSniperNoHeadshots && IsMannVsMachineMode()) {
			auto player = ToTFPlayer(ent);
			if (player != nullptr && player->IsBot() && player->GetTeamNumber() == TF_TEAM_RED) return true;
		}
		if ((rc_CTFProjectile_Arrow_StrikeTarget || rc_CTFWeaponBase_CanFireCriticalShot) && state.m_bSniperAllowHeadshots && IsMannVsMachineMode()) {
			return false;
		}
		
		if (rc_CTFWeaponBase_CalcIsAttackCritical && state.m_bRedBotNoRandomCrits && IsMannVsMachineMode()) {
			auto player = ToTFPlayer(ent);
			if (player != nullptr && player->IsBot() && player->GetTeamNumber() == TF_TEAM_RED) return true;
		}
		if ( rc_CTFWeaponBase_CalcIsAttackCritical && state.m_bBotRandomCrits && IsMannVsMachineMode()) {
			return false;
		}

		return DETOUR_MEMBER_CALL(CTFGameRules_IsPVEModeControlled)(ent);
	}
	
	DETOUR_DECL_MEMBER(void, CUpgrades_UpgradeTouch, CBaseEntity *pOther)
	{
		CTFPlayer *player = ToTFPlayer(pOther);
		bool was_in_upgrade = player != nullptr && player->m_Shared->m_bInUpgradeZone;

		if (state.m_bDisableUpgradeStations && player != nullptr && IsMannVsMachineMode()) {
			gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, "The Upgrade Station is disabled for this mission!");
			return;
		}
		DETOUR_MEMBER_CALL(CUpgrades_UpgradeTouch)(pOther);
	}
	const char *BroadcastSound(const char *sound)
	{
		if (IsMannVsMachineMode()) {
		//	DevMsg("CTeamplayRoundBasedRules::BroadcastSound(%d, \"%s\", 0x%08x)\n", iTeam, sound, iAdditionalSoundFlags);
			
			if (sound != nullptr && state.m_DisableSounds.count(std::string(sound)) != 0) {
				DevMsg("Blocked sound \"%s\" via CTeamplayRoundBasedRules::BroadcastSound\n", sound);
				return nullptr;
			}
			else if (sound != nullptr && state.m_OverrideSounds.count(std::string(sound)) != 0) {
				DevMsg("Blocked sound \"%s\" via CBaseEntity::EmitSound\n", sound);
				return state.m_OverrideSounds[sound].c_str();
			}
		}
		return sound;
	}

	DETOUR_DECL_MEMBER(void, CTFGameRules_BroadcastSound, int iTeam, const char *sound, int iAdditionalSoundFlags)
	{
		sound = BroadcastSound(sound);
		
		DETOUR_MEMBER_CALL(CTeamplayRoundBasedRules_BroadcastSound)(iTeam, sound, iAdditionalSoundFlags);
	}

	DETOUR_DECL_MEMBER(void, CTeamplayRoundBasedRules_BroadcastSound, int iTeam, const char *sound, int iAdditionalSoundFlags)
	{
		sound = BroadcastSound(sound);
		
		DETOUR_MEMBER_CALL(CTeamplayRoundBasedRules_BroadcastSound)(iTeam, sound, iAdditionalSoundFlags);
	}
	
	void ModifyEmitSound(EmitSound_t& params)
	{
		if (params.m_pSoundName && params.m_pSoundName[0] == '=') {
			char *pos;
			params.m_SoundLevel = (soundlevel_t) strtol(params.m_pSoundName + 1, &pos, 10);
			params.m_pSoundName = pos + 1;
		}
	}

	DETOUR_DECL_STATIC(void, CBaseEntity_EmitSound_static_emitsound, IRecipientFilter& filter, int iEntIndex, EmitSound_t& params)
	{
		static bool callfrom = false;
		if (!callfrom && IsMannVsMachineMode()) {
			const char *sound = params.m_pSoundName;
			if (iEntIndex > 0 && iEntIndex < 34 && strncmp(sound,"mvm/player/footsteps/robostep",26) == 0){
				filter = CReliableBroadcastRecipientFilter();
				edict_t *edict = INDEXENT( iEntIndex );
				if ( edict && !edict->IsFree() )
				{
					CTFPlayer *player = ToTFPlayer(GetContainingEntity(edict));
					if (player != nullptr && !player->IsBot() &&((state.m_bRedPlayersRobots && player->GetTeamNumber() == TF_TEAM_RED) //|| 
							/*(state.m_bBluPlayersRobots && player->GetTeamNumber() == TF_TEAM_BLUE)*/)) {
						//CBaseEntity_EmitSound_static_emitsound
						callfrom = true;
						//DevMsg("CBaseEntity::ModEmitSound(#%d, \"%s\")\n", iEntIndex, sound);
						player->EmitSound("MVM.BotStep");
						return;
					}
				}
				
			}
			if (state.m_bNoMvMDeathTune && sound != nullptr && strcmp(sound, "MVM.PlayerDied") == 0) {
				return;
			}
			
			if (sound != nullptr && state.m_DisableSounds.count(std::string(sound)) != 0) {
				DevMsg("Blocked sound \"%s\" via CBaseEntity::EmitSound\n", sound);
				return;
			}
			else if (sound != nullptr && state.m_OverrideSounds.count(std::string(sound)) != 0) {
				EmitSound_t es;
				es.m_nChannel = params.m_nChannel;
				es.m_pSoundName = state.m_OverrideSounds[sound].c_str();
				es.m_flVolume = params.m_flVolume;
				es.m_SoundLevel = params.m_SoundLevel;
				es.m_nFlags = params.m_nFlags;
				es.m_nPitch = params.m_nPitch;
				ModifyEmitSound(es);
				//CBaseEntity::EmitSound(filter,iEntIndex,es);
			
				DETOUR_STATIC_CALL(CBaseEntity_EmitSound_static_emitsound)(filter, iEntIndex, es);
				DevMsg("Blocked sound \"%s\" via CBaseEntity::EmitSound\n", sound);
				return;
			}
		}
		ModifyEmitSound(params);
		DETOUR_STATIC_CALL(CBaseEntity_EmitSound_static_emitsound)(filter, iEntIndex, params);
		callfrom = false;
	}
	
	DETOUR_DECL_STATIC(void, CBaseEntity_EmitSound_static_emitsound_handle, IRecipientFilter& filter, int iEntIndex, EmitSound_t& params, HSOUNDSCRIPTHANDLE& handle)
	{
		if (IsMannVsMachineMode()) {
			const char *sound = params.m_pSoundName;
			
			//DevMsg("CBaseEntity::EmitSound(#%d, \"%s\", 0x%04x)\n", iEntIndex, sound, (uint16_t)handle);
			
			if (sound != nullptr && state.m_DisableSounds.find(sound) != state.m_DisableSounds.end()) {
				DevMsg("Blocked sound \"%s\" via CBaseEntity::EmitSound\n", sound);
				return;
			}
			else if (sound != nullptr && state.m_OverrideSounds.find(sound) != state.m_OverrideSounds.end()) {
				EmitSound_t es;
				es.m_nChannel = params.m_nChannel;
				es.m_pSoundName = state.m_OverrideSounds[sound].c_str();
				es.m_flVolume = params.m_flVolume;
				es.m_SoundLevel = params.m_SoundLevel;
				es.m_nFlags = params.m_nFlags;
				es.m_nPitch = params.m_nPitch;
				ModifyEmitSound(es);
				DETOUR_STATIC_CALL(CBaseEntity_EmitSound_static_emitsound_handle)(filter, iEntIndex, es, handle);
				DevMsg("Blocked sound \"%s\" via CBaseEntity::EmitSound\n", sound);
				return;
			}
		}
		ModifyEmitSound(params);
		DETOUR_STATIC_CALL(CBaseEntity_EmitSound_static_emitsound_handle)(filter, iEntIndex, params, handle);
	}
	
//	RefCount rc_CTFPlayer_GiveDefaultItems;
//	DETOUR_DECL_MEMBER(void, CTFPlayer_GiveDefaultItems)
//	{
//		SCOPED_INCREMENT(rc_CTFPlayer_GiveDefaultItems);
//		DETOUR_MEMBER_CALL(CTFPlayer_GiveDefaultItems)();
//	}


	void ApplyOrClearRobotModel(CTFPlayer *player)
	{
		if (!state.m_bRedPlayersRobots && !state.m_bBluPlayersRobots)
			return;

		// Restore changes made by player animations on robot models
		auto find = state.m_Player_anim_cosmetics.find(player);
		if (find != state.m_Player_anim_cosmetics.end() && find->second != nullptr) {
			find->second->Remove();
			player->m_nRenderFX = 0;
			//servertools->SetKeyValue(player, "rendermode", "0");
			//player->SetRenderColorA(255);
			state.m_Player_anim_cosmetics.erase(find);
		}

		if ((state.m_bRedPlayersRobots && player->GetTeamNumber() == TF_TEAM_RED) || 
				(state.m_bBluPlayersRobots && player->GetTeamNumber() == TF_TEAM_BLUE)) {

			int model_class=player->GetPlayerClass()->GetClassIndex();
			const char *model = player->IsMiniBoss() ? g_szBotBossModels[model_class] : g_szBotModels[model_class];
			
			if (state.m_bPlayerRobotUsePlayerAnimation) {
				
				auto wearable = static_cast<CTFWearable *>(CreateEntityByName("tf_wearable"));

				//CEconWearable *wearable = static_cast<CEconWearable *>(ItemGeneration()->SpawnItem(PLAYER_ANIM_WEARABLE_ITEM_ID, Vector(0,0,0), QAngle(0,0,0), 6, 9999, "tf_wearable"));
				//DevMsg("Use human anims %d\n", wearable != nullptr);
				if (wearable != nullptr) {
					wearable->Spawn();
					wearable->m_bValidatedAttachedEntity = true;
					wearable->GiveTo(player);
					player->m_nRenderFX = 6;
					//servertools->SetKeyValue(player, "rendermode", "10");
					//player->SetRenderColorA(0);
					int model_index = CBaseEntity::PrecacheModel(model);
					wearable->SetModelIndex(model_index);
					for (int j = 0; j < MAX_VISION_MODES; ++j) {
						wearable->SetModelIndexOverride(j, model_index);
					}
					state.m_Player_anim_cosmetics[player] = wearable;
					player->GetPlayerClass()->SetCustomModel("", true);
					wearable->SetEffects(129);
				}
			}
			else {
				player->GetPlayerClass()->SetCustomModel(model, true);
			}
			player->UpdateModel();
		}
		else {
			player->GetPlayerClass()->SetCustomModel("", true);
			player->UpdateModel();
		}
		
	}

	bool SpyNoSapUnownedBuildings()
	{
		return state.m_bSpyNoSapUnownedBuildings;
	}

	int GetMaxRobotLimit()
	{
		return state.m_iRobotLimitSetByMission;
	}

	
	DETOUR_DECL_STATIC(const char *, TranslateWeaponEntForClass, const char *name, int classIndex)
	{
		auto result = DETOUR_STATIC_CALL(TranslateWeaponEntForClass)(name, classIndex);
		if (result != nullptr && result[0] == '\0') {
			return TranslateWeaponEntForClass_improved(name, classIndex);
		}
		return result;
	}

	RefCount rc_GetEntityForLoadoutSlot;
	DETOUR_DECL_MEMBER(CBaseEntity *, CTFPlayer_GetEntityForLoadoutSlot, int slot, bool flag)
	{
		SCOPED_INCREMENT(rc_GetEntityForLoadoutSlot);
		return DETOUR_MEMBER_CALL(CTFPlayer_GetEntityForLoadoutSlot)(slot, flag);
	}

	RefCount rc_CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot;
    DETOUR_DECL_MEMBER(CEconItemView *, CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, CTFPlayer *player, int slot, CBaseEntity &entity)
	{
        SCOPED_INCREMENT(rc_CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot);
        return DETOUR_MEMBER_CALL(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot)(player, slot, entity);
	}

	RefCount rc_CTFPlayer_GiveDefaultItems;
	std::unordered_set<CEconItemDefinition *> is_item_replacement;
	CEconItemView *item_view_replacement = nullptr;
	DETOUR_DECL_MEMBER(void , CTFPlayer_GiveDefaultItems)
	{
		SCOPED_INCREMENT(rc_CTFPlayer_GiveDefaultItems);
		DETOUR_MEMBER_CALL(CTFPlayer_GiveDefaultItems)();
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (!player->IsBot()) {
			ApplyOrClearRobotModel(player);
		}
		is_item_replacement.clear();
	}

	int LoadoutSlotReplace(int slot, CTFItemDefinition *item_def, int classIndex) 
	{
		if (rc_GetEntityForLoadoutSlot || rc_CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot || (rc_CTFPlayer_GiveDefaultItems && is_item_replacement.count(item_def))) {
			slot = item_def->GetLoadoutSlot(TF_CLASS_UNDEFINED);
		}
		return slot;
	}
	DETOUR_DECL_MEMBER(int, CTFItemDefinition_GetLoadoutSlot, int classIndex)
	{
		int slot = DETOUR_MEMBER_CALL(CTFItemDefinition_GetLoadoutSlot)(classIndex);
		auto item_def = reinterpret_cast<CTFItemDefinition *>(this);
		return slot == -1 && classIndex != TF_CLASS_UNDEFINED && item_def->m_iItemDefIndex != 0 ? LoadoutSlotReplace(slot, item_def, classIndex) : slot;
	}

	DETOUR_DECL_MEMBER(CEconItemView *, CTFPlayerInventory_GetItemInLoadout, int pclass, int slot)
	{
		auto inventory = reinterpret_cast<CTFPlayerInventory *>(this);
		CTFPlayer *player = (CTFPlayer *) UTIL_PlayerBySteamID(inventory->m_OwnerId);

		auto result = DETOUR_MEMBER_CALL(CTFPlayerInventory_GetItemInLoadout)(pclass, slot);

		if (result != nullptr && result->GetItemDefinition() != nullptr && IsMannVsMachineMode() && player != nullptr) {
			
			auto find_loadout = state.m_SelectedLoadoutItems.find(player->GetSteamID());
			if (find_loadout != state.m_SelectedLoadoutItems.end()) {
				for(int itemnum : find_loadout->second) {
					if (itemnum < (int) state.m_ExtraLoadoutItems.size()) {
						auto &extraitem = state.m_ExtraLoadoutItems[itemnum];
						if (extraitem.item != nullptr && (extraitem.class_index == pclass || extraitem.class_index == 0) && extraitem.loadout_slot == slot) {
							DevMsg("GiveItem %d %s\n", slot, GetItemNameForDisplay(extraitem.item));
							if (rc_CTFPlayer_GiveDefaultItems)
								is_item_replacement.insert(extraitem.item->GetItemDefinition());
							item_view_replacement = extraitem.item;
							
							//CEconEntity *entity = GetEconEntityAtLoadoutSlot(player, itemslot);
							//if (entity->IsWearable())
							//	entity->Remove();

							return extraitem.item;
						}
					}
				}
			}
			
			if (!state.m_ItemReplace.empty()) {
				const char *classname = TranslateWeaponEntForClass_improved(result->GetItemDefinition()->GetKeyValues()->GetString("item_class"), pclass);
				bool found = false;
				CEconItemDefinition *item_def = nullptr;
				CEconItemView *view = nullptr;
				for (const auto& entry : state.m_ItemReplace) {
					if (entry.entry->Matches(classname, result)) {
						found = true;
						item_def = entry.item_def;
						view = entry.item;
						break;
					}
				}
				if (found) {
					if (rc_CTFPlayer_GiveDefaultItems)
						is_item_replacement.insert(item_def);
						
					item_view_replacement = view;
					return view;
				}
			}

			if (state.m_ForceItems.parsed) {
				
			}

			/* only enforce the whitelist/blacklist if they are non-empty */

			if (!state.m_ItemWhitelist.empty()) {
				const char *classname = TranslateWeaponEntForClass_improved(result->GetItemDefinition()->GetKeyValues()->GetString("item_class"), pclass);
				bool found = false;
				for (const auto& entry : state.m_ItemWhitelist) {
					if (entry->Matches(classname, result)) {
						found = true;
						break;
					}
				}
				if (!found) {
					result = TFInventoryManager()->GetBaseItemForClass(pclass, slot);
					if (result != nullptr && result->GetItemDefinition() != nullptr) {
						found = false;
						classname = TranslateWeaponEntForClass_improved(result->GetItemDefinition()->GetKeyValues()->GetString("item_class"), pclass);
						for (const auto& entry : state.m_ItemWhitelist) {
							if (entry->Matches(classname, result)) {
								found = true;
								break;
							}
						}
					}
					if (!found) {
						DevMsg("[%s] GiveNamedItem(\"%s\"): denied by whitelist\n", player->GetPlayerName(), classname);
						return TFInventoryManager()->GetBaseItemForClass(pclass, 999);
					}
				}
			}
			
			if (!state.m_ItemBlacklist.empty()) {
				const char *classname = TranslateWeaponEntForClass_improved(result->GetItemDefinition()->GetKeyValues()->GetString("item_class"), pclass);
				bool found = false;
				for (const auto& entry : state.m_ItemBlacklist) {
					if (entry->Matches(classname, result)) {
						found = true;
						break;
					}
				}
				if (found) {
					result = TFInventoryManager()->GetBaseItemForClass(pclass, slot);
					if (result != nullptr && result->GetItemDefinition() != nullptr) {
						found = false;
						classname = TranslateWeaponEntForClass_improved(result->GetItemDefinition()->GetKeyValues()->GetString("item_class"), pclass);
						for (const auto& entry : state.m_ItemBlacklist) {
							if (entry->Matches(classname, result)) {
								found = true;
								break;
							}
						}
					}
					if (found) {
						DevMsg("[%s] GiveNamedItem(\"%s\"): denied by blacklist\n", player->GetPlayerName(), classname);
						return TFInventoryManager()->GetBaseItemForClass(pclass, 999);
					}
				}
			}
			
		}

		
		
	//	DevMsg("[%s] GiveNamedItem(\"%s\"): provisionally allowed\n", player->GetPlayerName(), classname);
		//CBaseEntity *entity = DETOUR_MEMBER_CALL(CTFPlayer_GiveNamedItem)(classname, i1, item_view, b1);

		// Disable cosmetics on robots, if player animations are not enabled
		/*if (PlayerUsesRobotModel(player) && item_view != nullptr && item_view->GetItemDefinition() != nullptr) {

			Msg("GiveItem %d\n", player->GetPlayerClass()->GetClassIndex());
			const char *item_slot = item_view->GetItemDefinition()->GetKeyValues()->GetString("item_slot", "");
			if ((FStrEq(item_slot, "head") || FStrEq(item_slot, "misc")) && !IsItemApplicableToRobotModel(player, static_cast<CBaseAnimating *>(entity))) {
				entity->Remove();
				return nullptr;
			}
		}*/
		
		return result;
	}

	ConVar cvar_givenameditem_blacklist("sig_mvm_blacklist_givenameditem", "1", FCVAR_NOTIFY,
		"Use item blacklist in GiveNamedItem for backward compatibility");

	RefCount rc_CTFPlayer_PickupWeaponFromOther;
	DETOUR_DECL_MEMBER(void, CTFPlayer_PickupWeaponFromOther, void *weapon)
	{
		SCOPED_INCREMENT(rc_CTFPlayer_PickupWeaponFromOther);
		DETOUR_MEMBER_CALL(CTFPlayer_PickupWeaponFromOther)(weapon);
	}

	DETOUR_DECL_MEMBER(CBaseEntity *, CTFPlayer_GiveNamedItem, const char *classname, int i1, const CEconItemView *item_view, bool b1)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (cvar_givenameditem_blacklist.GetBool() && IsMannVsMachineMode() && !player->IsBot() && !rc_CTFPlayer_PickupWeaponFromOther && item_view_replacement != item_view) {
			/* only enforce the whitelist/blacklist if they are non-empty */
			
			if (!state.m_ItemWhitelist.empty()) {
				bool found = false;
				for (const auto& entry : state.m_ItemWhitelist) {
					if (entry->Matches(classname, item_view)) {
						found = true;
						break;
					}
				}
				if (!found) {
					DevMsg("[%s] GiveNamedItem(\"%s\"): denied by whitelist\n", player->GetPlayerName(), classname);
					return nullptr;
				}
			}
			
			if (!state.m_ItemBlacklist.empty()) {
				bool found = false;
				for (const auto& entry : state.m_ItemBlacklist) {
					if (entry->Matches(classname, item_view)) {
						found = true;
						break;
					}
				}
				if (found) {
					DevMsg("[%s] GiveNamedItem(\"%s\"): denied by blacklist\n", player->GetPlayerName(), classname);
					return nullptr;
				}
			}

			
		}
		CBaseEntity *entity = DETOUR_MEMBER_CALL(CTFPlayer_GiveNamedItem)(classname, i1, item_view, b1);

		// Loadout replacement items need to be validated manually
		if (entity != nullptr && !is_item_replacement.empty()) {
			auto econ = rtti_cast<CEconEntity *>(entity);
			if (econ != nullptr && is_item_replacement.count(econ->GetItem()->GetItemDefinition()) != 0) {
				econ->Validate();
			}
		}
		return entity;
	}

	DETOUR_DECL_MEMBER(void, CUpgrades_GrantOrRemoveAllUpgrades, CTFPlayer * player, bool remove, bool refund)
	{


		// Delete refundable custom weapons from player inventory
		if (remove && !state.m_BoughtLoadoutItems.empty()) {
			auto &playerItems = state.m_BoughtLoadoutItems[player->GetSteamID()];
			auto &playerItemsSelected = state.m_SelectedLoadoutItems[player->GetSteamID()];
			for (auto it = playerItems.begin(); it != playerItems.end();) {
				int id = *it;
				if (state.m_ExtraLoadoutItems[id].allow_refund) {
					player->RemoveCurrency(-state.m_ExtraLoadoutItems[id].cost);
					it = playerItems.erase(it);
					playerItemsSelected.erase(id);
				}
				else {
					it++;
				}
			}
		}

		DETOUR_MEMBER_CALL(CUpgrades_GrantOrRemoveAllUpgrades)(player, remove, refund);
		
		// Reapply attributes to custom weapons, as if they come with attributes used by upgrades, those attributes will be removed
		ForEachTFPlayerEconEntity(player, [&](CEconEntity *entity) {
			bool isCustom = false;
			auto name = GetItemName(entity->GetItem(), isCustom);
			if (isCustom) {
				AddCustomWeaponAttributes(name, entity->GetItem());
			}
		});

		if (remove && !state.m_ItemAttributes.empty()) {
			ForEachTFPlayerEconEntity(player, [&](CEconEntity *entity) {

				CEconItemView *item_view = entity->GetItem();
				if (item_view == nullptr) return;
				ApplyItemAttributes(item_view, player, state.m_ItemAttributes);
			});
		}

		int oldwave = TFObjectiveResource()->m_nMannVsMachineWaveCount;
		TFObjectiveResource()->m_nMannVsMachineWaveCount = 2;
		auto kv = new KeyValues("MvM_UpgradesBegin");
		serverGameClients->ClientCommandKeyValues(player->edict(), kv);
		kv->deleteThis();
		kv = new KeyValues("MvM_UpgradesDone");
		serverGameClients->ClientCommandKeyValues(player->edict(), kv);
		kv->deleteThis();
		TFObjectiveResource()->m_nMannVsMachineWaveCount = oldwave;

		if (remove) {
			CSteamID steamid;
			player->GetSteamID(&steamid);
			// Delete old dropped weapons if a player refunded an upgrade
			ForEachEntityByRTTI<CTFDroppedWeapon>([&](CTFDroppedWeapon *weapon) {
				if (weapon->m_Item->m_iAccountID == steamid.GetAccountID()) {
					weapon->Remove();
				}
			});
		}
	}
	
	int ReplaceEconItemViewDefId(CEconItemView *pItem)
	{
		static int custom_weapon_def = -1;
		auto origItemDefId = -1;
		if (custom_weapon_def == -1) {
			auto attr = GetItemSchema()->GetAttributeDefinitionByName("custom weapon id");
			if (attr != nullptr)
				custom_weapon_def = attr->GetIndex();
		}
		if (pItem != nullptr) {
			auto attr = pItem->GetAttributeList().GetAttributeByID(custom_weapon_def);
			if (attr != nullptr) {
				origItemDefId = pItem->m_iItemDefinitionIndex;
				pItem->m_iItemDefinitionIndex = 0xF000 + (int)attr->GetValue().m_Float;
			}
		} 
		return origItemDefId;
	}

	void RestoreEconItemViewDefId(CEconItemView *pItem, int origItemDefId)
	{
		if (origItemDefId != -1 && pItem != nullptr) {
			pItem->m_iItemDefinitionIndex = origItemDefId;
		}
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_ReapplyItemUpgrades, CEconItemView *item_view)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (IsMannVsMachineMode() && !player->IsBot() /*player->GetTeamNumber() == TF_TEAM_RED*/) {
			if (!state.m_ItemAttributes.empty()) {
				ApplyItemAttributes(item_view, player, state.m_ItemAttributes);
			}
		}
		int origItemDefId = ReplaceEconItemViewDefId(item_view);
		DETOUR_MEMBER_CALL(CTFPlayer_ReapplyItemUpgrades)(item_view);
		RestoreEconItemViewDefId(item_view, origItemDefId);
	}

#if 0
	DETOUR_DECL_MEMBER(bool, CTFPlayer_ItemIsAllowed, CEconItemView *item_view)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		
		if (IsMannVsMachineMode() && item_view != nullptr) {
			int item_defidx = item_view->GetItemDefIndex();
			
			if (state.m_DisallowedItems.count(item_defidx) != 0) {
				DevMsg("[%s] CTFPlayer::ItemIsAllowed(%d): denied\n", player->GetPlayerName(), item_defidx);
				return false;
			}
		}
		
		return DETOUR_MEMBER_CALL(CTFPlayer_ItemIsAllowed)(item_view);
	}
#endif
	
	DETOUR_DECL_MEMBER(int, CCaptureFlag_GetMaxReturnTime)
	{
		auto flag = reinterpret_cast<CCaptureFlag *>(this);
		
		if (IsMannVsMachineMode()) {
			auto it = state.m_FlagResetTimes.find(STRING(flag->GetEntityName()));
			if (it != state.m_FlagResetTimes.end()) {
				return (*it).second;
			}
		}
		
		return DETOUR_MEMBER_CALL(CCaptureFlag_GetMaxReturnTime)();
	}
	
	
	RefCount rc_CTFGameRules_ctor;
	DETOUR_DECL_MEMBER(void, CTFGameRules_ctor)
	{
		SCOPED_INCREMENT(rc_CTFGameRules_ctor);
	DETOUR_MEMBER_CALL(CTFGameRules_ctor)();
	}
	
	RefCount rc_CTFGameRules_SetWinningTeam;
	DETOUR_DECL_MEMBER(void, CTFGameRules_SetWinningTeam, int team, int iWinReason, bool bForceMapReset, bool bSwitchTeams, bool bDontAddScore, bool bFinal)
	{

		SCOPED_INCREMENT(rc_CTFGameRules_SetWinningTeam);
		DETOUR_MEMBER_CALL(CTFGameRules_SetWinningTeam)(team, iWinReason, bForceMapReset, bSwitchTeams, bDontAddScore, bFinal);
	}

	DETOUR_DECL_MEMBER(void, CTFPlayerShared_AddCond, ETFCond nCond, float flDuration, CBaseEntity *pProvider)
	{
		if (rc_CTFGameRules_SetWinningTeam && nCond == TF_COND_CRITBOOSTED_BONUS_TIME)
        {
			return;
        }
		DETOUR_MEMBER_CALL(CTFPlayerShared_AddCond)(nCond, flDuration, pProvider);
	}
	
	bool rc_CTeamplayRoundBasedRules_State_Enter = false;
	bool last_game_was_win = false;
	DETOUR_DECL_MEMBER(void, CTeamplayRoundBasedRules_State_Enter, gamerules_roundstate_t newState)
	{
		/* the CTeamplayRoundBasedRules ctor calls State_Enter BEFORE the CTFGameRules ctor has had a chance to run yet
		 * and initialize stuff like m_bPlayingMannVsMachine etc; so don't do any of this stuff in that case! */
		 

		DevMsg("StateEnter %d\n", newState);
		bool startBonusTimer = false;
		if (rc_CTFGameRules_ctor <= 0) {
			
			DevMsg("Round state team win %d\n", TFGameRules()->GetWinningTeam());
			gamerules_roundstate_t oldState = TFGameRules()->State_Get();
			
		//	ConColorMsg(Color(0x00, 0xff, 0x00, 0xff), "[State] MvM:%d Reverse:%d oldState:%d newState:%d\n",
		//		IsMannVsMachineMode(), state.m_bReverseWinConditions, oldState, newState);
			
			if (IsMannVsMachineMode() && TFGameRules()->GetWinningTeam() != state.m_iEnemyTeamForReverse && state.m_bReverseWinConditions && g_pPopulationManager != nullptr && oldState == GR_STATE_TEAM_WIN && newState == GR_STATE_PREROUND) {
				
				unsigned int wave_pre = TFObjectiveResource()->m_nMannVsMachineWaveCount;
				//int GetTotalCurrency() return {}
				
				rc_CTeamplayRoundBasedRules_State_Enter = true;
				ConColorMsg(Color(0x00, 0xff, 0x00, 0xff), "GR_STATE_TEAM_WIN --> GR_STATE_PREROUND\n");
				
				CWave *wave = g_pPopulationManager->GetCurrentWave();
				
				ForEachTFBot([&](CTFBot *bot){
					if (bot->GetTeamNumber() != TEAM_SPECTATOR) {
						bot->CommitSuicide();
						bot->ForceChangeTeam(TEAM_SPECTATOR, true);
					}
				});

				// Distribute cash from packs
				int packsCurrency = TFObjectiveResource()->m_nMvMWorldMoney;
				
				for (int i = 0; i < ICurrencyPackAutoList::AutoList().Count(); ++i) {
					auto pack = rtti_cast<CCurrencyPack *>(ICurrencyPackAutoList::AutoList()[i]);
					if (pack == nullptr) continue;
					pack->SetDistributed(true);
				}
				TFObjectiveResource()->m_nMvMWorldMoney = 0;
				TFGameRules()->DistributeCurrencyAmount( packsCurrency, NULL, true, false, false );

				if (wave != nullptr) {
					// Distribute cash from unspawned packs
					int unallocatedCurrency = 0;
					FOR_EACH_VEC(wave->m_WaveSpawns, i) {
						CWaveSpawnPopulator *wavespawn = wave->m_WaveSpawns[i];
						if (wavespawn->m_unallocatedCurrency != -1) {
							unallocatedCurrency += wavespawn->m_unallocatedCurrency;
						}
					}
					TFGameRules()->DistributeCurrencyAmount( unallocatedCurrency, NULL, true, true, false );
					
					/* call this twice to ensure all wavespawns get to state DONE */
					ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "PRE  CWave::ForceFinish\n");
					wave->ForceFinish();
					ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "MID  CWave::ForceFinish\n");
					wave->ForceFinish();
					ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "POST CWave::ForceFinish\n");
					
					// is this necessary or not? unsure...
					// TODO: if enabled, need to block CTFPlayer::CommitSuicide call from ActiveWaveUpdate
				//	ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "PRE  CWave::ActiveWaveUpdate\n");
				//	wave->ActiveWaveUpdate();
				//	ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "POST CWave::ActiveWaveUpdate\n");
					
					ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "PRE  CWave::WaveCompleteUpdate\n");
					wave->WaveCompleteUpdate();
					ConColorMsg(Color(0xff, 0xff, 0x00, 0xff), "POST CWave::WaveCompleteUpdate\n");
				}
				startBonusTimer = true;
				last_game_was_win = true;
				if ( wave_pre == TFObjectiveResource()->m_nMannVsMachineMaxWaveCount) {
				//TFGameRules()->SetForceMapReset(true);
				// TODO(?): for all human players on TF_TEAM_BLUE: if !IsAlive(), then call ForceRespawn()

					return;
				}
			}
		}
		
		if (rc_CTFGameRules_ctor <= 0 && newState == GR_STATE_BETWEEN_RNDS && IsMannVsMachineMode()) {
			ConColorMsg(Color(0xff, 0x00, 0x00, 0xff), "Wave #%d initialized of mission %s\n", TFObjectiveResource()->m_nMannVsMachineWaveCount.Get(), STRING(TFObjectiveResource()->m_iszMvMPopfileName.Get()));
			if (state.m_ScriptManager != nullptr) {
				auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
				if (scriptManager != nullptr && scriptManager->CheckGlobal("OnWaveInit")) {
					lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount);
					scriptManager->Call(1, 0);
				}
			}
		}
		DETOUR_MEMBER_CALL(CTeamplayRoundBasedRules_State_Enter)(newState);

		if (startBonusTimer) {
			g_pPopulationManager->GetCurrentWave()->m_GetUpgradesAlertTimer.Start(3.0f);
		}
		rc_CTeamplayRoundBasedRules_State_Enter = false;
	}
	DETOUR_DECL_MEMBER(void, CMannVsMachineStats_RoundEvent_WaveEnd, bool success)
	{
		DevMsg("Wave end team win %d\n", TFGameRules()->GetWinningTeam());
		bool origSuccess = success;
		if (state.m_bReverseWinConditions && last_game_was_win) {
			DevMsg("State_Enter %d\n", rc_CTeamplayRoundBasedRules_State_Enter);
			success = true;
		}

		if (state.m_ScriptManager != nullptr) {
			auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");

			if (scriptManager->CheckGlobal(success ? "OnWaveSuccess" : "OnWaveFail")) {
				lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount + (origSuccess ? 1 : 0));
				scriptManager->Call(1, 0);
			}
		}

		DETOUR_MEMBER_CALL(CMannVsMachineStats_RoundEvent_WaveEnd)(success);
	}
	#define CONVAR_SCOPE_VALUE(name, value) \
	static ConVarRef convarScopedRef_##name(#name); \
	ConvarScopeValue convarScoped_##name(convarScopedRef_##name, #value);

	class ConvarScopeValue
	{
	public:
		ConvarScopeValue(ConVarRef &ref, std::string valueToSet) : cvar(ref), valueToRestore(ref.GetString()) {
			cvar.SetValue(valueToSet.c_str());
		}
		~ConvarScopeValue() {
			cvar.SetValue(valueToRestore.c_str());
		}
	public:
		ConVarRef cvar;
		std::string valueToRestore;
	};
	
	bool CheckPlayerClassLimit(CTFPlayer *player, int plclass, bool do_switch)
	{
		const char* classname = g_aRawPlayerClassNames[plclass];
		if (player->IsBot() || state.m_DisallowedClasses[plclass] == -1)
			return false;

		Msg("CheckPlayerLimit\n");
		int taken_slots[10];
		for (int i=0; i < 10; i++)
			taken_slots[i] = 0;

		ForEachTFPlayer([&](CTFPlayer *playerin){
			if(player != playerin && playerin->GetTeamNumber() == player->GetTeamNumber() && !playerin->IsBot()){
				int classnum = playerin->GetPlayerClass()->GetClassIndex();
				taken_slots[classnum]+=1;
			}
		});

		if (state.m_DisallowedClasses[plclass] <= taken_slots[plclass]){
			
			CONVAR_SCOPE_VALUE(tf_mvm_endless_force_on, 1);
			
			if (state.m_bSingleClassAllowed != -1) {
				gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, CFmtStr("%s %s %s", "Only",g_aRawPlayerClassNames[state.m_bSingleClassAllowed],"class is allowed in this mission"));

				player->HandleCommand_JoinClass(g_aRawPlayerClassNames[state.m_bSingleClassAllowed]);

			}
			else {
				const char *sound = g_sSounds[plclass];

				CRecipientFilter filter;
				filter.AddRecipient(player);
				CBaseEntity::EmitSound(filter, ENTINDEX(player), sound);

				gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, CFmtStr("%s %s %s", "Exceeded the",classname,"class limit in this mission"));

				if (do_switch) {
					for (int i=1; i < 10; i++){
						if(state.m_DisallowedClasses[i] == -1 || taken_slots[i] < state.m_DisallowedClasses[i]){
							player->HandleCommand_JoinClass(g_aRawPlayerClassNames[i]);
							break;
						}
					}
				}

				int msg_type = usermessages->LookupUserMessage("VGUIMenu");
				if (msg_type != -1) {
				
					bf_write *msg = engine->UserMessageBegin(&filter, msg_type);

					if (msg != nullptr) {
						msg->WriteString("class_red");
						msg->WriteByte(0x01);
						msg->WriteByte(0x00);

						engine->MessageEnd();
					}
				}
			}
			return true;
		}
		return false;
	}

	bool CheckPlayerClassLimit(CTFPlayer *player)
	{
		return CheckPlayerClassLimit(player, player->GetPlayerClass()->GetClassIndex(), true);
	}

	RefCount rc_CTFPlayer_HandleCommand_JoinClass;
	DETOUR_DECL_MEMBER(void, CTFPlayer_HandleCommand_JoinClass, const char *pClassName, bool b1)
	{
		SCOPED_INCREMENT(rc_CTFPlayer_HandleCommand_JoinClass);
		auto player = reinterpret_cast<CTFPlayer *>(this);

		if (rc_CTFPlayer_HandleCommand_JoinClass <= 1 && !player->IsBot() && player->GetTeamNumber() >= TF_TEAM_RED) {
			int class_index = 0;
			for (int i = 0; i < 10; i++) {
				if (FStrEq(pClassName, g_aRawPlayerClassNames[i])) {
					class_index = i;
					break;
				}
			}
			if (CheckPlayerClassLimit(player, class_index, false)) {
				return;
			}
		}

		/*if (!player->IsBot() && player->GetTeamNumber() >= TF_TEAM_RED && !player->m_Shared->m_bInUpgradeZone && TFGameRules()->State_Get() != GR_STATE_RND_RUNNING) {
			if (FStrEq(pClassName, "random") || FStrEq(pClassName, "civilian")) {
				player->m_Shared->m_iDesiredPlayerClass = 10;
				player->ForceRespawn();
				return;
			}
		}*/

		
		if (g_pPopulationManager != nullptr && !player->IsBot() && player->GetTeamNumber() == TF_TEAM_BLUE) {
			if (player->m_nCanPurchaseUpgradesCount > 0) {
				gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER , "#TF_MVM_NoClassUpgradeUI");
				return;
			}
			if (player->IsReadyToPlay() && !TFGameRules()->InSetup() && !g_pPopulationManager->IsInEndlessWaves()) {
            	gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CHAT , "#TF_MVM_NoClassChangeAfterSetup");
				return;
			}
		}
		DETOUR_MEMBER_CALL(CTFPlayer_HandleCommand_JoinClass)(pClassName, b1);

		// Avoid infinite loop
		if (rc_CTFPlayer_HandleCommand_JoinClass < 10 && player->GetTeamNumber() >= TF_TEAM_RED)
			CheckPlayerClassLimit(player);
	}

	DETOUR_DECL_MEMBER(void, CBasePlayer_ChangeTeam, int iTeamNum, bool b1, bool b2, bool b3)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		DETOUR_MEMBER_CALL(CBasePlayer_ChangeTeam)(iTeamNum, b1, b2, b3);
		if (!player->IsBot()) {
			ApplyOrClearRobotModel(player);
		}
	}
	DETOUR_DECL_MEMBER(void, CTFPlayer_ChangeTeam, int iTeamNum, bool b1, bool b2, bool b3)
	{
		DETOUR_MEMBER_CALL(CTFPlayer_ChangeTeam)(iTeamNum, b1, b2, b3);
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (player->GetTeamNumber() >= TF_TEAM_RED)
			CheckPlayerClassLimit(player);
	}

	void ApplyPlayerAttributes(CTFPlayer *player) {
		int classname = player->GetPlayerClass()->GetClassIndex();
		for(auto it = state.m_PlayerAttributes.begin(); it != state.m_PlayerAttributes.end(); ++it){
			auto attr = player->GetAttributeList()->GetAttributeByName(it->first.c_str());
			if (attr == nullptr || it->second != attr->GetValuePtr()->m_Float) {
				auto attr_def = GetItemSchema()->GetAttributeDefinitionByName(it->first.c_str());
				if (attr_def != nullptr) {
					player->GetAttributeList()->SetRuntimeAttributeValue(attr_def, it->second);
					player->TeamFortress_SetSpeed();
				}
			}
		}
		
		for(auto it = state.m_PlayerAttributesClass[classname].begin(); it != state.m_PlayerAttributesClass[classname].end(); ++it){
			auto attr = player->GetAttributeList()->GetAttributeByName(it->first.c_str());
			if (attr == nullptr || it->second != attr->GetValuePtr()->m_Float) {
				auto attr_def = GetItemSchema()->GetAttributeDefinitionByName(it->first.c_str());
				if (attr_def != nullptr) {
					player->GetAttributeList()->SetRuntimeAttributeValue(attr_def, it->second);
					player->TeamFortress_SetSpeed();
				}
			}
		}
	}

	const char *GetGiantLoopSound(CTFPlayer *player) {
		int iClass = player->GetPlayerClass()->GetClassIndex();
		switch ( iClass )
		{
		case TF_CLASS_HEAVYWEAPONS:
			return "MVM.GiantHeavyLoop";
		case TF_CLASS_SOLDIER:
			return "MVM.GiantSoldierLoop";
		case TF_CLASS_DEMOMAN:
			return "MVM.GiantDemomanLoop";
		case TF_CLASS_SCOUT:
			return "MVM.GiantScoutLoop";
		case TF_CLASS_PYRO:
			return "MVM.GiantPyroLoop";
		}
		return nullptr; 
	}

	void StopGiantSounds(CTFPlayer *player) {
		player->StopSound("MVM.GiantHeavyLoop");
		player->StopSound("MVM.GiantSoldierLoop");
		player->StopSound("MVM.GiantDemomanLoop");
		player->StopSound("MVM.GiantScoutLoop");
		player->StopSound("MVM.GiantPyroLoop");
	}

	//std::vector<CHandle<CTFBot>> spawned_bots_first_tick;
	//std::vector<CHandle<CTFPlayer>> spawned_players_first_tick;
	//extern GlobalThunk<CTETFParticleEffect> g_TETFParticleEffect;

	bool received_mission_message_tick[33] = {false};

	THINK_FUNC_DECL(DelayMissionInfoSend)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		DevMsg("Pass send\n");

		if (state.m_ExtraLoadoutItemsNotify)
			PrintToChat("\x07""7fd4ffThis mission allows you to equip custom items. Type !missionitems in chat to see available items for your class\n",player);

		//auto explanation = Mod::Pop::Wave_Extensions::GetWaveExplanation(0);
		
		bool player_empty = state.m_PlayerAttributes.empty();
		for (int i = 0; i < 10; i++) {
			if (!state.m_PlayerAttributesClass[i].empty()) {
				player_empty = false;
				break;
			}
		}

		if (!state.m_bNoMissionInfo && (
			//(explanation != nullptr && !explanation->empty()) ||
			!state.m_ItemWhitelist.empty() ||
			!state.m_ItemBlacklist.empty() ||
			!state.m_ItemAttributes.empty() ||
			!player_empty ||
			!state.m_DisallowedUpgrades.empty() ||
			!state.m_ItemReplace.empty() ||
			!state.m_ExtraLoadoutItems.empty() ||
			state.m_ForceItems.parsed ||
			state.m_bSniperAllowHeadshots ||
			state.m_bSniperHideLasers ||
			!state.m_RespecEnabled.GetValue() ||
			state.m_RespecLimit.GetValue() ||
			state.m_ImprovedAirblast.GetValue() ||
			state.m_SandmanStuns.GetValue() ||
			state.m_bNoReanimators
		)) { 
			PrintToChat("\x07""7fd4ffType !missioninfo in chat to check custom mission information\n",player);
		}
	}

	DETOUR_DECL_MEMBER(void, CTFGameRules_OnPlayerSpawned, CTFPlayer *player)
	{
		IClient *pClient = sv->GetClient( ENTINDEX(player)-1 );
		if (player->GetDeathTime() == 0.0f 
			&& !pClient->IsFakeClient() 
			&& TFGameRules()->State_Get() == GR_STATE_RND_RUNNING) {
			player->SetDeathTime(gpGlobals->curtime); 
		}
		DETOUR_MEMBER_CALL(CTFGameRules_OnPlayerSpawned)(player);

		//CTFBot *bot = ToTFBot(player);
		//if (bot != nullptr)
		//	spawned_bots_first_tick.push_back(bot);

		if (!player->IsBot() && !pClient->IsFakeClient() && IsMannVsMachineMode()) {
		//	spawned_players_first_tick.push_back(player);
			if (state.m_bExtendedUpgradesOnly) {
				auto mod = player->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::FakePropModule>("fakeprop");
				variant_t value;
				value.SetBool(false);
        		mod->props["m_bInUpgradeZone"] = {value, value};
			}
			ApplyPlayerAttributes(player);
			player->SetHealth(player->GetMaxHealth());

			// int isMiniboss = 0;
			// CALL_ATTRIB_HOOK_INT_ON_OTHER(player, isMiniboss, is_miniboss);
			// player->SetMiniBoss(isMiniboss);

			// float playerScale = 1.0f;
			// CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(player, playerScale, model_scale);
			
			// StopGiantSounds(player);
			/*if (isMiniboss) {
				const char *sound = GetGiantLoopSound(player);
				if (sound != nullptr){}
					player->EmitSound(sound);
			}*/
			

			for (auto &templ : state.m_PlayerSpawnTemplates) {
				if (templ.class_index == 0 || templ.class_index == player->GetPlayerClass()->GetClassIndex())
					templ.info.SpawnTemplate(player);
			}

			// Templates that only spawn the first time the player joins on the mission
			if (!state.m_PlayerSpawnOnceTemplatesAppliedTo.count(player)) {
				for (auto &templ : state.m_PlayerSpawnOnceTemplates) {
					templ.SpawnTemplate(player);
				}
				state.m_PlayerSpawnOnceTemplatesAppliedTo.insert(player);
			}

			// if (playerScale != 1.0f)
			// 	player->SetModelScale(playerScale);
			// else if (player->IsMiniBoss()) {
			// 	static ConVarRef miniboss_scale("tf_mvm_miniboss_scale");
			// 	player->SetModelScale(miniboss_scale.GetFloat());
			// }

			if (player->GetPlayerClass()->GetClassIndex() != TF_CLASS_UNDEFINED && !state.m_PlayerUpgradeSend.count(player) && 
					(strcmp(TFGameRules()->GetCustomUpgradesFile(), "") != 0 && strcmp(TFGameRules()->GetCustomUpgradesFile(), "scripts/items/mvm_upgrades.txt") != 0)) {

				state.m_PlayerUpgradeSend.insert(player);
				ResendUpgradeFile(false);
				if (!received_message_tick) {
					PrintToChat("\x07""ffb200This server uses custom upgrades. Make sure you have enabled downloads in options (Download all files or Don't download sound files)\n",player);
					
				}
			}
	
			if (!state.m_PlayerMissionInfoSend.count(player)) {
				state.m_PlayerMissionInfoSend.insert(player);
				DevMsg("Try send\n");
				
				THINK_FUNC_SET(player, DelayMissionInfoSend, gpGlobals->curtime + 1.0f);
				
			}

			ApplyOrClearRobotModel(player);
		}
	}

	/*DETOUR_DECL_MEMBER(void, CTFWeaponBase_UpdateHands)
	{
		DETOUR_MEMBER_CALL(CTFWeaponBase_UpdateHands)();
		auto weapon = reinterpret_cast<CTFWeaponBase *>(this);
		auto owner = weapon->GetTFPlayerOwner();
		if (owner != nullptr && state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()] != 0) {
			auto viewModel = owner->GetViewModel();
			if (viewModel != nullptr) {
				DevMsg("Indexes pre %d %d %d\n", weapon->m_iViewModelIndex + 0, weapon->GetModelIndex(), viewModel->GetModelIndex());
				weapon->SetModelIndex(state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				weapon->m_iViewModelIndex = state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()];
				viewModel->SetModelIndex(state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				for(int i = 0; i < MAX_VISION_MODES; i++) {
					weapon->SetModelIndexOverride(i, state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
					viewModel->SetModelIndexOverride(i, state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				}
			}
		}
	}

	DETOUR_DECL_MEMBER(void, CBaseCombatWeapon_SetViewModel)
	{
		DETOUR_MEMBER_CALL(CBaseCombatWeapon_SetViewModel)();
		auto weapon = reinterpret_cast<CTFWeaponBase *>(this);
		auto owner = weapon->GetTFPlayerOwner();
		if (owner != nullptr && state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()] != 0) {
			auto viewModel = owner->GetViewModel();
			if (viewModel != nullptr) {
				DevMsg("Indexes pre %d %d %d\n", weapon->m_iViewModelIndex + 0, weapon->GetModelIndex(), viewModel->GetModelIndex());
				weapon->SetModelIndex(state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				weapon->m_iViewModelIndex = state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()];
				viewModel->SetModelIndex(state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				for(int i = 0; i < MAX_VISION_MODES; i++) {
					weapon->SetModelIndexOverride(i, state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
					viewModel->SetModelIndexOverride(i, state.m_HandModelOverride[owner->GetPlayerClass()->GetClassIndex()]);
				}
			}
		}
	}*/

	DETOUR_DECL_MEMBER(const char *, CTFPlayerClassShared_GetHandModelName, int handIndex)
	{
		auto shared = reinterpret_cast<CTFPlayerClassShared *>(this);
		if (!state.m_HandModelOverride[shared->GetClassIndex()].empty()) {
			return state.m_HandModelOverride[shared->GetClassIndex()].c_str();
		}
		return DETOUR_MEMBER_CALL(CTFPlayerClassShared_GetHandModelName)(handIndex);
	}

	
	RefCount rc_CTFPlayer_Event_Killed;
	CTFPlayer *player_killer = nullptr;
	DETOUR_DECL_MEMBER(void, CTFPlayer_Event_Killed, const CTakeDamageInfo& info)
	{
		SCOPED_INCREMENT(rc_CTFPlayer_Event_Killed);
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (state.m_bPlayerRobotUsePlayerAnimation && !player->IsBot() && ((state.m_bRedPlayersRobots && player->GetTeamNumber() == TF_TEAM_RED) || 
				(state.m_bBluPlayersRobots && player->GetTeamNumber() == TF_TEAM_BLUE))) {
			
			auto find = state.m_Player_anim_cosmetics.find(player);
			if (find != state.m_Player_anim_cosmetics.end()) {
				if (find->second != nullptr) {
					int model_index = find->second->m_nModelIndexOverrides[0];
					player->GetPlayerClass()->SetCustomModel(modelinfo->GetModelName(modelinfo->GetModel(model_index)), true);
					find->second->Remove();
				}
			}
		}
		player_killer = ToTFPlayer(info.GetAttacker());
		DETOUR_MEMBER_CALL(CTFPlayer_Event_Killed)(info);
		player_killer = nullptr;
	}
	
	DETOUR_DECL_MEMBER(void, CTFPlayer_InputSetCustomModel, inputdata_t data)
	{
		DETOUR_MEMBER_CALL(CTFPlayer_InputSetCustomModel)(data);
		if (state.m_bFixSetCustomModelInput) {
			auto player = reinterpret_cast<CTFPlayer *>(this);
			player->GetPlayerClass()->m_bUseClassAnimations=true;
		}
		
	}

	DETOUR_DECL_MEMBER(const char *, CTFPlayer_GetOverrideStepSound, const char *pszBaseStepSoundName)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (!player->IsBot() && ((state.m_bRedPlayersRobots && player->GetTeamNumber() == TF_TEAM_RED))) {
			return "MVM.BotStep";
		}
		const char *sound = DETOUR_MEMBER_CALL( CTFPlayer_GetOverrideStepSound)(pszBaseStepSoundName);
		if( (FStrEq("MVM.BotStep", sound) && (!player->IsBot() && player->GetTeamNumber() == TF_TEAM_BLUE && !state.m_bBluPlayersRobots)) || cvar_bots_are_humans.GetBool()) {
			return pszBaseStepSoundName;
		}
		return sound;
	}
	DETOUR_DECL_MEMBER(const char *, CTFPlayer_GetSceneSoundToken)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (!player->IsBot() &&((state.m_bRedPlayersRobots && player->GetTeamNumber() == TF_TEAM_RED) || 
				(state.m_bBluPlayersRobots && player->GetTeamNumber() == TF_TEAM_BLUE))) {
			return "MVM_";
		}
		else if((!player->IsBot() && player->GetTeamNumber() == TF_TEAM_BLUE)) {
			return "";
		}
		//const char *token=DETOUR_MEMBER_CALL( CTFPlayer_GetSceneSoundToken)();
		//DevMsg("CTFPlayer::GetSceneSoundToken %s\n", token);
		return DETOUR_MEMBER_CALL( CTFPlayer_GetSceneSoundToken)();
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_DeathSound, const CTakeDamageInfo& info)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (!player->IsBot()) {
			auto teamnum = player->GetTeamNumber();
			if (state.m_bRedPlayersRobots && teamnum == TF_TEAM_RED) {
				player->SetTeamNumber(TF_TEAM_BLUE);
			}
			else if (!state.m_bBluPlayersRobots && player->GetTeamNumber() == TF_TEAM_BLUE) {
				player->SetTeamNumber(TF_TEAM_RED);
			}
			DETOUR_MEMBER_CALL(CTFPlayer_DeathSound)(info);
			player->SetTeamNumber(teamnum);
		}
		else
			DETOUR_MEMBER_CALL(CTFPlayer_DeathSound)(info);
	}

	DETOUR_DECL_MEMBER(void, CTFPowerup_Spawn)
	{
		DETOUR_MEMBER_CALL(CTFPowerup_Spawn)();
		if (state.m_bNoHolidayHealthPack) {
			auto powerup = reinterpret_cast<CBaseAnimating *>(this);
			powerup->SetModelIndexOverride( VISION_MODE_PYRO, 0 );
			powerup->SetModelIndexOverride( VISION_MODE_HALLOWEEN, 0 );
		}
	}

	DETOUR_DECL_MEMBER(CBaseObject *, CTFBot_GetNearestKnownSappableTarget)
	{
		CBaseObject *ret = DETOUR_MEMBER_CALL(CTFBot_GetNearestKnownSappableTarget)();

		if (ret != nullptr && (ret->GetMoveParent() != nullptr || (state.m_bSpyNoSapUnownedBuildings && ret->GetBuilder() == nullptr))) {
			return nullptr;
		}
		return ret;
	}
	
	// DETOUR_DECL_STATIC(int, CollectPlayers_CTFPlayer, CUtlVector<CTFPlayer *> *playerVector, int team, bool isAlive, bool shouldAppend)
	// {
	// 	if (rc_CTFGameRules_FireGameEvent__teamplay_round_start > 0 && (team == TF_TEAM_BLUE && !isAlive && !shouldAppend)) {
	// 		/* collect players on BOTH teams */
	// 		return CollectPlayers_RedAndBlue_IsBot(playerVector, team, isAlive, shouldAppend);
	// 	}
		
	// 	return DETOUR_STATIC_CALL(CollectPlayers_CTFPlayer)(playerVector, team, isAlive, shouldAppend);
	// }

	DETOUR_DECL_MEMBER(void, CTFGCServerSystem_PreClientUpdate)
	{
		DETOUR_MEMBER_CALL(CTFGCServerSystem_PreClientUpdate)();
		if (IsMannVsMachineMode() && cvar_max_red_players.GetInt() > 0) {
			static ConVarRef visible_max_players("sv_visiblemaxplayers");
			int maxplayers = visible_max_players.GetInt() + cvar_max_red_players.GetInt() - 6;
			if (state.m_iTotalMaxPlayers > 0 && maxplayers > state.m_iTotalMaxPlayers)
				maxplayers = state.m_iTotalMaxPlayers;
			visible_max_players.SetValue(maxplayers);
			
		}
	}
	/*DETOUR_DECL_MEMBER(bool, IVision_IsAbleToSee, CBaseEntity *ent, Vector *vec)
	{
		DevMsg ("abletosee1\n");
		bool ret = DETOUR_MEMBER_CALL(IVision_IsAbleToSee)(ent,vec);

		if (ret && ent != nullptr) {
			CTFBot *bot = ToTFBot(reinterpret_cast<IVision *>(this)->GetBot()->GetEntity());
			if (bot != nullptr && bot->IsPlayerClass(TF_CLASS_SPY) && ent->IsBaseObject() && (ent->GetMoveParent() != nullptr || (state.m_bSpyNoSapUnownedBuildings && ToBaseObject(ent)->GetBuilder() == nullptr))) {
				return false;
			}
		}
		return ret;
	}*/
	
	DETOUR_DECL_MEMBER(bool, CTFBotVision_IsIgnored, CBaseEntity *ent)
	{
		if (ent->IsBaseObject()) {
			CTFBot *bot = static_cast<CTFBot *>(reinterpret_cast<IVision *>(this)->GetBot()->GetEntity());
			if (bot != nullptr && bot->IsPlayerClass(TF_CLASS_SPY) && (ent->GetMoveParent() != nullptr || (state.m_bSpyNoSapUnownedBuildings && ToBaseObject(ent)->GetBuilder() == nullptr))) {
				return true;
			}
		}
		return DETOUR_MEMBER_CALL(CTFBotVision_IsIgnored)(ent);
	}


	DETOUR_DECL_MEMBER(ActionResult<CTFBot>, CTFBotSpyInfiltrate_Update, CTFBot *me, float interval)
	{
		if (me->m_enemySentry != nullptr)
		{
			auto obj = ToBaseObject(me->m_enemySentry);
			if (obj->GetMoveParent() != nullptr || (state.m_bSpyNoSapUnownedBuildings && obj->GetBuilder() == nullptr)) {
				me->m_enemySentry = nullptr;
			}
		}
		return DETOUR_MEMBER_CALL(CTFBotSpyInfiltrate_Update)(me,interval);
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_AdjustMinPlayerSpawnTime)
	{
		DETOUR_MEMBER_CALL(CPopulationManager_AdjustMinPlayerSpawnTime)();
		if (state.m_flRespawnWaveTimeBlue >= 0.0f) {
			float time = state.m_flRespawnWaveTimeBlue;

			if (!state.m_bFixedRespawnWaveTimeBlue)
				time = std::min( state.m_flRespawnWaveTimeBlue, 2.0f * TFObjectiveResource()->m_nMannVsMachineWaveCount + 2.0f);
			TFGameRules()->SetTeamRespawnWaveTime(TF_TEAM_BLUE, time);
		}
	}

	DETOUR_DECL_MEMBER(bool, IGameEventManager2_FireEvent, IGameEvent *event, bool bDontBroadcast)
	{
		auto mgr = reinterpret_cast<IGameEventManager2 *>(this);
		
		if (event != nullptr && strcmp(event->GetName(), "player_death") == 0 && (event->GetInt( "death_flags" ) & 0x0200 /*TF_DEATH_MINIBOSS*/) == 0) {
			auto player = UTIL_PlayerByUserId(event->GetInt("userid"));
			if (player != nullptr && player->GetTeamNumber() == TF_TEAM_BLUE && (!player->IsBot() || state.m_bDisplayRobotDeathNotice) )
				event->SetInt("death_flags", (event->GetInt( "death_flags" ) | 0x0200));
		}

		if (event != nullptr && last_game_was_win && strcmp(event->GetName(), "mvm_wave_failed") == 0) {
			last_game_was_win = false;
			mgr->FreeEvent(event);
			return false;
		}
		
		return DETOUR_MEMBER_CALL(IGameEventManager2_FireEvent)(event, bDontBroadcast);
	}

	DETOUR_DECL_MEMBER(bool, CTFGameRules_ClientConnected, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
	{
		auto gamerules = reinterpret_cast<CTFGameRules *>(this);
		
		if (gamerules->IsMannVsMachineMode() && state.m_iTotalSpectators >= 0) {
			int spectators = 0;
			int assigned = 0;
			ForEachTFPlayer([&](CTFPlayer *player){
				if(player->IsRealPlayer()) {
					if (player->GetTeamNumber() == TF_TEAM_BLUE || player->GetTeamNumber() == TF_TEAM_RED)
						assigned++;
					else
						spectators++;
				}
			});
			if (spectators >= state.m_iTotalSpectators && assigned >= GetVisibleMaxPlayers()) {
				strcpy(reject, "Exceeded total spectator count of the mission");
				return false;
			}
		}
		
		return DETOUR_MEMBER_CALL(CTFGameRules_ClientConnected)(pEntity, pszName, pszAddress, reject, maxrejectlen);
	}


	DETOUR_DECL_MEMBER(float, CTeamplayRoundBasedRules_GetMinTimeWhenPlayerMaySpawn, CBasePlayer *player)
	{
		if (state.m_iPlayerMiniBossMinRespawnTime >= 0 && !player->IsFakeClient() && ToTFPlayer(player)->IsMiniBoss() && !player->IsBot())
			return player->GetDeathTime() + state.m_iPlayerMiniBossMinRespawnTime;

		return DETOUR_MEMBER_CALL(CTeamplayRoundBasedRules_GetMinTimeWhenPlayerMaySpawn)(player);
	}
	
	DETOUR_DECL_MEMBER(bool, CTFBotTacticalMonitor_ShouldOpportunisticallyTeleport, CTFBot *bot)
	{
		return DETOUR_MEMBER_CALL(CTFBotTacticalMonitor_ShouldOpportunisticallyTeleport)(bot) && cvar_use_teleport.GetBool() && !bot->HasItem();
	}

	// CBaseAnimating * TemplateShootSpawn(CTFPlayer *player, CTFWeaponBase *weapon, bool &stopproj, std::function<CBaseAnimating *()> origShootFunc)
	// {
	// 	stopproj = false;
	// 	for(auto it = state.m_ShootTemplates.begin(); it != state.m_ShootTemplates.end(); it++) {
	// 		ShootTemplateData &temp_data = *it;
	// 		if (temp_data.weapon_classname != "" && !FStrEq(weapon->GetClassname(), temp_data.weapon_classname.c_str()))
	// 			continue;

	// 		if (temp_data.weapon != "" && !FStrEq(GetItemName(weapon->GetItem()), temp_data.weapon.c_str()))
	// 			continue;

	// 			// bool name_correct = FStrEq(weapon->GetItem()->GetStaticData()->GetName(), temp_data.weapon.c_str());

	// 			// if (!name_correct) {
	// 			// 	static int custom_weapon_def = -1;
	// 			// 	if (custom_weapon_def == -1) {
	// 			// 		auto attr = GetItemSchema()->GetAttributeDefinitionByName("custom weapon name");
	// 			// 		if (attr != nullptr)
	// 			// 			custom_weapon_def = attr->GetIndex();
	// 			// 	}
	// 			// 	auto attr = weapon->GetItem()->GetAttributeList().GetAttributeByID(custom_weapon_def);
	// 			// 	const char *value = nullptr;
	// 			// 	if (attr != nullptr && attr->GetValuePtr()->m_String != nullptr) {
	// 			// 		CopyStringAttributeValueToCharPointerOutput(attr->GetValuePtr()->m_String, &value);
	// 			// 	}
	// 			// 	if (value == nullptr || strcmp(value, temp_data.weapon.c_str()) != 0) {
	// 			// 		continue;
	// 			// 	}
	// 			// }
	// 		//}

	// 		if (temp_data.parent_to_projectile) {
	// 			CBaseAnimating *proj = origShootFunc();
	// 			if (proj != nullptr) {
	// 				Vector vec = temp_data.offset;
	// 				QAngle ang = temp_data.angles;
	// 				auto inst = temp_data.templ->SpawnTemplate(proj, vec, ang, true, nullptr);
	// 			}
				
	// 			return proj;
	// 		}
			
	// 		stopproj = temp_data.Shoot(player, weapon) | stopproj;
	// 	}
	// 	return nullptr;
	// }
	DETOUR_DECL_MEMBER(void, CTFWeaponBaseMelee_Smack )
	{
		auto weapon = reinterpret_cast<CTFWeaponBaseMelee*>(this);
		auto player = weapon->GetTFPlayerOwner();
		if (IsMannVsMachineMode() && player != nullptr && !player->IsFakeClient()) {
			bool stopproj;
			bool smacked = false;
			TemplateShootSpawn(state.m_ShootTemplates, player, weapon, stopproj, [&](){ smacked = true; DETOUR_MEMBER_CALL(CTFWeaponBaseMelee_Smack)(); return nullptr; });
			if (stopproj || smacked) {
				return;
			}
		}
 		DETOUR_MEMBER_CALL(CTFWeaponBaseMelee_Smack)();

	}

	DETOUR_DECL_MEMBER(CBaseAnimating *, CTFWeaponBaseGun_FireProjectile, CTFPlayer *player)
	{
		if (IsMannVsMachineMode() && !player->IsFakeClient()) {
			auto weapon = reinterpret_cast<CTFWeaponBaseGun*>(this);
			bool stopproj;
			auto proj = TemplateShootSpawn(state.m_ShootTemplates, player, weapon, stopproj, [&](){ return DETOUR_MEMBER_CALL(CTFWeaponBaseGun_FireProjectile)(player); });
			if (proj != nullptr) {
				return proj;
			}
			if (stopproj) {
				if (weapon->ShouldPlayFireAnim()) {
					player->DoAnimationEvent(PLAYERANIMEVENT_ATTACK_PRIMARY);
				}
				
				weapon->RemoveProjectileAmmo(player);
				weapon->m_flLastFireTime = gpGlobals->curtime;
				weapon->DoFireEffects();
				weapon->UpdatePunchAngles(player);
				
				if (player->m_Shared->IsStealthed() && weapon->ShouldRemoveInvisibilityOnPrimaryAttack()) {
					player->RemoveInvisibility();
				}
				return nullptr;
			}
		}
		
		return DETOUR_MEMBER_CALL(CTFWeaponBaseGun_FireProjectile)(player);
	}

	DETOUR_DECL_STATIC(int, GetBotEscortCount, int team)
	{
		return DETOUR_STATIC_CALL(GetBotEscortCount)(team) - state.m_iEscortBotCountOffset;
	}
	/*DETOUR_DECL_MEMBER(bool, CObjectSentrygun_FireRocket)
	{
		firerocket_sentrygun = reinterpret_cast<CObjectSentrygun *>(this);
		bool ret = DETOUR_MEMBER_CALL(CObjectSentrygun_FireRocket)();
		firerocket_sentrygun = nullptr;
		return ret;
	}*/


	bool IsInvalidWearableForModel(CTFPlayer *player, CEconWearable *wearable) 
	{
		auto item_view = wearable->GetItem();
		if (item_view != nullptr && item_view->GetItemDefinition() != nullptr) {

			const char *item_slot = item_view->GetItemDefinition()->GetKeyValues()->GetString("item_slot", "");
			int level = wearable->GetItem()->m_iEntityLevel;
			int itemid = item_view->m_iItemDefinitionIndex;
			
			if ((!player->IsBot() && ((FStrEq(item_slot, "head") || FStrEq(item_slot, "misc")) || itemid == -1)) && !IsItemApplicableToRobotModel(player, wearable, true)) {
				
				return true;
			}
		}
		return false;
	}

	THINK_FUNC_DECL(RemoveCosmeticDelay)
	{
		auto player = ToTFPlayer(this->GetOwnerEntity());
		// Disable cosmetics on robots, if player animations are not enabled
		if (player != nullptr && PlayerUsesRobotModel(player) && IsInvalidWearableForModel(player, reinterpret_cast<CTFWearable *>(this)))
			this->Remove();
	}

	DETOUR_DECL_MEMBER(void, CTFWearable_Equip, CBasePlayer *player)
	{
		CTFWearable *wearable = reinterpret_cast<CTFWearable *>(this); 
		
		THINK_FUNC_SET(wearable, RemoveCosmeticDelay, gpGlobals->curtime);
		DETOUR_MEMBER_CALL(CTFWearable_Equip)(player);
		
		if (ToTFBot(player) == nullptr) {
			for (auto &info : state.m_WeaponSpawnTemplates) {
				for (auto &entry : info.weapons) {
					if (entry->Matches(wearable->GetClassname(), wearable->GetItem())) {
						auto inst = info.info.SpawnTemplate(player);
						if (inst != nullptr)
							state.m_ItemEquipTemplates[wearable] = info.info.SpawnTemplate(player);
						break;
					}
				}
			}
		}
	}

	THINK_FUNC_DECL(EquipDelaySetHidden)
	{
		PrintToChatAll(CFmtStr("Set Hidden %d\n", this->GetEffects()));
		auto weapon = reinterpret_cast<CBaseCombatWeapon *>(this);
		if (weapon->GetOwner() != nullptr && weapon != weapon->GetOwner()->GetActiveWeapon()) {
			weapon->GetOwner()->Weapon_Switch(weapon);
			//this->SetEffects(this->GetEffects() | EF_NODRAW);
		}
	}
	
	THINK_FUNC_DECL(EquipDelaySetVisible)
	{
		PrintToChatAll("Set Visible\n");
		//this->SetEffects(this->GetEffects() & ~(EF_NODRAW));
	}
	
	THINK_FUNC_DECL(EquipDelaySetVisible2)
	{
		this->SetEffects(this->GetEffects() | EF_NODRAW);
		this->NetworkProp()->MarkPVSInformationDirty();
		this->DispatchUpdateTransmitState();
		//this->SetEffects(this->GetEffects() & ~(EF_NODRAW));
	}
	
	THINK_FUNC_DECL(EquipDelaySetVisible3)
	{
		this->SetEffects(this->GetEffects() & ~(EF_NODRAW));
		this->NetworkProp()->MarkPVSInformationDirty();
		this->DispatchUpdateTransmitState();
		//this->SetEffects(this->GetEffects() & ~(EF_NODRAW));
	}

	DETOUR_DECL_MEMBER(int, CBaseCombatWeapon_UpdateTransmitState)
	{
		auto wep = reinterpret_cast<CBaseCombatWeapon *>(this);
		auto owner = ToTFPlayer(wep->GetOwnerEntity());
		if (owner != nullptr && owner->GetActiveWeapon() != wep) {
			return wep->SetTransmitState(FL_EDICT_DONTSEND);
		}
		return DETOUR_MEMBER_CALL(CBaseCombatWeapon_UpdateTransmitState)();
	}

	// DETOUR_DECL_MEMBER(bool, CTFWeaponBase_Deploy)
	// {
	// 	auto wep = reinterpret_cast<CTFWeaponBase *>(this);
	// 	auto ret = DETOUR_MEMBER_CALL(CTFWeaponBase_Deploy)();
	// 	if (ret) {
	// 		wep->m_iState = WEAPON_IS_ACTIVE;
	// 	}
	// 	return ret;
	// }

	// DETOUR_DECL_MEMBER(bool, CTFWeaponBase_Holster, CBaseCombatWeapon *pSwitchingTo)
	// {
	// 	auto wep = reinterpret_cast<CTFWeaponBase *>(this);
	// 	auto ret = DETOUR_MEMBER_CALL(CTFWeaponBase_Holster)(pSwitchingTo);
	// 	if (ret) {
	// 		wep->m_iState = WEAPON_IS_CARRIED_BY_PLAYER;
	// 	}
	// 	return ret;
	// }

	RefCount rc_CTFPlayer_Spawn;
	DETOUR_DECL_MEMBER(void, CBaseCombatWeapon_Equip, CBaseCombatCharacter *owner)
	{
		auto ent = reinterpret_cast<CBaseCombatWeapon *>(this);
		DETOUR_MEMBER_CALL(CBaseCombatWeapon_Equip)(owner);
		if (!state.m_WeaponSpawnTemplates.empty() && rc_CTFPlayer_Spawn == 0) {
			if (ToTFBot(owner) == nullptr) {
				if (state.m_ItemEquipTemplates.find(ent) == state.m_ItemEquipTemplates.end()) {
					for (auto &info : state.m_WeaponSpawnTemplates) {
						for (auto &entry : info.weapons) {
							if (entry->Matches(ent->GetClassname(), ent->GetItem())) {
								auto inst = info.info.SpawnTemplate(owner);
								if (inst != nullptr)
									state.m_ItemEquipTemplates[ent] = inst;
								break;
							}
						}
					}
				}
			}
		}
	}

	DETOUR_DECL_MEMBER(void, CTFPlayerClassShared_SetCustomModel, const char *s1, bool b1)
	{
		DETOUR_MEMBER_CALL(CTFPlayerClassShared_SetCustomModel)(s1, b1);
		if (s1 != nullptr) {
			CTFPlayer *player = reinterpret_cast<CTFPlayerClassShared *>(this)->GetOuter();
			if (PlayerUsesRobotModel(player)) {
				int numwearables = player->GetNumWearables();
				for (int i = 0; i < numwearables; i++) {
					auto *wearable = player->GetWearable(i);
					if (wearable != nullptr && IsInvalidWearableForModel(player, wearable)) {
						wearable->Remove();
					}
				}
			}
		}
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_Spawn)
	{
		CTFPlayer *player = reinterpret_cast<CTFPlayer *>(this); 

		if (ToTFBot(player) == nullptr) {
			if (player->GetEntityModule<EntityModule>("parenttemplatemodule") != nullptr) {
				for (auto templ : g_templateInstances) {
					if (templ->parent == player && !templ->ignore_parent_alive_state) {
						templ->OnKilledParent(false);
					}
				}
			}
			
		}
		
		SCOPED_INCREMENT(rc_CTFPlayer_Spawn);
		DETOUR_MEMBER_CALL(CTFPlayer_Spawn)();
		if (ToTFBot(player) == nullptr && !state.m_WeaponSpawnTemplates.empty()) {
			ForEachTFPlayerEconEntity(player, [&](CEconEntity *entity) {
				if (!entity->IsMarkedForDeletion()) {
					for (auto &info : state.m_WeaponSpawnTemplates) {
						for (auto &entry : info.weapons) {
							if (entry->Matches(entity->GetClassname(), entity->GetItem())) {
								state.m_ItemEquipTemplates[entity] = info.info.SpawnTemplate(player);
								break;
							}
						}
					}
				}
			});
		}
	}

	DETOUR_DECL_MEMBER(void, CEconEntity_UpdateOnRemove)
	{
		
		auto entity = reinterpret_cast<CEconEntity *>(this);
		auto find = state.m_ItemEquipTemplates.find(entity);
		if (find != state.m_ItemEquipTemplates.end()) {
			find->second->OnKilledParent(false);
			state.m_ItemEquipTemplates.erase(find);
		}
        DETOUR_MEMBER_CALL(CEconEntity_UpdateOnRemove)();
    }

	RefCount rc_CTFSpellBook_RollNewSpell;
	DETOUR_DECL_MEMBER(void, CTFSpellBook_RollNewSpell, int tier, bool forceReroll)
	{
		SCOPED_INCREMENT(rc_CTFSpellBook_RollNewSpell);
		auto entity = reinterpret_cast<CTFSpellBook *>(this);
		state.m_SpellBookNextRollTier[entity] = tier;
        DETOUR_MEMBER_CALL(CTFSpellBook_RollNewSpell)(tier, forceReroll);
    }

	DETOUR_DECL_MEMBER(void, CTFSpellBook_SetSelectedSpell, int spell)
	{
		if (rc_CTFSpellBook_RollNewSpell) {
			DETOUR_MEMBER_CALL(CTFSpellBook_SetSelectedSpell)(spell);
			return;
		}

		auto entity = reinterpret_cast<CTFSpellBook *>(this);
		int tier = state.m_SpellBookNextRollTier[entity];
		if (tier == 0 && !state.m_SpellBookNormalRoll.empty()) {
			spell = RandomInt(0, state.m_SpellBookNormalRoll.size() - 1);

        	//DETOUR_MEMBER_CALL(CTFSpellBook_SetSelectedSpell)(state.m_SpellBookRareRoll[spell].first);
			entity->m_iSelectedSpellIndex = state.m_SpellBookNormalRoll[spell].first;
			entity->m_iSpellCharges = state.m_SpellBookNormalRoll[spell].second;
		}
		else if (tier == 1 && !state.m_SpellBookRareRoll.empty()) {
			spell = RandomInt(0, state.m_SpellBookRareRoll.size() - 1);
        	//DETOUR_MEMBER_CALL(CTFSpellBook_SetSelectedSpell)(state.m_SpellBookRareRoll[spell].first);
			entity->m_iSelectedSpellIndex = state.m_SpellBookRareRoll[spell].first;
			entity->m_iSpellCharges = state.m_SpellBookRareRoll[spell].second;
		}
		else {
        	DETOUR_MEMBER_CALL(CTFSpellBook_SetSelectedSpell)(spell);
		}
		state.m_SpellBookNextRollTier.erase(entity);
    }
	
	DETOUR_DECL_MEMBER(void, CUpgrades_PlayerPurchasingUpgrade, CTFPlayer *player, int itemslot, int upgradeslot, bool sell, bool free, bool b3)
	{
		if (!sell && !b3) {
			auto upgrade = reinterpret_cast<CUpgrades *>(this);
			
			if (upgradeslot >= 0 && upgradeslot < CMannVsMachineUpgradeManager::Upgrades().Count()) {
				const char *upgradename = upgrade->GetUpgradeAttributeName(upgradeslot);
				for (std::string &str : state.m_DisallowedUpgrades)
				{
					if (strtol(str.c_str(), nullptr, 0) == upgradeslot + 1)
					{	
						gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, CFmtStr("%s upgrade is not allowed in this mission", upgradename));
						return;
					}
					else if (FStrEq(upgradename, str.c_str())) {
						gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, CFmtStr("%s upgrade is not allowed in this mission", upgradename));
						return;
					}
				}
				for (auto &entry : state.m_DisallowedUpgradesExtra)
				{
					// Check name
					bool nameMatch = strtol(entry.name.c_str(), nullptr, 0) == upgradeslot + 1 || FStrEq(upgradename, entry.name.c_str());
					if (!nameMatch) continue;
						
					// Check upgrade level
                    int cur_step;
                    bool over_cap;
                    int max_step = GetUpgradeStepData(player, itemslot, upgradeslot, cur_step, over_cap);

					if (cur_step < entry.max) continue;

					// Check item match
					bool foundMatch = true;
					std::string incompatibleItem;
					if (!entry.entries.empty()) {
						foundMatch = false;
						if (entry.checkAllSlots) {
							ForEachTFPlayerEconEntity(player, [&](CEconEntity *item){
								if (item != nullptr && item->GetItem() != nullptr) {
									for(auto &entry : entry.entries) {
										if (entry->Matches(item->GetClassname(), item->GetItem())) {
											foundMatch = true;
											incompatibleItem = GetItemNameForDisplay(item->GetItem());
											return false;
										}
									}
								}
								return true;
							});
							
						}
						else {
							auto item = GetEconEntityAtLoadoutSlot(player, itemslot);
							if (item != nullptr && item->GetItem() != nullptr) {
								for(auto &entry : entry.entries) {
									if (entry->Matches(item->GetClassname(), item->GetItem())) {
										foundMatch = true;
										break;
									}
								}
							}
						}
					}

					if (foundMatch && !entry.ifUpgradePresent.empty()) {
						foundMatch = false;
						for (auto [name, level] : entry.ifUpgradePresent) {
							for (int i = 0; i < CMannVsMachineUpgradeManager::Upgrades().Count(); i++) {
								const char *upgradename2 = upgrade->GetUpgradeAttributeName(i);
								if (FStrEq(upgradename2, name.c_str())) {
									
									int cur_step;
									bool over_cap;
									int max_step = GetUpgradeStepData(player, itemslot, i, cur_step, over_cap);

									if (cur_step >= level) {
										foundMatch = true;
										incompatibleItem = name;
										break;
									}
								}
							}
						}
					}
					
					if (foundMatch) {
						gamehelpers->TextMsg(ENTINDEX(player), TEXTMSG_DEST_CENTER, CFmtStr("You cannot buy %s%s upgrades for this weapon in this mission\n%s%s", entry.max != 0 ? "more ": "", upgradename, 
						incompatibleItem.c_str(), !incompatibleItem.empty() ? " blocks this upgrade" : ""));
						return;
					}
				}
			}
		}
		DETOUR_MEMBER_CALL(CUpgrades_PlayerPurchasingUpgrade)(player, itemslot, upgradeslot, sell, free, b3);
	}

    DETOUR_DECL_MEMBER(bool, CObjectSentrygun_FindTarget)
    {
        bool ret{DETOUR_MEMBER_CALL(CObjectSentrygun_FindTarget)()};
        if(state.m_bNoWranglerShield){
            CObjectSentrygun* sentry{reinterpret_cast<CObjectSentrygun*>(this)};
            sentry->m_nShieldLevel = 0;
        }
        return ret;
    }

	void DisplayMainMissionInfo(CTFPlayer *player);
	void DisplayWhitelistInfo(CTFPlayer *player);
	void DisplayBlacklistInfo(CTFPlayer *player);
	void DisplayItemAttributeInfo(CTFPlayer *player);
	void DisplayItemAttributeListInfo(CTFPlayer *player, int id);
	void DisplayPlayerAttributeInfo(CTFPlayer *player);
	void DisplayPlayerAttributeListInfo(CTFPlayer *player, int id);
	void DisplayDescriptionInfo(CTFPlayer *player);
	void DisplayItemReplaceInfo(CTFPlayer *player);
	void DisplayDisallowedUpgradesInfo(CTFPlayer *player);
	void DisplayExtraLoadoutItemsClassInfo(CTFPlayer *player);
	void DisplayExtraLoadoutItemsInfo(CTFPlayer *player, int id);
	void DisplayForcedItemsClassInfo(CTFPlayer *player);
	void DisplayForcedItemsInfo(CTFPlayer *player, int id);

	IBaseMenu *DisplayExtraLoadoutItemsClass(CTFPlayer *player, int class_index, bool autoHide);
	IBaseMenu *DisplayExtraLoadoutItems(CTFPlayer *player, bool autoHide);
	IBaseMenu *DisplayExtraLoadoutItemsBuy(CTFPlayer *player, int itemId, bool autoHide);

	class SelectMainMissionInfoHandler : public IMenuHandler
    {
    public:

        SelectMainMissionInfoHandler(CTFPlayer * pPlayer) : IMenuHandler() {
            this->player = pPlayer;
            
        }

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
            const char *info = menu->GetItemInfo(item, nullptr);
            if (FStrEq(info, "whitelist")) {
				DisplayWhitelistInfo(player);
			}
			else if (FStrEq(info, "blacklist")) {
				DisplayBlacklistInfo(player);
			}
			else if (FStrEq(info, "itemattributes")) {
				DisplayItemAttributeInfo(player);
			}
			else if (FStrEq(info, "playerattributes")) {
				DisplayPlayerAttributeInfo(player);
			}
			else if (FStrEq(info, "description")) {
				DisplayDescriptionInfo(player);
			}
			else if (FStrEq(info, "disallowedupgrades")) {
				DisplayDisallowedUpgradesInfo(player);
			}
			else if (FStrEq(info, "itemreplace")) {
				DisplayItemReplaceInfo(player);
			}
			else if (FStrEq(info, "extraloadoutitems")) {
				DisplayExtraLoadoutItemsClassInfo(player);
			}
			else if (FStrEq(info, "forceditems")) {
				DisplayForcedItemsClassInfo(player);
			}
        }

		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
			//menu->Destroy(false);
			HandleSecurity sec(g_Ext.GetIdentity(), g_Ext.GetIdentity());
			handlesys->FreeHandle(menu->GetHandle(), &sec);
		}

        virtual void OnMenuDestroy(IBaseMenu *menu) {
            delete this;
        }

        CHandle<CTFPlayer> player;
    };

	class SelectMissionInfoHandler : public IMenuHandler
    {
    public:

        SelectMissionInfoHandler(CTFPlayer * pPlayer) : IMenuHandler() {
            this->player = pPlayer;
            
        }

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {

        }

		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
            if (reason == MenuEnd_ExitBack || reason == MenuEnd_Exit) {
                GoBack();
            }
			//menu->Destroy(false);
			HandleSecurity sec(g_Ext.GetIdentity(), g_Ext.GetIdentity());
			handlesys->FreeHandle(menu->GetHandle(), &sec);
		}

        virtual void OnMenuDestroy(IBaseMenu *menu) {
            delete this;
        }

		virtual void GoBack() {
            DisplayMainMissionInfo(player);
		}

        CHandle<CTFPlayer> player;
    };

	class SelectItemAttributeHandler : public SelectMissionInfoHandler
    {
    public:
        SelectItemAttributeHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			DisplayItemAttributeListInfo(player, id);
        }

		virtual void GoBack() {
            DisplayMainMissionInfo(player);
		}
    };

	class SelectItemAttributeListHandler : public SelectMissionInfoHandler
    {
    public:
        SelectItemAttributeListHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void GoBack() {
            DisplayItemAttributeInfo(player);
		}
    };

	class SelectPlayerAttributeHandler : public SelectMissionInfoHandler
    {
    public:
        SelectPlayerAttributeHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			DisplayPlayerAttributeListInfo(player, id);
        }

		virtual void GoBack() {
            DisplayMainMissionInfo(player);
		}
    };

	class SelectPlayerAttributeListHandler : public SelectMissionInfoHandler
    {
    public:
        SelectPlayerAttributeListHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void GoBack() {
            DisplayPlayerAttributeInfo(player);
		}
    };
	
	class SelectExtraLoadoutItemsClassInfoHandler : public SelectMissionInfoHandler
    {
    public:
        SelectExtraLoadoutItemsClassInfoHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			DisplayExtraLoadoutItemsInfo(player, id);
        }

		virtual void GoBack() {
            DisplayMainMissionInfo(player);
		}
    };
	

	class SelectExtraLoadoutItemsInfoHandler : public SelectMissionInfoHandler
    {
    public:
        SelectExtraLoadoutItemsInfoHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void GoBack() {
            DisplayExtraLoadoutItemsClassInfo(player);
		}
    };

	class SelectForcedItemsClassInfoHandler : public SelectMissionInfoHandler
    {
    public:
        SelectForcedItemsClassInfoHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			DisplayForcedItemsInfo(player, id);
        }

		virtual void GoBack() {
            DisplayMainMissionInfo(player);
		}
    };
	

	class SelectForcedItemsInfoHandler : public SelectMissionInfoHandler
    {
    public:
        SelectForcedItemsInfoHandler(CTFPlayer *pPlayer) : SelectMissionInfoHandler(pPlayer) {}

		virtual void GoBack() {
            DisplayForcedItemsClassInfo(player);
		}
    };

	class ExtraLoadoutItemsHander : public IMenuHandler
	{
	public:
		ExtraLoadoutItemsHander(bool autoHide) : IMenuHandler(), autoHide(autoHide) {}
		bool autoHide = false;

		virtual bool OnSetHandlerOption(const char *option, const void *data)
		{
			if (FStrEq(option, "SigsegvExtraLoadoutItemsIsAutoHide")) {
				*(bool *)data = autoHide;
				return true;
			}
			return false;
		}
	};

	class SelectExtraLoadoutItemsClassHandler : public ExtraLoadoutItemsHander
    {
    public:
        SelectExtraLoadoutItemsClassHandler(CTFPlayer *pPlayer, bool autoHide) : ExtraLoadoutItemsHander(autoHide) {
			this->player = pPlayer;
		}

		virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			if (player == nullptr)
				return;

			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			DisplayExtraLoadoutItemsClass(player, id, autoHide);
        }

		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
			//menu->Destroy(false);
			HandleSecurity sec(g_Ext.GetIdentity(), g_Ext.GetIdentity());
			handlesys->FreeHandle(menu->GetHandle(), &sec);
		}
		
        virtual void OnMenuDestroy(IBaseMenu *menu) {
            delete this;
        }

		CHandle<CTFPlayer> player;
    };

	class SelectExtraLoadoutItemsHandler : public ExtraLoadoutItemsHander
    {
    public:

        SelectExtraLoadoutItemsHandler(CTFPlayer * pPlayer, bool autoHide) : ExtraLoadoutItemsHander(autoHide) {
            this->player = pPlayer;
        }

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int item) {
			int id = strtol(menu->GetItemInfo(item, nullptr), nullptr, 10);
			
			if (id >= state.m_ExtraLoadoutItems.size()) return;

			auto &set = state.m_SelectedLoadoutItems[player->GetSteamID()];
			auto find = set.find(id);
			if (find != set.end()) {
				set.erase(find);
			}
			else {
				auto &item = state.m_ExtraLoadoutItems[id];

				if (item.cost > 0) {
					DisplayExtraLoadoutItemsBuy(player, id, autoHide);
					return;
				}

				for (auto it = set.begin(); it != set.end(); ) {
					auto &item_compare = state.m_ExtraLoadoutItems[*it];
					if ((item_compare.class_index == item.class_index || item_compare.class_index == 0) && item_compare.loadout_slot == item.loadout_slot) {
						it = set.erase(it);
					}
					else {
						it++;
					}
				}
				set.insert(id);
			}
			
			if (this->player->IsAlive())
			{
				if (PointInRespawnRoom(this->player, this->player->WorldSpaceCenter(), false)) {
					player->ForceRegenerateAndRespawn();
				}
				else if (state.m_bExtraLoadoutItemsAllowEquipOutsideSpawn) {
					player->GiveDefaultItemsNoAmmo();
				}
				if (!TFGameRules()->InSetup()) {
					auto &item = state.m_ExtraLoadoutItems[id];
					auto weapon = rtti_cast<CTFWeaponBase *>(player->GetEntityForLoadoutSlot(item.loadout_slot));
					if (weapon != nullptr) {
						weapon->m_iClip1 = 0;
						weapon->m_iClip2 = 0;
						weapon->m_flEnergy = 0;
					}
				}
			}
			
			DisplayExtraLoadoutItemsClass(player, player->GetPlayerClass()->GetClassIndex(), autoHide);
        }

		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
			//menu->Destroy(false);
			HandleSecurity sec(g_Ext.GetIdentity(), g_Ext.GetIdentity());
			handlesys->FreeHandle(menu->GetHandle(), &sec);
		}
		
        virtual void OnMenuDestroy(IBaseMenu *menu) {
            delete this;
        }

        CHandle<CTFPlayer> player;
    };

	class SelectExtraLoadoutItemsBuyHandler : public ExtraLoadoutItemsHander
    {
    public:

        SelectExtraLoadoutItemsBuyHandler(CTFPlayer * pPlayer, int itemId, bool autoHide) : ExtraLoadoutItemsHander(autoHide) {
            this->player = pPlayer;
            this->itemId = itemId;
        }

        virtual void OnMenuSelect(IBaseMenu *menu, int client, unsigned int menuitem) {
			auto item = state.m_ExtraLoadoutItems[itemId];
			bool regenerate = false;
			CSteamID steamid;
			player->GetSteamID(&steamid);
			if (FStrEq(menu->GetItemInfo(menuitem, nullptr), "Buy")) {
				if (item.cost > 0 && player->GetCurrency() >= item.cost && state.m_BoughtLoadoutItems[steamid].count(itemId) == 0) {
					player->RemoveCurrency(item.cost);
					state.m_BoughtLoadoutItems[steamid].insert(itemId);
					// Equip bought item
					auto &set = state.m_SelectedLoadoutItems[player->GetSteamID()];
					for (auto it = set.begin(); it != set.end(); ) {
						auto &item_compare = state.m_ExtraLoadoutItems[*it];
						if ((item_compare.class_index == item.class_index || item_compare.class_index == 0) && item_compare.loadout_slot == item.loadout_slot) {
							it = set.erase(it);
						}
						else {
							it++;
						}
					}
					regenerate = true;
					set.insert(itemId);
				}
			}
			if (FStrEq(menu->GetItemInfo(menuitem, nullptr), "Sell")) {
				if (item.allow_refund && state.m_BoughtLoadoutItems[steamid].count(itemId) == 1) {
					player->RemoveCurrency(-item.cost);
					state.m_BoughtLoadoutItems[steamid].erase(itemId);
					regenerate = true;
				}
			}
			if (FStrEq(menu->GetItemInfo(menuitem, nullptr), "Equip")) {
				auto &set = state.m_SelectedLoadoutItems[player->GetSteamID()];
				for (auto it = set.begin(); it != set.end(); ) {
					auto &item_compare = state.m_ExtraLoadoutItems[*it];
					if ((item_compare.class_index == item.class_index || item_compare.class_index == 0) && item_compare.loadout_slot == item.loadout_slot) {
						it = set.erase(it);
					}
					else {
						it++;
					}
				}
				regenerate = true;

				set.insert(itemId);
			}
			if (FStrEq(menu->GetItemInfo(menuitem, nullptr), "Unequip")) {
				auto &set = state.m_SelectedLoadoutItems[player->GetSteamID()];
				set.erase(itemId);
				regenerate = true;
			}

			if (regenerate && this->player->IsAlive())
			{
				if (PointInRespawnRoom(this->player, this->player->WorldSpaceCenter(), false)) {
					player->ForceRegenerateAndRespawn();
				}
				else if (state.m_bExtraLoadoutItemsAllowEquipOutsideSpawn) {
					player->GiveDefaultItemsNoAmmo();
				}

			}
			
			DisplayExtraLoadoutItemsClass(player, player->GetPlayerClass()->GetClassIndex(), autoHide);
        }

		virtual void OnMenuEnd(IBaseMenu *menu, MenuEndReason reason)
		{
			if (reason == MenuEnd_ExitBack || reason == MenuEnd_Exit) {
			DisplayExtraLoadoutItemsClass(player, player->GetPlayerClass()->GetClassIndex(), autoHide);
			}
		}
		
        virtual void OnMenuDestroy(IBaseMenu *menu) {
            delete this;
        }

        CHandle<CTFPlayer> player;
        int itemId;
    };

	void DisplayMainMissionInfo(CTFPlayer *player)
	{
		SelectMainMissionInfoHandler *handler = new SelectMainMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
		DevMsg("Mission Menu\n");
        menu->SetDefaultTitle("Mission info menu");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		if (!state.m_bNoMissionInfo) {
			auto explanation = Mod::Pop::Wave_Extensions::GetWaveExplanation(0);
			if (explanation != nullptr && !explanation->empty()) {
				ItemDrawInfo info1("Mission Description", ITEMDRAW_DEFAULT);
				menu->AppendItem("description", info1);
			}
			if (!state.m_ItemWhitelist.empty()) {
				ItemDrawInfo info1("Item Whitelist", ITEMDRAW_DEFAULT);
				menu->AppendItem("whitelist", info1);
			}
			if (!state.m_ItemBlacklist.empty()) {
				ItemDrawInfo info1("Item Blacklist", ITEMDRAW_DEFAULT);
				menu->AppendItem("blacklist", info1);
			}
			if (!state.m_ItemAttributes.empty()) {
				ItemDrawInfo info1("Item Attributes", ITEMDRAW_DEFAULT);
				menu->AppendItem("itemattributes", info1);
			}
			bool player_empty = state.m_PlayerAttributes.empty();
			for (int i = 0; i < 10; i++) {
				if (!state.m_PlayerAttributesClass[i].empty()) {
					player_empty = false;
					break;
				}
			}
			if (!player_empty) {
				ItemDrawInfo info1("Player Attributes", ITEMDRAW_DEFAULT);
				menu->AppendItem("playerattributes", info1);
			}
			if (!state.m_DisallowedUpgrades.empty()) {
				ItemDrawInfo info1("Disallowed Upgrades", ITEMDRAW_DEFAULT);
				menu->AppendItem("disallowedupgrades", info1);
			}
			if (!state.m_ItemReplace.empty()) {
				ItemDrawInfo info1("Item Replacement", ITEMDRAW_DEFAULT);
				menu->AppendItem("itemreplace", info1);
			}
			if (!state.m_ExtraLoadoutItems.empty()) {
				ItemDrawInfo info1("Extra Loadout Items", ITEMDRAW_DEFAULT);
				menu->AppendItem("extraloadoutitems", info1);
			}
			if (state.m_ForceItems.parsed) {
				ItemDrawInfo info1("Forced Items", ITEMDRAW_DEFAULT);
				menu->AppendItem("forceditems", info1);
			}
			if (state.m_bSniperAllowHeadshots) {
				ItemDrawInfo info1("Sniper bots can headshot", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (state.m_bSniperHideLasers) {
				ItemDrawInfo info1("No laser on Sniper bots", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (!state.m_RespecEnabled.GetValue()) {
				ItemDrawInfo info1("Upgrade refunding disabled", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (state.m_RespecLimit.GetValue() != 0) {
				static ConVarRef tf_mvm_respec_credit_goal("tf_mvm_respec_credit_goal");
				ItemDrawInfo info1(CFmtStr("Collect %d credits to earn an upgrade refund, up to %d times", tf_mvm_respec_credit_goal.GetInt(), state.m_RespecLimit.GetValue()), ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (state.m_ImprovedAirblast.GetValue()) {
				ItemDrawInfo info1("Pyro bots can airblast grenades and arrows", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (state.m_SandmanStuns.GetValue()) {
				ItemDrawInfo info1("Sandman balls can stun enemy targets", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			if (state.m_bNoReanimators) {
				ItemDrawInfo info1("No reanimators", ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
			
		}
		DevMsg("Item Count %d\n", menu->GetItemCount());
		if (menu->GetItemCount() == 1) {
            ItemDrawInfo info1(" ", ITEMDRAW_NOTEXT);
            menu->AppendItem(" ", info1);
        }
		else if (menu->GetItemCount() == 0) {
            ItemDrawInfo info1("No custom mission information available", ITEMDRAW_DISABLED);
            menu->AppendItem(" ", info1);
            ItemDrawInfo info2(" ", ITEMDRAW_NOTEXT);
            menu->AppendItem(" ", info2);
		}
		
        menu->Display(ENTINDEX(player), 10);
	}
	
	void DisplayWhitelistInfo(CTFPlayer *player)
	{
		SelectMissionInfoHandler *handler = new SelectMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Whitelisted Items");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (const auto& entry : state.m_ItemWhitelist) {
			const char *str = entry->GetInfo();
			ItemDrawInfo info1(str, ITEMDRAW_DISABLED);
			menu->AppendItem("attr", info1);
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayBlacklistInfo(CTFPlayer *player)
	{
		SelectMissionInfoHandler *handler = new SelectMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Blacklisted Items");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (const auto& entry : state.m_ItemBlacklist) {
			const char *str = entry->GetInfo();
			ItemDrawInfo info1(str, ITEMDRAW_DISABLED);
			menu->AppendItem("attr", info1);
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayItemReplaceInfo(CTFPlayer *player)
	{
		SelectMissionInfoHandler *handler = new SelectMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Item Replacement");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (const auto& entry : state.m_ItemReplace) {
			const char *from_str = entry.entry->GetInfo();
			const char *to_str = entry.name.c_str();
			ItemDrawInfo info1(CFmtStr("%s -> %s", from_str, to_str), ITEMDRAW_DISABLED);
			menu->AppendItem("attr", info1);
		}

        menu->Display(ENTINDEX(player), 0);
	}


	void DisplayDescriptionInfo(CTFPlayer *player)
	{
		SelectMissionInfoHandler *handler = new SelectMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Description");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		auto res = TFObjectiveResource();
		int wave_count = res->m_nMannVsMachineWaveCount;
		for (int i = 0; i < wave_count; i++) {
			auto explanation = Mod::Pop::Wave_Extensions::GetWaveExplanation(i);
			if (explanation != nullptr && !explanation->empty()) {
				for (const auto& entry : *explanation) {
					std::string str = entry;
					
					int pos = 0;
					while ((pos = str.find('{', pos)) != -1) {
						int posend = str.find('}', pos);
						if (posend == -1) {
							posend = str.size() - 1;
						}
						str.replace(pos, posend - pos + 1, "");
					}

					ItemDrawInfo info1(str.c_str(), ITEMDRAW_DISABLED);
					menu->AppendItem("info", info1);
				}
			}
		}

		

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayDisallowedUpgradesInfo(CTFPlayer *player)
	{
		SelectMissionInfoHandler *handler = new SelectMissionInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Disallowed Upgrades");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (const auto& entry : state.m_DisallowedUpgrades) {
			int id = strtol(entry.c_str(),nullptr,10);
			
			if (id > 0 && g_hUpgradeEntity.GetRef() != nullptr && id < CMannVsMachineUpgradeManager::Upgrades().Count()) {

				ItemDrawInfo info1(g_hUpgradeEntity->GetUpgradeAttributeName(id), ITEMDRAW_DISABLED);
				menu->AppendItem("info", info1);
			}
			else {
				ItemDrawInfo info1(entry.c_str(), ITEMDRAW_DISABLED);
				menu->AppendItem("info", info1);
			}
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayItemAttributeInfo(CTFPlayer *player)
	{
		SelectItemAttributeHandler *handler = new SelectItemAttributeHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Item Attributes");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		int i = 0;
		for (const auto& entry : state.m_ItemAttributes) {
			const char *str = entry.entry->GetInfo();
			ItemDrawInfo info1(str, ITEMDRAW_DEFAULT);
			std::string num = std::to_string(i);
			menu->AppendItem(num.c_str(), info1);
			i++;
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayItemAttributeListInfo(CTFPlayer *player, int id)
	{
		SelectItemAttributeListHandler *handler = new SelectItemAttributeListHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle(CFmtStr("%s Attributes", state.m_ItemAttributes[id].entry->GetInfo()));
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		if (id < (int)state.m_ItemAttributes.size()) {
			auto &attribs = state.m_ItemAttributes[id].attributes;
			for (auto &entry : attribs) {
				attribute_data_union_t value;
				auto attr_def = entry.first;
				attr_def->GetType()->InitializeNewEconAttributeValue(&value);
				if (attr_def->GetType()->BConvertStringToEconAttributeValue(attr_def, entry.second.c_str(), &value, true)) {
					std::string str;
					if (FormatAttributeString(str, attr_def, value)) {
						ItemDrawInfo info1(str.c_str(), ITEMDRAW_DISABLED);
						menu->AppendItem("", info1);
					}
				}
				attr_def->GetType()->UnloadEconAttributeValue(&value);
			}
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayPlayerAttributeInfo(CTFPlayer *player)
	{
		SelectPlayerAttributeHandler *handler = new SelectPlayerAttributeHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Player Attributes");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		if (!state.m_PlayerAttributes.empty())
		{
			ItemDrawInfo info1("All player classes", ITEMDRAW_DEFAULT);
			menu->AppendItem("0", info1);
		}

		for (int i = 0; i < 10; i++) {
			if (!state.m_PlayerAttributesClass[i].empty()) {
				ItemDrawInfo info1(g_aPlayerClassNames_NonLocalized[i], ITEMDRAW_DEFAULT);
				std::string num = std::to_string(i);
				menu->AppendItem(num.c_str(), info1);
			}
		}

        menu->Display(ENTINDEX(player), 0);
	}

	void DisplayPlayerAttributeListInfo(CTFPlayer *player, int id)
	{
		SelectPlayerAttributeListHandler *handler = new SelectPlayerAttributeListHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
		if (id == 0) {
			menu->SetDefaultTitle("All Class Attributes");
		}
		else {
        	menu->SetDefaultTitle(CFmtStr("%s Attributes", g_aPlayerClassNames_NonLocalized[id]));
		}
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		std::map<std::string, float> *map;

		if (id == 0) {
			map = &state.m_PlayerAttributes;
		}
		else {
			map = &state.m_PlayerAttributesClass[id];
		}
		for (auto &entry : *map) {
			attribute_data_union_t value;
			value.m_Float = entry.second;
			std::string str;
			if (FormatAttributeString(str, GetItemSchema()->GetAttributeDefinitionByName(entry.first.c_str()), value)) {
				ItemDrawInfo info1(str.c_str(), ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
		}

        menu->Display(ENTINDEX(player), 0);
	}
	
	void DisplayForcedItemsClassInfo(CTFPlayer *player)
	{
		SelectForcedItemsClassInfoHandler *handler = new SelectForcedItemsClassInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Forced items");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		bool has_class[11] = {0};

		for (auto &items_class : {state.m_ForceItems.items, state.m_ForceItems.items_no_remove}) {
			for (int i = 0; i < 11; i++) {
				auto &vec = items_class[i]; 
				if (!vec.empty()) {
					has_class[i] = true;
					if (i == 0 || i == 10) {
						for (int j = 1; j < 10; j++) {
							has_class[j] = true;
						}
					}
				}
			}
		}
		for (int i = 1; i < 10; i++) {
			if (has_class[i]) {
				ItemDrawInfo info1(g_aPlayerClassNames_NonLocalized[i], ITEMDRAW_DEFAULT);
				std::string num = std::to_string(i);
				menu->AppendItem(num.c_str(), info1);
			}
		}
		
        menu->Display(ENTINDEX(player), 10);
	}

	void DisplayForcedItemsInfo(CTFPlayer *player, int id)
	{
		SelectForcedItemsInfoHandler *handler = new SelectForcedItemsInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle(CFmtStr("Forced items (%s)", g_aPlayerClassNames_NonLocalized[id]));
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (auto &items_class : {state.m_ForceItems.items, state.m_ForceItems.items_no_remove}) {
			for (auto &vec : {items_class[id], items_class[0], items_class[10]}) {
				for (size_t i = 0; i < vec.size(); i++) {
					auto &item = vec[i];
					char buf[256];
					snprintf(buf, sizeof(buf), "%s", item.name.c_str());
					ItemDrawInfo info1(buf, ITEMDRAW_DISABLED);
					menu->AppendItem("", info1);
				}
			}
		}
		
        menu->Display(ENTINDEX(player), 10);
	}

	void DisplayExtraLoadoutItemsClassInfo(CTFPlayer *player)
	{
		SelectExtraLoadoutItemsClassInfoHandler *handler = new SelectExtraLoadoutItemsClassInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Extra loadout items");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		bool has_class[10] = {0};

		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];
			
			if (item.class_index == 0) {
				for (int j = 0; j < 10; j++) {
					has_class[j] = true;
				}
			}
			else {
				has_class[item.class_index] = true;
			}
		}
		for (int i = 1; i < 10; i++) {
			if (has_class[i]) {
				ItemDrawInfo info1(g_aPlayerClassNames_NonLocalized[i], ITEMDRAW_DEFAULT);
				std::string num = std::to_string(i);
				menu->AppendItem(num.c_str(), info1);
			}
		}
		
        menu->Display(ENTINDEX(player), 10);
	}

	void DisplayExtraLoadoutItemsInfo(CTFPlayer *player, int id)
	{
		SelectExtraLoadoutItemsInfoHandler *handler = new SelectExtraLoadoutItemsInfoHandler(player);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle(CFmtStr("Extra loadout items (%s)", g_aPlayerClassNames_NonLocalized[id]));
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];

			if (id == item.class_index || item.class_index == 0) {
				char buf[256];
				snprintf(buf, sizeof(buf), "%s: %s", loadoutStrings[item.loadout_slot], item.name.c_str());
				ItemDrawInfo info1(buf, ITEMDRAW_DISABLED);
				menu->AppendItem("", info1);
			}
		}
		
        menu->Display(ENTINDEX(player), 10);
	}
	
	class EmptyHandler : public IMenuHandler
	{
	public:
		EmptyHandler() : IMenuHandler() {
		}
	};
	EmptyHandler empty_handler_def;

	THINK_FUNC_DECL(PlayerExtraLoadoutMenuThink)
	{
		bool nextThink = false;
		auto player = reinterpret_cast<CTFPlayer *>(this);
		void *menu = nullptr;
		if (menus->GetDefaultStyle()->GetClientMenu(player->entindex(), &menu) == MenuSource_BaseMenu && menu != nullptr && ((IBaseMenu *)menu)->GetHandler() != nullptr) {
			bool autoHide = false;
			((IBaseMenu *)menu)->GetHandler()->OnSetHandlerOption("SigsegvExtraLoadoutItemsIsAutoHide", &autoHide);
			if (autoHide) {
				nextThink = true;
				if (!player->m_Shared->m_bInUpgradeZone && !PointInRespawnRoom(player, player->GetAbsOrigin(), false)) {
					menus->GetDefaultStyle()->CancelClientMenu(ENTINDEX(player));
					auto panel = menus->GetDefaultStyle()->CreatePanel();
					ItemDrawInfo info1("", ITEMDRAW_RAWLINE);
					panel->DrawItem(info1);
					ItemDrawInfo info2("", ITEMDRAW_RAWLINE);
					panel->DrawItem(info2);
					panel->SetSelectableKeys(255);
					panel->SendDisplay(ENTINDEX(player), &empty_handler_def, 1);
					return;
				}
			}
		}
		
		if (nextThink) {
			this->SetNextThink(gpGlobals->curtime + 0.05f, "PlayerExtraLoadoutMenuThink");
		}
	}

	bool HasExtraLoadoutItems(int class_index) {
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			if (state.m_ExtraLoadoutItems[i].class_index == class_index || state.m_ExtraLoadoutItems[i].class_index == 0) {
				return true;
			}
		}
		return false;
	}

	bool IsExtraItemAvailable(int currentWave, CSteamID steamId, ExtraLoadoutItem &item, int itemId)
	{
		return currentWave >= item.min_wave && currentWave <= item.max_wave && !(item.hidden && !state.m_BoughtLoadoutItems[steamId].count(itemId));
	}

	IBaseMenu *DisplayExtraLoadoutItems(CTFPlayer *player, bool autoHide)
	{
		SelectExtraLoadoutItemsClassHandler *handler = new SelectExtraLoadoutItemsClassHandler(player, autoHide);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle("Extra loadout items");
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		bool has_class[10] = {0};

		bool hasHidden = false;
		int wave = TFObjectiveResource()->m_nMannVsMachineWaveCount;
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];
			
			if (!IsExtraItemAvailable(wave, player->GetSteamID(), item, i)) {
				hasHidden = true;
				continue;
			}

			if (item.class_index == 0) {
				for (int j = 0; j < 10; j++) {
					has_class[j] = true;
				}
			}
			else {
				has_class[item.class_index] = true;
			}
		}
		for (int i = 1; i < 10; i++) {
			if (has_class[i]) {
				ItemDrawInfo info1(g_aPlayerClassNames_NonLocalized[i], ITEMDRAW_DEFAULT);
				std::string num = std::to_string(i);
				menu->AppendItem(num.c_str(), info1);
			}
		}

		if (menu->GetItemCount() == 1) {
            ItemDrawInfo info1(" ", ITEMDRAW_NOTEXT);
            menu->AppendItem(" ", info1);
        }
		else if (menu->GetItemCount() == 0) {
            ItemDrawInfo info1(hasHidden ? "No extra loadout items currently available" : "No extra loadout items available", ITEMDRAW_DISABLED);
            menu->AppendItem(" ", info1);
            ItemDrawInfo info2(" ", ITEMDRAW_NOTEXT);
            menu->AppendItem(" ", info2);
		}

		if (autoHide)
			THINK_FUNC_SET(player, PlayerExtraLoadoutMenuThink, gpGlobals->curtime+0.05f);

        menu->Display(ENTINDEX(player), 10);
		return menu;
	}

	IBaseMenu *DisplayExtraLoadoutItemsClass(CTFPlayer *player, int class_index, bool autoHide)
	{
		SelectExtraLoadoutItemsHandler *handler = new SelectExtraLoadoutItemsHandler(player, autoHide);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
        menu->SetDefaultTitle(CFmtStr("Extra loadout items (%s)", g_aPlayerClassNames_NonLocalized[class_index]));
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		int wave = TFObjectiveResource()->m_nMannVsMachineWaveCount;
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];

			if ((class_index == item.class_index || item.class_index == 0) && wave >= item.min_wave && wave <= item.max_wave) {

				bool selected = state.m_SelectedLoadoutItems[player->GetSteamID()].count(i);

				if (item.hidden && !state.m_BoughtLoadoutItems[player->GetSteamID()].count(i)) {
					continue;
				}
				
				// Display item cost if not free
				char cost[32] = "";
				if (item.cost > 0 && !state.m_BoughtLoadoutItems[player->GetSteamID()].count(i)) {
					snprintf(cost, sizeof(cost), "($%d)", item.cost);
				}

				char buf[256];
				snprintf(buf, sizeof(buf), "%s: %s %s %s", loadoutStrings[item.loadout_slot], item.name.c_str(), cost, selected ? "(selected)" : "");
				ItemDrawInfo info1(buf, ITEMDRAW_DEFAULT);
				std::string num = std::to_string(i);
				menu->AppendItem(num.c_str(), info1);
			}
		}

		if (menu->GetItemCount() == 1) {
            ItemDrawInfo info1(" ", ITEMDRAW_NOTEXT);
            menu->AppendItem(" ", info1);
        }
		else if (menu->GetItemCount() == 0) {
            menu->Destroy();
			
			return DisplayExtraLoadoutItems(player, autoHide);
		}
		
		if (autoHide)
			THINK_FUNC_SET(player, PlayerExtraLoadoutMenuThink, gpGlobals->curtime+0.05f);
		
        menu->Display(ENTINDEX(player), 10);
		return menu;
	}

	IBaseMenu *DisplayExtraLoadoutItemsBuy(CTFPlayer *player, int itemId, bool autoHide)
	{
		SelectExtraLoadoutItemsBuyHandler *handler = new SelectExtraLoadoutItemsBuyHandler(player, itemId, autoHide);
        IBaseMenu *menu = menus->GetDefaultStyle()->CreateMenu(handler, g_Ext.GetIdentity());
        
		auto &item = state.m_ExtraLoadoutItems[itemId];

        menu->SetDefaultTitle(CFmtStr("%s", GetItemNameForDisplay(item.item)));
        menu->SetMenuOptionFlags(MENUFLAG_BUTTON_EXIT);

		auto &attrs = item.item->GetAttributeList().Attributes();
		for (int i = 0; i < attrs.Count(); i++) {
			auto &attr = attrs[i];

			std::string format_str;
			if (!FormatAttributeString(format_str, attr.GetStaticData(), *attr.GetValuePtr()))
				continue;
			ItemDrawInfo info1(format_str.c_str(), ITEMDRAW_DISABLED);
            menu->AppendItem(" ", info1);
		}
		
		CSteamID steamid;
		player->GetSteamID(&steamid);
		if (!state.m_BoughtLoadoutItems[steamid].count(itemId)) {
			char buf[256];
			snprintf(buf, sizeof(buf), "Buy ($%d)", item.cost);
			ItemDrawInfo info1(buf, player->GetCurrency() >= item.cost ? ITEMDRAW_DEFAULT : ITEMDRAW_DISABLED);
			menu->AppendItem("Buy", info1);
		}

		if (state.m_BoughtLoadoutItems[steamid].count(itemId)) {
			if (state.m_SelectedLoadoutItems[steamid].count(itemId)) {
				ItemDrawInfo info1("Unequip", ITEMDRAW_DEFAULT);
				menu->AppendItem("Unequip", info1);
			}
			else {
				ItemDrawInfo info1("Equip", ITEMDRAW_DEFAULT);
				menu->AppendItem("Equip", info1);
			}

			if (item.allow_refund) {
				char buf[256];
				snprintf(buf, sizeof(buf), "Sell ($%d)", item.cost);
				ItemDrawInfo info2("Sell", ITEMDRAW_DEFAULT);
				menu->AppendItem("Sell", info2);
			}
		}

        menu->Display(ENTINDEX(player), 10);
		return menu;
	}
	
	DETOUR_DECL_MEMBER(bool, CTFPlayer_ClientCommand, const CCommand& args)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		if (player != nullptr) {
			if (strcmp(args[0], "sig_missioninfo") == 0) {
				DisplayMainMissionInfo(player);
				return true;
			}
			else if (strcmp(args[0], "sig_missionitems") == 0) {
				DisplayExtraLoadoutItemsClass(player, player->GetPlayerClass()->GetClassIndex(), false);
				return true;
			}
		}
		
		return DETOUR_MEMBER_CALL(CTFPlayer_ClientCommand)(args);
	}
	
	DETOUR_DECL_STATIC(void, Host_Say, edict_t *edict, const CCommand& args, bool team )
	{
		CBaseEntity *entity = GetContainingEntity(edict);
		if (edict != nullptr) {
			const char *p = args.ArgS();
			int len = strlen(p);
			if (*p == '"')
			{
				p++;
				len -=2;
			}
			if (strncmp(p, "!missioninfo",len) == 0 || strncmp(p, "/missioninfo",len) == 0) {
				DisplayMainMissionInfo(ToTFPlayer(entity));
				return;
			}
			if (strncmp(p, "!missionitems",len) == 0 || strncmp(p, "/missionitems",len) == 0) {
				DisplayExtraLoadoutItemsClass(ToTFPlayer(entity), ToTFPlayer(entity)->GetPlayerClass()->GetClassIndex(), false);
				return;
			}
		}
		DETOUR_STATIC_CALL(Host_Say)(edict, args, team);
	}
	
	RefCount rc_CTFPlayer_ModifyOrAppendCriteria;

	DETOUR_DECL_MEMBER(void, CTFPlayer_ModifyOrAppendCriteria, void *criteria)
	{
		SCOPED_INCREMENT_IF(rc_CTFPlayer_ModifyOrAppendCriteria, IsMannVsMachineMode() && state.m_bNoThrillerTaunt);
		DETOUR_MEMBER_CALL(CTFPlayer_ModifyOrAppendCriteria)(criteria);
	}

	DETOUR_DECL_MEMBER(void, CTFAmmoPack_InitAmmoPack, CTFPlayer * player, CTFWeaponBase *weapon, int i1, bool b1, bool b2, float f1)
	{
		SCOPED_INCREMENT_IF(rc_CTFPlayer_ModifyOrAppendCriteria, IsMannVsMachineMode() && state.m_bNoCritPumpkin);
		DETOUR_MEMBER_CALL(CTFAmmoPack_InitAmmoPack)(player, weapon, i1, b1, b2, f1);
	}

	DETOUR_DECL_STATIC(bool, TF_IsHolidayActive)
	{
		
		if (rc_CTFPlayer_ModifyOrAppendCriteria)
			return false;
		else
			return DETOUR_STATIC_CALL(TF_IsHolidayActive)();
	}

	DETOUR_DECL_MEMBER(void, CTFGameRules_BetweenRounds_Think)
	{
		float time_to_start = TFGameRules()->GetRestartRoundTime();
		DETOUR_MEMBER_CALL(CTFGameRules_BetweenRounds_Think)();
		if (state.m_flRestartRoundTime != -1.0f && TFGameRules()->GetRestartRoundTime() != -1.0f && TFGameRules()->GetRestartRoundTime() != time_to_start) {
			TFGameRules()->SetRestartRoundTime(gpGlobals->curtime + state.m_flRestartRoundTime);
		}
	}

	DETOUR_DECL_MEMBER(bool, CEngineSoundServer_PrecacheSound,const char *sound, bool flag1, bool flag2)
	{
		// Add different volume sounds to precache
		if (sound && sound[0] == '=') {
			char *pos;
			strtol(sound + 1, &pos, 10);
			sound = pos + 1;
		}
		return DETOUR_MEMBER_CALL(CEngineSoundServer_PrecacheSound)(sound, flag1, flag2);
	}

	int event_popfile = 0;
	int GetEventPopfile() {
		return event_popfile;
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_UpdateObjectiveResource)
	{
		DETOUR_MEMBER_CALL(CPopulationManager_UpdateObjectiveResource)();
		event_popfile = TFObjectiveResource()->m_nMvMEventPopfileType;
		if (state.m_bZombiesNoWave666)
			TFObjectiveResource()->m_nMvMEventPopfileType = 0;
	}

	DETOUR_DECL_MEMBER(bool, NextBotManager_ShouldUpdate, INextBot *bot)
	{
		if (state.m_bFastNPCUpdate) {
			auto ent = bot->GetEntity();
			if (ent != nullptr && !ent->IsPlayer()) {
				return true;
			}
		}
		
		return DETOUR_MEMBER_CALL(NextBotManager_ShouldUpdate)(bot);
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_StartCurrentWave)
	{
		state.m_PlayersByWaveStart.clear();
		ForEachTFPlayer([&](CTFPlayer *player) {
			if (!player->IsBot() && player->IsAlive()) {
				state.m_PlayersByWaveStart.insert(player);
			}
		});

		DETOUR_MEMBER_CALL(CPopulationManager_StartCurrentWave)();

		if (state.m_ScriptManager != nullptr) {
			ConColorMsg(Color(0xff, 0x00, 0x00, 0xff), "Wave #%d started\n", TFObjectiveResource()->m_nMannVsMachineWaveCount.Get());
			auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
			if (scriptManager->CheckGlobal("OnWaveStart")) {
				lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount);
				scriptManager->Call(1, 0);
			}
		}
	}

	DETOUR_DECL_MEMBER(bool, CTFPlayer_IsReadyToSpawn)
	{
		auto player = reinterpret_cast<CTFPlayer *>(this);
		//DevMsg(" Check spawn\n");
		if (state.m_bNoRespawnMidwave && TFGameRules()->State_Get() == GR_STATE_RND_RUNNING && !player->IsBot() && state.m_PlayersByWaveStart.count(player) == 0 ) {
			//DevMsg("Stop Spawn\n");
			return false;
		}
		return DETOUR_MEMBER_CALL(CTFPlayer_IsReadyToSpawn)();
	}

	DETOUR_DECL_MEMBER(void, CHeadlessHatmanAttack_RecomputeHomePosition)
	{
		if (state.m_bHHHNoControlPointLogic) {
			return;
		}
		DETOUR_MEMBER_CALL(CHeadlessHatmanAttack_RecomputeHomePosition)();
	}
	
	DETOUR_DECL_MEMBER(void, CTFPlayer_RemoveCurrency, int amount)
	{
		if (rc_CTFPlayer_Event_Killed && state.m_DeathPenalty.Get() != 0 && TFGameRules()->State_Get() != GR_STATE_RND_RUNNING) {
			return;
		}
		DETOUR_MEMBER_CALL(CTFPlayer_RemoveCurrency)(amount);
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_EndPurchasableUpgrades)
	{
		bool reset = false;
		if (!state.m_RespecEnabled.Get() && TFObjectiveResource()->m_nMannVsMachineWaveCount == 1u && TFGameRules()->State_Get() == GR_STATE_RND_RUNNING) {
			reset = true;
			TFObjectiveResource()->m_nMannVsMachineWaveCount = 2;
		}
		DETOUR_MEMBER_CALL(CTFPlayer_EndPurchasableUpgrades)();
		if (reset) {
			TFObjectiveResource()->m_nMannVsMachineWaveCount = 1;
		}
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_PlayerRunCommand, CUserCmd* cmd, IMoveHelper* moveHelper)
	{
		CTFPlayer* player = reinterpret_cast<CTFPlayer*>(this);
		if((state.m_iBunnyHop == 1) && player->IsAlive() && (cmd->buttons & 2) /*&& (player->GetFlags() & 1) */ && 
				(player->GetGroundEntity() == nullptr) && !player->CanAirDash()){
			cmd->buttons &= ~2;
		}
		// if (player->IsBot() && player->CanMoveDuringTaunt() && player->m_Shared->InCond(TF_COND_TAUNTING)) {
		// 	QAngle angMoveAngle = cmd->viewangles;
		// 	float flSign = cmd->sidemove != 0.f ? 1.f : -1.f;
		// 	float flMaxTurnSpeed = player->m_flTauntTurnSpeed;
		// 	float flTargetTurnSpeed = player->m_flTauntTurnSpeed;
		// 	if ( player->m_flTauntTurnAccelerationTime > 0.f )
		// 	{
		// 		flTargetTurnSpeed = clamp( flTargetTurnSpeed + flSign * ( gpGlobals->frametime / player->m_flTauntTurnAccelerationTime ) * flMaxTurnSpeed, 0.f, flMaxTurnSpeed );
		// 	}
		// 	else
		// 	{
		// 		flTargetTurnSpeed = flMaxTurnSpeed;
		// 	}
			
		// 	float flSmoothTurnSpeed = 0.f;
		// 	if ( flMaxTurnSpeed > 0.f )
		// 	{
		// 		flSmoothTurnSpeed = SimpleSpline( flTargetTurnSpeed / flMaxTurnSpeed ) * flMaxTurnSpeed;
		// 	}

		// 	// only let these button through
		// 	if ( cmd->sidemove < 0 )
		// 	{
		// 		angMoveAngle += QAngle( 0.f, flSmoothTurnSpeed * gpGlobals->frametime, 0.f );
		// 	}
		// 	else if( cmd->sidemove > 0 )
		// 	{
		// 		angMoveAngle += QAngle( 0.f, -flSmoothTurnSpeed * gpGlobals->frametime, 0.f );
		// 	}
		// 	cmd->buttons = cmd->buttons & ( IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK );
		// 	cmd->sidemove = 0.0f;

		// 	VectorCopy( angMoveAngle, cmd->viewangles );
		// }
		DETOUR_MEMBER_CALL(CTFPlayer_PlayerRunCommand)(cmd, moveHelper);
	}

	DETOUR_DECL_MEMBER(void, CTFGameMovement_PreventBunnyJumping)
	{
		if(!state.m_iBunnyHop){
			DETOUR_MEMBER_CALL(CTFGameMovement_PreventBunnyJumping)();
		}
	}

	DETOUR_DECL_MEMBER(bool, CHeadlessHatmanLocomotion_ShouldCollideWith, CBaseEntity *entity)
	{
		if (state.m_bHHHNonSolidToPlayers && entity->IsPlayer()) {
			return false;
		}
		return DETOUR_MEMBER_CALL(CHeadlessHatmanLocomotion_ShouldCollideWith)(entity);
	}

	RefCount rc_CHeadlessHatmanAttack_AttackTarget;
	CBaseEntity *hatman_target = nullptr;
	DETOUR_DECL_MEMBER(void, CHeadlessHatmanAttack_AttackTarget, CBaseEntity *hatman, CBaseEntity *target, float range)
	{
		SCOPED_INCREMENT(rc_CHeadlessHatmanAttack_AttackTarget);
		hatman_target = target;
		DETOUR_MEMBER_CALL(CHeadlessHatmanAttack_AttackTarget)( hatman, target, range);
	}

	DETOUR_DECL_STATIC(void, CalculateMeleeDamageForce, CTakeDamageInfo *info, Vector &vecMeleeDir, const Vector &vecForceOrigin, float flScale )
	{
		if (rc_CHeadlessHatmanAttack_AttackTarget) {
			vecMeleeDir.z = 0;
			hatman_target->SetAbsVelocity(hatman_target->GetAbsVelocity() + Vector(0,0,460));
		}
		/*DevMsg("MeleeDaamgeForce %f %d\n", vecMeleeDir.z, rc_CHeadlessHatmanAttack_AttackTarget);*/
		DETOUR_STATIC_CALL(CalculateMeleeDamageForce)(info, vecMeleeDir, vecForceOrigin, flScale);
	}

	DETOUR_DECL_MEMBER(void, CBasePlayer_ShowViewPortPanel, const char *name, bool show, KeyValues *kv)
	{
		if (IsMannVsMachineMode() && state.m_bSingleClassAllowed != -1 && (strcmp(name, "class_red") == 0 || strcmp(name, "class_blue") == 0 )) {
			CTFPlayer *player = reinterpret_cast<CTFPlayer *>(this);
			player->HandleCommand_JoinClass(g_aRawPlayerClassNames[state.m_bSingleClassAllowed]);
			return;
		}
		DETOUR_MEMBER_CALL(CBasePlayer_ShowViewPortPanel)( name, show, kv);
	}

	RefCount rc_CZombieBehavior_OnKilled;
	DETOUR_DECL_MEMBER(EventDesiredResult< CZombie >, CZombieBehavior_OnKilled, CZombie *zombie, const CTakeDamageInfo &info)
	{
		SCOPED_INCREMENT_IF(rc_CZombieBehavior_OnKilled, IsMannVsMachineMode() && state.m_bNoSkeletonSplit)
		return DETOUR_MEMBER_CALL(CZombieBehavior_OnKilled)( zombie, info);
	}

	DETOUR_DECL_STATIC(CBaseEntity*, CreateSpellSpawnZombie, CBaseCombatCharacter *pCaster, const Vector& vSpawnPosition, int nSkeletonType)
	{
		if (rc_CZombieBehavior_OnKilled) {
			return nullptr;
		}
		return DETOUR_STATIC_CALL(CreateSpellSpawnZombie)(pCaster, vSpawnPosition, nSkeletonType);
	}

	RefCount rc_ILocomotion_StuckMonitor;
	DETOUR_DECL_MEMBER(void, ILocomotion_StuckMonitor)
	{
		SCOPED_INCREMENT(rc_ILocomotion_StuckMonitor);
		DETOUR_MEMBER_CALL(ILocomotion_StuckMonitor)();
	}
	DETOUR_DECL_MEMBER(float, PlayerLocomotion_GetDesiredSpeed)
	{
		float ret = DETOUR_MEMBER_CALL(PlayerLocomotion_GetDesiredSpeed)();
		if (rc_ILocomotion_StuckMonitor) {
			ret *= state.m_fStuckTimeMult;
		}
		return ret;
	}

	DETOUR_DECL_STATIC(void, CPopulationManager_FindDefaultPopulationFileShortNames, CUtlVector<CUtlString> &vec)
	{
		DETOUR_STATIC_CALL(CPopulationManager_FindDefaultPopulationFileShortNames)(vec);
		
		KeyValues *kv = new KeyValues("kv");
		if (kv->LoadFromFile(filesystem, "banned_missions.txt")) {
			FOR_EACH_SUBKEY(kv, kv_mission) {
				if (stricmp(kv_mission->GetName(), STRING(gpGlobals->mapname)) == 0) {
					vec.FindAndRemove(kv_mission->GetString());
				}
			}
		}
		kv->deleteThis();
	}

	float vote_tf_mvm_popfile_time = 0.0f;
	DETOUR_DECL_MEMBER(void, CMannVsMachineChangeChallengeIssue_ExecuteCommand)
	{
		vote_tf_mvm_popfile_time = gpGlobals->curtime;
		DETOUR_MEMBER_CALL(CMannVsMachineChangeChallengeIssue_ExecuteCommand)();
	}

	DETOUR_DECL_STATIC(void, tf_mvm_popfile, const CCommand& args)
	{

		if (vote_tf_mvm_popfile_time + 10 > gpGlobals->curtime && vote_tf_mvm_popfile_time <= gpGlobals->curtime && args.ArgC() > 1) {
			CUtlVector<CUtlString> vec;
			CPopulationManager::FindDefaultPopulationFileShortNames(vec);
			if (vec.Find(args[1]) == -1 && !(FStrEq(args[1], STRING(gpGlobals->mapname)) && vec.Find("normal") != -1)) {
				return;
			}
		}
		DETOUR_STATIC_CALL(tf_mvm_popfile)(args);
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_DropCurrencyPack, int pack, int amount, bool forcedistribute, CTFPlayer *moneymaker )
	{
		int creditTeam = state.m_SetCreditTeam.Get();
		if (creditTeam == 0) {
			creditTeam = TF_TEAM_RED;
		}

		CTFPlayer *player = reinterpret_cast<CTFPlayer *>(this);		
		if (moneymaker != nullptr && TFTeamMgr()->GetTeam(creditTeam)->GetNumPlayers() > 0 && moneymaker->IsBot() && (creditTeam != 0 && moneymaker->GetTeamNumber() != creditTeam)) {
			// forcedistribute = false;
			// The moneymaker must be on the team that collects credits. If not, just find a random player on credit team
			moneymaker = (CTFPlayer *) TFTeamMgr()->GetTeam(creditTeam)->GetPlayer(RandomInt(0, TFTeamMgr()->GetTeam(creditTeam)->GetNumPlayers() - 1));
		}

		DETOUR_MEMBER_CALL(CTFPlayer_DropCurrencyPack)(pack, amount, forcedistribute, moneymaker);

	}

	DETOUR_DECL_MEMBER(void, CTFPowerup_DropSingleInstance, Vector &velocity, CBaseCombatCharacter *owner, float flThrowerTouchDelay, float flResetTime)
	{
		if (state.m_bNoCreditsVelocity && rtti_cast<CCurrencyPack *>(reinterpret_cast<CTFPowerup *>(this)) != nullptr) {
			velocity.Init();
		}
		DETOUR_MEMBER_CALL(CTFPowerup_DropSingleInstance)(velocity, owner, flThrowerTouchDelay, flResetTime);
	}

	DETOUR_DECL_MEMBER(void, CCurrencyPack_Spawn)
	{
		auto currency = reinterpret_cast<CCurrencyPack *>(this);
		if (state.m_bForceRedMoney && !currency->IsDistributed()) {
			auto maker = player_killer;
			
			int creditTeam = state.m_SetCreditTeam.Get();
			if (creditTeam == 0) {
				creditTeam = TF_TEAM_RED;
			}

			if (TFTeamMgr()->GetTeam(creditTeam)->GetNumPlayers() > 0 && (maker == nullptr || (maker->IsBot() && (maker != 0 && maker->GetTeamNumber() != creditTeam)))) {
				maker = (CTFPlayer *) TFTeamMgr()->GetTeam(creditTeam)->GetPlayer(RandomInt(0, TFTeamMgr()->GetTeam(creditTeam)->GetNumPlayers() - 1));
			}
			
			currency->DistributedBy(player_killer);
		}
		DETOUR_MEMBER_CALL(CCurrencyPack_Spawn)();
	}

	DETOUR_DECL_MEMBER(float, CTFGameRules_ApplyOnDamageAliveModifyRules, CTakeDamageInfo &info, CBaseEntity *entity, void *extra)
	{
		float damage = DETOUR_MEMBER_CALL(CTFGameRules_ApplyOnDamageAliveModifyRules)(info, entity, extra);
		if (damage < 0.0f && entity->IsPlayer() && state.m_bRestoreNegativeDamageHealing) {
			HealPlayer(ToTFPlayer(entity), ToTFPlayer(info.GetAttacker()), rtti_cast<CEconEntity *>(info.GetWeapon()), -damage, true, state.m_bRestoreNegativeDamageOverheal ? DMG_IGNORE_MAXHEALTH : DMG_GENERIC);
		}
		return damage;
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_RestoreCheckpoint)
	{
		DETOUR_MEMBER_CALL(CPopulationManager_RestoreCheckpoint)();
		// Reset bought loadout items
		ForEachTFPlayer([](CTFPlayer *player) {
			CSteamID steamid;
			player->GetSteamID(&steamid);
			auto &playerItems = state.m_BoughtLoadoutItems[steamid];
			auto &playerItemsCheckpoint = state.m_BoughtLoadoutItemsCheckpoint[steamid];
			auto &playerItemsSelected = state.m_SelectedLoadoutItems[steamid];
			playerItems = playerItemsCheckpoint;
			
			for (auto it = playerItemsSelected.begin(); it != playerItemsSelected.end(); ) {
				if (*it >= (int)state.m_ExtraLoadoutItems.size() || ((state.m_ExtraLoadoutItems[*it].cost != 0 || state.m_ExtraLoadoutItems[*it].hidden) && !playerItems.count(*it))) {
					it = playerItemsSelected.erase(it);
				}
				else {
					it++;
				}
			}
		});
	}

	DETOUR_DECL_MEMBER(void, CPopulationManager_SetCheckpoint, int wave)
	{
		DETOUR_MEMBER_CALL(CPopulationManager_SetCheckpoint)(wave);
		// Save bought items
		ForEachTFPlayer([](CTFPlayer *player) {
			CSteamID steamid;
			player->GetSteamID(&steamid);
			auto &playerItems = state.m_BoughtLoadoutItems[steamid];
			state.m_BoughtLoadoutItemsCheckpoint[steamid] = playerItems;
		});
	}

	class PlayerLoadoutUpdatedListener : public IBitBufUserMessageListener
	{
	public:

		CTFPlayer *player = nullptr;
		virtual void OnUserMessage(int msg_id, bf_write *bf, IRecipientFilter *pFilter)
		{
			if (pFilter->GetRecipientCount() > 0) {
				int id = pFilter->GetRecipientIndex(0);
				player = ToTFPlayer(UTIL_PlayerByIndex(id));
			}
		}
		virtual void OnPostUserMessage(int msg_id, bool sent)
		{
			if (sent && player != nullptr) {
				if (!player->IsBot())
					ApplyForceItems(state.m_ForceItems, player, false, false);
			}
		}
	};

	inline void OverrideParticle(char const *&name)
	{
		if (!state.m_ParticleOverride.empty()) {
			auto find = state.m_ParticleOverride.find(name);
			if (find != state.m_ParticleOverride.end()) {
				name = find->second.c_str();
			}
		}
	}

	DETOUR_DECL_STATIC(void, DispatchParticleEffect_2, const char *pszParticleName, ParticleAttachment_t iAttachType, CBaseEntity *pEntity, int iAttachmentPoint, bool bResetAllParticlesOnEntity)
	{
		OverrideParticle(pszParticleName);
		DETOUR_STATIC_CALL(DispatchParticleEffect_2)(pszParticleName, iAttachType, pEntity, iAttachmentPoint, bResetAllParticlesOnEntity);
	}
	
	DETOUR_DECL_STATIC(void, DispatchParticleEffect_6, const char *pszParticleName, ParticleAttachment_t iAttachType, CBaseEntity *pEntity, const char *pszAttachmentName, Vector vecColor1, Vector vecColor2, bool bUseColors, bool bResetAllParticlesOnEntity)
	{
		OverrideParticle(pszParticleName);
		DETOUR_STATIC_CALL(DispatchParticleEffect_6)(pszParticleName, iAttachType, pEntity, pszAttachmentName, vecColor1, vecColor2, bUseColors, bResetAllParticlesOnEntity);
	}

	DETOUR_DECL_STATIC(void, DispatchParticleEffect_3, const char *pszParticleName, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pEntity)
	{
		OverrideParticle(pszParticleName);
		DETOUR_STATIC_CALL(DispatchParticleEffect_3)(pszParticleName, vecOrigin, vecAngles, pEntity);
	}

	DETOUR_DECL_STATIC(void, DispatchParticleEffect_4, const char *pszParticleName, Vector vecOrigin, Vector vecStart, QAngle vecAngles, CBaseEntity *pEntity)
	{
		OverrideParticle(pszParticleName);
		DETOUR_STATIC_CALL(DispatchParticleEffect_4)(pszParticleName, vecOrigin, vecStart, vecAngles, pEntity);
	}

	DETOUR_DECL_STATIC(void, DispatchParticleEffect_7, const char *pszParticleName, Vector vecOrigin, QAngle vecAngles, Vector vecColor1, Vector vecColor2, bool bUseColors, CBaseEntity *pEntity, ParticleAttachment_t iAttachType)
	{
		OverrideParticle(pszParticleName);
		DETOUR_STATIC_CALL(DispatchParticleEffect_7)(pszParticleName, vecOrigin, vecAngles, vecColor1, vecColor2, bUseColors, pEntity, iAttachType);
	}

	DETOUR_DECL_MEMBER(void, CBaseObject_StartBuilding, CBaseEntity *builder)
	{
		DETOUR_MEMBER_CALL(CBaseObject_StartBuilding)(builder);
		auto obj = reinterpret_cast<CBaseObject *>(this);
		CTFPlayer *owner = obj->GetBuilder();
		if (owner != nullptr) {
			for (auto &entry : state.m_BuildingPointTemplates) {
				if (entry.building_type == obj->GetType() && entry.building_mode == obj->GetObjectMode() && (entry.allow_bots || !owner->IsBot())) {

					// If item entry is invalid, always spawn the building template
					
					bool found = !entry.entry;
					if (!found) {
						ForEachTFPlayerEconEntity(owner, [&](CEconEntity *entity) {
							if (entry.entry->Matches(entity->GetClassname(), entity->GetItem())) {
								found = true;
							}
						});
					}
					
					if (found) {
						entry.info.SpawnTemplate(obj);
					}
				}
			}
		}
	}

	void AwardExtraItem(CTFPlayer *player, std::string &name, bool equipNow) {
		
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];
			if (item.definitionName == name) {
				CSteamID steamid;
				player->GetSteamID(&steamid);
				state.m_BoughtLoadoutItems[steamid].insert(i);

				auto &set = state.m_SelectedLoadoutItems[steamid];
				for (auto it = set.begin(); it != set.end(); ) {
					auto &item_compare = state.m_ExtraLoadoutItems[*it];
					if ((item_compare.class_index == item.class_index || item_compare.class_index == 0) && item_compare.loadout_slot == item.loadout_slot) {
						it = set.erase(it);
					}
					else {
						it++;
					}
				}
				set.insert(i);
				if (equipNow && (item.class_index == 0 || item.class_index == player->GetPlayerClass()->GetClassIndex())) {
					GiveItemByName(player, name.c_str());
				}
			}
		}
	}

	void StripExtraItem(CTFPlayer *player, std::string &name, bool removeNow) {
		
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			auto &item = state.m_ExtraLoadoutItems[i];
			if (item.definitionName == name) {
				CSteamID steamid;
				player->GetSteamID(&steamid);
				bool removed = false;
				if (state.m_BoughtLoadoutItems[steamid].erase(i) != 0) 
					removed = true;

				if (state.m_SelectedLoadoutItems[steamid].erase(i) != 0) 
					removed = true;
				
				if (removeNow && removed && (item.class_index == 0 || item.class_index == player->GetPlayerClass()->GetClassIndex()))
					player->GiveDefaultItemsNoAmmo();
			}
		}
	}

	void ResetExtraItems(CTFPlayer *player) {
		bool removedSomething = false;
		for (size_t i = 0; i < state.m_ExtraLoadoutItems.size(); i++) {
			CSteamID steamid;
			player->GetSteamID(&steamid);
			if (state.m_BoughtLoadoutItems[steamid].erase(i) != 0) 
				removedSomething = true;

			if (state.m_SelectedLoadoutItems[steamid].erase(i) != 0) 
				removedSomething = true;
		}
		if (removedSomething) {
			player->GiveDefaultItemsNoAmmo();
		}
	}

	DETOUR_DECL_MEMBER(bool, CTFBotSpawner_Spawn, const Vector& where, CUtlVector<CHandle<CBaseEntity>> *ents)
	{
		auto result = DETOUR_MEMBER_CALL(CTFBotSpawner_Spawn)(where, ents);
		if (result && ents != nullptr && state.m_ScriptManager != nullptr) {
			auto player = ToTFBot(ents->Tail());
			if (player != nullptr) {
				auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
				if (scriptManager->CheckGlobal("OnWaveSpawnBot")) {
					Util::Lua::LEntityAlloc(scriptManager->GetState(), player);
					lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount);
					lua_newtable(scriptManager->GetState());
					int idx = 1;
					for (auto &tag : player->m_Tags) {
						lua_pushstring(scriptManager->GetState(), tag.Get());
						lua_rawseti(scriptManager->GetState(), -2, idx);
						idx++;
					}
					scriptManager->Call(3, 0);
				}
			}
		}
		return result;
	}

	DETOUR_DECL_MEMBER(bool, CTankSpawner_Spawn, const Vector& where, CUtlVector<CHandle<CBaseEntity>> *ents)
	{
		auto result = DETOUR_MEMBER_CALL(CTankSpawner_Spawn)(where, ents);
		if (result && ents != nullptr && state.m_ScriptManager != nullptr) {
			auto entity = ents->Tail();
			if (entity != nullptr) {
				auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
				if (scriptManager->CheckGlobal("OnWaveSpawnTank")) {
					Util::Lua::LEntityAlloc(scriptManager->GetState(), entity);
					lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount);
					scriptManager->Call(2, 0);
				}
			}
		}
		return result;
	}
	
	class CTFNavAreaIncursionLess
	{
	public:
		bool Less( const CTFNavArea *a, const CTFNavArea *b, void *pCtx )
		{
			int playerTeam = state.m_HumansMustJoinTeam.Get() ? TF_TEAM_BLUE : TF_TEAM_RED;
			return a->GetIncursionDistance(playerTeam) < b->GetIncursionDistance(playerTeam);
		}
	};

	inline float SkewedRandomValue()
	{
		float x = RandomFloat(0, 1.0f);
		float y = RandomFloat(0, 1.0f);
		return x < y ? y : x;	
	}

	DETOUR_DECL_MEMBER(int, CSpawnLocation_FindSpawnLocation, Vector& vSpawnPosition)
	{
		auto location = reinterpret_cast<CSpawnLocation *>(this);
		
		if (state.m_ScriptManager != nullptr) {
			auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
			if (scriptManager->CheckGlobal("GetWaveSpawnLocation")) {
				lua_pushstring(scriptManager->GetState(), state.m_SpawnLocations[location].name.c_str());
				scriptManager->Call(1, 2);
				int result = luaL_optinteger(scriptManager->GetState(), -2, -1);
				if (result != -1) {
                    auto vec = Util::Lua::LVectorGetNoCheckNoInline(scriptManager->GetState(), -1);
					if (vec != nullptr) {
						vSpawnPosition = *vec;
					}
					lua_settop(scriptManager->GetState(), 0);
					return result;
                }
				lua_settop(scriptManager->GetState(), 0);
			};
		}

		auto &data = state.m_SpawnLocations[location];
		if (data.advanced) {
			Msg("advanced spawn\n");
			VPROF_BUDGET( "CSpawnLocation::FindSpawnLocationAdvanced", "NextBot" );

			std::vector<Vector> playerPositions; 
			CUtlSortVector<CTFNavArea *, CTFNavAreaIncursionLess> theaterAreaVector;

			CTFNavArea::MakeNewTFMarker();

			ForEachTFPlayer([&](CTFPlayer *player) {

				if (!player->IsAlive() || !player->IsRealPlayer())
					return;
				playerPositions.push_back(player->GetAbsOrigin());
				// collect areas surrounding this invader
				CUtlVector< CNavArea * > nearbyAreaVector;
				CollectSurroundingAreas( &nearbyAreaVector, player->GetLastKnownArea(), data.maxDistance );

				for( int i=0; i<nearbyAreaVector.Count(); ++i )
				{
					CTFNavArea *area = (CTFNavArea *)nearbyAreaVector[i];
					if ( !area->IsTFMarked() )
					{		
						area->TFMark();

						if ( area->IsPotentiallyVisibleToTeam( player->GetTeamNumber() ) )
							continue;

						if ( !area->IsValidForWanderingPopulation() )
							continue;

						theaterAreaVector.Insert( area );
					}
				}
			});

			if (data.minDistance > 0) {
				for (int i = theaterAreaVector.Count() - 1; i >= 0; i--) {
					bool found = false;
					for (auto &vec : playerPositions) {
						if (theaterAreaVector[i]->GetCenter().DistToSqr(vec) < data.minDistance )
						{
							theaterAreaVector.FastRemove(i);
							found = true;
							break;
						}
					}
					if (found) {
						break;
					}
				}
			}
			
			if ( theaterAreaVector.Count() == 0 )
			{
				return NULL;
			}

			const int maxRetries = 5;
			CTFNavArea *spawnArea = NULL;

			for( int r=0; r<maxRetries; ++r )
			{
				int which = 0;

				switch(data.relative)
				{
				case AHEAD:
					// areas are sorted from behind to ahead - weight the selection to choose ahead
					which = SkewedRandomValue() * theaterAreaVector.Count();
					break;

				case BEHIND:
					// areas are sorted from behind to ahead - weight the selection to choose behind
					which = ( 1.0f - SkewedRandomValue() ) * theaterAreaVector.Count();
					break;

				case ANYWHERE:
					// choose any valid area at random
					which = RandomFloat( 0.0f, 1.0f ) * theaterAreaVector.Count();
					break;
				}

				if ( which >= theaterAreaVector.Count() )
					which = theaterAreaVector.Count()-1;

				spawnArea = theaterAreaVector[ which ];


				// well behaved spawn area
				vSpawnPosition = spawnArea->GetCenter();
				Msg("position %f %f %f %d \n", vSpawnPosition.x, vSpawnPosition.y, vSpawnPosition.z, spawnArea->GetTFAttributes());
				return SPAWN_LOCATION_NAV;
			}
			Msg("Not found\n");
			return SPAWN_LOCATION_NOT_FOUND;
		}
		return DETOUR_MEMBER_CALL(CSpawnLocation_FindSpawnLocation)(vSpawnPosition);
	}

	DETOUR_DECL_MEMBER(void, CDynamicProp_Spawn)
	{
		DETOUR_MEMBER_CALL(CDynamicProp_Spawn)();
		if (state.m_NoRomevisionCosmetics.GetValue()) {
			auto entity = reinterpret_cast<CBaseEntity *>(this);
			if (FStrEq(STRING(entity->GetModelName()), "models/bots/boss_bot/carrier_parts.mdl")) {
				entity->SetModelIndexOverride(VISION_MODE_ROME, entity->GetModelIndex());
			}
		}
	}

	DETOUR_DECL_MEMBER(int, CTFGameRules_GetTeamAssignmentOverride, CTFPlayer *pPlayer, int iWantedTeam, bool b1)
	{
		if (TFGameRules()->IsMannVsMachineMode() && pPlayer->IsRealPlayer()
			&& iWantedTeam == TF_TEAM_RED) {
				
			int totalPlayers = 0;
			ForEachTFPlayerOnTeam(TFTeamMgr()->GetTeam(TF_TEAM_RED), [&totalPlayers, pPlayer](CTFPlayer *player){
				if (player->IsRealPlayer() && pPlayer != player) {
					totalPlayers += 1;
				}
			});

			if (totalPlayers < iGetTeamAssignmentOverride) {
				Log( "MVM assigned %s to defending team (%d more slots remaining after us)\n", pPlayer->GetPlayerName(), iGetTeamAssignmentOverride - totalPlayers - 1 );
				// Set Their Currency
				int nRoundCurrency = MannVsMachineStats_GetAcquiredCredits();
				nRoundCurrency += g_pPopulationManager->m_nStartingCurrency;

				// deduct any cash that has already been spent
				int spentCurrency = g_pPopulationManager->GetPlayerCurrencySpent(pPlayer);
				pPlayer->SetCurrency(nRoundCurrency - spentCurrency);
				return TF_TEAM_RED;
			}
			else {
				// no room
				Log( "MVM assigned %s to spectator, all slots for defending team are in use, or reserved for lobby members\n",
				     pPlayer->GetPlayerName() );
				return TEAM_SPECTATOR;
			}
			
		}

		/* it's important to let the call happen, because pPlayer->m_nCurrency
		 * is set to its proper value in the call (stupid, but whatever) */
		auto iResult = DETOUR_MEMBER_CALL(CTFGameRules_GetTeamAssignmentOverride)(pPlayer, iWantedTeam, b1);
		
		return iResult;
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_RememberUpgrade, int iPlayerClass, CEconItemView *pItem, int iUpgrade, int nCost, bool bDowngrade )
	{
		int origItemDefId = ReplaceEconItemViewDefId(pItem);
		DETOUR_MEMBER_CALL(CTFPlayer_RememberUpgrade)(iPlayerClass, pItem, iUpgrade, nCost, bDowngrade);
		RestoreEconItemViewDefId(pItem, origItemDefId);
	}

	DETOUR_DECL_MEMBER(void, CTFPlayer_ForgetFirstUpgradeForItem, CEconItemView *pItem)
	{
		int origItemDefId = ReplaceEconItemViewDefId(pItem);
		DETOUR_MEMBER_CALL(CTFPlayer_RememberUpgrade)(pItem);
		RestoreEconItemViewDefId(pItem, origItemDefId);
	}

	DETOUR_DECL_MEMBER(bool, CSpawnLocation_Parse, KeyValues *kv)
	{
		const char *name = kv->GetName();
		const char *value = kv->GetString();

		if (*name == '\0')
			return false;

		if (FStrEq(name, "Where") || FStrEq(name, "ClosestPoint")) {
			auto &data = state.m_SpawnLocations[reinterpret_cast<CSpawnLocation *>(this)];
			
			if (kv->GetFirstSubKey() != nullptr) {
				data.name = "advanced";
				data.advanced = true;
				FOR_EACH_SUBKEY(kv, subkey) {
					if (FStrEq(subkey->GetName(), "MinDistance")) {
						data.minDistance = subkey->GetFloat();
					}
					else if (FStrEq(subkey->GetName(), "MaxDistance")) {
						data.maxDistance = subkey->GetFloat();
					}
					else if (FStrEq(subkey->GetName(), "Relative")) {
						if (FStrEq(subkey->GetString(), "Ahead")) {
							data.relative = AHEAD;
						}
						else if (FStrEq(subkey->GetString(), "Behind")) {
							data.relative = AHEAD;
						}
						else if (FStrEq(subkey->GetString(), "Anywhere")) {
							data.relative = ANYWHERE;
						}
					}
				}
				return true;
			}
			else {
				data.name = value;
				return DETOUR_MEMBER_CALL(CSpawnLocation_Parse)(kv);
			}
		}
		return false;
	}

	// DETOUR_DECL_STATIC(void, MessageWriteString,const char *name)
	// {
	// 	DevMsg("MessageWriteString %s\n",name);
	// 	DETOUR_STATIC_CALL(MessageWriteString)(name);
	// }
	
//	RefCount rc_CTFGameRules_FireGameEvent_teamplay_round_start;
//	DETOUR_DECL_MEMBER(void, CTFGameRules_FireGameEvent, IGameEvent *event)
//	{
//		SCOPED_INCREMENT_IF(rc_CTFGameRules_FireGameEvent_teamplay_round_start, FStrEq(event->GetName(), "teamplay_round_start"));
//		
//		DETOUR_MEMBER_CALL(CTFGameRules_FireGameEvent)(event);
//	}
//	
//	DETOUR_DECL_MEMBER(void, CTFPlayer_ChangeTeam, int iTeamNum, bool b1, bool b2, bool b3)
//	{
//		auto player = reinterpret_cast<CTFPlayer *>(this);
//		
//		if (rc_CTFGameRules_FireGameEvent_teamplay_round_start > 0 && IsMannVsMachineMode() && state.m_bReverseWinConditions && iTeamNum == TEAM_SPECTATOR) {
//			ConColorMsg(Color(0x00, 0xff, 0xff, 0xff), "BLOCKING ChangeTeam(TEAM_SPECTATOR) for player #%d \"%s\" on team %d\n",
//				ENTINDEX(player), player->GetPlayerName(), player->GetTeamNumber());
//			return;
//		}
//		
//		DETOUR_MEMBER_CALL(CTFPlayer_ChangeTeam)(iTeamNum, b1, b2, b3);
//	}
	
	
//	DETOUR_DECL_MEMBER(void, CPopulationManager_PostInitialize)
//	{
//		DETOUR_MEMBER_CALL(CPopulationManager_PostInitialize)();
//	}
	
	void Parse_DisallowedUpgrade(KeyValues *kv) {
		if (kv->GetFirstSubKey() == nullptr) {
			state.m_DisallowedUpgrades.push_back(kv->GetString());
		}
		else {
			DisallowUpgradeEntry &entry = state.m_DisallowedUpgradesExtra.emplace_back();
			FOR_EACH_SUBKEY(kv, subkey) {
				if (FStrEq(subkey->GetName(), "Upgrade")) {
					entry.name = subkey->GetString();
				}
				else if (FStrEq(subkey->GetName(), "MaxLevel")) {
					entry.max = subkey->GetInt();
				}
				else if (FStrEq(subkey->GetName(), "CheckAllSlots")) {
					entry.checkAllSlots = subkey->GetInt();
				}
				else if (FStrEq(subkey->GetName(), "IfUpgradePresent")) {
					FOR_EACH_SUBKEY(subkey, subkeyUpgrade) {
						entry.ifUpgradePresent.push_back({subkeyUpgrade->GetName(), subkeyUpgrade->GetInt()});
					}
				}
				else {
					auto itemEntry = Parse_ItemListEntry(subkey,"DisallowedUpgrade", false);
					if (itemEntry != nullptr)
						entry.entries.push_back(std::move(itemEntry));
				}
			}
		}
	}
	
	PlayerPointTemplateInfo Parse_PlayerSpawnTemplate(KeyValues *kv) {
		PlayerPointTemplateInfo info;
		info.info = Parse_SpawnTemplate(kv);
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "Class")) {
				info.class_index = GetClassIndexFromString(subkey->GetString());
			}
		}
		return info;
	}

	bool Parse_ItemListEntry(KeyValues *kv, std::vector<std::unique_ptr<ItemListEntry>> &list, const char *name) 
	{
		list.push_back(Parse_ItemListEntry(kv, name));
		/*if (FStrEq(kv->GetName(), "Classname")) {
			DevMsg("%s: Add Classname entry: \"%s\"\n", name, kv->GetString());
			list.push_back(std::make_unique<ItemListEntry_Classname>(kv->GetString()));
		} else if (FStrEq(kv->GetName(), "Name") || FStrEq(kv->GetName(), "ItemName")) {
			DevMsg("%s: Add Name entry: \"%s\"\n", name, kv->GetString());
			list.push_back(std::make_unique<ItemListEntry_Name>(kv->GetString()));
		} else if (FStrEq(kv->GetName(), "DefIndex")) {
			DevMsg("%s: Add DefIndex entry: %d\n", name, kv->GetInt());
			list.push_back(std::make_unique<ItemListEntry_DefIndex>(kv->GetInt()));
		} else {
			DevMsg("%s: Found DEPRECATED entry with key \"%s\"; treating as Classname entry: \"%s\"\n", name, kv->GetName(), kv->GetString());
			list.push_back(std::make_unique<ItemListEntry_Classname>(kv->GetString()));
		}*/
		return true;
	}

	void Parse_PlayerItemEquipSpawnTemplate(WeaponPointTemplateInfo &info, KeyValues *kv) {
		info.info = Parse_SpawnTemplate(kv);
		FOR_EACH_SUBKEY(kv, subkey) {
			if (!FStrEq(subkey->GetName(), "Name")) {
				auto entry = Parse_ItemListEntry(subkey,"ItemEquipSpawnTemplate", false);
				if (entry)
					info.weapons.push_back(std::move(entry));
			}
		}
		return;
	}

	void Parse_ItemWhitelist(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			Parse_ItemListEntry(subkey, state.m_ItemWhitelist, "ItemWhitelist");
		}
	}

	void Parse_ItemBlacklist(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			Parse_ItemListEntry(subkey, state.m_ItemBlacklist, "ItemBlackList");
		}
	}

	void Parse_ItemReplacement(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			CEconItemDefinition *item_def = GetItemSchema()->GetItemDefinitionByName(subkey->GetName());
			if (item_def != nullptr) {
				FOR_EACH_SUBKEY(subkey, subkey2) {
					CEconItemView *view = CEconItemView::Create();
					view->Init(item_def->m_iItemDefIndex);
					view->m_iItemID = RandomInt(INT_MIN, INT_MAX) + (uintptr_t)subkey;
					
					std::string name;
					if (state.m_CustomWeapons.find(subkey->GetString()) != state.m_CustomWeapons.end()) {
						name = subkey->GetString();
					}
					else {
						name = GetItemNameForDisplay(item_def->m_iItemDefIndex);
					}

					//if (state.m_CustomWeapons.find(subkey->GetName()) != state.m_CustomWeapons.end())
					Mod::Pop::PopMgr_Extensions::AddCustomWeaponAttributes(subkey->GetName(), view);
					state.m_ItemReplace.push_back({item_def, Parse_ItemListEntry(subkey2,"ItemReplacement"), view, name});
				}
			}
			
		}
	}

	void Parse_PlayerBuildingSpawnTemplate(KeyValues *kv) {
		BuildingPointTemplateInfo info;
		info.info = Parse_SpawnTemplate(kv);
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "BuildingType")) {
				auto buildingName = subkey->GetString();
				
				if (FStrEq(buildingName, "Sentry")) {
					info.building_type = OBJ_SENTRYGUN;
				}
				else if (FStrEq(buildingName, "Dispenser")) {
					info.building_type = OBJ_DISPENSER;
				}
				else if (FStrEq(buildingName, "Teleporter Entry")) {
					info.building_type = OBJ_TELEPORTER;
				}
				else if (FStrEq(buildingName, "Teleporter Exit")) {
					info.building_type = OBJ_TELEPORTER;
					info.building_mode = 1;
				}
				else if (FStrEq(buildingName, "Sapper")) {
					info.building_type = OBJ_ATTACHMENT_SAPPER;
				}
			}
			else if (FStrEq(subkey->GetName(), "AllowBots")) {
				info.allow_bots = subkey->GetBool();
			}
			else if (!FStrEq(subkey->GetName(), "Name")) {
				auto entry = Parse_ItemListEntry(subkey,"BuildingSpawnTemplate", false);
				if (entry) {
					info.entry = std::move(entry);
				}
			}
		}
		state.m_BuildingPointTemplates.push_back(std::move(info));
	}

	void Parse_ExtraLoadoutItemsClass(KeyValues *subkey2, int classname)
	{
		ExtraLoadoutItem item;
		item.class_index = classname;
		item.loadout_slot = GetSlotFromString(subkey2->GetName());
		const char *item_name = nullptr;
		if (subkey2->GetFirstSubKey() == nullptr) {
			item_name = subkey2->GetString();
		}
		else {
			FOR_EACH_SUBKEY(subkey2, subkey3) {
				const char *name = subkey3->GetName();
				if (FStrEq(name, "Item")) {
					item_name = subkey3->GetString();
				}
				else if (FStrEq(name, "Cost")) {
					item.cost = subkey3->GetInt();
				}
				else if (FStrEq(name, "AllowedMinWave")) {
					item.min_wave = subkey3->GetInt();
				}
				else if (FStrEq(name, "AllowedMaxWave")) {
					item.max_wave = subkey3->GetInt();
				}
				else if (FStrEq(name, "AllowRefund")) {
					item.allow_refund = subkey3->GetBool();
				}
				else if (FStrEq(name, "Hidden")) {
					item.hidden = subkey3->GetBool();
				}
			}
		}
		CEconItemDefinition *item_def = GetItemSchema()->GetItemDefinitionByName(item_name);
		if (item_def != nullptr) {
			item.item = CEconItemView::Create();
			item.item->Init(item_def->m_iItemDefIndex);
			item.item->m_iItemID = RandomInt(INT_MIN, INT_MAX) + (uintptr_t)subkey2;
			std::string name;
			if (state.m_CustomWeapons.find(item_name) != state.m_CustomWeapons.end()) {
				name = item_name;
			}
			else {
				name = GetItemNameForDisplay(item_def->m_iItemDefIndex);
			}
			Mod::Pop::PopMgr_Extensions::AddCustomWeaponAttributes(item_name, item.item);
			static int isExtraLoadoutItemId = GetItemSchema()->GetAttributeDefinitionByName("is extra loadout item")->GetIndex();
			item.item->GetAttributeList().SetRuntimeAttributeValueByDefID(isExtraLoadoutItemId, 1.0f);
			item.name = name;
			item.definitionName = item_name;
			if (!item.hidden) {
				state.m_ExtraLoadoutItemsNotify = true;
			}
			state.m_ExtraLoadoutItems.push_back(item);
		}
	}

	void Parse_ExtraLoadoutItems(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "AllowEquipOutsideSpawn")) {
				state.m_bExtraLoadoutItemsAllowEquipOutsideSpawn = subkey->GetBool();
				continue;
			}
			int classname = 0;
			for(int i=1; i < 11; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			
			if (classname != 0) {
				FOR_EACH_SUBKEY(subkey, subkey2) {
					Parse_ExtraLoadoutItemsClass(subkey2, classname);
				}
			}
			else {
				Parse_ExtraLoadoutItemsClass(subkey, classname);
			}
		}
	}
	bool checkClassLimitNextTick = false;

	void Parse_ClassBlacklist(KeyValues *kv)
	{
		int amount_classes_blacklisted = 0;
		int class_not_blacklisted = 0;
		FOR_EACH_SUBKEY(kv, subkey) {
			for(int i=1; i < 11; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					if (state.m_DisallowedClasses[i] != 0 && subkey->GetInt() == 0) {
						amount_classes_blacklisted+=1;
					}
					state.m_DisallowedClasses[i]= subkey->GetInt();
					
					break;
				}
			}
		}

		if (amount_classes_blacklisted == 8) {
			for(int i=1; i < 10; i++){
				if (state.m_DisallowedClasses[i] != 0) {	
					state.m_bSingleClassAllowed = i;
				}
			}
		}
		//for(int i=1; i < 10; i++){
		//	DevMsg("%d ",state.m_DisallowedClasses[i]);
		//}
		//DevMsg("\n");
		checkClassLimitNextTick = true;
	}

	void Parse_FlagResetTime(KeyValues *kv)
	{
		const char *name = nullptr; // required
		int reset_time = 0;         // required
		
		bool got_time = false;
		
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "Name")) {
				name = subkey->GetString();
			} else if (FStrEq(subkey->GetName(), "ResetTime")) {
				reset_time = subkey->GetInt();
				got_time = true;
			} else {
				Warning("Unknown key \'%s\' in FlagResetTime block.\n", subkey->GetName());
			}
		}
		
		bool fail = false;
		if (name == nullptr) {
			Warning("Missing Name key in FlagResetTime block.\n");
			fail = true;
		}
		if (!got_time) {
			Warning("Missing ResetTime key in FlagResetTime block.\n");
			fail = true;
		}
		if (fail) return;
		
		DevMsg("FlagResetTime: \"%s\" => %d\n", name, reset_time);
		state.m_FlagResetTimes.emplace(name, reset_time);
	}
	
	void Parse_ExtraSpawnPoint(KeyValues *kv)
	{
		const char *name = nullptr; // required
		int teamnum = TEAM_INVALID; // required
		Vector origin;              // required
		bool start_disabled = false;
		
		bool got_x = false;
		bool got_y = false;
		bool got_z = false;
		
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "Name")) {
				name = subkey->GetString();
			} else if (FStrEq(subkey->GetName(), "TeamNum")) {
				teamnum = Clamp(subkey->GetInt(), (int)TF_TEAM_RED, (int)TF_TEAM_BLUE);
			} else if (FStrEq(subkey->GetName(), "StartDisabled")) {
				start_disabled = subkey->GetBool();
			} else if (FStrEq(subkey->GetName(), "X")) {
				origin.x = subkey->GetFloat(); got_x = true;
			} else if (FStrEq(subkey->GetName(), "Y")) {
				origin.y = subkey->GetFloat(); got_y = true;
			} else if (FStrEq(subkey->GetName(), "Z")) {
				origin.z = subkey->GetFloat(); got_z = true;
			} else {
				Warning("Unknown key \'%s\' in ExtraSpawnPoint block.\n", subkey->GetName());
			}
		}
		
		bool fail = false;
		if (name == nullptr) {
			Warning("Missing Name key in ExtraSpawnPoint block.\n");
			fail = true;
		}
		if (teamnum == TEAM_INVALID) {
			Warning("Missing TeamNum key in ExtraSpawnPoint block.\n");
			fail = true;
		}
		if (!got_x) {
			Warning("Missing X key in ExtraSpawnPoint block.\n");
			fail = true;
		}
		if (!got_y) {
			Warning("Missing Y key in ExtraSpawnPoint block.\n");
			fail = true;
		}
		if (!got_z) {
			Warning("Missing Z key in ExtraSpawnPoint block.\n");
			fail = true;
		}
		if (fail) return;
		
		// Is it actually OK that we're spawning this stuff during parsing...?
		
		auto spawnpoint = rtti_cast<CTFTeamSpawn *>(CreateEntityByName("info_player_teamspawn"));
		if (spawnpoint == nullptr) {
			Warning("Parse_ExtraSpawnPoint: CreateEntityByName(\"info_player_teamspawn\") failed\n");
			return;
		}
		
		spawnpoint->SetName(AllocPooledString(name));
		
		spawnpoint->SetAbsOrigin(origin);
		spawnpoint->SetAbsAngles(vec3_angle);
		
		spawnpoint->ChangeTeam(teamnum);

		servertools->SetKeyValue(spawnpoint, "startdisabled", start_disabled ? "1" : "0");
		
		spawnpoint->Spawn();
		spawnpoint->Activate();
		
		state.m_ExtraSpawnPoints.emplace_back(spawnpoint);
		
		DevMsg("Parse_ExtraSpawnPoint: #%d, %08x, name \"%s\", teamnum %d, origin [ % 7.1f % 7.1f % 7.1f ], angles [ % 7.1f % 7.1f % 7.1f ]\n",
			ENTINDEX(spawnpoint), (uintptr_t)spawnpoint, STRING(spawnpoint->GetEntityName()), spawnpoint->GetTeamNumber(),
			VectorExpand(spawnpoint->GetAbsOrigin()), VectorExpand(spawnpoint->GetAbsAngles()));
	}
	
	void Parse_ExtraTankPath(KeyValues *kv)
	{
		const char *name = nullptr; // required
		std::vector<Vector> points; // required
		
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "Name")) {
				name = subkey->GetString();
			} else if (FStrEq(subkey->GetName(), "Node")) {
				Vector point; char c;
				if (sscanf(subkey->GetString(), " %f %f %f %c", &point.x, &point.y, &point.z, &c) == 3) {
					points.push_back(point);
				} else {
					Warning("Invalid value \'%s\' for Node key in ExtraTankPath block.\n", subkey->GetString());
				}
			} else {
				Warning("Unknown key \'%s\' in ExtraTankPath block.\n", subkey->GetName());
			}
		}
		
		bool fail = false;
		if (name == nullptr) {
			Warning("Missing Name key in ExtraTankPath block.\n");
			fail = true;
		}
		if (points.empty()) {
			Warning("No Node keys specified in ExtraTankPath block.\n");
			fail = true;
		}
		if (fail) return;
		
		state.m_ExtraTankPaths.emplace_back(name);
		auto& path = state.m_ExtraTankPaths.back();
		
		for (const auto& point : points) {
			if (!path.AddNode(point)) {
				Warning("Parse_ExtraTankPath: ExtraTankPath::AddNode reports that entity creation failed!\n");
				state.m_ExtraTankPaths.pop_back();
				return;
			}
		}
		
		
		DevMsg("Parse_ExtraTankPath: name \"%s\", %zu nodes\n", name, points.size());
	}

	void FindUsedEntityNames(std::string &str, FixupNames &entity_names, FixupNames &entity_names_used) {
		size_t pos = 0;
		size_t start = 0;

		while(pos <= str.size()){
			char c = pos < str.size() ? str[pos] : '\0';
			if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '*' || c == '$' || c == '_')) {
				std::string strsub = str.substr(start, pos-start);
				if (entity_names.contains(strsub)){
					entity_names_used.insert(strsub);
				}
				start = pos+1;
			}
			pos++;
		}
	}

	bool InsertFixupPattern(std::string &str, FixupNames &entity_names) {
		bool changed = false;
		size_t pos = 0;
		size_t start = 0;

		while(pos <= str.size()){
			char c = pos < str.size() ? str[pos] : '\0';
			if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '*' || c == '$' || c == '_')) {
				std::string strsub = str.substr(start, pos-start);
				if (entity_names.find(strsub) != entity_names.end()){
					str.insert(start,"\1");
					pos+=1;
					str.insert(pos,"\1");
					changed = true;
					//templ.fixup_names.insert(str);
				}
				start = pos+1;
			}
			pos++;
		}
		return changed;
	}
	
	void GetMinMaxFromSolid(KeyValues *kv, Vector &min, Vector &max)
	{
		float x1, x2, x3, y1, y2, y3, z1, z2, z3;
		FOR_EACH_SUBKEY(kv, side_kv) {
			
			if (FStrEq(side_kv->GetName(), "side")) {
				auto plane = side_kv->GetString("plane");
				sscanf(plane, "(%f %f %f) (%f %f %f) (%f %f %f)", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3);
				min.x = std::min({x1, x2, x3, min.x});
				min.y = std::min({y1, y2, y3, min.y});
				min.z = std::min({z1, z2, z3, min.z});
				max.x = std::max({x1, x2, x3, max.x});
				max.y = std::max({y1, y2, y3, max.y});
				max.z = std::max({z1, z2, z3, max.z});
			}
		}
	}
	
	void Parse_PointTemplate(KeyValues *kv)
	{
		
		std::string tname = kv->GetName();

		std::transform(tname.begin(), tname.end(), tname.begin(),
    	[](unsigned char c){ return std::tolower(c); });

		FixupNames entity_names;
		FixupNames entity_names_used;
		PointTemplate &templ = Point_Templates().emplace(tname,PointTemplate()).first->second;
		templ.name = tname;
		FOR_EACH_SUBKEY(kv, subkey) {

			const char *cname = subkey->GetName();
			if (FStrEq(cname, "OnSpawnOutput") || FStrEq(cname, "OnParentKilledOutput") || FStrEq(cname, "OnAllKilledOutput")){
				
				//auto input = templ.onspawn_inputs.emplace(templ.onspawn_inputs.end());
				std::string target;
				std::string action;
				float delay = 0.0f;
				std::string param;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					const char *name = subkey2->GetName();
					if (FStrEq(name, "Target")) {
						target = subkey2->GetString();
					}
					else if (FStrEq(name, "Action")) {
						action = subkey2->GetString();
					}
					else if (FStrEq(name, "Delay")) {
						delay = subkey2->GetFloat();
					}
					else if (FStrEq(name, "Param")) {
						param = subkey2->GetString();
					}
				}

				if (FStrEq(cname, "OnSpawnOutput")) {
					templ.on_spawn_triggers.push_back({target, action, param, delay});
				}
				else if (FStrEq(cname, "OnParentKilledOutput")) {
					templ.on_parent_kill_triggers.push_back({target, action, param, delay});
				}
				else if (FStrEq(cname, "OnAllKilledOutput")) {
					templ.on_kill_triggers.push_back({target, action, param, delay});
				}
				
				//DevMsg("Added onspawn %s %s \n", input->target.c_str(), input->input.c_str());
			}
			else if (FStrEq(cname, "NoFixup")){
				templ.no_fixup = subkey->GetBool();
			}
			else if (FStrEq(cname, "KeepAlive")){
				templ.keep_alive = subkey->GetBool();
			}
			else if (FStrEq(cname, "RemoveIfKilled")){
				templ.remove_if_killed = subkey->GetString();
			}
			else {
				EntityKeys keyvalues;

				const char *classname = cname;

				bool has_solid = false;
				Vector solid_min = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
				Vector solid_max = Vector(FLT_MIN, FLT_MIN, FLT_MIN);
				Vector origin = vec3_origin;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					const char *name = subkey2->GetName();
					if (FStrEq(name,"origin")){
						sscanf(subkey2->GetString(), "%f %f %f", &origin.x, &origin.y, &origin.z);
					}
					if (FStrEq(name,"classname")){
						classname = subkey2->GetString();
					}
					else if (subkey2->GetFirstSubKey() != NULL && FStrEq(name,"connections")){
						FOR_EACH_SUBKEY(subkey2, subkey3) {
							keyvalues.insert({subkey3->GetName(),subkey3->GetString()});
						}
					}
					else if (subkey2->GetFirstSubKey() != NULL && FStrEq(name,"solid")) {
						has_solid = true;
						DevMsg("Has solid\n");
						GetMinMaxFromSolid(subkey2, solid_min, solid_max);
					}
					else if (subkey2->GetFirstSubKey() == NULL)
						keyvalues.insert({name,subkey2->GetString()});
				}

				DevMsg("Has solid %d\n", has_solid);
				if (has_solid) {

					solid_min -= origin;
					solid_max -= origin;
					char buf[120];
					snprintf(buf, 120, "%f %f %f", solid_min.x, solid_min.y, solid_min.z);
					DevMsg("Min %s\n", buf);
					keyvalues.insert({"mins", buf});

					snprintf(buf, 120, "%f %f %f", solid_max.x, solid_max.y, solid_max.z);
					DevMsg("Max %s\n", buf);
					keyvalues.insert({"maxs", buf});

				}
				keyvalues.insert({"classname", classname});

				if (keyvalues.find("targetname") != keyvalues.end())
					entity_names.insert(keyvalues.find("targetname")->second);
				templ.entities.push_back(keyvalues);

				DevMsg("add Entity %s %s to template %s\n", cname, keyvalues.find("classname")->second.c_str(), tname.c_str());
			}
		}

		if (!templ.no_fixup) {
			for (auto &in : templ.on_spawn_triggers) {
				FindUsedEntityNames(in.target, entity_names, entity_names_used);
				FindUsedEntityNames(in.param, entity_names, entity_names_used);
			}
			for (auto &in : templ.on_parent_kill_triggers) {
				FindUsedEntityNames(in.target, entity_names, entity_names_used);
				FindUsedEntityNames(in.param, entity_names, entity_names_used);
			}
			FindUsedEntityNames(templ.remove_if_killed, entity_names, entity_names_used);
			
			for (auto it = templ.entities.begin(); it != templ.entities.end(); ++it){
				for (auto it2 = it->begin(); it2 != it->end(); ++it2){
					if (!FStrEq(it2->first.c_str(), "targetname")) {
						FindUsedEntityNames(it2->second, entity_names, entity_names_used);
					}
				}
			}

			for (auto &in : templ.on_spawn_triggers) {
				InsertFixupPattern(in.target, entity_names_used);
				InsertFixupPattern(in.param, entity_names_used);
			}
			for (auto &in : templ.on_parent_kill_triggers) {
				InsertFixupPattern(in.target, entity_names_used);
				InsertFixupPattern(in.param, entity_names_used);
			}
			InsertFixupPattern(templ.remove_if_killed, entity_names_used);
			
			for (auto it = templ.entities.begin(); it != templ.entities.end(); ++it){
				for (auto it2 = it->begin(); it2 != it->end(); ++it2){
					std::string str=it2->second;
						
					InsertFixupPattern(str, entity_names_used);
					it2->second = str;
				}
			}
		}
		templ.fixup_names = entity_names_used;
		

		CBaseEntity *maker = CreateEntityByName("env_entity_maker");
		maker->KeyValue("EntityTemplate",tname.c_str());
		maker->KeyValue("targetname",tname.c_str());
		DispatchSpawn(maker);
		maker->Activate();

		
	}

	void Parse_PointTemplates(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			Parse_PointTemplate(subkey);
		}
		
		DevMsg("Parse_PointTemplates\n");
	}
	
	void Parse_PlayerAttributes(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			int classname = 0;
			for(int i=1; i < 11; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			if (classname == 0) {
				if (GetItemSchema()->GetAttributeDefinitionByName(subkey->GetName()) != nullptr) {
					attribute_data_union_t value;
					LoadAttributeDataUnionFromString(GetItemSchema()->GetAttributeDefinitionByName(subkey->GetName()), value, subkey->GetString());
					state.m_PlayerAttributes[subkey->GetName()] = value.m_Float;
					DevMsg("Parsed attribute %s %f\n", subkey->GetName(),value.m_Float);
				}
			}
			else 
			{
				state.m_bDeclaredClassAttrib = true;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					if (GetItemSchema()->GetAttributeDefinitionByName(subkey2->GetName()) != nullptr) {
						attribute_data_union_t value;
						LoadAttributeDataUnionFromString(GetItemSchema()->GetAttributeDefinitionByName(subkey2->GetName()), value, subkey2->GetString());
						state.m_PlayerAttributesClass[classname][subkey2->GetName()] = value.m_Float;
						DevMsg("Parsed attribute %s %f\n", subkey2->GetName(),subkey2->GetFloat());
					}
				}
			}
		}
		
		DevMsg("Parsed attributes\n");
	}
	void Parse_PlayerAddCond(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			int classname = 0;
			for(int i=1; i < 11; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			std::vector<ETFCond> *conds = nullptr;
			if (classname == 0) {
				const char *name = subkey->GetName();
				if (FStrEq(name, "Index")) {
					state.m_PlayerAddCond.push_back((ETFCond)subkey->GetInt());
				} else if (FStrEq(name, "Name")) {
					ETFCond cond = GetTFConditionFromName(subkey->GetString());
					if (cond != -1) {
						state.m_PlayerAddCond.push_back(cond);
					} else {
						Warning("Unrecognized condition name \"%s\" in AddCond block.\n", subkey->GetString());
					}
				}
			}
			else {
				state.m_bDeclaredClassAttrib = true;
				FOR_EACH_SUBKEY(subkey, subkey2) {
					const char *name = subkey->GetName();
					if (FStrEq(name, "Index")) {
						state.m_PlayerAddCondClass[classname].push_back((ETFCond)subkey->GetInt());
					} else if (FStrEq(name, "Name")) {
						ETFCond cond = GetTFConditionFromName(subkey->GetString());
						if (cond != -1) {
							state.m_PlayerAddCondClass[classname].push_back(cond);
						} else {
							Warning("Unrecognized condition name \"%s\" in AddCond block.\n", subkey->GetString());
						}
					}
				}
			}
		}
		
		DevMsg("Parsed addcond\n");
	}

	void Parse_OverrideSounds(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			state.m_OverrideSounds[subkey->GetName()]=subkey->GetString();
			if (!enginesound->PrecacheSound(subkey->GetString(), true))
				CBaseEntity::PrecacheScriptSound(subkey->GetString());
		}
		
		DevMsg("Parsed addcond\n");
	}

	void Parse_CustomWeapon(KeyValues *kv)
	{
		CustomWeapon weapon;
		weapon.name = kv->GetName();
		FOR_EACH_SUBKEY(kv, subkey) {
			if (subkey->GetFirstSubKey() != nullptr) {
				Parse_CustomWeapon(subkey);
			}
			else if (FStrEq(subkey->GetName(), "Name")) {
				weapon.name = subkey->GetString();
			}
			else if (FStrEq(subkey->GetName(), "OriginalItemName")){
				weapon.originalId = static_cast<CTFItemDefinition *>(GetItemSchema()->GetItemDefinitionByName(subkey->GetString()));

				// Replace stock weapons with upgradeable versions. This is so the game properly replaces stock weapons with requested custom weapons in extra loadout
				if (weapon.originalId != nullptr && FStrEq(weapon.originalId->GetKeyValues()->GetString("item_quality"), "normal")) {
					auto upgradeable = static_cast<CTFItemDefinition *>(GetItemSchema()->GetItemDefinitionByName(fmt::format("upgradeable {}", subkey->GetString()).c_str()));
					if (upgradeable != nullptr) {
						weapon.originalId = upgradeable;
					}
				}
			}
			else {
				CEconItemAttributeDefinition *attr_def = GetItemSchema()->GetAttributeDefinitionByName(subkey->GetName());
				
				if (attr_def == nullptr) {
					Warning("[popmgr_extensions] Error: couldn't find any attributes in the item schema matching \"%s\".\n", subkey->GetName());
				}
				else
					weapon.attributes[attr_def] = subkey->GetString();
			}
		}
		if (!weapon.name.empty() && weapon.originalId != nullptr) {
			weapon.attributes[GetItemSchema()->GetAttributeDefinitionByName("custom weapon name")] = weapon.name;
			weapon.attributes[GetItemSchema()->GetAttributeDefinitionByName("custom weapon id")] = std::to_string(state.m_CustomWeapons.size() + 1);
			state.m_CustomWeapons[weapon.name] = weapon;
		}
	}

	void Parse_SpellBookRoll(KeyValues *kv, std::vector<std::pair<int, int>> &spell_roll)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			for (int i = 0; i < SPELL_TYPE_COUNT; i++) {
				if (FStrEq(subkey->GetName(), SPELL_TYPE[i])) {
					spell_roll.push_back({i, subkey->GetInt()});
					break;
				}
			}
		}
	}

	void Parse_HandModelOverride(KeyValues *kv)
	{
		FOR_EACH_SUBKEY(kv, subkey) {
			int classname = 0;
			for(int i=1; i < 11; i++){
				if(FStrEq(g_aRawPlayerClassNames[i],subkey->GetName())){
					classname=i;
					break;
				}
			}
			if (classname != 0) {
				state.m_HandModelOverride[classname] = subkey->GetString();
				CBaseEntity::PrecacheModel(subkey->GetString());
			}
		}
	}

	/*void Parse_SprayDecal(KeyValues *kv)
	{
		if (state.m_SprayDecals.size() >= 22) {
			Warning("[popmgr_extensions] There can only be up to 22 SprayDecal blocks working properly.\n");
		}

		SprayDecal decal;
		bool has_decal = false;
		FOR_EACH_SUBKEY(kv, subkey) {
			if (FStrEq(subkey->GetName(), "Texture")) {
				CRC32_t value;
				if (LoadUserDataFile(value, kv->GetString())) {
					decal.texture = value;
					has_decal = true;
				}
				else {
					Warning("[popmgr_extensions] Error: There is no texture named \"%s\" in SprayDecal.\n", subkey->GetName());
				}
			}
			else if (FStrEq(subkey->GetName(), "Origin")){
				sscanf(subkey->GetString(), "%f %f %f", &decal.pos.x, &decal.pos.y, &decal.pos.z);
			}
			else if (FStrEq(subkey->GetName(), "Angles")){
				sscanf(subkey->GetString(), "%f %f %f", &decal.angle.x, &decal.angle.y, &decal.angle.z);
			}
		}
		if (has_decal) {
			state.m_SprayDecals.push_back(decal);
		}
		else {
			Warning("[popmgr_extensions] Error: Missing texture key in SprayDecal.\n");
		}
	}*/
	/*DETOUR_DECL_MEMBER(void, CTFPlayer_ReapplyPlayerUpgrades)
	{
		DETOUR_MEMBER_CALL(CTFPlayer_ReapplyPlayerUpgrades)();
		auto player = reinterpret_cast<CTFPlayer *>(this);
		DevMsg("Adding attributes\n");
		for(auto it = state.m_PlayerAttributes.begin(); it != state.m_PlayerAttributes.end(); ++it){
			player->AddCustomAttribute("hidden maxhealth non buffed",it->second, 60.0f);
			player->AddCustomAttribute("move speed bonus",2.0f, 60.0f);
			player->AddCustomAttribute("fire rate bonus",0.5f, 60.0f);
			player->AddCustomAttribute(it->first.c_str(),it->second, 7200);
			DevMsg("Added attribute %s %f\n",it->first.c_str(), it->second);
		}
	}*/

	// In case popfile fails to load, restart the round (unless if its during popmgr spawn, because most other entities did not spawn yet)

	bool pop_parse_successful = false;
	DETOUR_DECL_MEMBER(void, CPopulationManager_JumpToWave, unsigned int wave, float f1)
	{
		DevMsg("[%8.3f] JumpToWave %d\n", gpGlobals->curtime, TFGameRules()->State_Get());
		
		pop_parse_successful = false;

		int state = TFGameRules()->State_Get();
		DETOUR_MEMBER_CALL(CPopulationManager_JumpToWave)(wave, f1);

		DevMsg("[%8.3f] JumpToWave %d %d\n", gpGlobals->curtime, TFGameRules()->State_Get(), pop_parse_successful);

		if (!pop_parse_successful && state != GR_STATE_PREGAME) {
			TFGameRules()->State_Transition(GR_STATE_PREROUND);
			TFGameRules()->State_Transition(GR_STATE_BETWEEN_RNDS);
		}

	}

	bool parse_warning = false;
	SpewOutputFunc_t LocalSpewOutputFunc = nullptr;
	SpewRetval_t Spew_ClientForward(SpewType_t type, const char *pMsg)
	{
		const char *ignoremsg = "Can't open download/scripts/items/";
		if (strncmp(pMsg, ignoremsg, strlen(ignoremsg)) == 0)
			return LocalSpewOutputFunc(type, pMsg);

		ignoremsg = "Setting CBaseEntity to non-brush model models/weapons/w_models/w_rocket.mdl";
		if (strncmp(pMsg, ignoremsg, strlen(ignoremsg)) == 0)
			return LocalSpewOutputFunc(type, pMsg);

		for (int i = 1; i <= gpGlobals->maxClients; ++i) {
			CBasePlayer *player = UTIL_PlayerByIndex(i);
			if (player != nullptr)
				engine->ClientPrintf( INDEXENT(i), pMsg );	
		}


		if (type == SPEW_WARNING || type == SPEW_ASSERT || type == SPEW_ERROR) {
			parse_warning = true;
		}

		return LocalSpewOutputFunc(type, pMsg);
	}

	ConVar cvar_parse_errors("sig_mvm_print_parse_errors", "1", FCVAR_NOTIFY,
		"Print mission parse errors in console");	

	/*THINK_FUNC_DECL(DoSprayDecal) {
		std::vector<CTFBot *> bots;
		ForEachTFBot([&](CTFBot *bot) {
			if (bots.size() < state.m_SprayDecals.size()) {
				bots.push_back(bot);
			}
		});

		for (int i = 0; i < bots.size(); i++) {
			auto bot = bots[i];
			auto &decal = state.m_SprayDecals[i];
			Vector forward;
			trace_t	tr;	

			AngleVectors(decal.angle, &forward);
			UTIL_TraceLine(decal.pos, decal.pos + forward * 2048, 
				MASK_SOLID_BRUSHONLY, bot, COLLISION_GROUP_NONE, & tr);

			UTIL_PlayerDecalTrace( &tr, ENTINDEX(bot) );
		}
	};*/
	
	// The mission had changed, reset player inputs and stuff
	DETOUR_DECL_MEMBER(void, CPopulationManager_ResetMap)
	{
		ForEachTFPlayer([&](CTFPlayer *player){ 
			player->m_OnUser1->DeleteAllElements();
			player->m_OnUser2->DeleteAllElements();
			player->m_OnUser3->DeleteAllElements();
			player->m_OnUser4->DeleteAllElements();
			player->SetName(NULL_STRING);
			delete player->GetExtraEntityData();

			player->m_extraEntityData = nullptr;
		});
		state.m_SelectedLoadoutItems.clear();
		state.m_BoughtLoadoutItems.clear();
		state.m_BoughtLoadoutItemsCheckpoint.clear();
		Mod::Etc::Mapentity_Additions::ClearFakeProp();

		if (state.m_ScriptManager != nullptr) {
			state.m_ScriptManager->Remove();
		}
		DETOUR_MEMBER_CALL(CPopulationManager_ResetMap)();
	}

	bool reading_popfile = false;
	RefCount rc_CPopulationManager_Parse;
	DETOUR_DECL_MEMBER(bool, CPopulationManager_Parse)
	{
		Msg("Parse\n");
	//	DevMsg("CPopulationManager::Parse\n");
		ForEachTFPlayer([&](CTFPlayer *player){
            if (!player->IsBot())
                menus->GetDefaultStyle()->CancelClientMenu(ENTINDEX(player));

			if (!player->IsAlive()) return;
			
			if (!state.m_ItemAttributes.empty()) {
				for (int i =0 ; i < player->WeaponCount(); i++) {
					if (player->GetWeapon(i) != nullptr) {
						player->GetWeapon(i)->Remove();
					}
				}
				for (int i =0 ; i < player->GetNumWearables(); i++) {
					if (player->GetWearable(i) != nullptr) {
						player->GetWearable(i)->Remove();
					}
				}
			}

			if (state.m_bRedPlayersRobots || state.m_bBluPlayersRobots){
				
				// Restore changes made by player animations on robot models
				auto find = state.m_Player_anim_cosmetics.find(player);
				if (find != state.m_Player_anim_cosmetics.end() && find->second != nullptr) {
					find->second->Remove();
					servertools->SetKeyValue(player, "rendermode", "0");
					player->SetRenderColorA(255);
					state.m_Player_anim_cosmetics.erase(find);
				}

				player->GetPlayerClass()->SetCustomModel("", true);
				player->UpdateModel();
			}
		});
				
		SetVisibleMaxPlayers();

		string_t popfileName = TFObjectiveResource()->m_iszMvMPopfileName;
		if (state.m_LastMissionName != STRING(popfileName)) {
			state.m_LastMissionName = STRING(popfileName);
		}
		
	//	if ( state.m_iRedTeamMaxPlayers > 0) {
	//		ResetMaxRedTeamPlayers(6);
	//	}
		if ( state.m_iTotalMaxPlayers > 0) {
			ResetMaxTotalPlayers(10);
		}

		std::string prevNavFile = state.m_CustomNavFile;
		state.Reset();
		
	//	Redirects parsing errors to the client
		if (cvar_parse_errors.GetBool()) {
			parse_warning = false;
			LocalSpewOutputFunc = GetSpewOutputFunc();
			SpewOutputFunc(Spew_ClientForward);
		}
		
		SCOPED_INCREMENT(rc_CPopulationManager_Parse);
		reading_popfile = true;
		bool ret = DETOUR_MEMBER_CALL(CPopulationManager_Parse)();
		reading_popfile = false;

		// Reset nav mesh
		if (state.m_CustomNavFile != prevNavFile) {
			
			string_t oldMapName = gpGlobals->mapname;
			if (!state.m_CustomNavFile.empty()) {

				if (!filesystem->FileExists(CFmtStr("maps/%s.nav", state.m_CustomNavFile.c_str()))) {
					Warning("The custom nav file %s might not exist\n", state.m_CustomNavFile.c_str());
				}
			}

			ForEachEntityByRTTI<CBaseCombatCharacter>([&](CBaseCombatCharacter *character) {
				character->ClearLastKnownArea();
			});

			if (!state.m_CustomNavFile.empty()) {
				gpGlobals->mapname = AllocPooledString(state.m_CustomNavFile.c_str());
			}
			TheNavMesh->Load();
			gpGlobals->mapname = oldMapName;
		}

		if (cvar_parse_errors.GetBool())
			SpewOutputFunc(LocalSpewOutputFunc);

		if (parse_warning) {
			PrintToChatAll("\x07""ffb2b2""It is possible that a parsing error had occured. Check console for details\n");
		}
		
		pop_parse_successful = ret;

		for(auto it = state.m_SpawnTemplates.begin(); it != state.m_SpawnTemplates.end(); it++) {
			it->SpawnTemplate(nullptr);
			//Msg("SpawnTemplateLate %s\n", it->template_name.c_str());
		}

		state.m_SpawnTemplates.clear();

		for (auto &path : state.m_ExtraTankPaths) {
			path.SpawnNodes();
		}

		if (state.m_ScriptManager == nullptr && (!state.m_Scripts.empty() || !state.m_ScriptFiles.empty())) {
			state.m_ScriptManager = CreateEntityByName("$script_manager");
			DispatchSpawn(state.m_ScriptManager);
			state.m_ScriptManager->SetName(AllocPooledString("popscript"));
			auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
			for (auto &script : state.m_ScriptFiles) {
            	scriptManager->DoFile(script.c_str(), true);
			}
			for (auto &script : state.m_Scripts) {
            	scriptManager->DoString(script.c_str(), true);
			}
			state.m_ScriptManager->Activate();
		}
		if (state.m_ScriptManager != nullptr) {
			auto scriptManager = state.m_ScriptManager->GetOrCreateEntityModule<Mod::Etc::Mapentity_Additions::ScriptModule>("script");
			if (scriptManager->CheckGlobal("OnWaveReset")) {
				lua_pushinteger(scriptManager->GetState(), TFObjectiveResource()->m_nMannVsMachineWaveCount);
				scriptManager->Call(1, 0);
			}
		}
		TogglePatches();
//		THINK_FUNC_SET(g_pPopulationManager, DoSprayDecal, gpGlobals->curtime+1.0f);
		return ret;
	}

	const char *include_instead_of_merging_key_names[] = {
		"ItemAttributes",
		"ForceItem",
		"ForceItemNoRemove",
		"FlagResetTime",
		"ExtraSpawnPoint",
		"ExtraTankPath",
		"DisallowUpgrade",
		"SpawnTemplate",
		"PlayerSpawnTemplate",
		"PlayerSpawnOnceTemplate",
		"PlayerItemEquipSpawnTemplate",
		"PlayerShootTemplate",
		"BuildingSpawnTemplate",
		"LuaScript"
	};
	DETOUR_DECL_MEMBER(void, KeyValues_MergeBaseKeys, CUtlVector<KeyValues *> &keys)
	{
		if (reading_popfile) {
			auto kv = reinterpret_cast<KeyValues *>(this);
			KeyValues *appendKvFirst = nullptr;
			KeyValues *appendKvLast = nullptr;
			for (int i = 0; i < keys.Count(); i++) {
				std::vector<KeyValues *> del_kv;
				FOR_EACH_SUBKEY(keys[i], subkey) {
					auto name = subkey->GetName();
					for (size_t j = 0; j < ARRAYSIZE(include_instead_of_merging_key_names); j++) {
						if (FStrEq(name, include_instead_of_merging_key_names[j])) {
							auto kvCopy = subkey->MakeCopy();
							if (appendKvFirst == nullptr) {
								appendKvFirst = kvCopy;
							}
							if (appendKvLast != nullptr) {
								appendKvLast->SetNextKey(kvCopy);
							}
							appendKvLast = kvCopy;
							del_kv.push_back(subkey);
						}
					}
				}
				for (auto subkey : del_kv) {
				//	DevMsg("Deleting key \"%s\"\n", subkey->GetName());
					keys[i]->RemoveSubKey(subkey);
					subkey->deleteThis();
				}
			}
			if (appendKvFirst != nullptr) {
				if (kv->GetFirstSubKey() != nullptr) {
					appendKvLast->SetNextKey(kv->GetFirstSubKey()->GetNextKey());
					kv->GetFirstSubKey()->SetNextKey(appendKvFirst);
				}
				else {
					kv->AddSubKey(appendKvFirst);
				}
			}
		}
		DETOUR_MEMBER_CALL(KeyValues_MergeBaseKeys)(keys);
	}

	RefCount rc_CPopulationManager_IsValidPopfile;
	DETOUR_DECL_MEMBER(bool, CPopulationManager_IsValidPopfile, CUtlString name)
	{
		SCOPED_INCREMENT(rc_CPopulationManager_IsValidPopfile);
		return true;
		//return DETOUR_MEMBER_CALL(CPopulationManager_IsValidPopfile)(name);
	}
    
	RefCount rc_Parse_Popfile;
    void Parse_Popfile(KeyValues* kv, IBaseFileSystem* filesystem)
    {
		SCOPED_INCREMENT(rc_Parse_Popfile);
		std::vector<KeyValues *> del_kv;
		
		// Parse PointTemplates first
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			if (FStrEq(name, "PointTemplates")) {
				Parse_PointTemplates(subkey);
				del_kv.push_back(subkey);
			}
		}
		
		FOR_EACH_SUBKEY(kv, subkey) {
			const char *name = subkey->GetName();
			
			bool del = true;
			if (FStrEq(name, "BotsDropSpells")) {
				state.m_SpellsEnabled.Set(subkey->GetBool());
			} else if (FStrEq(name, "SpellsEnabled")) {
				state.m_SpellsEnabled.Set(subkey->GetBool());
			} else if (FStrEq(name, "GiantsDropRareSpells")) {
				state.m_bGiantsDropRareSpells = subkey->GetBool();
			} else if (FStrEq(name, "SpellDropRateCommon")) {
				state.m_flSpellDropRateCommon = Clamp(subkey->GetFloat(), 0.0f, 1.0f);
			} else if (FStrEq(name, "SpellDropRateGiant")) {
				state.m_flSpellDropRateGiant = Clamp(subkey->GetFloat(), 0.0f, 1.0f);
			} else if (FStrEq(name, "SpellDropForBotsInTeam")) {
				if (FStrEq(subkey->GetString(), "Red")) {
					state.m_iSpellDropForTeam = 2;
				}
				else if (FStrEq(subkey->GetString(), "Blue")) {
					state.m_iSpellDropForTeam = 3;
				}
				else {
					state.m_iSpellDropForTeam = atoi(subkey->GetString());
				}
			} else if (FStrEq(name, "NoReanimators")) {
				state.m_bNoReanimators = subkey->GetBool();
			} else if (FStrEq(name, "NoMvMDeathTune")) {
				state.m_bNoMvMDeathTune = subkey->GetBool();
			} else if (FStrEq(name, "SniperHideLasers")) {
				state.m_bSniperHideLasers = subkey->GetBool();
			} else if (FStrEq(name, "SniperAllowHeadshots")) {
				state.m_bSniperAllowHeadshots = subkey->GetBool();
			} else if (FStrEq(name, "NoRedSniperBotHeadshots")) {
				state.m_bRedSniperNoHeadshots = subkey->GetBool();
			} else if (FStrEq(name, "DisableUpgradeStations")) {
				state.m_bDisableUpgradeStations = subkey->GetBool();
			} else if (FStrEq(name, "RemoveGrapplingHooks")) {
				state.m_flRemoveGrapplingHooks = subkey->GetFloat();
			} else if (FStrEq(name, "ReverseWinConditions")) {
				state.m_bReverseWinConditions = subkey->GetBool();
			} else if (FStrEq(name, "MedievalMode")) {
				state.m_MedievalMode.Set(subkey->GetBool());
			} else if (FStrEq(name, "GrapplingHook")) {
				state.m_GrapplingHook.Set(subkey->GetBool());
			} else if (FStrEq(name, "RespecEnabled")) {
				state.m_RespecEnabled.Set(subkey->GetBool());
			} else if (FStrEq(name, "RespecLimit")) {
				state.m_RespecLimit.Set(subkey->GetInt());
			} else if (FStrEq(name, "BonusRatioHalf")) {
				state.m_BonusRatioHalf.Set(subkey->GetFloat());
			} else if (FStrEq(name, "BonusRatioFull")) {
				state.m_BonusRatioFull.Set(subkey->GetFloat());
			} else if (FStrEq(name, "FixedBuybacks")) {
				state.m_FixedBuybacks.Set(subkey->GetBool());
			} else if (FStrEq(name, "BuybacksPerWave")) {
				state.m_BuybacksPerWave.Set(subkey->GetInt());
			} else if (FStrEq(name, "DeathPenalty")) {
				state.m_DeathPenalty.Set(subkey->GetInt());
			} else if (FStrEq(name, "SentryBusterFriendlyFire")) {
				state.m_SentryBusterFriendlyFire.Set(subkey->GetBool());
			} else if (FStrEq(name, "BotPushaway")) {
				state.m_BotPushaway.Set(subkey->GetBool());
			} else if (FStrEq(name, "HumansMustJoinTeam")) {
				
				if (FStrEq(subkey->GetString(),"blue")){
					state.m_HumansMustJoinTeam.Set(true);
					if (!state.m_AllowJoinTeamBlueMax.IsOverridden() )
						state.m_AllowJoinTeamBlueMax.Set(6);
					if (!state.m_SetCreditTeam.IsOverridden() )
						state.m_SetCreditTeam.Set(3);
					SetVisibleMaxPlayers();
					ForEachTFPlayer([&](CTFPlayer *player){
						if(player->GetTeamNumber() == TF_TEAM_RED && !player->IsBot()){
							player->ForceChangeTeam(TF_TEAM_BLUE, true);
						}
					});
				}
				else
					state.m_HumansMustJoinTeam.Set(false);
			} else if (FStrEq(name, "AllowJoinTeamBlue")) {
				state.m_AllowJoinTeamBlue.Set(subkey->GetBool());
				SetVisibleMaxPlayers();
			} else if (FStrEq(name, "AllowJoinTeamBlueMax")) {
				state.m_AllowJoinTeamBlueMax.Set(subkey->GetInt());
				SetVisibleMaxPlayers();
			} else if (FStrEq(name, "BluHumanFlagPickup")) {
				state.m_BluHumanFlagPickup.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluHumanFlagCapture")) {
				state.m_BluHumanFlagCapture.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluHumanInfiniteAmmo")) {
				state.m_BluHumanInfiniteAmmo.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluHumanInfiniteCloak")) {
				state.m_BluHumanInfiniteCloak.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluPlayersAreRobots")) {
				state.m_bBluPlayersRobots = subkey->GetBool();
			} else if (FStrEq(name, "RedPlayersAreRobots")) {
				state.m_bRedPlayersRobots = subkey->GetBool();
			} else if (FStrEq(name, "FixSetCustomModelInput")) {
				state.m_bFixSetCustomModelInput = subkey->GetBool();
			} else if (FStrEq(name, "BotsAreHumans")) {
				state.m_BotsHumans.Set(subkey->GetInt());
			} else if (FStrEq(name, "SetCreditTeam")) {
				state.m_SetCreditTeam.Set(subkey->GetInt());
			} else if (FStrEq(name, "EnableDominations")) {
				state.m_EnableDominations.Set(subkey->GetBool());
			} else if (FStrEq(name, "VanillaMode")) {
				state.m_VanillaMode.Set(subkey->GetBool());
			} else if (FStrEq(name, "BodyPartScaleSpeed")) {
				state.m_BodyPartScaleSpeed.Set(subkey->GetFloat());
			} else if (FStrEq(name, "SandmanStun")) {
				state.m_SandmanStuns.Set(subkey->GetBool());
			} else if (FStrEq(name, "MedigunShieldDamage")) {
				state.m_MedigunShieldDamage.Set(subkey->GetBool());
			} else if (FStrEq(name, "StandableHeads")) {
				state.m_StandableHeads.Set(subkey->GetBool());
			} else if (FStrEq(name, "NoRomevisionCosmetics")) {
				state.m_NoRomevisionCosmetics.Set(subkey->GetBool());
			} else if (FStrEq(name, "CreditsBetterRadiusCollection")) {
				state.m_CreditsBetterRadiusCollection.Set(subkey->GetBool());
			} else if (FStrEq(name, "AimTrackingIntervalMultiplier")) {
				state.m_AimTrackingIntervalMultiplier.Set(subkey->GetFloat());
			} else if (FStrEq(name, "ImprovedAirblast")) {
				state.m_ImprovedAirblast.Set(subkey->GetBool());
			} else if (FStrEq(name, "FlagCarrierMovementPenalty")) {
				state.m_FlagCarrierMovementPenalty.Set(subkey->GetFloat());
			} else if (FStrEq(name, "TextPrintTime")) {
				state.m_TextPrintSpeed.Set(subkey->GetFloat());
			} else if (FStrEq(name, "BotTeleportUberDuration")) {
				state.m_TeleporterUberDuration.Set(subkey->GetFloat());
			} else if (FStrEq(name, "BluHumanTeleportOnSpawn")) {
				state.m_BluHumanTeleport.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluHumanBotTeleporter")) {
				state.m_BluHumanTeleportPlayer.Set(subkey->GetBool());
			} else if (FStrEq(name, "BotsUsePlayerTeleporters")) {
				state.m_BotsUsePlayerTeleporters.Set(subkey->GetBool());
			} else if (FStrEq(name, "MaxEntitySpeed")) {
				state.m_MaxVelocity.Set(subkey->GetFloat());
			} else if (FStrEq(name, "MaxSpeedLimit")) {
				state.m_MaxSpeedEnable.Set(subkey->GetFloat() != 520.0f);
				state.m_MaxSpeedLimit.Set(subkey->GetFloat());
				state.m_BotRunFast.Set(subkey->GetFloat() > 520.0f);
				state.m_BotRunFastUpdate.Set(subkey->GetFloat() > 520.0f);
				state.m_BotRunFastJump.Set(subkey->GetFloat() > 521.0f);
				if (subkey->GetFloat() > 3500.0f)
					state.m_MaxVelocity.Set(subkey->GetFloat());

			} else if (FStrEq(name, "ConchHealthOnHit")) {
				state.m_ConchHealthOnHitRegen.Set(subkey->GetFloat());
			} else if (FStrEq(name, "MarkedForDeathLifetime")) {
				state.m_MarkOnDeathLifetime.Set(subkey->GetFloat());
			} else if (FStrEq(name, "VacNumCharges")) {
				state.m_VacNumCharges.Set(subkey->GetInt());
			} else if (FStrEq(name, "DoubleDonkWindow")) {
				state.m_DoubleDonkWindow.Set(subkey->GetFloat());
			} else if (FStrEq(name, "ConchSpeedBoost")) {
				state.m_ConchSpeedBoost.Set(subkey->GetFloat());
			} else if (FStrEq(name, "StealthDamageReduction")) {
				state.m_StealthDamageReduction.Set(subkey->GetFloat());
			} else if (FStrEq(name, "AllowFlagCarrierToFight")) {
				state.m_AllowFlagCarrierToFight.Set(subkey->GetBool());
			} else if (FStrEq(name, "HealOnKillOverhealMelee")) {
				state.m_HealOnKillOverhealMelee.Set(subkey->GetBool());
			} else if (FStrEq(name, "MaxActiveSkeletons")) {
				state.m_MaxActiveZombie.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHHealthPerPlayer")) {
				state.m_HHHHealthPerPlayer.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHHealthBase")) {
				state.m_HHHHealthBase.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHAttackRange")) {
				state.m_HHHAttackRange.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHChaseRange")) {
				state.m_HHHChaseRange.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHTerrifyRange")) {
				state.m_HHHTerrifyRange.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHQuitRange")) {
				state.m_HHHQuitRange.Set(subkey->GetInt());
			} else if (FStrEq(name, "HHHChaseDuration")) {
				state.m_HHHChaseDuration.Set(subkey->GetFloat());
			} else if (FStrEq(name, "ForceRobotBleed")) {
				state.m_ForceRobotBleed.Set(subkey->GetBool());
			} else if (FStrEq(name, "BotHumansHaveRobotVoice")) {
				state.m_BotHumansHaveRobotVoice.Set(subkey->GetBool());
			} else if (FStrEq(name, "BotHumansHaveEyeGlow")) {
				state.m_BotHumansHaveEyeGlow.Set(subkey->GetBool());
			} else if (FStrEq(name, "CustomEyeParticle")) {
				state.m_EyeParticle.Set(subkey->GetString());
				PrecacheParticleSystem( subkey->GetString() );
			} else if (FStrEq(name, "FlagEscortCountOffset")) {
				state.m_BotEscortCount.Set(subkey->GetInt() + 4);
		//	} else if (FStrEq(name, "NoNewInspection")) {
		//		state.m_CustomAttrDisplay.Set(!subkey->GetBool());
			} else if (FStrEq(name, "ForceHoliday")) {
				DevMsg("Forcing holiday\n");
				CBaseEntity *ent = CreateEntityByName("tf_logic_holiday");
				ent->KeyValue("holiday_type", "2");
				DispatchSpawn(ent);
				ent->Activate();
				state.m_ForceHoliday.Set(FStrEq(subkey->GetString(),"Halloween") ? 2 : subkey->GetInt());
			} else if (FStrEq(name, "RobotLimit")) {
				state.m_RobotLimit.Set(subkey->GetInt());
				state.m_iRobotLimitSetByMission = subkey->GetInt();
			} else if (FStrEq(name, "CustomUpgradesFile")) {
				//static const std::string prefix("download/scripts/items/");
				state.m_CustomUpgradesFile.Set(subkey->GetString());
			} else if (FStrEq(name, "DisableSound")) {
				state.m_DisableSounds.emplace(subkey->GetString());
			} else if (FStrEq(name, "SendBotsToSpectatorImmediately")) {
				state.m_SendBotsToSpectatorImmediately.Set(subkey->GetBool());
			} else if (FStrEq(name, "BotsRandomCrit")) {
				state.m_bBotRandomCrits = subkey->GetBool();
			} else if (FStrEq(name, "NoRedBotsRandomCrit")) {
				state.m_bRedBotNoRandomCrits = subkey->GetBool();
			} else if (FStrEq(name, "NoHolidayPickups")) {
				state.m_bNoHolidayHealthPack = subkey->GetBool();
				if (state.m_bNoHolidayHealthPack) {
					ForEachEntityByRTTI<CTFPowerup>([&](CTFPowerup *powerup){
						powerup->SetModelIndexOverride( VISION_MODE_PYRO, 0 );
						powerup->SetModelIndexOverride( VISION_MODE_HALLOWEEN, 0 );
					});
				}
			} else if (FStrEq(name, "NoSapUnownedBuildings")) {
				state.m_bSpyNoSapUnownedBuildings = subkey->GetBool();
			} else if (FStrEq(name, "RespawnWaveTimeBlue")) {
				state.m_flRespawnWaveTimeBlue = subkey->GetFloat();
			} else if (FStrEq(name, "FixedRespawnWaveTimeBlue")) {
				state.m_bFixedRespawnWaveTimeBlue = subkey->GetBool();
			} else if (FStrEq(name, "DisplayRobotDeathNotice")) {
				state.m_bDisplayRobotDeathNotice = subkey->GetBool();
			} else if (FStrEq(name, "PlayerMiniboss")) {
				state.m_bPlayerMinigiant = subkey->GetBool();
			} else if (FStrEq(name, "PlayerScale")) {
				state.m_fPlayerScale = subkey->GetFloat();
			} else if (FStrEq(name, "DisplayRobotDeathNotice")) {
				state.m_bDisplayRobotDeathNotice = subkey->GetBool();
			} else if (FStrEq(name, "Ribit")) {
				state.m_bPlayerRobotUsePlayerAnimation = subkey->GetBool();
			} else if (FStrEq(name, "MaxRedPlayers")) {
				state.m_RedTeamMaxPlayers.Set(subkey->GetInt());
				//if (state.m_iRedTeamMaxPlayers > 0)
				//	ResetMaxRedTeamPlayers(state.m_iRedTeamMaxPlayers);
			} else if (FStrEq(name, "PlayerMiniBossMinRespawnTime")) {
				state.m_iPlayerMiniBossMinRespawnTime = subkey->GetInt();
			} else if (FStrEq(name, "PlayerRobotsUsePlayerAnimation")) {
				state.m_bPlayerRobotUsePlayerAnimation = subkey->GetBool();
			//} else if (FStrEq(name, "FlagEscortCountOffset")) {
			//	state.m_iEscortBotCountOffset = subkey->GetInt();
			} else if (FStrEq(name, "WaveStartCountdown")) {
				state.m_flRestartRoundTime = subkey->GetFloat();
			} else if (FStrEq(name, "ZombiesNoWave666")) {
				state.m_bZombiesNoWave666 = subkey->GetBool();
			} else if (FStrEq(name, "FastNPCUpdate")) {
				state.m_bFastNPCUpdate = subkey->GetBool();
			} else if (FStrEq(name, "StuckTimeMultiplier")) {
				state.m_fStuckTimeMult = subkey->GetFloat();
			} else if (FStrEq(name, "NoCreditsVelocity")) {
				state.m_bNoCreditsVelocity = subkey->GetBool();
			} else if (FStrEq(name, "MaxTotalPlayers")) {

			} else if (FStrEq(name, "MaxSpectators")) {
				SetMaxSpectators(subkey->GetInt());
			} else if (FStrEq(name, "ItemWhitelist")) {
				Parse_ItemWhitelist(subkey);
			} else if (FStrEq(name, "ItemBlacklist")) {
				Parse_ItemBlacklist(subkey);
			} else if (FStrEq(name, "ItemAttributes")) {
				Parse_ItemAttributes(subkey, state.m_ItemAttributes);
			} else if (FStrEq(name, "ClassLimit")) {
				Parse_ClassBlacklist(subkey);
		//	} else if (FStrEq(name, "DisallowedItems")) {
		//		Parse_DisallowedItems(subkey);
			} else if (FStrEq(name, "FlagResetTime")) {
				Parse_FlagResetTime(subkey);
			} else if (FStrEq(name, "ExtraSpawnPoint")) {
				Parse_ExtraSpawnPoint(subkey);
			} else if (FStrEq(name, "ExtraTankPath")) {
				Parse_ExtraTankPath(subkey);
			} else if (FStrEq(name, "PlayerAttributes")) {
				Parse_PlayerAttributes(subkey);
			} else if (FStrEq(name, "PlayerAddCond")) {
				Parse_PlayerAddCond(subkey);
			} else if (FStrEq(name, "OverrideSounds")) {
				Parse_OverrideSounds(subkey);
			} else if (FStrEq(name, "ForceItem")) {
				Parse_ForceItem(subkey, state.m_ForceItems, false);
			} else if (FStrEq(name, "ForceItemNoRemove")) {
				Parse_ForceItem(subkey, state.m_ForceItems, true);
			} else if (FStrEq(name, "HandModelOverride")) {
				Parse_HandModelOverride(subkey);
			} else if (FStrEq(name, "ExtendedUpgrades")) {
				Mod::MvM::Extended_Upgrades::Parse_ExtendedUpgrades(subkey);
			} else if (FStrEq(name, "SpawnTemplate")) {
				auto templ_info = Parse_SpawnTemplate(subkey);
				if (templ_info.templ == nullptr)
				{
					state.m_SpawnTemplates.push_back(templ_info);
				}
				else {
				//	Msg("SpawnTemplate %s\n", templ_info.template_name.c_str());
					templ_info.SpawnTemplate(nullptr);
				}
			} else if (FStrEq(name, "PlayerSpawnTemplate")) {
				PlayerPointTemplateInfo info = Parse_PlayerSpawnTemplate(subkey);
				state.m_PlayerSpawnTemplates.push_back(info);
			} else if (FStrEq(name, "PlayerItemEquipSpawnTemplate")) {
				state.m_WeaponSpawnTemplates.emplace_back();
				WeaponPointTemplateInfo &info = state.m_WeaponSpawnTemplates.back();//
				Parse_PlayerItemEquipSpawnTemplate(info, subkey);
			} else if (FStrEq(name, "PlayerSpawnOnceTemplate")) {
				PointTemplateInfo info = Parse_SpawnTemplate(subkey);
				info.ignore_parent_alive_state = true;
				state.m_PlayerSpawnOnceTemplates.push_back(info);
			} else if (FStrEq(name, "PlayerShootTemplate")) {
				ShootTemplateData data;
				if (Parse_ShootTemplate(data, subkey))
					state.m_ShootTemplates.push_back(data);
			} else if (FStrEq(name, "CustomWeapon")) {
				Parse_CustomWeapon(subkey);
			} else if (FStrEq(name, "SpellBookNormalRoll")) {
				Parse_SpellBookRoll(subkey, state.m_SpellBookNormalRoll);
			} else if (FStrEq(name, "SpellBookRareRoll")) {
				Parse_SpellBookRoll(subkey, state.m_SpellBookRareRoll);
			} else if (FStrEq(name, "DisallowUpgrade")) {
				Parse_DisallowedUpgrade(subkey);
			} else if (FStrEq(name, "Description")) {
				state.m_Description.push_back(subkey->GetString());
			} else if (FStrEq(name, "NoThrillerTaunt")) {
				state.m_bNoThrillerTaunt = subkey->GetBool();
			} else if (FStrEq(name, "NoCritPumpkin")) {
				state.m_bNoCritPumpkin = subkey->GetBool();
			} else if (FStrEq(name, "NoMissionInfo")) {
				state.m_bNoMissionInfo = subkey->GetBool();
			} else if (FStrEq(name, "NoJoinMidwave")) {
				state.m_bNoRespawnMidwave = subkey->GetBool();
			} else if (FStrEq(name, "HHHNoControlPointLogic")) {
				state.m_bHHHNoControlPointLogic = subkey->GetBool();
			} else if (FStrEq(name, "MinibossSentrySingleKill")) {
				state.m_bMinibossSentrySingleKill = subkey->GetBool();
			} else if (FStrEq(name, "ExtendedUpgradesOnly")) {
				state.m_bExtendedUpgradesOnly = subkey->GetBool();
			} else if (FStrEq(name, "ExtendedUpgradesNoUndo")) {
				state.m_bExtendedUpgradesNoUndo = subkey->GetBool();
			} else if (FStrEq(name, "HHHNonSolidToPlayers")) {
				state.m_bHHHNonSolidToPlayers = subkey->GetBool();
			} else if (FStrEq(name, "ItemReplacement")) {
				Parse_ItemReplacement(subkey);
			} else if (FStrEq(name, "ExtraLoadoutItems")) {
				Parse_ExtraLoadoutItems(subkey);
            } else if (FStrEq(name, "NoWranglerShield")) {
                state.m_bNoWranglerShield = subkey->GetBool();
			} else if (FStrEq(name, "BunnyHop")) {
				state.m_iBunnyHop = subkey->GetInt();
			} else if (FStrEq(name, "Accelerate")) {
				state.m_Accelerate.Set(subkey->GetFloat());
			} else if (FStrEq(name, "AirAccelerate")) {
				state.m_AirAccelerate.Set(subkey->GetFloat());
			} else if (FStrEq(name, "NoSkeletonSplit")) {
				state.m_bNoSkeletonSplit = subkey->GetBool();
			} else if (FStrEq(name, "RestoreNegativeDamageHealing")) {
				state.m_bRestoreNegativeDamageHealing = subkey->GetBool();
			} else if (FStrEq(name, "RestoreNegativeDamageOverheal")) {
				state.m_bRestoreNegativeDamageOverheal = subkey->GetBool();
			} else if (FStrEq(name, "TurboPhysics")) {
				state.m_TurboPhysics.Set(subkey->GetBool());
			} else if (FStrEq(name, "UpgradeStationKeepWeapons")) {
				state.m_UpgradeStationRegenCreators.Set(!subkey->GetBool());
				state.m_UpgradeStationRegen.Set(subkey->GetBool());
			} else if (FStrEq(name, "AllowBluPlayerReanimators")) {
				state.m_AllowBluePlayerReanimators.Set(subkey->GetBool());
			} else if (FStrEq(name, "RemoveBluVelocityLimit")) {
				state.m_BluVelocityRemoveLimit.Set(subkey->GetBool());
			} else if (FStrEq(name, "FastEntityNameLookup")) {
				state.m_FastEntityNameLookup.Set(subkey->GetBool());
			} else if (FStrEq(name, "AllowMultipleSappers")) {
				state.m_AllowMultipleSappers.Set(subkey->GetBool());
			} else if (FStrEq(name, "EngineerPushRange")) {
				state.m_EngineerPushRange.Set(subkey->GetFloat());
			} else if (FStrEq(name, "FixHuntsmanDamageBonus")) {
				state.m_FixHuntsmanDamageBonus.Set(subkey->GetBool());
			} else if (FStrEq(name, "DefaultMiniBossScale")) {
				state.m_DefaultBossScale.Set(subkey->GetFloat());
			} else if (FStrEq(name, "FastWholeMapTriggers")) {
				state.m_FastWholeMapTriggers.Set(subkey->GetBool());
			} else if (FStrEq(name, "FastWholeMapBuildInTriggersMinRadius")) {
				state.m_FastWholeMapTriggersAll.Set(subkey->GetFloat());
			} else if (FStrEq(name, "BluHumanSpawnNoShoot")) {
				state.m_BluHumanSpawnNoShoot.Set(subkey->GetBool());
			} else if (FStrEq(name, "BluHumanSpawnProtection")) {
				state.m_BluHumanSpawnProtection.Set(subkey->GetBool());
			} else if (FStrEq(name, "AutoWeaponStrip")) {
				state.m_AutoWeaponStrip.Set(subkey->GetBool());
			} else if (FStrEq(name, "AllowBotExtraSlots")) {
				state.m_AllowBotsExtraSlots.Set(subkey->GetBool());
			} else if (FStrEq(name, "RemoveUnusedOffhandViewmodel")) {
				state.m_RemoveOffhandViewmodel.Set(subkey->GetBool());
			} else if (FStrEq(name, "RemoveBotExpressions")) {
				state.m_RemoveBotExpressions.Set(subkey->GetBool());
			} else if (FStrEq(name, "ExtraBotSlotsNoDeathcam")) {
				state.m_ExtraBotSlotsNoDeathcam.Set(subkey->GetBool());
			} else if (FStrEq(name, "NoBluHumanFootsteps")) {
				state.m_NoRobotFootsteps.Set(subkey->GetBool());
			} else if (FStrEq(name, "SentryHintBombForwardRange")) {
				state.m_SentryHintBombForwardRange.Set(subkey->GetFloat());
			} else if (FStrEq(name, "SentryHintBombBackwardRange")) {
				state.m_SentryHintBombBackwardRange.Set(subkey->GetFloat());
			} else if (FStrEq(name, "SentryHintMinDistanceFromBomb")) {
				state.m_SentryHintMinDistanceFromBomb.Set(subkey->GetFloat());
			} else if (FStrEq(name, "PathTrackIsServerEntity")) {
				state.m_PathTrackIsServerEntity.Set(subkey->GetBool());
			} else if (FStrEq(name, "EnemyTeamForReverse")) {
				if (FStrEq(subkey->GetString(), "Red")) {
					state.m_iEnemyTeamForReverse = 2;
				}
				else if (FStrEq(subkey->GetString(), "Blu") || FStrEq(subkey->GetString(), "Blue")) {
					state.m_iEnemyTeamForReverse = 3;
				}
			} else if (FStrEq(name, "LuaScript")) {
				if (subkey->GetFirstSubKey() == nullptr) {
					state.m_Scripts.push_back(subkey->GetString());
				}
				else {
					FOR_EACH_SUBKEY(subkey, subkey2) {
						if (FStrEq(subkey2->GetName(), "File")) {
							state.m_ScriptFiles.push_back(subkey2->GetString());
						}
						else {
							state.m_Scripts.push_back(subkey2->GetString());
						}
					}
				}
			} else if (FStrEq(name, "LuaScriptFile")) {
				state.m_ScriptFiles.push_back(subkey->GetString());
			} else if (FStrEq(name, "ForceRedMoney")) {
				state.m_bForceRedMoney = subkey->GetBool();
			} else if (FStrEq(name, "CustomNavFile")) {
				char strippedFile[128];
				V_StripExtension(subkey->GetString(), strippedFile, sizeof(strippedFile));
				state.m_CustomNavFile = strippedFile;
			// } else if (FStrEq(name, "SprayDecal")) {
			// 	Parse_SprayDecal(subkey);
			
			} else if (FStrEq(name, "ScriptSoundOverrides") || FStrEq(name, "CustomScriptSoundFile")) {
				if (!filesystem->FileExists(subkey->GetString())) {
					Warning("The custom sound script file %s might not exist\n", subkey->GetString());
				}
				soundemitterbase->AddSoundOverrides(subkey->GetString(), true);
				KeyValues *kvsnd = new KeyValues( "" );
				
				if (kvsnd->LoadFromFile(filesystem, subkey->GetString())) {
					for (KeyValues *pKeys = kvsnd; pKeys != nullptr; pKeys = pKeys->GetNextKey()) {
						CBaseEntity::PrecacheScriptSound(pKeys->GetName());
					}
				}
				kvsnd->deleteThis();
			} else if (FStrEq(name, "OverrideParticles")) {
				FOR_EACH_SUBKEY(subkey, subkey2) {
					state.m_ParticleOverride[subkey2->GetName()] = subkey2->GetString();
				}
			} else if (FStrEq(name, "BuildingSpawnTemplate")) {
				Parse_PlayerBuildingSpawnTemplate(subkey);
			} else if (FStrEq(name, "PrecacheScriptSound"))  { CBaseEntity::PrecacheScriptSound (subkey->GetString());
			} else if (FStrEq(name, "PrecacheSound"))        { enginesound->PrecacheSound       (subkey->GetString(), false);
			} else if (FStrEq(name, "PrecacheModel"))        { CBaseEntity::PrecacheModel       (subkey->GetString(), false);
			} else if (FStrEq(name, "PrecacheSentenceFile")) { engine     ->PrecacheSentenceFile(subkey->GetString(), false);
			} else if (FStrEq(name, "PrecacheDecal"))        { engine     ->PrecacheDecal       (subkey->GetString(), false);
			} else if (FStrEq(name, "PrecacheGeneric"))      { engine     ->PrecacheGeneric     (subkey->GetString(), false);
			} else if (FStrEq(name, "PrecacheParticle"))     { PrecacheParticleSystem( subkey->GetString() );
			} else if (FStrEq(name, "PreloadSound"))         { enginesound->PrecacheSound       (subkey->GetString(), true);
			} else if (FStrEq(name, "PreloadModel"))         { CBaseEntity::PrecacheModel       (subkey->GetString(), true);
			} else if (FStrEq(name, "PreloadSentenceFile"))  { engine     ->PrecacheSentenceFile(subkey->GetString(), true);
			} else if (FStrEq(name, "PreloadDecal"))         { engine     ->PrecacheDecal       (subkey->GetString(), true);
			} else if (FStrEq(name, "PreloadGeneric"))       { engine     ->PrecacheGeneric     (subkey->GetString(), true);
			} else {
				del = false;
			}
			
			if (del) {
			//	DevMsg("Key \"%s\": processed, will delete\n", name);
				del_kv.push_back(subkey);
			} else {
			//	DevMsg("Key \"%s\": passthru\n", name);
			}
		}
		
		for (auto subkey : del_kv) {
		//	DevMsg("Deleting key \"%s\"\n", subkey->GetName());
			kv->RemoveSubKey(subkey);
			subkey->deleteThis();
		}
    }

	RefCount rc_KeyValues_LoadFromFile;
	DETOUR_DECL_MEMBER(bool, KeyValues_LoadFromFile, IBaseFileSystem *filesystem, const char *resourceName, const char *pathID, bool refreshCache)
	{
	//	DevMsg("KeyValues::LoadFromFile\n");
		
		++rc_KeyValues_LoadFromFile;
		
		auto result = DETOUR_MEMBER_CALL(KeyValues_LoadFromFile)(filesystem, resourceName, pathID, refreshCache);
		--rc_KeyValues_LoadFromFile;
		
		if (result && rc_CPopulationManager_Parse > 0 && rc_KeyValues_LoadFromFile == 0 && rc_CPopulationManager_IsValidPopfile == 0 && rc_Parse_Popfile == 0) {
			auto kv = reinterpret_cast<KeyValues *>(this);

            Parse_Popfile(kv, filesystem);
		}
		
		return result;
	}

	
	void ResetMaxRedTeamPlayers(int resetTo) {
		int redplayers = 0;
		ForEachTFPlayer([&](CTFPlayer *player) {
			if (player->GetTeamNumber() == TF_TEAM_RED && player->IsRealPlayer())
				redplayers += 1;
		});
		if (redplayers > resetTo) {
			ForEachTFPlayer([&](CTFPlayer *player) {
				if (player->GetTeamNumber() == TF_TEAM_RED && redplayers > resetTo && player->IsRealPlayer() && !PlayerIsSMAdmin(player)) {
					player->ForceChangeTeam(TEAM_SPECTATOR, false);
					redplayers -= 1;
				}
			});
		}
		if (resetTo > 0) {
			iGetTeamAssignmentOverride = resetTo;
			iPreClientUpdate = GetVisibleMaxPlayers();
		}
		else {
			iGetTeamAssignmentOverride = 6;
			iPreClientUpdate = 6;
		}
		TogglePatches();
	}
	void SetMaxSpectators(int resetTo) {
		
		int spectators = 0;
		ForEachTFPlayer([&](CTFPlayer *player) {
			if (player->GetTeamNumber() < 2 && player->IsRealPlayer())
				spectators += 1;
		});
		if (spectators > resetTo) {
			ForEachTFPlayer([&](CTFPlayer *player) {
				if (player->GetTeamNumber() < TF_TEAM_RED && spectators > resetTo && player->IsRealPlayer()) {
					player->HandleCommand_JoinTeam("red");

					if (player->GetTeamNumber() >= TF_TEAM_RED) {
						spectators -= 1;
					}
					else if (!PlayerIsSMAdmin(player)){
						// Kick if cannot switch to red team
						engine->ServerCommand(CFmtStr("kickid %d %s\n", player->GetUserID(), "Exceeded total player limit for the mission"));
						spectators -= 1;
					}
					else {
						PrintToChat("This mission has a reduced spectator count. If you weren't an admin, you would have been kicked\n", player);
					}
				}
			});
		}
		state.m_iTotalSpectators = resetTo;
	}
	int GetMaxSpectators()
	{
		return state.m_iTotalSpectators;
	}
	void ResetMaxTotalPlayers(int resetTo) {
		
		int totalplayers = 0;
		ForEachTFPlayer([&](CTFPlayer *player) {
			if (player->IsRealPlayer())
				totalplayers += 1;
		});
		if (totalplayers > resetTo) {
			ForEachTFPlayer([&](CTFPlayer *player) {
				if (player->GetTeamNumber() < 2 && totalplayers > resetTo && player->IsRealPlayer() && !PlayerIsSMAdmin(player)) {
					engine->ServerCommand(CFmtStr("kickid %d %s\n", player->GetUserID(), "Exceeded total player limit for the mission"));
					totalplayers -= 1;
				}
			});
			ForEachTFPlayer([&](CTFPlayer *player) {
				if (player->GetTeamNumber() >= 2 && totalplayers > state.m_iTotalMaxPlayers && player->IsRealPlayer() && !PlayerIsSMAdmin(player)) {
					engine->ServerCommand(CFmtStr("kickid %d %s\n", player->GetUserID(), "Exceeded total player limit for the mission"));
					totalplayers -= 1;
				}
			});
		}
		if (resetTo > 0)
			iClientConnected = std::max(5, resetTo - 1);
		else 
			iClientConnected = 9;
		TogglePatches();
	}

	int GetVisibleMaxPlayers() {
		int max = 6;
		if (cvar_max_red_players.GetInt() > 0)
			max = cvar_max_red_players.GetInt();
		
		static ConVarRef sig_mvm_jointeam_blue_allow("sig_mvm_jointeam_blue_allow");
		static ConVarRef sig_mvm_jointeam_blue_allow_max("sig_mvm_jointeam_blue_allow_max");
		if (sig_mvm_jointeam_blue_allow.GetInt() != 0 && sig_mvm_jointeam_blue_allow_max.GetInt() > 0)
			max += sig_mvm_jointeam_blue_allow_max.GetInt();
		
		static ConVarRef sig_mvm_jointeam_blue_allow_force("sig_mvm_jointeam_blue_allow_force");
		if (sig_mvm_jointeam_blue_allow_force.GetInt() != 0 && sig_mvm_jointeam_blue_allow_max.GetInt() > 0)
			max = sig_mvm_jointeam_blue_allow_max.GetInt();
		return max;
	}

	void SetVisibleMaxPlayers() {
		iPreClientUpdate = GetVisibleMaxPlayers();
		TogglePatches();
	}
	
	class CMod : public IMod, public IModCallbackListener, public IFrameUpdatePostEntityThinkListener
	{
	public:
		CMod() : IMod("Pop:PopMgr_Extensions")
		{
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_PlayerRunCommand,					 "CTFPlayer::PlayerRunCommand");
			MOD_ADD_DETOUR_MEMBER(CTFGameMovement_PreventBunnyJumping,			 "CTFGameMovement::PreventBunnyJumping");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_PlayerKilled,                     "CTFGameRules::PlayerKilled");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_ShouldDropSpellPickup,            "CTFGameRules::ShouldDropSpellPickup");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_DropSpellPickup,                  "CTFGameRules::DropSpellPickup");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_IsUsingSpells,                    "CTFGameRules::IsUsingSpells");
			MOD_ADD_DETOUR_MEMBER_PRIORITY(CTFPlayer_ReapplyItemUpgrades,        "CTFPlayer::ReapplyItemUpgrades", LOWEST);
			MOD_ADD_DETOUR_STATIC(CTFReviveMarker_Create,                        "CTFReviveMarker::Create");
			MOD_ADD_DETOUR_MEMBER(CTFSniperRifle_CreateSniperDot,                "CTFSniperRifle::CreateSniperDot");
			MOD_ADD_DETOUR_MEMBER(CTFSniperRifle_CanFireCriticalShot,            "CTFSniperRifle::CanFireCriticalShot");
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBase_CanFireCriticalShot,             "CTFWeaponBase::CanFireCriticalShot");
			MOD_ADD_DETOUR_MEMBER(CTFProjectile_Arrow_StrikeTarget,              "CTFProjectile_Arrow::StrikeTarget");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_IsPVEModeControlled,              "CTFGameRules::IsPVEModeControlled");
			MOD_ADD_DETOUR_MEMBER(CUpgrades_UpgradeTouch,                        "CUpgrades::UpgradeTouch");
			
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_BroadcastSound,                   "CTFGameRules::BroadcastSound");
			MOD_ADD_DETOUR_MEMBER(CTeamplayRoundBasedRules_BroadcastSound,       "CTeamplayRoundBasedRules::BroadcastSound");
			MOD_ADD_DETOUR_STATIC(CBaseEntity_EmitSound_static_emitsound,        "CBaseEntity::EmitSound [static: emitsound]");
			MOD_ADD_DETOUR_STATIC(CBaseEntity_EmitSound_static_emitsound_handle, "CBaseEntity::EmitSound [static: emitsound + handle]");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_GiveDefaultItems,                    "CTFPlayer::GiveDefaultItems");
		//	MOD_ADD_DETOUR_MEMBER(CTFPlayer_ManageRegularWeapons,                "CTFPlayer::ManageRegularWeapons");
			MOD_ADD_DETOUR_MEMBER(CTFItemDefinition_GetLoadoutSlot,              "CTFItemDefinition::GetLoadoutSlot");
			MOD_ADD_DETOUR_MEMBER(CTFPlayerInventory_GetItemInLoadout,           "CTFPlayerInventory::GetItemInLoadout");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_PickupWeaponFromOther,               "CTFPlayer::PickupWeaponFromOther");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_GiveNamedItem,                       "CTFPlayer::GiveNamedItem");
		//	MOD_ADD_DETOUR_MEMBER(CTFPlayer_ItemIsAllowed,                       "CTFPlayer::ItemIsAllowed");
			MOD_ADD_DETOUR_MEMBER(CCaptureFlag_GetMaxReturnTime,                 "CCaptureFlag::GetMaxReturnTime");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_HandleCommand_JoinClass,             "CTFPlayer::HandleCommand_JoinClass");
			
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_OnPlayerSpawned,                  "CTFGameRules::OnPlayerSpawned");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_Event_Killed,                        "CTFPlayer::Event_Killed");
			
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_ctor,                    "CTFGameRules::CTFGameRules [C1]");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_SetWinningTeam,          "CTFGameRules::SetWinningTeam");
			MOD_ADD_DETOUR_MEMBER(CTeamplayRoundBasedRules_State_Enter, "CTeamplayRoundBasedRules::State_Enter");
			MOD_ADD_DETOUR_MEMBER(CMannVsMachineStats_RoundEvent_WaveEnd,        "CMannVsMachineStats::RoundEvent_WaveEnd");
			
			
			//MOD_ADD_DETOUR_MEMBER(CTFGameRules_FireGameEvent,           "CTFGameRules::FireGameEvent");
			MOD_ADD_DETOUR_MEMBER(CBasePlayer_ChangeTeam,               "CBasePlayer::ChangeTeam [int, bool, bool, bool]");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_ChangeTeam,                 "CTFPlayer::ChangeTeam");
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBase_CalcIsAttackCritical,   "CTFWeaponBase::CalcIsAttackCritical");
			MOD_ADD_DETOUR_MEMBER(CUpgrades_GrantOrRemoveAllUpgrades,   "CUpgrades::GrantOrRemoveAllUpgrades");
			MOD_ADD_DETOUR_MEMBER(CTFPowerup_Spawn,   "CTFPowerup::Spawn");
			//MOD_ADD_DETOUR_STATIC(UserMessageBegin,                 "UserMessageBegin");
			//MOD_ADD_DETOUR_STATIC(MessageWriteString,                 "MessageWriteString");
			
		//	MOD_ADD_DETOUR_MEMBER(CPopulationManager_PostInitialize, "CPopulationManager::PostInitialize");
			
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_Parse, "CPopulationManager::Parse");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_IsValidPopfile, "CPopulationManager::IsValidPopfile");
			MOD_ADD_DETOUR_MEMBER(KeyValues_LoadFromFile,   "KeyValues::LoadFromFile");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_GetOverrideStepSound, "CTFPlayer::GetOverrideStepSound");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_GetSceneSoundToken, "CTFPlayer::GetSceneSoundToken");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_InputSetCustomModel, "CTFPlayer::InputSetCustomModel");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_DeathSound, "CTFPlayer::DeathSound");
			MOD_ADD_DETOUR_MEMBER(CTFRevolver_CanFireCriticalShot, "CTFRevolver::CanFireCriticalShot");

			MOD_ADD_DETOUR_MEMBER(CPopulationManager_AdjustMinPlayerSpawnTime, "CPopulationManager::AdjustMinPlayerSpawnTime");

			MOD_ADD_DETOUR_MEMBER(CTFBot_GetNearestKnownSappableTarget, "CTFBot::GetNearestKnownSappableTarget");
			//MOD_ADD_DETOUR_MEMBER(IVision_IsAbleToSee, "IVision::IsAbleToSee");
			MOD_ADD_DETOUR_MEMBER(CTFBotVision_IsIgnored, "CTFBotVision::IsIgnored");
			MOD_ADD_DETOUR_MEMBER(IGameEventManager2_FireEvent, "IGameEventManager2::FireEvent");
			MOD_ADD_DETOUR_MEMBER(CTeamplayRoundBasedRules_GetMinTimeWhenPlayerMaySpawn, "CTeamplayRoundBasedRules::GetMinTimeWhenPlayerMaySpawn");

			// Bots use player teleporters
			MOD_ADD_DETOUR_MEMBER(CTFBotTacticalMonitor_ShouldOpportunisticallyTeleport,                  "CTFBotTacticalMonitor::ShouldOpportunisticallyTeleport");
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBaseGun_FireProjectile,        "CTFWeaponBaseGun::FireProjectile");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_JumpToWave,        "CPopulationManager::JumpToWave");
			MOD_ADD_DETOUR_MEMBER(CTFWearable_Equip,        "CTFWearable::Equip");
			MOD_ADD_DETOUR_MEMBER(CBaseCombatWeapon_Equip, "CBaseCombatWeapon::Equip");
			
			
			MOD_ADD_DETOUR_MEMBER(CTFPlayerClassShared_SetCustomModel,        "CTFPlayerClassShared::SetCustomModel");
			//MOD_ADD_DETOUR_STATIC(GetBotEscortCount,        "GetBotEscortCount");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_Spawn,          "CTFPlayer::Spawn");
			MOD_ADD_DETOUR_MEMBER(CEconEntity_UpdateOnRemove,              "CEconEntity::UpdateOnRemove");
			MOD_ADD_DETOUR_MEMBER(CTFSpellBook_RollNewSpell,              "CTFSpellBook::RollNewSpell");
			MOD_ADD_DETOUR_MEMBER(CTFSpellBook_SetSelectedSpell,          "CTFSpellBook::SetSelectedSpell");
			MOD_ADD_DETOUR_MEMBER(CUpgrades_PlayerPurchasingUpgrade,          "CUpgrades::PlayerPurchasingUpgrade");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_ClientCommand,          "CTFPlayer::ClientCommand");
			MOD_ADD_DETOUR_STATIC(Host_Say, "Host_Say");
			
			
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_ModifyOrAppendCriteria, "CTFPlayer::ModifyOrAppendCriteria");
			MOD_ADD_DETOUR_MEMBER(CTFAmmoPack_InitAmmoPack, "CTFAmmoPack::InitAmmoPack");
			
			MOD_ADD_DETOUR_STATIC(TF_IsHolidayActive, "TF_IsHolidayActive");
			MOD_ADD_DETOUR_MEMBER(CTFGameRules_BetweenRounds_Think, "CTFGameRules::BetweenRounds_Think");

			MOD_ADD_DETOUR_MEMBER(CTFPlayerShared_AddCond, "CTFPlayerShared::AddCond");
			MOD_ADD_DETOUR_MEMBER(CEngineSoundServer_PrecacheSound, "CEngineSoundServer::PrecacheSound");
			
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_UpdateObjectiveResource, "CPopulationManager::UpdateObjectiveResource");
			
			MOD_ADD_DETOUR_MEMBER(NextBotManager_ShouldUpdate, "NextBotManager::ShouldUpdate");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_IsReadyToSpawn, "CTFPlayer::IsReadyToSpawn");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_StartCurrentWave, "CPopulationManager::StartCurrentWave");
			MOD_ADD_DETOUR_MEMBER(CHeadlessHatmanAttack_RecomputeHomePosition, "CHeadlessHatmanAttack::RecomputeHomePosition");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_RemoveCurrency, "CTFPlayer::RemoveCurrency");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_EndPurchasableUpgrades, "CTFPlayer::EndPurchasableUpgrades");
			MOD_ADD_DETOUR_MEMBER(CHeadlessHatmanLocomotion_ShouldCollideWith, "CHeadlessHatmanLocomotion::ShouldCollideWith");

			MOD_ADD_DETOUR_MEMBER(CHeadlessHatmanAttack_AttackTarget, "CHeadlessHatmanAttack::AttackTarget");
			MOD_ADD_DETOUR_STATIC(CalculateMeleeDamageForce, "CalculateMeleeDamageForce");
			MOD_ADD_DETOUR_MEMBER(CBasePlayer_ShowViewPortPanel, "CBasePlayer::ShowViewPortPanel");

			MOD_ADD_DETOUR_MEMBER(CZombieBehavior_OnKilled, "CZombieBehavior::OnKilled");
			MOD_ADD_DETOUR_STATIC(CreateSpellSpawnZombie, "CreateSpellSpawnZombie");

			MOD_ADD_DETOUR_MEMBER(ILocomotion_StuckMonitor, "ILocomotion::StuckMonitor");
			MOD_ADD_DETOUR_MEMBER(PlayerLocomotion_GetDesiredSpeed, "PlayerLocomotion::GetDesiredSpeed");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_DropCurrencyPack, "CTFPlayer::DropCurrencyPack");
			MOD_ADD_DETOUR_MEMBER(CTFPowerup_DropSingleInstance, "CTFPowerup::DropSingleInstance");
			MOD_ADD_DETOUR_MEMBER_PRIORITY(CTFGameRules_ApplyOnDamageAliveModifyRules, "CTFGameRules::ApplyOnDamageAliveModifyRules", LOW);
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_ResetMap, "CPopulationManager::ResetMap");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_RestoreCheckpoint, "CPopulationManager::RestoreCheckpoint");
			MOD_ADD_DETOUR_MEMBER(CPopulationManager_SetCheckpoint, "CPopulationManager::SetCheckpoint");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_GetEntityForLoadoutSlot, "CTFPlayer::GetEntityForLoadoutSlot");
            MOD_ADD_DETOUR_MEMBER(CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot, "CTFPlayerSharedUtils::GetEconItemViewByLoadoutSlot");
            MOD_ADD_DETOUR_MEMBER(KeyValues_MergeBaseKeys, "KeyValues::MergeBaseKeys");
			
			
            //MOD_ADD_DETOUR_MEMBER(CTFWeaponBase_UpdateHands, "CTFWeaponBase::UpdateHands");
            //MOD_ADD_DETOUR_MEMBER(CBaseCombatWeapon_SetViewModel, "CBaseCombatWeapon::SetViewModel");
            MOD_ADD_DETOUR_MEMBER(CTFPlayerClassShared_GetHandModelName, "CTFPlayerClassShared::GetHandModelName");
            //MOD_ADD_DETOUR_MEMBER(CTFBotSpyInfiltrate_Update, "CTFBotSpyInfiltrate::Update");

            MOD_ADD_DETOUR_STATIC(DispatchParticleEffect_2, "DispatchParticleEffect [overload 2]");
            MOD_ADD_DETOUR_STATIC(DispatchParticleEffect_3, "DispatchParticleEffect [overload 3]");
            MOD_ADD_DETOUR_STATIC(DispatchParticleEffect_4, "DispatchParticleEffect [overload 4]");
            MOD_ADD_DETOUR_STATIC(DispatchParticleEffect_6, "DispatchParticleEffect [overload 6]");
            MOD_ADD_DETOUR_STATIC(DispatchParticleEffect_7, "DispatchParticleEffect [overload 7]");
            MOD_ADD_DETOUR_MEMBER(CBaseObject_StartBuilding, "CBaseObject::StartBuilding");
            MOD_ADD_DETOUR_MEMBER(CObjectSentrygun_FindTarget, "CObjectSentrygun::FindTarget");
            //MOD_ADD_DETOUR_MEMBER(CBaseCombatWeapon_UpdateTransmitState, "CBaseCombatWeapon::UpdateTransmitState");
            //MOD_ADD_DETOUR_MEMBER(CTFWeaponBase_Deploy, "CTFWeaponBase::Deploy");
            //MOD_ADD_DETOUR_MEMBER(CTFWeaponBase_Holster, "CTFWeaponBase::Holster");
			MOD_ADD_DETOUR_MEMBER(CTFBotSpawner_Spawn, "CTFBotSpawner::Spawn");
			MOD_ADD_DETOUR_MEMBER(CTankSpawner_Spawn, "CTankSpawner::Spawn");
			MOD_ADD_DETOUR_MEMBER(CCurrencyPack_Spawn, "CCurrencyPack::Spawn");

			MOD_ADD_DETOUR_MEMBER(CDynamicProp_Spawn, "CDynamicProp::Spawn");
			MOD_ADD_DETOUR_MEMBER(CTFWeaponBaseMelee_Smack, "CTFWeaponBaseMelee::Smack");
			MOD_ADD_DETOUR_MEMBER_PRIORITY(CTFGameRules_GetTeamAssignmentOverride, "CTFGameRules::GetTeamAssignmentOverride", LOWEST);
			MOD_ADD_DETOUR_STATIC(TranslateWeaponEntForClass, "TranslateWeaponEntForClass");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_RememberUpgrade, "CTFPlayer::RememberUpgrade");
			MOD_ADD_DETOUR_MEMBER(CTFPlayer_ForgetFirstUpgradeForItem, "CTFPlayer::ForgetFirstUpgradeForItem");
			MOD_ADD_DETOUR_MEMBER(CSpawnLocation_FindSpawnLocation, "CSpawnLocation::FindSpawnLocation");
			MOD_ADD_DETOUR_MEMBER(CSpawnLocation_Parse, "CSpawnLocation::Parse");
			
			
			
			
			
			// Remove banned missions from the list
			MOD_ADD_DETOUR_STATIC(CPopulationManager_FindDefaultPopulationFileShortNames, "CPopulationManager::FindDefaultPopulationFileShortNames");
			MOD_ADD_DETOUR_MEMBER(CMannVsMachineChangeChallengeIssue_ExecuteCommand, "CMannVsMachineChangeChallengeIssue::ExecuteCommand");
			MOD_ADD_DETOUR_STATIC(tf_mvm_popfile, "tf_mvm_popfile");

			//MOD_ADD_DETOUR_MEMBER(CPopulationManager_Spawn,             "CPopulationManager::Spawn");
			//MOD_ADD_DETOUR_MEMBER(CTFBaseRocket_SetDamage, "CTFBaseRocket::SetDamage");
			//MOD_ADD_DETOUR_MEMBER(CTFProjectile_SentryRocket_Create, "CTFProjectile_SentryRocket::Create");
			//MOD_ADD_DETOUR_MEMBER(CMatchInfo_GetNumActiveMatchPlayers, "CMatchInfo::GetNumActiveMatchPlayers");
			//MOD_ADD_DETOUR_STATIC(CollectPlayers_CTFPlayer,   "CollectPlayers<CTFPlayer>");
			//MOD_ADD_DETOUR_MEMBER(CTFGCServerSystem_PreClientUpdate,   "CTFGCServerSystem::PreClientUpdate");
			

			//team_size_extract.Init();
			//team_size_extract.Check();
			//int value = team_size_extract.Extract();
			//DevMsg("Team Size Extract %d\n", value);
			this->AddPatch(new CPatch_GetTeamAssignmentOverride());
			//this->AddPatch(new CPatch_CTFGameRules_ClientConnected());
			this->AddPatch(new CPatch_CTFGCServerSystem_PreClientUpdate());
		}
		
		virtual void OnUnload() override
		{
			state.Reset();
		}
		
		virtual void OnEnable() override
		{
			usermsgs->HookUserMessage2(usermsgs->GetMessageIndex("PlayerLoadoutUpdated"), &player_loadout_updated_listener);
			LoadItemNames();
		}

		virtual void OnDisable() override
		{
			ForEachTFPlayer([&](CTFPlayer *player){
                if (!player->IsBot())
                    menus->GetDefaultStyle()->CancelClientMenu(ENTINDEX(player));
            });
			usermsgs->UnhookUserMessage2(usermsgs->GetMessageIndex("PlayerLoadoutUpdated"), &player_loadout_updated_listener);
			state.Reset();
		}
		
		virtual bool ShouldReceiveCallbacks() const override { return this->IsEnabled(); }
		
		virtual void LevelInitPreEntity() override
		{
			
			state.Reset();
			state.m_PlayerUpgradeSend.clear();

		}

		virtual void LevelInitPostEntity() override
		{

			// Precache bones that are not available on robot models
			for (int i = 1; i < 10; i++) {
				state.m_CachedRobotModelIndex[i] = AllocPooledString(g_szBotModels[i]);
				state.m_CachedRobotModelIndex[i+10] = AllocPooledString(g_szBotBossModels[i]);
				const char *robot_model = g_szBotModels[i];
				const char *player_model = CFmtStr("models/player/%s.mdl", g_aRawPlayerClassNamesShort[i]);

				studiohdr_t *studio_player = mdlcache->GetStudioHdr(mdlcache->FindMDL(player_model));
				studiohdr_t *studio_robot = mdlcache->GetStudioHdr(mdlcache->FindMDL(robot_model));
				//anim_robot->SetModel(robot_model);
				//anim_player->SetModel(player_model);

				if (studio_robot == nullptr) {
					Msg("%s is null\n", robot_model);
					break;
				}
				else if (studio_player == nullptr) {
					Msg("%s is null\n", player_model);
					break;
				}

				int numbones_robot = studio_robot->numbones;
				int numbones_player = studio_player->numbones;

				std::unordered_set<std::string> &bones = state.m_MissingRobotBones[i];

				for (int j = 0; j < numbones_player; j++) {
					auto *bone = studio_player->pBone(j);
					bones.insert(bone->pszName());
				}

				for (int j = 0; j < numbones_robot; j++) {
					auto *bone = studio_robot->pBone(j);
					if (bone->flags != 0)
						bones.erase(bone->pszName());
				}
			}
			if (strlen(cvar_custom_upgrades_file.GetString()) > 0) {
				ResendUpgradeFile(true);
			}
			SetVisibleMaxPlayers();
		}

		virtual void LevelShutdownPostEntity() override
		{
			state.Reset();
			state.m_PlayerUpgradeSend.clear();
			
			for (int i = 0; i < 10; i++)
				state.m_MissingRobotBones[i].clear();
		}
		
		virtual void FrameUpdatePostEntityThink() override
		{
			if (gpGlobals->tickcount % 66 == 24) {
				ForEachTFPlayer([](CTFPlayer *player){
					if (!player->IsAlive()) return;

					for (int i =0; i < MAX_WEAPONS; i++) {
						CBaseCombatWeapon *cbcw = player->Weapon_GetSlot(i);
						if (cbcw == nullptr || cbcw == player->GetActiveWeapon()) continue;
						if (cbcw->GetEffects() & 512) {
							cbcw->RemoveEffects(512);
						}
						else {
							cbcw->AddEffects(512);
						}
					}
				});
			}
			if (!IsMannVsMachineMode()) return;
			
			if (state.m_flRemoveGrapplingHooks >= 0.0f) {
				ForEachEntityByClassnameRTTI<CTFProjectile_GrapplingHook>("tf_projectile_grapplinghook", [](CTFProjectile_GrapplingHook *proj){
					float dt = gpGlobals->curtime - proj->m_flTimeInit;
					if (dt > state.m_flRemoveGrapplingHooks) {
						DevMsg("Removing tf_projectile_grapplinghook #%d (alive for %.3f seconds)\n", ENTINDEX(proj), dt);
						proj->Remove();
					}
				});
			}
			if (state.m_PlayerAttributes.size() > 0 || state.m_PlayerAddCond.size() > 0 || state.m_bDeclaredClassAttrib)
				ForEachTFPlayer([&](CTFPlayer *player){
					if (player->IsBot()) return;
					//if (player->GetTeamNumber() != TF_TEAM_RED) return;
					

					ApplyPlayerAttributes(player);
					if (!player->IsAlive()) return;
					int classname = player->GetPlayerClass()->GetClassIndex();

					for(auto cond : state.m_PlayerAddCond){
						if (!player->m_Shared->InCond(cond)){
							player->m_Shared->AddCond(cond,-1.0f);
						}
					}

					for(auto cond : state.m_PlayerAddCondClass[classname]){
						if (!player->m_Shared->InCond(cond)){
							player->m_Shared->AddCond(cond,-1.0f);
						}
					}
				});
			if (state.m_iTotalSpectators >= 0) {
				int spectators = 0;
				ForEachTFPlayer([&](CTFPlayer *player){
					
					if(player->IsRealPlayer()) {
						if (!(player->GetTeamNumber() == TF_TEAM_BLUE || player->GetTeamNumber() == TF_TEAM_RED))
							spectators++;
					}
				});
				state.m_AllowSpectators.Set(spectators < state.m_iTotalSpectators);
				if (spectators > state.m_iTotalSpectators) {
					ForEachTFPlayer([&](CTFPlayer *player) {
						if (player->GetTeamNumber() < TF_TEAM_RED && spectators > state.m_iTotalSpectators && player->IsRealPlayer()) {
							player->HandleCommand_JoinTeam("red");

							if (player->GetTeamNumber() >= TF_TEAM_RED) {
								spectators -= 1;
							}
							else if (!PlayerIsSMAdmin(player)){
								// Kick if cannot switch to red team
								engine->ServerCommand(CFmtStr("kickid %d %s\n", player->GetUserID(), "Exceeded total player limit for the mission"));
								spectators -= 1;
							}
						}
					});
				}
				//ConVarRef allowspectators("mp_allowspectators");
				//allowspectators.SetValue(spectators >= state.m_iTotalSpectators ? "0" : "1");
			}

			// If send to spectator immediately is checked, check for any dead non spectator bot
			if (bot_killed_check) {
				bool need_check_again = false;
				ForEachTFPlayer([&](CTFPlayer *player){
					if (!player->IsAlive() && player->IsBot() && player->GetTeamNumber() > 1){
						if(gpGlobals->curtime - player->GetDeathTime() > 0.2f)
							player->ForceChangeTeam(TEAM_SPECTATOR, false);
						else
							need_check_again = true;
					}
				});
				bot_killed_check = need_check_again;
			}
			
			for (auto it = item_loading_queue.begin(); it != item_loading_queue.end(); ) {
				CBaseAnimating *entity = *it;
				if (entity == nullptr) {
					it = item_loading_queue.erase(it);
					continue;
				}

				if (entity->GetNumBones() == 0) {
					it++;
				}
				else {
					CTFPlayer *owner = ToTFPlayer(entity->GetOwnerEntity());
					if (owner != nullptr && !IsItemApplicableToRobotModel(owner, entity, false))
						entity->Remove();

					it = item_loading_queue.erase(it);
				}
			}

			received_message_tick = false;
			for (int i = 0 ; i < 33; i++)
				received_mission_message_tick[i] = false;

			if (checkClassLimitNextTick) {
				checkClassLimitNextTick = false;
				ForEachTFPlayer([&](CTFPlayer *player){
					if(player->GetTeamNumber() == TF_TEAM_RED && !player->IsBot()){
						CheckPlayerClassLimit(player);
					}
				});
			}
		}
	private:
		PlayerLoadoutUpdatedListener player_loadout_updated_listener;
	};
	CMod s_Mod;
	
	void TogglePatches() {
		s_Mod.ToggleAllPatches(false);
		s_Mod.ToggleAllPatches(true);
	}
	
	ConVar cvar_enable("sig_pop_popmgr_extensions", "0", FCVAR_NOTIFY,
		"Mod: enable extended KV in CPopulationManager::Parse",
		[](IConVar *pConVar, const char *pOldValue, float flOldValue){
			s_Mod.Toggle(static_cast<ConVar *>(pConVar)->GetBool());
		});

    ConVar cvar_custom_popfile{"sig_custom_popfile", "", FCVAR_NONE,
        "Force load specified popfile on map start in PvP "
        "(use tf_mvm_popfile instead for MvM)",
        [](IConVar* cvar, const char* old_value, auto){
            if(TFGameRules()->IsMannVsMachineMode())
                return;
            const char* value_c_str{cvar_custom_popfile.GetString()};
            std::string_view value{value_c_str};
            if(value == ""){
                state.Reset();
                ForEachTFPlayer([](CTFPlayer* player){
                    player->GetAttributeList()->DestroyAllAttributes();        
                });
                ConVarRef restart_cvar{"mp_restartgame_immediate"};
                restart_cvar.SetValue(1);
                return;
            }
            if(!filesystem->FileExists(value_c_str)){
                Warning("Cannot load popfile: file \"%s\" does not exist\n",
                        value_c_str);
                return;
            }
            state.Reset();
            ForEachTFPlayer([](CTFPlayer* player){
                if(player == nullptr) return;
                if(player->GetAttributeList() != nullptr)
                    player->GetAttributeList()->DestroyAllAttributes();        
	            for(int i{0}; i < player->GetNumWearables(); ++i){
		            CEconWearable* wearable = player->GetWearable(i);
		            if((wearable != nullptr)
                        && (wearable->GetItem() != nullptr)
                    ) wearable->GetItem()->GetAttributeList().DestroyAllAttributes();
	            }
	            for(int i{0}; i < player->WeaponCount(); ++i){
		            CBaseCombatWeapon* weapon = player->GetWeapon(i);
		            if((weapon != nullptr)
                        && (weapon->GetItem() != nullptr)
                    ) weapon->GetItem()->GetAttributeList().DestroyAllAttributes();
	            }
            });
            KeyValues* kv{new KeyValues{""}};
			reading_popfile = true;
            if(kv->LoadFromFile(filesystem, value_c_str))
                Parse_Popfile(kv, filesystem);
			reading_popfile = false;
            ConVarRef restart_cvar{"mp_restartgame_immediate"};
            restart_cvar.SetValue(1);
            kv->deleteThis();
        }};
	
	
	class CKVCond_PopMgr : public IKVCond
	{
	public:
		virtual bool operator()() override
		{
			return s_Mod.IsEnabled();
		}
	};
	CKVCond_PopMgr cond;
	
}
