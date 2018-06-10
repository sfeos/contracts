
UNLOCKED=`cleos wallet list | grep 'default \*' | wc -l`

if [ $UNLOCKED != 1 ]
then
    echo "Please unlock your default wallet using 'cleos wallet unlock'."
    exit 1
fi

echo "Launching nodeos..."

nodeos -e -p eosio --plugin eosio::wallet_api_plugin --plugin eosio::chain_api_plugin --plugin eosio::history_api_plugin --contracts-console --delete-all-blocks > /dev/null 2>&1 &

sleep 1
RES=0

echo "Started nodeos..."

cleos create account eosio sfeos EOS6TFjLqyVyjyTeFs7stFVfyYiDxYcbMSuSarSLbykidKXQWMto1 EOS7vj6SP5bmLnLKsGEgY6YsLX8v1rHCfRZNMWNjBBm9bvRWx4UsT
cleos set contract sfeos . ./Ship/Ship.wast ./Ship/Ship.abi
if [ $? -ne 0 ]; then { echo "ERROR: Failed to install contract, aborting." ; RES=1; } fi

echo "Installed contract..."

cleos push action sfeos add '["sfeos","My Ship!"]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to install create ship." ; RES=1; } fi

# TODO: why does this produce a "duplicate transaction" error
#cleos push action sfeos getship '["sfeos"]' -p sfeos@active
#if [ $? -ne 0 ]; then { echo "ERROR: Failed to get ship." ; RES=1; } fi

cleos push action sfeos update '["sfeos",5,5,0,110,220]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to update ship." ; RES=1; } fi

cleos push action sfeos getship '["sfeos"]' -p sfeos@active
if [ $? -ne 0 ]; then { echo "ERROR: Failed to get ship." ; RES=1; } fi


kill $!

if [ $RES -eq 0 ]
then
    echo "Test SUCCESS"
else
    echo "Test FAILURE"
fi

exit $RES