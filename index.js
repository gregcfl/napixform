const jtransformer = require('./build/Debug/jtransformer.node');

const tmplate = JSON.stringify(require("./trip-template").tripTemplateBase)
const inputData = JSON.stringify(require("./trip-template").inputData);
const functs = require("./trip-template").xfuncts;

const xfmed = jtransformer.Convert(tmplate, inputData);
console.log('After Convert : \n', xfmed);
console.log(functs.zipper("L:", ":R"));


module.exports = jtransformer;
