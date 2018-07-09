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

var express = require('express');
var path = require('path');
var app = express();

var EosLib = require('eosjs');
var resources = require('./resources');
var nodeos = require('./nodeos');
var contract = require('./contract');
var config = require('./config');

var eos;


var init = false;
var args = process.argv.slice(2);
if(args.includes('--init')) {
    init = true;
    nodeos.deleteAllBlocks();
}

nodeos.spawn().then(() => {
    initEosLib();

    if(init) {
        initContracts()
            .then(() => startServer() );
    }
    else {
        loadContracts()
            .then(() => startServer());
    }
});
console.log("Waiting for nodeos to be ready...");



function initEosLib() {
    eos = EosLib({
        httpEndpoint: config.endpoint,
        chainId: config.chainId,
        verbose: config.verbose,
        keyProvider: [ config.privateKey ]
    });
}

function initContracts() {
    console.log("Initializing accounts and contracts...");

    return contract.publish(eos, 'res', './Resources/Resources', config.publicKey)
        .then(() => contract.publish(eos, 'ship', './Ship/Ship', config.publicKey))
        .then(() => resources.init());
}

function loadContracts() {
    return contract.load(eos, 'sfeos.res')
        .then(() => contract.load(eos, 'sfeos.ship'));
}

function startServer() {
    app.set('port', config.port);

    app.use(express.static(path.join(__dirname, './public')));

    var server = app.listen(app.get('port'), function () {
        console.log('The server is running on http://localhost:' + app.get('port'));
    });
}