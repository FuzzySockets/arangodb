'use strict';
var db = require('@arangodb').db;
var usersName = module.context.collectionName('users');

if (db._collection(usersName) === null) {
  db._create(usersName, {isSystem: true});
}
