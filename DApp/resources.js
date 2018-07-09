
const contract = require('./contract');

const resources = {
    steel: {
        id: 1,
        name: "Steel"
    },
    aluminium: {
        id: 2,
        name: "Aluminium"
    },
    diamond: {
        id: 3,
        name: "Diamond"
    },
    silicon: {
        id: 4,
        name: "Silicon"
    },

    graphene: {
        id: 5,
        name: "Graphene"
    },
    electronics: {
        id: 6,
        name: "Electronics"
    },

    basic_hull: {
        id: 7,
        name: "Basic Hull"
    },
    graphene_hull: {
        id: 8,
        name: "Graphene Hull"
    },
    basic_scanner: {
        id: 9,
        name: "Basic Scanner"
    },
    basic_shield_generator: {
        id: 10,
        name: "Basic Shield Generator"
    },
    basic_engine_core: {
        id: 11,
        name: "Basic Engine Core"
    },
    graphene_engine_core: {
        id: 12,
        name: "Graphene Engine Core"
    }
};

const recipes = {
    steel:                      {},
    aluminium:                  {},
    diamond:                    {},
    silicon:                    {},

    graphene:                   { diamond: 2 },
    electronics:                { silicon: 4 },

    basic_hull:                 { steel: 1000,      aluminium: 50 },
    graphene_hull:              { graphene: 1000,   aluminium: 50 },
    basic_scanner:              { steel: 10,        electronics: 50 },
    basic_shield_generator:     { aluminium: 100,   electronics: 150 },
    basic_engine_core:          { steel: 500,       aluminium: 50,    electronics: 50 },
    graphene_engine_core:       { graphene: 500,    aluminium: 50,    electronics: 50 }

};

const supply = {
    steel:      1000000,
    aluminium:  1000000,
    diamond:    1000000,
    silicon:    1000000,
};

//module.exports = { resources: resources, recipes: recipes, supply: supply };


var resAccount = 'sfeos.res';
var options = {
  authorization: resAccount + '@active',
  broadcast: true,
  sign: true
}

function init() {
    return loadResources()
        .then(() => loadAllRecipes())
        .then(() => loadInitialSupply());
}
module.exports.init = init;


function loadResources() {
    var all = [];
    for(var key in resources) {
        var res = resources[key];
        all.push(createResource(res.id, res.name));
    }

    return Promise.all(all);
}

function createResource(id, name) {
    return contract.get(resAccount).addresource(id,  name, options);
}

function loadAllRecipes() {
    var all = [];
    for(var key in recipes) {
        var recipe = recipes[key];
        var resourceId = resources[key].id;

        for(var ingKey in recipe) {
            var ingredientId = resources[ingKey].id;
            var quantity = recipe[ingKey];

            all.push( contract.get(resAccount).addingr(resourceId, ingredientId, quantity, options) );
        }
    }

    return Promise.all(all);
}

function loadInitialSupply() {
  var all = [];

  for(var key in supply) {
      var resourceId = resources[key].id;
      var quantity = supply[key];

      all.push( contract.get(resAccount).mint(resourceId, quantity, options) );
  }

  return Promise.all(all);
}