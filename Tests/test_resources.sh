
UNLOCKED=`cleos wallet list | grep 'default \*' | wc -l`

if [ $UNLOCKED != 1 ]
then
    echo "Please unlock your default wallet using 'cleos wallet unlock'."
    exit 1
fi

echo "Launching nodeos..."

nodeos -e -p eosio --plugin eosio::wallet_api_plugin --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --contracts-console --delete-all-blocks > /dev/null 2>&1 &

sleep 6
RES=0

echo "Started nodeos..."

cleos create account eosio sfeos EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos set contract sfeos ./Resources  ./Resources/Resources.wast ./Resources/Resources.abi
if [ $? -ne 0 ]; then { echo "ERROR: Failed to install contract." ; kill $! ; exit 1; } fi

echo "Installed contract..."

cleos push action sfeos addresource '[1, "Oxygen"]' -p sfeos@active
cleos push action sfeos addresource '[2, "Hydrogen"]' -p sfeos@active
cleos push action sfeos addresource '[3, "Water"]' -p sfeos@active

# require 1 oxygen and 2 hyrogen
cleos push action sfeos addingr '[3, 1, 1]' -p sfeos@active
cleos push action sfeos addingr '[3, 2, 1]' -p sfeos@active

# mint initial resources
cleos push action sfeos mint '[1, 1]' -p sfeos@active
cleos push action sfeos mint '[2, 2]' -p sfeos@active



#if [ $? -ne 0 ]; then { echo "ERROR: Failed to add recipe." ; RES=1; } fi


cleos push action sfeos getresource '[3]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to get recipe." ; RES=1; } fi

cleos push action sfeos craft '["sfeos", 3, 1]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to craft resource." ; RES=1; } fi

cleos push action sfeos getinventory '["sfeos"]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to get resources." ; RES=1; } fi

kill $!

if [ $RES -eq 0 ]
then
    echo "Test SUCCESS"
else
    echo "Test FAILURE"
fi

exit $RES