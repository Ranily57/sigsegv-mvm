#ifndef _INCLUDE_SIGSEGV_STUB_TFPLAYER_H_
#define _INCLUDE_SIGSEGV_STUB_TFPLAYER_H_


#include "stub/baseanimating.h"
#include "stub/econ.h"
#include "prop.h"


class CBaseCombatWeapon;
class CTFWeaponBase;
class CTFPlayer;


class CBaseCombatCharacter : public CBaseFlex
{
public:
	CBaseCombatWeapon *GetActiveWeapon() const { return this->m_hActiveWeapon; }
	
private:
	DECL_SENDPROP(CHandle<CBaseCombatWeapon>, m_hActiveWeapon);
};

class CBasePlayer : public CBaseCombatCharacter
{
public:
	const char *GetPlayerName() { return m_szNetname.GetPtr(); }
	
	void EyeVectors(Vector *pForward, Vector *pRight = nullptr, Vector *pUp = nullptr) { return ft_EyeVectors(this, pForward, pRight, pUp); }
	
	bool IsBot() const                                             { return vt_IsBot        (this); }
	void CommitSuicide(bool bExplode = false, bool bForce = false) {        vt_CommitSuicide(this, bExplode, bForce); }
	void ForceRespawn()                                            {        vt_ForceRespawn (this); }
	
private:
	DECL_DATAMAP(char, m_szNetname);
	
	static MemberFuncThunk<CBasePlayer *, void, Vector *, Vector *, Vector *> ft_EyeVectors;
	
	static MemberVFuncThunk<const CBasePlayer *, bool>             vt_IsBot;
	static MemberVFuncThunk<      CBasePlayer *, void, bool, bool> vt_CommitSuicide;
	static MemberVFuncThunk<      CBasePlayer *, void>             vt_ForceRespawn;
};

class CBaseMultiplayerPlayer : public CBasePlayer
{
public:
	bool SpeakConceptIfAllowed(int iConcept, const char *modifiers = nullptr, char *pszOutResponseChosen = nullptr, size_t bufsize = 0, IRecipientFilter *filter = nullptr)
	{
		return vt_SpeakConceptIfAllowed(this, iConcept, modifiers, pszOutResponseChosen, bufsize, filter);
	}
	
private:
	static MemberVFuncThunk<CBaseMultiplayerPlayer *, bool, int, const char *, char *, size_t, IRecipientFilter *> vt_SpeakConceptIfAllowed;
};


enum
{
	TF_CLASS_UNDEFINED = 0,
	
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HEAVYWEAPONS,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	
	TF_CLASS_COUNT,
};


