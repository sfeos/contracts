#include "Resources.hpp"

namespace SFEOS {
    using namespace eosio;
    using std::string;

    class Resources : public contract {
        using contract::contract;

        public:
            const static uint32_t MaxIngredients = 5;

            Resources(account_name self):contract(self) {}


            /**
             * Action types are actually base32 encoded 64-bit integers. This means they are limited to the 
             * characters a-z, 1-5,  and '.' for the first 12 characters. If there is a 13th character then 
             * it is restricted to the first 16 characters ('.' and a-p). 
             **/

            /**
             * Creates a new resource.  Will be called by the contract owner to set up recipes and the required
             * ingredients.  After calling this function the addingr action should be called to add ingredients
             * into the receipe. 
             */
            void addresource(uint64_t _resource_id, string _name) {
                // Can only be invoked by contract owner
                require_auth(_self);

                // Load table 
                resources_table resources(_self, _self);

                // Make sure there isn't already a receipe for this resource
                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator == resources.end(), "ID for resource already exists in resources index");

                // Insert the recipe.  
                // First argument is the account name of the "payer of the storage." 
                resources.emplace(_self, [&](auto& _resource) {
                    _resource.resource_id = _resource_id;
                    _resource.name = _name;
                    _resource.crafting_recipe.ingredient_count = 0;
                });
            }

            /**
             * Adds an ingredient to a resource receipe.  Resource must have been created with the addresource action.
             * 
             * _resource_id     -- the ID fo the recipe to add this ingredient to.
             * _ingredient_id   -- the ID of the ingredient.
             * _quantity        -- the quantity of the ingredient required to craft the recipe.
             */
            void addingr(uint64_t _resource_id, uint64_t _ingredient_id, uint64_t _quantity) {
                // Can only be invoked by contract owner
                require_auth(_self);

                // Load table 
                resources_table resources(_self, _self);

                // Make sure the recipe is there
                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator != resources.end(), "Resource not found");

                // Get a reference to the recipe
                auto current_resource = resources.get(_resource_id);
                eosio_assert(current_resource.crafting_recipe.ingredient_count < MaxIngredients, "Recipe already has the maximum number of ingredients");

