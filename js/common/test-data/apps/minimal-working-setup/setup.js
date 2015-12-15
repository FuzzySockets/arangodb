var db = require("internal").db;
var col = module.context.collectionName("setup");
if (db._collection(col)) {
  db._create(col);
}