enum ETFCond
{
	TF_COND_AIMING                           =   0,
	TF_COND_ZOOMED                           =   1,
	TF_COND_DISGUISING                       =   2,
	TF_COND_DISGUISED                        =   3,
	TF_COND_STEALTHED                        =   4,
	TF_COND_INVULNERABLE                     =   5,
	TF_COND_TELEPORTED                       =   6,
	TF_COND_TAUNTING                         =   7,
	TF_COND_INVULNERABLE_WEARINGOFF          =   8,
	TF_COND_STEALTHED_BLINK                  =   9,
	TF_COND_SELECTED_TO_TELEPORT             =  10,
	TF_COND_CRITBOOSTED                      =  11,
	TF_COND_TMPDAMAGEBONUS                   =  12,
	TF_COND_FEIGN_DEATH                      =  13,
	TF_COND_PHASE                            =  14,
	TF_COND_STUNNED                          =  15,
	TF_COND_OFFENSEBUFF                      =  16,
	TF_COND_SHIELD_CHARGE                    =  17,
	TF_COND_DEMO_BUFF                        =  18,
	TF_COND_ENERGY_BUFF                      =  19,
	TF_COND_RADIUSHEAL                       =  20,
	TF_COND_HEALTH_BUFF                      =  21,
	TF_COND_BURNING                          =  22,
	TF_COND_HEALTH_OVERHEALED                =  23,
	TF_COND_URINE                            =  24,
	TF_COND_BLEEDING                         =  25,
	TF_COND_DEFENSEBUFF                      =  26,
	TF_COND_MAD_MILK                         =  27,
	TF_COND_MEGAHEAL                         =  28,
	TF_COND_REGENONDAMAGEBUFF                =  29,
	TF_COND_MARKEDFORDEATH                   =  30,
	TF_COND_NOHEALINGDAMAGEBUFF              =  31,
	TF_COND_SPEED_BOOST                      =  32,
	TF_COND_CRITBOOSTED_PUMPKIN              =  33,
	TF_COND_CRITBOOSTED_USER_BUFF            =  34,
	TF_COND_CRITBOOSTED_DEMO_CHARGE          =  35,
	TF_COND_SODAPOPPER_HYPE                  =  36,
	TF_COND_CRITBOOSTED_FIRST_BLOOD          =  37,
	TF_COND_CRITBOOSTED_BONUS_TIME           =  38,
	TF_COND_CRITBOOSTED_CTF_CAPTURE          =  39,
	TF_COND_CRITBOOSTED_ON_KILL              =  40,
	TF_COND_CANNOT_SWITCH_FROM_MELEE         =  41,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK        =  42,
	TF_COND_REPROGRAMMED                     =  43,
	TF_COND_CRITBOOSTED_RAGE_BUFF            =  44,
	TF_COND_DEFENSEBUFF_HIGH                 =  45,
	TF_COND_SNIPERCHARGE_RAGE_BUFF           =  46,
	TF_COND_DISGUISE_WEARINGOFF              =  47,
	TF_COND_MARKEDFORDEATH_SILENT            =  48,
	TF_COND_DISGUISED_AS_DISPENSER           =  49,
	TF_COND_SAPPED                           =  50,
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGED =  51,
	TF_COND_INVULNERABLE_USER_BUFF           =  52,
	TF_COND_HALLOWEEN_BOMB_HEAD              =  53,
	TF_COND_HALLOWEEN_THRILLER               =  54,
	TF_COND_RADIUSHEAL_ON_DAMAGE             =  55,
	TF_COND_CRITBOOSTED_CARD_EFFECT          =  56,
	TF_COND_INVULNERABLE_CARD_EFFECT         =  57,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST       =  58,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST        =  59,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST         =  60,
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST      =  61,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST       =  62,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST        =  63,
	TF_COND_STEALTHED_USER_BUFF              =  64,
	TF_COND_MEDIGUN_DEBUFF                   =  65,
	TF_COND_STEALTHED_USER_BUFF_FADING       =  66,
	TF_COND_BULLET_IMMUNE                    =  67,
	TF_COND_BLAST_IMMUNE                     =  68,
	TF_COND_FIRE_IMMUNE                      =  69,
	TF_COND_PREVENT_DEATH                    =  70,
	TF_COND_MVM_BOT_STUN_RADIOWAVE           =  71,
	TF_COND_HALLOWEEN_SPEED_BOOST            =  72,
	TF_COND_HALLOWEEN_QUICK_HEAL             =  73,
	TF_COND_HALLOWEEN_GIANT                  =  74,
	TF_COND_HALLOWEEN_TINY                   =  75,
	TF_COND_HALLOWEEN_IN_HELL                =  76,
	TF_COND_HALLOWEEN_GHOST_MODE             =  77,
	TF_COND_MINICRITBOOSTED_ON_KILL          =  78,
	TF_COND_OBSCURED_SMOKE                   =  79,
	TF_COND_PARACHUTE_DEPLOYED               =  80,
	TF_COND_BLASTJUMPING                     =  81,
	TF_COND_HALLOWEEN_KART                   =  82,
	TF_COND_HALLOWEEN_KART_DASH              =  83,
	TF_COND_BALLOON_HEAD                     =  84,
	TF_COND_MELEE_ONLY                       =  85,
	TF_COND_SWIMMING_CURSE                   =  86,
	TF_COND_FREEZE_INPUT                     =  87,
	TF_COND_HALLOWEEN_KART_CAGE              =  88,
	TF_COND_DONOTUSE_0                       =  89,
	TF_COND_RUNE_STRENGTH                    =  90,
	TF_COND_RUNE_HASTE                       =  91,
	TF_COND_RUNE_REGEN                       =  92,
	TF_COND_RUNE_RESIST                      =  93,
	TF_COND_RUNE_VAMPIRE                     =  94,
	TF_COND_RUNE_WARLOCK                     =  95,
	TF_COND_RUNE_PRECISION                   =  96,
	TF_COND_RUNE_AGILITY                     =  97,
	TF_COND_GRAPPLINGHOOK                    =  98,
	TF_COND_GRAPPLINGHOOK_SAFEFALL           =  99,
	TF_COND_GRAPPLINGHOOK_LATCHED            = 100,
	TF_COND_GRAPPLINGHOOK_BLEEDING           = 101,
	TF_COND_AFTERBURN_IMMUNE                 = 102,
	TF_COND_RUNE_KNOCKOUT                    = 103,
	TF_COND_RUNE_IMBALANCE                   = 104,
	TF_COND_CRITBOOSTED_RUNE_TEMP            = 105,
	TF_COND_PASSTIME_INTERCEPTION            = 106,
	TF_COND_SWIMMING_NO_EFFECTS              = 107,
	TF_COND_PURGATORY                        = 108,
	TF_COND_RUNE_KING                        = 109,
	TF_COND_RUNE_PLAGUE                      = 110,
	TF_COND_RUNE_SUPERNOVA                   = 111,
	TF_COND_PLAGUE                           = 112,
	TF_COND_KING_BUFFED                      = 113,
	TF_COND_TEAM_GLOWS                       = 114,
	TF_COND_KNOCKED_INTO_AIR                 = 115,
	TF_COND_COMPETITIVE_WINNER               = 116,
	TF_COND_COMPETITIVE_LOSER                = 117,
	TF_COND_NO_TAUNTING                      = 118,
};


