
const fs = require('fs');
var contracts = {};

function publish(eos, name, path, publicKey) {
    var wasm = fs.readFileSync(path + '.wasm');
    var abi = fs.readFileSync(path + '.abi');
    var account = 'sfeos.' + name;

    return createAccount(eos, account, publicKey, publicKey)
        .then(() => eos.setcode(account, 0, 0, wasm))
        .then(() => eos.setabi(account, JSON.parse(abi)))
        .then(() => eos.contract(account))
        .then((con) => contracts[account] = con);
}
module.exports.publish = publish;

function get(name) {
    return contracts[name];
}
module.exports.get = get;

function load(eos, name) {
    return eos.contract(name).then((con) => contracts[name] = con);
}
module.exports.load = load;

function createAccount(eos, name, contractPublicKey, ownerPublicKey) {
    return eos.transaction(tr => {
        tr.newaccount({
            creator: 'eosio',
            name: name,
            owner: ownerPublicKey,
            active: contractPublicKey
        });
    });
}
