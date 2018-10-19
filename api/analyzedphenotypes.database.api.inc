<?php
/**
 * @file
 *
 * API: CHADO DATABASE
 * Manage property and operation pertaining to data in chado.db.
 */



/**
 * DATA SOURCE
 * Fetch all records in chado.db as an associative array
 * where the key is database id number and value is an array containing database name and description.
 * Records sorted by name alphabetically.
 *
 * @return
 *   An array, containing database information records.
 */
function ap_get_database() {
  // Fetch database properties in chado.db, sorted by name.
  $result = chado_query(
    "SELECT db_id, name, description FROM {db} ORDER BY name ASC"
  );

  $database = null;

  if ($result->rowCount()) {
    foreach($result as $db) {
      $database[ $db->db_id ] = array(
        'name' => $db->name,
        'description' => $db->description,
      );
    }
  }


  return ($database) ? $database : null;
}


/**
 * FUNCTION:
 * Match a database record given a database id number.
 *
 * @param $dbid
 *   Database id number corresponding to db_id in chado.db table.
 *
 * @return
 *   An array, containing database id, name and description.
 */
function ap_match_database($dbid) {
  $databse = null;
  
  // Fetch database record that matches database id. 
  $result = chado_query(
    "SELECT db_id, name, description FROM {db} WHERE db_id = :db_id LIMIT 1",
    array(':db_id' => $dbid)
  );

  if ($result->rowCount()) {
    $db = $result->fetchObject();
    
    $database = array(
      'db_id' => $db->db_id,
      'name'  => $db->name,
      'description' => $db->description,
    );
  }
  
  
  return $database;
}