class CTFPlayerClassShared
{
public:
	void NetworkStateChanged();
	void NetworkStateChanged(void *pVar);
	
	CTFPlayer *GetOuter();
	
	bool IsClass(int iClass) const { return (this->m_iClass == iClass); }
	
	int GetClassIndex() const { return this->m_iClass; }
	// TODO: accessor for m_iszClassIcon
	// TODO: accessor for m_iszCustomModel
	
private:
	DECL_SENDPROP(int,      m_iClass);
	DECL_SENDPROP(string_t, m_iszClassIcon);
	DECL_SENDPROP(string_t, m_iszCustomModel);
};
class CTFPlayerClass : public CTFPlayerClassShared {};

class CTFPlayerShared
{
public:
	void NetworkStateChanged();
	void NetworkStateChanged(void *pVar);
	
	CTFPlayer *GetOuter();
	
	void AddCond(ETFCond cond, float duration = -1.0f, CBaseEntity *provider = nullptr) {        ft_AddCond   (this, cond, duration, provider); }
	void RemoveCond(ETFCond cond, bool b1 = false)                                      {        ft_RemoveCond(this, cond, b1); }
	bool InCond(ETFCond cond) const                                                     { return ft_InCond    (this, cond); }
	
	void StunPlayer(float duration, float amount, int flags, CTFPlayer *stunner) { ft_StunPlayer(this, duration, amount, flags, stunner); }
	
	DECL_SENDPROP(float, m_flRageMeter);
	DECL_SENDPROP(bool,  m_bRageDraining);
	DECL_SENDPROP(bool,  m_bInUpgradeZone);
	
private:
	static MemberFuncThunk<      CTFPlayerShared *, void, ETFCond, float, CBaseEntity * > ft_AddCond;
	static MemberFuncThunk<      CTFPlayerShared *, void, ETFCond, bool                 > ft_RemoveCond;
	static MemberFuncThunk<const CTFPlayerShared *, bool, ETFCond                       > ft_InCond;
	static MemberFuncThunk<      CTFPlayerShared *, void, float, float, int, CTFPlayer *> ft_StunPlayer;
};

