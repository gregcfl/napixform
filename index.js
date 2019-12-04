const jtransformer = require('./build/Release/jtransformer.node');

const tmplate = JSON.stringify(require("./trip-template").tripTemplateBase)
const inputData = JSON.stringify(require("./trip-template").inputData);
const functs = require("./trip-template").xfuncts;

const xfmed = jtransformer.Convert(tmplate, inputData, functs);
console.log('After Convert : \n', xfmed);

module.exports = jtransformer;
