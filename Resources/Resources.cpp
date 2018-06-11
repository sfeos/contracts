#include "Resources.hpp"

namespace SFEOS {
    using namespace eosio;
    using std::string;

    class Resources : public contract {
        using contract::contract;

        public:

            Resources(account_name self):contract(self) {}


            /**
             * Action types are actually base32 encoded 64-bit integers. This means they are limited to the 
             * characters a-z, 1-5,  and '.' for the first 12 characters. If there is a 13th character then 
             * it is restricted to the first 16 characters ('.' and a-p). 
             **/

            /**
             * Creates a new recipe.  Will be called by the contract owner to set up recipes and the required
             * ingredients.  After calling this function the addingr action should be called to add ingredients
             * into the receipe. 
             */
            void addrecipe(uint64_t _resource_id, string _name) {
                // Can only be invoked by contract owner
                require_auth(_self);

                // Load table 
                recipeIndex recipes(_self, _self);

                // Make sure there isn't already a receipe for this resource
                auto iterator = recipes.find(_resource_id);
                eosio_assert(iterator == recipes.end(), "ID for resource already exists in recipes index");

                // Insert the recipe.  
                // First argument is the account name of the "payer of the storage." 
                recipes.emplace(_self, [&](auto& _receipe) {
                    _receipe.resource_id = _resource_id;
                    _receipe.name = _name;
                    _receipe.ingredient_count = 0;
                });

            }

            /**
             * Adds an ingredient to a receipe created with the addrecipe action
             * 
             * _resource_id     -- the ID fo the recipe to add this ingredient to.
             * _ingredient_id   -- the ID of the ingredient.
             * _quantity        -- the quantity of the ingredient required to craft the recipe.
             */
            void addingr(uint64_t _resource_id, uint64_t _ingredient_id, uint64_t _quantity) {
                // Can only be invoked by contract owner
                require_auth(_self);

                // Load table 
                recipeIndex recipes(_self, _self);

                // Make sure the recipe is there
                auto iterator = recipes.find(_resource_id);
                eosio_assert(iterator != recipes.end(), "Recipe not found");

                // Get a reference to the recipe
                auto currentRecipe = recipes.get(_resource_id);
                eosio_assert(currentRecipe.ingredient_count < 5, "Recipe already has the maximum number of ingredients");

                recipes.modify(iterator, _self, [&](auto& _recipe) {
                    _recipe.ingredient_count++;

                    ingredient _ingredient;
                    _ingredient.resource_id = _ingredient_id;
                    _ingredient.quantity = _quantity;

                    switch(_recipe.ingredient_count) {
                        case 1:
                            _recipe.ingredient1 = _ingredient; break;
                        case 2:
                            _recipe.ingredient2 = _ingredient; break;
                        case 3:
                            _recipe.ingredient3 = _ingredient; break;
                        case 4:
                            _recipe.ingredient4 = _ingredient; break;
                        case 5:
                            _recipe.ingredient5 = _ingredient; break;
                    }
                });
            }

            /**
             * Mints a new resource and assigns it the contract owner.
             * 
             * _resource_id     -- the ID of the resource to mint.
             * _name            -- the name of the resource to mint.
             * _quantity        -- the quantity of the resource to mint.
             */
            void mint(uint64_t _resource_id, string _name, uint64_t _quantity) {
                // Can only be invoked by contract owner
                require_auth(_self);

                resourceIndex resources(_self, _self);


                // Make sure there isn't already a receipe for this resource
                // TODO: allow adding additional resources when record exists.
                auto iterator = resources.find(_resource_id);
                eosio_assert(iterator == resources.end(), "ID for resource already exists in resources index");

                // Insert the resource.  
                resources.emplace(_self, [&](auto& _resource) {
                    _resource.resource_id = _resource_id;
                    _resource.name = _name;
                    _resource.owner = _self;
                    _resource.quantity_owned = _quantity;
                });
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

                
            }

            /**
             * Returns the details of the recipe in JSON format.
             */
            void getrecipe(uint64_t _resource_id) {
                // Can be invoked by anybody 

                recipeIndex recipes(_self, _self);

                // Make sure the recipe is there
                auto iterator = recipes.find(_resource_id);
                eosio_assert(iterator != recipes.end(), "Recipe not found");

                // Get a reference to the recipe
                auto currentRecipe = recipes.get(_resource_id);
                print("{ ", 
                        "\"name\": \"", currentRecipe.name.c_str(), "\", ", 
                        "\"resource_id\": ", currentRecipe.resource_id, ", ",
                        "\"ingredient_count\": ", currentRecipe.ingredient_count, ", ",
                        "\"ingredient1\": { \"resource_id\": ", currentRecipe.ingredient1.resource_id, ", \"quantity\": ", currentRecipe.ingredient1.quantity, " }, ",
                        "\"ingredient2\": { \"resource_id\": ", currentRecipe.ingredient2.resource_id, ", \"quantity\": ", currentRecipe.ingredient2.quantity, " }, ",
                        "\"ingredient3\": { \"resource_id\": ", currentRecipe.ingredient3.resource_id, ", \"quantity\": ", currentRecipe.ingredient3.quantity, " }, ",
                        "\"ingredient4\": { \"resource_id\": ", currentRecipe.ingredient4.resource_id, ", \"quantity\": ", currentRecipe.ingredient4.quantity, " }, ",
                        "\"ingredient5\": { \"resource_id\": ", currentRecipe.ingredient5.resource_id, ", \"quantity\": ", currentRecipe.ingredient5.quantity, " } ",
                      " }");
            }

        private:
            /***********************************
             ** struct definitions
             **********************************/

            //@abi table ingredient i64
            struct ingredient {
                uint64_t resource_id;
                uint64_t quantity;

                EOSLIB_SERIALIZE(ingredient, (resource_id)(quantity))
            };

            //@abi table recipe i64
            struct recipe {
                uint64_t resource_id;
                string name;
                uint32_t ingredient_count;
                
                // Types can only be: vector, struct, class or a built-in type
                // Ugly workaround 
                ingredient ingredient1;
                ingredient ingredient2;
                ingredient ingredient3;
                ingredient ingredient4;
                ingredient ingredient5;

                uint64_t primary_key() const { return resource_id; }

                EOSLIB_SERIALIZE(recipe, (resource_id)(name)(ingredient_count)(ingredient1)(ingredient2)(ingredient3)(ingredient4)(ingredient5))
            };

            //@abi table resource i64
            struct resource {
                uint64_t resource_id;
                string name;
                uint64_t owner;
                uint64_t quantity_owned;
                
                // TODO: can we store custom attributes in the database?

                uint64_t primary_key() const { return resource_id; }
                account_name get_owner() const { return owner; }

                EOSLIB_SERIALIZE(resource, (resource_id)(name)(owner)(quantity_owned))
            };

            /***********************************
             ** index declarations 
             **********************************/
            typedef multi_index<N(recipe), recipe> recipeIndex;
            typedef multi_index<N(resource), resource,
                indexed_by<N(byowner), const_mem_fun<resource, uint64_t, &resource::get_owner> > > resourceIndex;

    };

    EOSIO_ABI(Resources, (addrecipe)(addingr)(mint)(craft)(getrecipe))
}