class CTFPlayer : public CBaseMultiplayerPlayer
{
public:
	CTFPlayerClass *GetPlayerClass()             { return m_PlayerClass.GetPtr(); }
	const CTFPlayerClass *GetPlayerClass() const { return m_PlayerClass.GetPtr(); }
	
	bool IsPlayerClass(int iClass) const;
	bool IsMiniBoss() const { return this->m_bIsMiniBoss; }
	
	CTFWeaponBase *GetActiveTFWeapon() const;
	
	void ForceChangeTeam(int team, bool b1)              { ft_ForceChangeTeam          (this, team, b1); }
	void StartBuildingObjectOfType(int iType, int iMode) { ft_StartBuildingObjectOfType(this, iType, iMode); }
	
//	typedef int taunts_t;
//	void Taunt(taunts_t, int);
	
	DECL_SENDPROP(CTFPlayerShared, m_Shared);
	
private:
	DECL_SENDPROP(CTFPlayerClass, m_PlayerClass);
	DECL_SENDPROP(bool,           m_bIsMiniBoss);
	
	static MemberFuncThunk<CTFPlayer *, void, int, bool> ft_ForceChangeTeam;
	static MemberFuncThunk<CTFPlayer *, void, int, int > ft_StartBuildingObjectOfType;
//	static MemberFuncThunk<CTFPlayer *, void, taunts_t, int> ft_Taunt;
};

class CTFPlayerSharedUtils
{
public:
	static CEconItemView *GetEconItemViewByLoadoutSlot(CTFPlayer *player, int slot, CEconEntity **ent = nullptr) { return ft_GetEconItemViewByLoadoutSlot(player, slot, ent); }
	
private:
	static StaticFuncThunk<CEconItemView *, CTFPlayer *, int, CEconEntity **> ft_GetEconItemViewByLoadoutSlot;
};


inline CTFPlayer *CTFPlayerClassShared::GetOuter()
{
	static int off = Prop::FindOffsetAssert("CTFPlayer", "m_PlayerClass");
	return (CTFPlayer *)((uintptr_t)this - off);
}

inline void CTFPlayerClassShared::NetworkStateChanged()           { this->GetOuter()->NetworkStateChanged(); }
inline void CTFPlayerClassShared::NetworkStateChanged(void *pVar) { this->GetOuter()->NetworkStateChanged(pVar); }


inline CTFPlayer *CTFPlayerShared::GetOuter()
{
	static int off = Prop::FindOffsetAssert("CTFPlayer", "m_Shared");
	return (CTFPlayer *)((uintptr_t)this - off);
}

inline void CTFPlayerShared::NetworkStateChanged()           { this->GetOuter()->NetworkStateChanged(); }
inline void CTFPlayerShared::NetworkStateChanged(void *pVar) { this->GetOuter()->NetworkStateChanged(pVar); }


inline CBasePlayer *ToBasePlayer(CBaseEntity *pEntity)
{
	if (pEntity == nullptr)   return nullptr;
	if (!pEntity->IsPlayer()) return nullptr;
	
	return rtti_cast<CBasePlayer *>(pEntity);
}

inline CBaseMultiplayerPlayer *ToBaseMultiplayerPlayer(CBaseEntity *pEntity)
{
	if (pEntity == nullptr)   return nullptr;
	if (!pEntity->IsPlayer()) return nullptr;
	
	return rtti_cast<CBaseMultiplayerPlayer *>(pEntity);
}

inline CTFPlayer *ToTFPlayer(CBaseEntity *pEntity)
{
	if (pEntity == nullptr)   return nullptr;
	if (!pEntity->IsPlayer()) return nullptr;
	
	return rtti_cast<CTFPlayer *>(pEntity);
}


ETFCond GetTFConditionFromName(const char *name);
const char *GetTFConditionName(ETFCond cond);


#endif