var FoxxApplication = require("@arangodb/foxx").Controller;
var controller = new FoxxApplication(module.context);

controller.get('/test', function (req, res) {
  'use strict';
  res.json(true);
});
