var db = require("internal").db;
var col = module.context.collectionName("setup_teardown");
db._drop(col);
