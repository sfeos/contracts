#include "Ship.hpp"

namespace SFEPOS {
    using namespace eosio;
    using std::string;

    class Ships : public contract {
        using contract::contract;
        
        public:
            static const uint64_t MaxHealth = 1000;
            static const uint64_t MaxEnergy = 1000;

            Ships(account_name self):contract(self) {}

            //@abi action
            void add(const account_name account, string& name, uint64_t level) {
                /**
                 * Action must be initiated by either the owner of an account or somebody with the account authorization.
                 */
                require_auth(account);

                /**
                 * We access the "ships" table as creating an object of type "shipIndex"
                 * As parameters we pass code & scope - _self from the parent contract
                 */
                shipIndex ships(_self, _self);

                /**
                 * We must verify that the account doesn't exist yet
                 * If the account is not found the iterator variable should be players.end()
                 */
                auto iterator = ships.find(account);
                eosio_assert(iterator == ships.end(), "Address for account already exists");

                /**
                 * We add the new player in the table
                 * The first argument is the payer of the storage which will store the data
                 */
                ships.emplace(account, [&](auto& player) {
                    player.account_name = account;
                    player.name = name;
                    player.x = 0;
                    player.y = 0;
                    player.ghosted = 0;
                    player.health = MaxHealth;
                    player.energy = MaxEnergy;
                });
            }

            //@abi action
            void update(const account_name account, uint64_t _x, uint64_t _y, uint64_t _ghosted, int64_t _health, int64_t _energy) {
                require_auth(account);

                /**
                 * Load the ships table and verify that the ship is present in it.
                 */
                shipIndex ships(_self, _self);

                auto iterator = ships.find(account);
                eosio_assert(iterator != ships.end(), "Address for account not found");

                /**
                 * Modify the attributes of the ship in the table.
                 */
                ships.modify(iterator, account, [&](auto& ship) {
                    ship.x = _x;
                    ship.y = _y;
                    ship.ghosted = _ghosted;
                    ship.health = _health;
                    ship.energy = _energy;
                });
            }

            //@abi action
            void getship(const account_name account) {
                shipIndex ships(_self, _self);

                auto iterator = ships.find(account);
                eosio_assert(iterator != ships.end(), "Address for account not found");

                /**
                 * The "get" function returns a constant reference to the object containing the specified secondary key
                 */
                auto currentShip = ships.get(account);
                print("{ ", 
                        "\"name\": \"", currentShip.name.c_str(), "\", ", 
                        "\"x\": ", currentShip.x,                 ", ",
                        "\"y\": ", currentShip.y,                 ", ",
                        "\"ghosted\": ", currentShip.ghosted,     ", ",
                        "\"health\": ", currentShip.health,       ", ",  
                        "\"energy\": ", currentShip.energy,       " }");
            }

        private:

            //@abi table ship i64
            struct ship {
                uint64_t account_name;
                string name;
                uint64_t x;
                uint64_t y;
                uint64_t ghosted;
                uint64_t health;
                uint64_t energy;

                uint64_t primary_key() const { return account_name; }

                EOSLIB_SERIALIZE(ship, (account_name)(name)(x)(y)(ghosted)(health)(energy))
            };

            typedef multi_index<N(ship), ship> shipIndex;
    };

    EOSIO_ABI(Ships, (add)(update)(getship))
}