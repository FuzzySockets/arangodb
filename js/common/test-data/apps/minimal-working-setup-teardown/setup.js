var db = require("internal").db;
var col = module.context.collectionName("setup_teardown");
if (db._collection(col)) {
  db._create(col);
}

