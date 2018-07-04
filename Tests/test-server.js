  /* 
   * -- Pre-requisites -- 
   * 
   * 1) Enable CORS headers in nodeos HTTP plugin.  To do so, open config.ini and add:
   *    access-control-allow-origin = *
   * 
   * 2) Replace chainId with the chain ID of the blockchain you are testing with.  To get that issue this command:
   *    cleos get info
   * 
   * 3) Unlock your wallet and obtain the public and private keys from it.  
   *    First call: cleos wallet unlock --password ...
   *    Then call: cleos wallet private_keys
   */

  /*
   * Run "npm install" first!
   * Then npm start
   * 
   */

const fs = require('fs');
var express = require('express');
var path = require('path');
var app = express();
const { spawn } = require('child_process');
var EosLib = require('eosjs');
var recipes = require('./recipes');

var eos;
var privateKey = "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3";
var publicKey = "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV";
var chainId = 'cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f';
var contracts = {};

var init = false;
var args = process.argv.slice(2);
if(args.includes('--init')) {
    init = true;
}

spawnNodeos();
console.log("Waiting for ndoeos to be ready...");


setTimeout(function() {
    initEosLib();

    if(init) {
        initContracts()
            .then(() => startServer() );
    }
    else {
        startServer();
    }
    
}, 12000);






function spawnNodeos() {
    var nodeosCmd = 'nodeos'
    var nodeosParams = ['-e', '-p', 'eosio', '--plugin', 'eosio::wallet_api_plugin', 
                        '--plugin', 'eosio::chain_api_plugin', '--plugin', 'eosio::history_api_plugin', 
                        '--contracts-console' ];
                        
    if(init)
        nodeosParams.push('--delete-all-blocks');
    
    const nodeos = spawn(nodeosCmd, nodeosParams);
    nodeos.on('exit', function (code, signal) {
        console.log('nodeos process exited with ' +
                `code ${code} and signal ${signal}`);
    });
    nodeos.stdout.on('data', (data) => {
        console.log('' + data);
    });
    nodeos.stderr.on('data', (data) => {
        if(('' + data).indexOf('Produced block') > 0) return; // prevent anoying log messages every 500ms 

        console.error('' + data);
    });
}

function initEosLib() {
    eos = EosLib({
        httpEndpoint: 'http://localhost:8888',
        chainId: chainId,
        verbose: true,
        keyProvider: [ privateKey ]
    });
}

function initContracts() {
    console.log("Initializing accounts and contracts...");

    return initContract('res', './Resources/Resources')
          .then(() => initContract('ship', './Ship/Ship'))
          .then(() => loadResources())
          .then(() => loadAllRecipes())
          .then(() => loadInitialSupply());

}

function initContract(name, path) {
    var wasm = fs.readFileSync(path + '.wasm');
    var abi = fs.readFileSync(path + '.abi');
    var account = 'sfeos.' + name;

    return createAccount(account, publicKey)
        .then(() => eos.setcode(account, 0, 0, wasm))
        .then(() => eos.setabi(account, JSON.parse(abi)))
        .then(() => eos.contract(account))
        .then((con) => contracts[account] = con);
}

function createAccount(name, pub) {
    return eos.transaction(tr => {
        tr.newaccount({
            creator: 'eosio',
            name: name,
            owner: publicKey,
            active: pub
        });
    });
}


var resAccount = 'sfeos.res';
var options = {
    authorization: resAccount + '@active',
    broadcast: true,
    sign: true
}


function loadResources() {
    var all = [];
    for(var key in recipes.resources) {
        var res = recipes.resources[key];
        all.push(createResource(res.id, res.name));
    }
    
    return Promise.all(all);
}

function createResource(id, name) {
    return contracts[resAccount].addresource(id,  name, options);
}

function loadAllRecipes() {
    var all = [];
    for(var key in recipes.recipes) {
        var recipe = recipes.recipes[key];
        var resourceId = recipes.resources[key].id;

        for(var ingKey in recipe) {
            var ingredientId = recipes.resources[ingKey].id;
            var quantity = recipe[ingKey];

            all.push( contracts[resAccount].addingr(resourceId, ingredientId, quantity, options) );
        }
    }

    return Promise.all(all);
}

function loadInitialSupply() {
    var all = [];

    for(var key in recipes.supply) {
        var resourceId = recipes.resources[key].id;
        var quantity = recipes.supply[key];

        all.push( contracts[resAccount].mint(resourceId, quantity, options) );
    }

    return Promise.all(all);
}


function startServer() {
    app.set('port', 3000);

    app.use(express.static(path.join(__dirname, '.')));


    var server = app.listen(app.get('port'), function () {
    console.log('The server is running on http://localhost:' + app.get('port'));
    });
}