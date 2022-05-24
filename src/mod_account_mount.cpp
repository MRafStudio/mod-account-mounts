#include "Config.h"
#include "ScriptMgr.h"
#include "Chat.h"
#include "Player.h"

class AccountMounts : public PlayerScript
{
    static const bool limitrace = false; // Если установлено значение true, изучение ездовых животных будет только у персонажей из одной фракции.
public:
    AccountMounts() : PlayerScript("AccountMounts") { }

    void OnLogin(Player* pPlayer)
    {
        if (sConfigMgr->GetBoolDefault("Account.Mounts.Enable", true))
        {
            if (sConfigMgr->GetBoolDefault("Account.Mounts.Announce", false))
            {
				WorldSession* session = pPlayer->GetSession();
                std::string message = "";
                switch (session->GetSessionDbLocaleIndex())
                {
                case LOCALE_ruRU:
                {
                    message = "На сервере запущен модуль";
                    break;
                }
                default:
                    message = "This server is running the";
                    break;
                }
                ChatHandler(pPlayer->GetSession()).SendSysMessage(message + " |cff4CFF00AccountMounts |r");
            }
            std::vector<uint32> Guids;
            QueryResult result1 = CharacterDatabase.Query("SELECT guid, race FROM characters WHERE account = {}", pPlayer->GetSession()->GetAccountId());
            if (!result1)
                return;

            do
            {
                Field* fields = result1->Fetch();
    
                //uint32 guid = fields[0].GetUInt32(); //unused variable
                uint32 race = fields[1].Get<uint8>();

                if ((Player::TeamIdForRace(race) == Player::TeamIdForRace(pPlayer->getRace())) || !limitrace)
                    Guids.push_back(result1->Fetch()[0].Get<uint32>());

            } while (result1->NextRow());

            std::vector<uint32> Spells;

            for (auto& i : Guids)
            {
                QueryResult result2 = CharacterDatabase.Query("SELECT spell FROM character_spell WHERE guid = {}", i);
                if (!result2)
                    continue;

                do
                {
                    Spells.push_back(result2->Fetch()[0].Get<uint32>());
                } while (result2->NextRow());
            }

            for (auto& i : Spells)
            {
                auto sSpell = sSpellStore.LookupEntry(i);
                if (sSpell->Effect[0] == SPELL_EFFECT_APPLY_AURA && sSpell->EffectApplyAuraName[0] == SPELL_AURA_MOUNTED)
                    pPlayer->learnSpell(sSpell->Id);
            }
        }
    }
};

void AddAccountMountsScripts()
{
    new AccountMounts;
}
