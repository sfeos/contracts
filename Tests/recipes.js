
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
  electronics:  {
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

module.exports = { resources: resources, recipes: recipes };