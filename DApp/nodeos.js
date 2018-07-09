
const { spawn } = require('child_process');
const http = require('http');
const config = require('./config');

var nodeos;
var endpoint = 'http://localhost:8888/';

var nodeosParams = ['-e', '-p', 'eosio', '--plugin', 'eosio::wallet_api_plugin', 
                        '--plugin', 'eosio::chain_api_plugin', '--plugin', 'eosio::history_api_plugin', 
                        '--contracts-console' ];

function deleteAllBlocks() {
    nodeosParams.push('--delete-all-blocks');
}
module.exports.deleteAllBlocks = deleteAllBlocks;



function spawnNodeos() {
    nodeos = spawn('nodeos', nodeosParams);
    nodeos.on('exit', function (code, signal) {
        console.log('nodeos process exited with ' +
                `code ${code} and signal ${signal}`);
    });
    nodeos.stdout.on('data', (data) => {
        if(!config.verbose) return;

        console.log('' + data);
    });
    nodeos.stderr.on('data', (data) => {
        if(!config.verbose) return;
        
        if(('' + data).indexOf('Produced block') > 0) return; // prevent anoying log messages every 500ms 

        console.error('' + data);
    });

    return waitForLaunch();
}
module.exports.spawn = spawnNodeos;

function waitForLaunch() {
    return new Promise((resolve, reject) => {
        var retryCount = 0;

        function tryConnect() {
            setTimeout(() => {
                testConnection().then(() => {
                    setTimeout(resolve, 1000);
                    console.log('nodeos is up and running...');
                }, (err) => {
                    retryCount++;
                    if(retryCount < 20) {
                        tryConnect();
                    }
                    else {
                        console.log('nodeos failed to start up!!!');
                        reject(err);
                    }
                });
            }, 2000);
        };
        
        tryConnect();
    });
}

function testConnection() {
    return new Promise((resolve, reject) => {
        http.get(endpoint + 'v1/chain/get_info', (resp) => {
            resp.on('end', () => {
                resolve();
            });
            resp.on('data', (data) => { });
        }).on('error', (err) => {
            reject(err);
        });
    });
}


function killNodeos() {
    nodeos.kill();
}

module.exports.kill = killNodeos;