                resources.modify(iterator, _self, [&](auto& _resource) {
                    _resource.crafting_recipe.ingredient_count++;

                    ingredient _ingredient;
                    _ingredient.resource_id = _ingredient_id;
                    _ingredient.quantity = _quantity;

                    _resource.crafting_recipe.ingredients.push_back(_ingredient);
                });
            }

            /**
             * Mints a new resource and assigns it the contract owner.
             * 
             * _resource_id     -- the ID of the resource to mint.
             * _name            -- the name of the resource to mint.
             * _quantity        -- the quantity of the resource to mint.
             */
            void mint(uint64_t _resource_id, uint64_t _quantity) {
                // Can only be invoked by contract owner
                require_auth(_self);

                resources_table resources(_self, _self);

                // Make sure the resource exists
                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator != resources.end(), "Resource not found");

                updateResource(_self, _resource_id, _quantity);
            }

            /**
             * Crafts a resource and assigns it to the specified account.  The owner must have
             * the necessary ingredients and required quantities for the craft action to succeed.
             * 
             * account      -- the account that will become the owner of the resource.
             * _resource_id -- the ID of the resource to craft.
             * _quantity    -- the quantity of the resource to craft.
             * 
             * TODO: figure out how to handle non-fungible resources. 
             */
            void craft(const account_name account, uint64_t _resource_id, uint64_t _quantity) {
                // Can only be invoked by cafter
                require_auth(account);

                // Load the resource for the resource we are crafting.
                resources_table resources(_self, _self);

                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator != resources.end(), "Resouce not found");

                auto current_resource = resources.get(_resource_id);

                // Load the list of resources that this account has.
                inventory_table ownership(_self, _self); 

                auto iterator1 = ownership.find(account);
                eosio_assert(iterator1 != ownership.end(), "Account not found");

                auto owner_resources = ownership.get(account);

                // Modify the resource counts of the account to produce the crafted resource. 
                for(auto it = current_resource.crafting_recipe.ingredients.begin(); it != current_resource.crafting_recipe.ingredients.end(); it++ ) {

                    updateResource(account, it->resource_id, -1 * it->quantity * _quantity);

                }

                // Assign the crafted resource to the account
                updateResource(account, _resource_id, _quantity);
                
            }

            /**
             * Returns the details of the resource in JSON format.
             * 
             * Can be invoked by anybody.
             */
            void getresource(uint64_t _resource_id) {

                resources_table resources(_self, _self);

                // Make sure the recipe is there
                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator != resources.end(), "Recipe not found");

                // Get a reference to the recipe
                auto current_resource = resources.get(_resource_id);

                // Convert into JSON
                std::string ingredients_str;
                ingredients_str.append("[ ");
                for(auto it = current_resource.crafting_recipe.ingredients.begin(); it != current_resource.crafting_recipe.ingredients.end(); it++ ) {
                    
                    if(it != current_resource.crafting_recipe.ingredients.begin())
                        ingredients_str.append(", ");

                    ingredients_str.append("{ \"resource_id\": ");
                    ingredients_str.append( std::to_string( it->resource_id ) );
                    ingredients_str.append(", \"quantity\": ");
                    ingredients_str.append( std::to_string( it->quantity ) );
                    ingredients_str.append(" }");
                }
                ingredients_str.append(" ]");

                print("{ ", 
                        "\"name\": \"", current_resource.name.c_str(), "\", ", 
                        "\"resource_id\": ", current_resource.resource_id, ", ",
                        "\"crafting_recipe\": { ",
                          "\"ingredient_count\": ", current_resource.crafting_recipe.ingredient_count, ", ",
                          "\"ingredients\":", ingredients_str,
                        " }"
                      " }");
            }

            /**
             * Returns an inventory of resources for the associated account in JSON format.
             * 
             * Can be invoked by anybody.
             */ 
            void getinventory(const account_name account) {
                
                inventory_table ownership(_self, _self);

                auto iterator = ownership.find(account);
                eosio_assert(iterator != ownership.end(), "Account not found");

                // Create JSON string that will be returned
                std::string json;
                json.append("[ ");

                // Load all resources associated with the specified account using the "byowner" index.
                auto ownership_record = ownership.get(account);

                // Iterate through owned resources in the vector 
                int i = 0;
                for(auto it = ownership_record.resources.begin(); it != ownership_record.resources.end(); it++,i++ )    {

                    if(i > 0)
                        json.append(", ");

                    json.append("{ ");

                    json.append("\"resource_id\": ");
                    json.append( std::to_string( it->resource_id ) );

                    json.append(", \"quantity_owned\": ");
                    json.append( std::to_string( it->quantity ) );

                    json.append(" }");
                } 

                json.append(" ]");
                print(json);
            }

        private:
            void updateResource(account_name account, uint64_t _resource_id, int64_t _quantity) {
                inventory_table ownership(_self, _self);

                auto iterator = ownership.find(account);
                if(iterator != ownership.end()) {
                    // Modify resource on exitsing record
                    bool found = false;

                    ownership.modify(iterator, _self, [&](auto& _ownership) {

                        for( auto& _res : _ownership.resources ) {
                            if(_res.resource_id == _resource_id) {
                                
                                int64_t result = _res.quantity + _quantity;
                                eosio_assert(result >= 0, "Can not have negative quantity of resource");

                                _res.quantity = result;
                                found = true;
                            }
                        }

                        if(found == false) {
                            resource_quantity qty;
                            qty.resource_id = _resource_id;
                            qty.quantity = _quantity;

                            eosio_assert(_quantity >= 0, "Can not have negative quantity of resource");

                            _ownership.resources.push_back(qty);
                        }
                    });
                }
                else {
                    // Insert the record.  
                    ownership.emplace(_self, [&](auto& _ownership) {
                        _ownership.owner = _self;

                        resource_quantity qty;
                        qty.resource_id = _resource_id;
                        qty.quantity = _quantity;

                        eosio_assert(_quantity >= 0, "Can not have negative quantity of resource");

                        _ownership.resources.push_back(qty);
                    });
                }
            }

            /***********************************
             ** struct definitions
             **********************************/

            struct ingredient {
                uint64_t resource_id;
                uint64_t quantity;

                EOSLIB_SERIALIZE(ingredient, (resource_id)(quantity))
            };

            struct recipe {
                uint32_t ingredient_count;
                vector<ingredient> ingredients;

                // Other crafting requirements, like skill level, go here...

                EOSLIB_SERIALIZE(recipe, (ingredient_count)(ingredients))
            };

            //@abi table resource i64
            struct resource {
                uint64_t resource_id;
                string name;
                recipe crafting_recipe;

                uint64_t primary_key() const { return resource_id; }

                EOSLIB_SERIALIZE(resource, (resource_id)(name)(crafting_recipe))
            };

            struct resource_quantity {
                uint64_t resource_id;
                uint64_t quantity;

                EOSLIB_SERIALIZE(resource_quantity, (resource_id)(quantity))
            };

            //@abi table inventory i64
            struct inventory {
                account_name owner;
                vector<resource_quantity> resources;

                uint64_t primary_key() const { return owner; }

                EOSLIB_SERIALIZE(inventory, (owner)(resources))
            };

            /***********************************
             ** index declarations 
             **********************************/
            typedef multi_index<N(resource), resource> resources_table;
            typedef multi_index<N(inventory), inventory> inventory_table;

    };

    EOSIO_ABI(Resources, (addresource)(addingr)(mint)(craft)(getresource)(getinventory))
}