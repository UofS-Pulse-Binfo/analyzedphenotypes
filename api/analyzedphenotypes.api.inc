<?php
/**
 * @file
 * Contains API required by this module.
 */


/**
 * API: PROJECT/EXPERIMENT PROPERTY
 * Manage property and operation pertaining to data in chado.project table.
 * Default to: return all project as an associative array where key is project_id and value is project name.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - match_key  : Fetch all project records that match a keyword.
 *   - match_name : Fetch project record that matches a project name.
 *   - match_id   : Fetch project record that matches a project id.
 *   - default    : Fetch all project records.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - project_keyword : keywords that form project title or name.
 *   - project_name    : project title or name.
 *
 * @return Array $projectprop
 *   An associative array where the key is project_id and value is project_name;
 *
 * @see analyzedphenotypes_validator_project() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in validator.inc.
 * @see analyzedphenotypes_data_json() in .module.
 */
function analyzedphenotypes_projectprop($property = null, $parameter = null) {
  // Update: used in data downloader form.
  $projectprop = null;

  // Fetch all project/experiment records in chado.project.
  $result = chado_query(
    "SELECT project_id, TRIM(name) AS name FROM {project} ORDER BY name ASC"
  );

  if ($result->rowCount() <= 0) {
    // 0 project in the system.
    return $projectprop;
  }


  switch($property) {
    //
    case 'match_key':
      // Fetch all records in chado.project that match a keyword.
      // In:
      //   .module - analyzedphenotypes_data_json() : create project list in JSON.
      // Parameters:
      //   $property : match_key.
      //   $parameter['project_keyword'] : a string, keyword entered.

      // To ensure list will not grow longer, limit the result to x.
      $projectprop_limitrows = 10;

      foreach($result as $i => $project) {
        if (($i + 1) > $projectprop_limitrows) {
          break;
        }

        if (stripos($project->name, trim($parameter['project_keyword'])) !== FALSE) {
          // Match found.
          // Project has the keyword, save it.
          $projectprop[$project->project_id] = $project->name;
        }
      }

      break;

    //
    case 'match_name':
      // Fetch record in chado.project that matches experiment name or title.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_save() : Stage #4 - Save Data.
      //   .validators.inc - analyzedphenotypes_validator_project() : Validate project exists.
      // Parameters:
      //   $property : match_name.
      //   $parameter['project_name'] : a string, project name or title.

      foreach($result as $project) {
        if (strtolower($project->name) == strtolower(trim($parameter['project_name']))) {
          // Match found.
          $projectprop = array(
            'project_id' => $project->project_id,
            'project_name' => $project->name,
          );

          break;
        }
      }

      break;

    //
    case 'match_id':
      // Fetch record in chado.project that matches experiment name or title.
      // In:
      //
      // Parameters:
      //   $property : match_id.
      //   $parameter['project_id'] : integer, project id number.

      foreach($result as $project) {
        if ($project->project_id == trim($parameter['project_id'])) {
          // Match found
          $projectprop = array(
            'project_id' => $project->project_id,
            'project_name' => $project->name,
          );

          break;
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      // Fetch all project records in chado.project table.
      // In:
      //
      // Parameters:
      //   $property : null.
      //   $parameter : null.
      $projectprop = $result->fetchAllKeyed();
  }


  return $projectprop;
}


/**
 * API: GENUS PROPERTY
 * Manage property and operation pertaining to Genus in chado.organism.
 * Default to: return all genus as an array.
 * Dependencies: analyzedphenotypes_systemvariables().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - project_genus  : Fetch the genus a project or experiment is specific to.
 *   - assign_genus   : Assign a genus to a project.
 *   - default        : Fetch all Genus records.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - project_id     : project id number
 *   - project_name   : project title or name.
 *   - project_genus  : genus associated to a project.
 *
 * @return $genusprop
 *   Genus as string or list of genus as an array.
 *
 * @see analyzedphenotypes_loader_form_upload() in admin.form.inc.
 * @see analyzedphenotypes_validator_genus() in validator.inc.
 * @see analyzedphenotypes_save_tsv_data() in validator.inc.
 */
function analyzedphenotypes_genusprop($property = null, $parameter = null) {
  // update: used in data downloader form.
  $genusprop = null;


  switch($property) {
    //
    case 'project_genus':
      // Fetch associated genus of a project.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_upload() : Stage #1 - Select file and upload file.
      // Parameters:
      //  $property : project_genus.
      //  $parameter['project_id'] : project id number.
      //  $parameter['project_name'] : project name or title.

      // System variable: genus.
      $sysvars = analyzedphenotypes_systemvariables('terms');
      $sysvar_genus = variable_get($sysvars['ap_genus']);
      unset($sysvars);

      if (isset($parameter['project_id'])) {
        $field = 't1.project_id';
        $value = 'project_id';
      }
      else {
        $field = 't1.name';
        $value = 'project_name';
      }

      // Fetch projectprop.value of type (type_id) Genus for a project id or project name.
      $sql = sprintf(
        "SELECT t2.value FROM {project} AS t1 INNER JOIN {projectprop} AS t2 USING (project_id)
        WHERE t2.type_id = :type_id AND %s = :value LIMIT 1", $field
      );

      $result = chado_query($sql,
        array(
          ':type_id' => $sysvar_genus,
          ':value' => $parameter[$value],
        )
      );

      $genusprop = ($result->rowCount() == 1) ? $result->fetchField() : $genusprop;

      break;

    //
    case 'assign_genus':
      // Assign a genus to project.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_save() : Stage #4 - Save data.
      // Parameters:
      //  $property : assign_genus.
      //  $parameter['project_id'] : project id number.
      //  $parameter['project_genus'] : genus selected.

      // System variable: genus.
      $sysvars = analyzedphenotypes_systemvariables('terms');
      $sysvar_genus = variable_get($sysvars['ap_genus']);
      unset($sysvars);

      $values = array(
        'project_id' => $parameter['project_id'],
        'type_id' => $sysvar_genus,
        'value' => $parameter['project_genus'],
      );

      chado_insert_record('projectprop', $values);

      break;

    //
    case 'sysvar_genus_set':
      // Fetch all genus with controlled vocabulary (cv) and database (db) configuration
      // variables set. This will limit genus suggested in select genus field to only
      // genus properly configured.
      // In:
      //   .admin.form.inc -analyzedphenotypes_loader_form_upload() : Stage #1 - Select project and upload file.
      // Parameters:
      //  $property : sysvar_genus_set.
      //  $parameter : null.

      // System variable: Genus cv, db and ontology.
      $sysvars = analyzedphenotypes_systemvariables('cvdbon');

      $result = chado_query(
        "SELECT LOWER(genus) FROM {organism} GROUP BY genus ORDER BY genus ASC"
      )
      ->fetchCol();

      $genus_set = array();

      foreach($result as $g) {
        $sysvar_key = 'ap_' . str_replace(' ', '_', $g);

        // Genus cv and db settings.
        $sysvar_genus_cv = variable_get($sysvars[$sysvar_key]['cv']);
        $sysvar_genus_db = variable_get($sysvars[$sysvar_key]['db']);

        if ($sysvar_genus_cv > 0 && $sysvar_genus_db > 0) {
          // Save those with cv and db set.
          $genus_set[] = ucfirst($g);
        }
      }

      $genusprop = (count($genus_set)) ? $genus_set : 0;

      break;

    //
    // Define additional case here.

    //
    default:
      // Fetch all unique genus in chado.organism table.
      // In:
      //
      // Parameters:
      //  $property : null.
      //  $parameter : null.

      $result = chado_query(
        "SELECT genus FROM {organism} GROUP BY genus ORDER BY genus ASC"
      );

      $genusprop = ($result->rowCount() > 0) ? $result->fetchCol() : $genusprop;
  }


  return $genusprop;
}


/**
 * API: STOCKS PROPERTY
 * Manage property and operation pertaining to Stocks in chado.stock.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - match_name : Fetch the stock given stock name and uniquename.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - uniquename
 *   - name
 *
 * @return String $stockprop
 *   Stock id number.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_stockprop($property, $parameter = null) {
  $stockprop = null;

  switch($property) {
    //
    case 'match_name':
      // Match a stock/germplasm given name and accession combination.
      // In:
      //   .validators.inc - analyzedphenotypes_validator_value() : validate data cell value.
      // Parameters:
      //  $property : match_name.
      //  $parameter['name'] : stock name.
      //  $parameter['uniquename'] : stock unique name.
      $result = chado_query(
        "SELECT stock_id FROM {stock} WHERE name = :name AND uniquename = :uniquename LIMIT 1",
        array(
          ':name' => $parameter['name'],
          ':uniquename' => $parameter['uniquename'],
        )
      );

      $stockprop = ($result) ? $result->fetchField() : $stockprop;

      break;

    //
    // Define additional case here.

    // Default:
  }


  return $stockprop;
}


/**
 * API: ONTOLOGY PROPERTY
 * Manage property and operation pertaining to ontology.
 * Default to:
 * Dependencies: analyzedphenotypes_systemvariables().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - map_ontology_trait : Map the ontology to trait in describe trait form (stage 3).
 *   - suggest            : Suggest ontology in ontology field in describe trait form (stage 3) based on project genus and keywords in the trait.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - ontology_cvterm_id : Cvterm id number as ontology term.
 *   - trait_cvterm_id    : Cvterm id number as trait term.
 *   - project_genus      : Project genus.
 *   - trait_name         : Human-readable trait name.
 *   - ontology           : Ontology/cvterm name.
 *   - genus              : Project genus.
 *
 * @return Array $ontologyprop
 *   Array of ontology properties.
 *
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 */
function analyzedphenotypes_ontologyprop($property = null, $parameter = null) {
  $ontologyprop = null;


  switch($property) {
    //
    case 'map_ontology_trait':
      // Relate a trait to an ontology in cvterm_relationship table.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_save() : Stage #4 - Save data.
      // Parameters:
      //  $property : map_ontology_trait.
      //  $parameter['trait_cvterm_id'] : trait cvterm_id from chado.cvterm.
      //  $parameter['ontology_cvterm_id'] : ontology term cvterm_id.

      // System variables: related.
      $sysvars = analyzedphenotypes_systemvariables('terms');
      $sysvar_related = variable_get($sysvars['ap_related']);
      unset($sysvars);

      $values = array(
        'type_id' => $sysvar_related,
        'object_id' => $parameter['trait_cvterm_id'],
        'subject_id' => $parameter['ontology_cvterm_id'],
      );

      chado_insert_record('cvterm_relationship', $values);

      break;

    //
    case 'suggest':
      // Suggest ontology terms that relates to a trait name (using words in the trait as keywords).
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_describe() : Stage #3 - Describe trait names.
      // Parameters:
      //  $property : suggest.
      //  $parameter['trait_name'] : trait name.

      // To ensure list will not grow longer, limit the result to x.
      $ontologyprop_limitrows = 10;

      // System variables: Genus ontology.
      $sysvars = analyzedphenotypes_systemvariables('genus_vars', array(
        'genus' => $parameter['genus'],
      ));

      $sysvar_genus_ontology = variable_get($sysvars['ontology']);
      unset($sysvars);

      if ($sysvar_genus_ontology) {
        // The trait. No unit pls.
        $trait_name = strtolower(trim($parameter['trait_name']));

        $ontology_terms = analyzedphenotypes_cvprop('terms_in_cv', array(
          'cv_id' => $sysvar_genus_ontology,
        ));

        $ctr = 0;
        foreach($ontology_terms as $i => $term) {
          // Use each word in the trait name as keyword to search for ontology terms.
          $ontology = strtolower(trim($term['name']));
          $keywords = explode(' ', $ontology);

          foreach($keywords as $key) {
            if ($ctr >= $ontologyprop_limitrows) {
              break;
            }
            else {
              if (strlen($key) > 2 && stristr($trait_name, $key)) {
                $ontologyprop[$i] = $term;

                $ctr++;
              }
            }
          }
        }
      }

      break;

    //
    case 'match_ontology':
      // Fetch ontology information from chado.cvterm.
      // In:
      //   .admin.form.inc - analyzedphenotypes_validate_field_required() : hook_validate().
      // Parameters:
      //  $property : match_ontology.
      //  $parameter['genus'] : project genus.
      //  $parameter['ontology'] : cv name (ontology).

      // System variables: Genus ontology.
      $sysvars = analyzedphenotypes_systemvariables('genus_vars', array(
        'genus' => $parameter['genus'],
      ));
      $sysvar_genus_ontology = variable_get($sysvars['ontology']);
      unset($sysvars);

      $cvtermprop = analyzedphenotypes_cvprop('get_cvterm', array(
        'name' => $parameter['ontology'],
        'cv_id' => $sysvar_genus_ontology,
      ));

      if ($cvtermprop) {
        $ontologyprop[$cvtermprop['cvterm_id']] = $cvtermprop['name'];
      }

      break;

    //
    case 'download_type':
      $datafileprop = array(
        'tsv' => '.TSV - Tab Separated Values',
        'csv' => '.CSV - Comma Separated Values',
        'xlsx'=> '.XLSX -Microsoft Excel Spreadsheet File',
      );

      break;

    //
    // Define additional case here.

    // Default:
  }


  return $ontologyprop;
}


/**
 * API: CONTROLLED VOCABULARIES
 * Manage property and operation pertaining to controlled vocabularies in chado.cv and chado.cvterm.
 * Default to: Return all controlled vocabularies in chado.cv.
 * Dependencies: analyzedphenotypes_systemvariables(), analyzedphenotypes_scale_photo(), analyzedphenotypes_dbprop().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - match_key         : Match a record in chado.cvterm given a keyword.
 *   - match_cvid        : Match a record in chado.cv given a cv_id.
 *   - terms_in_cv       : Fetch all records in chado.cvterm given a cv_id.
 *   - get_cvterm        : Fetch records in chado.cvterm, chado.dbxref and chado.cvterm_relationship (term properties) given a trait.
 *   - insert_cvterm     : Insert a record into chado.cvterm.
 *   - cvterm_save_photo : Handle saving of photo to a trait/cvterm.
 *   - cvterm_get_photo  : Get photo information.
 *   - suggest_term      : Suggest cvterm name with the cv name it is in.
 *   - term_cv           : Given a string containing cvterm name and cv name (eg. Genus (taxonomic_rank)) in fields, return the cvterm id.
 *   - term_id           : Given a cvterm id number return a string value containing the cvterm name and cv name in cvterm name (cv name) format.
 *   - default           : All records in chado.cv.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - cvterm_keyword
 *   - cv_id
 *   - cvterm_id
 *   - name
 *   - photo
 *   - term_cv
 *   - project_genus
 *
 * @return $cvprop
 *   Array of term properties.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_cvprop($property = null, $parameter = null) {
  $cvprop = null;

  switch($property) {
    //
    case 'match_key':
      // Fetch terms that match keywords.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_describe() : Stage #3 - Describe trait.
      // Parameters:
      //  $property : match_key.
      //  $parameter['cvterm_keyword'] : string, cvterm name keyword.

      // System variables: Genus cv.
      $sysvars = analyzedphenotypes_systemvariables('genus_vars', array(
        'genus' => $parameter['genus'],
      ));
      $sysvar_genus_cv = variable_get($sysvars['cv']);
      unset($sysvars);

      // Find a cvterm that matches a keyword.
      $result = chado_query(
        "SELECT cvterm_id, name FROM {cvterm} WHERE cv_id = :cv_id AND LOWER(name) LIKE :cvterm_key ORDER BY name ASC",
        array(
          'cv_id' => $sysvar_genus_cv,
          'cvterm_key' => '%' . strtolower(trim($parameter['cvterm_keyword'])) . '%',
        )
      );

      if ($result->rowCount() > 0) {
        $cvprop = $result->fetchAllKeyed();
      }

      break;

    case 'match_cvid':
      // Match cv term given an cv id number from chado.cv table.
      // In:
      //
      // Parameters:
      //  $property : match_cvid.
      //  $parameter['cv_id'] : controlled vocabulary id number.

      $cv = tripal_get_cv(array(
        'cv_id' => $parameter['cv_id'],
      ));

      if ($cv) {
        $cvprop = array(
          'cv_id' => $cv->cv_id,
          'name' => $cv->name,
          'definition' => $cv->definition,
        );
      }

      break;

    //
    case 'terms_in_cv':
      // Fetch all cvterms in a controlled vocabulary (cv).
      // In:
      //   .module - analyzedphenotypes_data_json() : Create list of terms in JSON format.
      // Parameters:
      //  $property : term_in_cv.
      //  $parameter['cv_id'] : controlled vocabulary id number.

      $terms = chado_query(
        "SELECT cvterm_id, name, definition FROM {cvterm} WHERE cv_id = :cv_id ORDER BY name ASC",
        array(':cv_id' => $parameter['cv_id'])
      );

      if ($terms->rowCount() > 0) {
        foreach($terms as $term) {
          $cvprop[$term->cvterm_id] = array(
            'name' => $term->name,
            'definition' => $term->definition,
          );
        }
      }

      break;

    case 'rfriendly':
      // These rules/pattern are from rawphenotypes.
      $word_rules = 'of,to,have,on,at';
      $char_rules = '(,),/,-,:,;,%';
      $replace_rules = '# = num,/ = div,? = unsure,- = to';

      $arr_match = array();
      $arr_replace = array();
      $r = explode(',', $replace_rules);

      // Convert the rule to key and value pair. The key is the matching character/word and
      // the values is the replacement value when key is found in a string.
      foreach($r as $g) {
        list($match, $replace) = explode('=', $g);
        $arr_match[] = trim($match);
        $arr_replace[] = trim($replace);
      }

      // Convert special characters transformation rules in string to array.
      $char_rules = explode(',', $char_rules);

      // Convert words transformation rules in string to array.
      $word_rules = explode(',', $word_rules);

      // Remove leading and trailing spaces from the selected trait.
      // Convert string to lowercase.
      $selected_trait = trim(strtolower($parameter['column_header']));
      // 1. Break the column header in string to individual words,
      //    and remove all words that matches an entry in the words transfomation rules.
      $w = explode(' ', $selected_trait);
      foreach($w as $c) {
        $c = trim($c);

        // Skip the words in the traits that are present in the
        // words transformation rules.
        if (!in_array($c, $word_rules)) {
          // Do match and replace, as well as, removal of special characters
          // only when the current word is not in the words transformation rules.
          // 2. Match and replace based on match and replace rule.
          $c = str_replace($arr_match, $arr_replace, $c);
          // 3. Remove all special characters listed in remove chars rule.
          $c = str_replace($char_rules, '', $c);

          // All transformation rules applied, make sure that
          // the result is not a blank space.
          if (!empty($c)) {
            $rfriendly[] = trim($c);
          }
        }
      }

      // Final transformation is replacing all spaces to dots/period (.)
      $columnsprop = ucfirst(implode('.', $rfriendly));

      break;

    //
    case 'get_cvterm':
      // Fetch a term given a cvterm id number.
      // In:
      //
      // Parameters:
      //  $property : get_cvterm.
      //  $parameter['genus'] : project genus.
      //  $parameter['cv_id'] : controlled vocabulary id number.

      $term = array();

      if (isset($parameter['cvterm_id'])) {
        // Global serach.
        $index = 'cvterm_id';
      }
      else {
        // Term in controlled vocabulary.
        if (isset($parameter['cv_id'])) {
          $term['cv_id'] = $parameter['cv_id'];
        }
        else {
          // System variables: Genus cv.
          $sysvars = analyzedphenotypes_systemvariables('genus_vars', array(
            'genus' => $parameter['genus'],
          ));

          $sysvar_genus_cv = variable_get($sysvars['cv']);
          unset($sysvars);

          $term['cv_id'] = $sysvar_genus_cv;
        }

        $index = 'name';
      }

      $term[$index] = $parameter[$index];
      if (function_exists('chado_get_cvterm')) {
        $cvterm = chado_get_cvterm($term);
      }
      else {
        $cvterm = tripal_get_cvterm($term);
      }

      $term[$index] = $parameter[$index];

      $cvterm = tripal_get_cvterm($term);
      $definition = (isset($cvterm->definition)) ? $cvterm->definition : '';

      if ($cvterm) {
        // Term found. Get other info.
        $cvprop = array(
          'cvterm_id'  => $cvterm->cvterm_id,
          'dbxref_id'  => $cvterm->dbxref_id,
          // An object.
          'cv_id'      => $cvterm->cv_id->cv_id,
          'cv_name'    => $cvterm->cv_id->name,
          'name'       => $cvterm->name,
          'definition' => $definition,
          'photo_1'    => $cvterm->cvterm_id . '_1.jpg',
          'photo_2'    => $cvterm->cvterm_id . '_2.jpg',
        );

        // Databse.
        $result = chado_query(
          "SELECT db_id FROM {dbxref} WHERE dbxref_id = :dbxref_id LIMIT 1",
          array(':dbxref_id' => $cvprop['dbxref_id'])
        );

        if ($result->rowCount() == 1) {
          $db_id = $result->fetchField();
          $cvprop['db_id'] = $db_id;
        }
        else {
          $cvprop['db_id'] = null;
        }

        // Crop ontology.
        unset($result);
        // System variables: related.
        $sysvars = analyzedphenotypes_systemvariables('terms');
        $sysvar_related = variable_get($sysvars['ap_related']);
        unset($sysvars);

        // Fetch crop ontology of trait in chado.cvterm_relationship.
        $result = chado_query(
          "SELECT name FROM {cvterm} WHERE cvterm_id = (SELECT subject_id FROM {cvterm_relationship} WHERE type_id = :related AND object_id = :cvterm_id LIMIT 1)",
          array(
            ':related' => $sysvar_related,
            ':cvterm_id' => $cvprop['cvterm_id'],
          )
        );

        $cvprop['crop_ontology'] = ($result->rowCount() == 1) ? $result->fetchField() : null;

        // Scale.
        $cvprop['scale'] = null;
      }

      break;

    //
    case 'insert_cvterm':
      // Insert/add controlled vocabulary term.
      // In:
      //   admin.form.inc - analyzedphenotypes_loader_fom_save() : Stage #4 - Save data.
      // Parameters:
      //  $property : insert_cvterm.
      //  $parameter['genus'] : project genus.
      //  $parameter['name'] : cvterm term name/title.
      //  $parameter['definition'] : cvterm name definition.

      // System variable: Genus db.
      $sysvars = analyzedphenotypes_systemvariables('genus_vars', array(
        'genus' => $parameter['genus'],
      ));

      $sysvar_genus_db = variable_get($sysvars['db']);
      $dbprop = analyzedphenotypes_dbprop('match_db', array(
        'db_id' => $sysvar_genus_db,
      ));
      $db_name = $dbprop['name'];

      // System variable: Genus cv.
      $sysvar_genus_cv = variable_get($sysvars['cv']);
      $cvprop = analyzedphenotypes_cvprop('match_cvid', array(
        'cv_id' => $sysvar_genus_cv,
      ));
      $cv_name = $cvprop['name'];

      $cvterm = tripal_insert_cvterm(array(
        'id' => trim($db_name) . ':' . $parameter['name'],
        'name' => $parameter['name'],
        'cv_name'  => $cv_name,
        'definition' => $parameter['definition'],
      ));

      if ($cvterm) {
        $cvprop = $cvterm->cvterm_id;
      }

      break;

    //
    case 'cvterm_save_photo':
      // Save photo/image of controlled vocabulary term.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_save() : Stage #4 - Save data.
      // Parameters:
      //  $property : cvterm_save_photo.
      //  $parameter['cvterm_id'] : controlled vocabulary cvterm id number.

      $cvterm_id = $parameter['cvterm_id'];

      foreach($parameter['photo'] as $i => $photo) {
        $tmp_file = file_save_upload($photo, array(), FALSE, FILE_EXISTS_REPLACE);

        if ($tmp_file) {
          $file_obj = file_load($tmp_file->fid);

          $source_file = drupal_realpath($file_obj->uri);
          $info = image_get_info($source_file);

          $scale = analyzedphenotypes_scale_photo(array(
            'height' => $info['height'],
            'width' => $info['width'],
            'scale_to' => 400,
          ));

          $photo_dir = 'public://ap-photo';
          file_prepare_directory($photo_dir, FILE_CREATE_DIRECTORY);

          $new_image = imagecreatetruecolor($scale['width'], $scale['height']);
          $img_file  = imagecreatefromjpeg($source_file);
          imagecopyresampled($new_image, $img_file, 0, 0, 0, 0, $scale['width'], $scale['height'], $info['width'], $info['height']);

          $new_filename = $cvterm_id . '_' . ($i + 1) . '.jpg';
          imagejpeg($new_image, drupal_realpath('public://ap-photo') . '/' . $new_filename, 100);

          imagedestroy($img_file);
          imagedestroy($new_image);

          $tmp_file->filename = $new_filename;
          $tmp_file->uri = 'public://ap-photo/' . $new_filename;
          $tmp_file->status = FILE_STATUS_PERMANENT;
          file_save($tmp_file);
          file_usage_add($tmp_file, 'analyzedphenotypes', 'file', 1);
        }
      }

      break;

    //
    case 'cvterm_get_photo':
      // Get photo saved for a controlled vocabulary term.
      // In:
      //   .admin.form.inc - analyzedphenotypes_loader_form_describe() : Stage #3 - Describe trait.
      // Parameters:
      //  $property : cvterm_get_photo.
      //  $parameter['cvterm_id'] : controlled vocabulary term.

      $photo = array('ap_photo_1', 'ap_photo_2');

      foreach($photo as $i => $v) {
        $filename = $parameter['cvterm_id'] . '_' . ($i + 1) . '.jpg';
        $filedir = 'public://ap-photo';

        $fileuri = drupal_realpath($filedir) . '/' . $filename;

        if (file_exists($fileuri)) {
          $info = image_get_info($fileuri);

          $cvprop[$v] = array(
            'filename' => $filename,
            'uri' => $filedir . '/' . $filename,
            'height' => $info['height'],
            'width' => $info['width'],
          );
        }
        else {
          $cvprop[$v] = null;
        }
      }

      break;

    //
    case 'suggest_term':
      // Suggest a cvterm in cvterm term name (cv name) format.
      // In:
      //   .module - analyzedphenotypes_data_json()
      // Parameters:
      //  $property : suggest_term.
      //  $parameter['keyword'] : string, keyword.

      $result = chado_query(
        "SELECT TRIM(t2.name), TRIM(t1.name) AS cv_name FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING (cv_id)
        WHERE LOWER(t2.name) LIKE LOWER(:keyword) ORDER BY t1.name LIMIT 20",
        array(
          ':keyword' => '%' . $parameter['keyword'] . '%',
        )
      );

      if ($result->rowCount() > 0) {
        $cvprop = $result->fetchAllKeyed();
      }

      break;

    //
    case 'term_cv':
      // Fetch cvterm_id of controlled vocabulary term in cvterm name (cv name) format.
      // In:
      //   .admin.form.inc - system_settings_form_validate() : check to see if term exists.
      //   .admin.form.inc - system_settings_form_config_submit() : fetch cvterm id of cvterm name.
      // Parameters:
      //  $property : term_cv.
      //  $parameter['term_cv'] : term name in term name (cv name) format.

      $result = chado_query(
        "SELECT cvterm_id FROM {cvterm} AS t1 INNER JOIN {cv} AS t2 USING(cv_id)
        WHERE CONCAT(t1.name, ' (' , t2.name , ')') = :term_cv LIMIT 1",
        array(
          ':term_cv' => $parameter['term_cv']
        )
      );

      if ($result->rowCount() > 0) {
        $cvprop = $result->fetchField();
      }

      break;

    //
    case 'term_id':
      // Fetch cvterm name and cv name given a cvterm_id number.
      // Im:
      //   .admin.form.inc - analyzedphenotypes_admin_settings() : configuration page.
      // Parameters:
      //  $property : term_id.
      //  $parameter['term_id'] : cvterm id number.

      $result = chado_query(
        "SELECT t1.name || ' (' || t2.name || ')' AS term FROM {cvterm} AS t1 INNER JOIN {cv} AS t2
        USING(cv_id) WHERE t1.cvterm_id = :term_id LIMIT 1",
        array(
          ':term_id' => $parameter['term_id'],
        )
      );

      if ($result->rowCount() > 0) {
        $cvprop = $result->fetchField();
      }

      break;

    //
    // Define additional case here.

    //
    default:
      // Return all cvs in chado.cv table.
      // In:
      //
      // Parameters:
      //  $property : null.
      //  $parameter: null.
      $cvs = chado_query(
        "SELECT cv_id, name, definition FROM {cv} ORDER BY name ASC"
      );

      if ($cvs->rowCount() > 0) {
        foreach($cvs as $cv) {
          $cvprop[$cv->cv_id] = array(
            'name' => $cv->name,
            'definition' => $cv->definition,
          );
        }
      }
  }


  return $cvprop;
}


///////


/**
 * API: DATABASE PROPERTY
 * Manage property and operation pertaining to database in chado.db table.
 * Default to: Return all databases in chado.db table.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - match_db  : Match a record in chado.db given a database id.
 *   - default   : All records in chado.db table.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - db_id
 *
 * @return Array $dbprop
 *   Array of database properties.
 *
 * @see analyzedphenotypes_admin_settings() in admin.form.inc.
 */
function analyzedphenotypes_dbprop($property = null, $parameter = null) {
  $dbprop = null;

  // Fetch all databases in chado.db table.
  $dbs = chado_query(
    "SELECT db_id, name, description FROM {db} ORDER BY name ASC"
  );

  if ($dbs->rowCount() <= 0) {
    // 0 Database in the system.
    return $dbprop;
  }

  switch($property) {
    //
    case 'match_db':
      // Fetch database information given a database id.
      // In:
      //   .api.inc - analyzedphenotypes_cvprop() : fetch database id.
      // Parameters:
      //  $property : match_db.
      //  $parameter['db_id'] : database id number in chado.db.

      foreach($dbs as $db) {
        if ($db->db_id == $parameter['db_id']) {
          $dbprop = array(
            'db_id' => $db->db_id,
            'name' => $db->name,
            'description' => $db->description,
          );

          break;
        }
      }

      break;

    //
    // Define additional case here.

    //
    default:
      // Fetach all databse information in chado.db.
      // In:
      //   admin.form.inc - analyzedphenotypes_admin_settings() : configuration page.
      // Parameters:
      //   $property : null.
      //   $parameter : null.

      foreach($dbs as $db) {
        $dbprop[$db->db_id] = array(
          'name' => $db->name,
          'description' => $db->description,
        );
      }
  }

  return $dbprop;
}


/**
 * API: COLUMN HEADERS
 * Manage property and operation pertaining to column headers.
 * Default to: Return expected column headers.
 * Dependencies: analyzedphenotypes_systemvariables().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - combination       : Return a subset of expected column headers.
 *   - match_combination : Check if a given subset/combination matches a record in the database.
 *   - default           : Return all expected column headers and data type.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - data_file
 *
 * @return $datafileprop
 *   Contents fo file as Text stream or Array of file extensions/columns.
 *
 * @see analyzedphenotypes_dragdrop() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_datafile() in validators.inc.
 * @see analyzedphenotypes_validator_tsv_data() in validators.inc.
 */
function analyzedphenotypes_columnsprop($property = null, $parameter = null) {
  $columnsprop = null;

  // Trait name, data type, index number
  $columnsprop_expectedcols = array(
    'Trait Name'          => 'trait',        // #0
    'Germplasm Accession' => 'alphanumeric', // #1
    'Germplasm Name'      => 'alphanumeric', // #2
    'Year'                => 'year',         // #3
    'Location'            => 'text',         // #4
    'Replicate'           => 'number',       // #5
    'Value'               => '',             // #6
    'Data Collector'      => 'text',         // #7
  );

  switch($property) {
    //
    case 'combination':
      $cols = array_keys($columnsprop_expectedcols);

      $columnsprop = array(
        $cols[0],  // Trait Name.
        $cols[1],  // Germplasm Accession.
        $cols[3],  // Year.
        $cols[4],  // Location.
        $cols[5],  // Replicate.
      );

      break;

    //
    case 'match_combination':
      $cols = array_keys($columnsprop_expectedcols);

      // All system variables term.
      $vars = analyzedphenotypes_systemvariables('terms');
      $arr_plantprop = array();

      foreach($vars as $i => $var) {
        $property = trim(str_replace('ap_', '', $i));
        $var_value = variable_get($var);

        // term (eg. location, replicate) = variable value.
        $arr_plantprop[$property] = $var_value;
      }

      $result = chado_query(
        "SELECT phenotype.phenotype_id
         FROM
           {phenotype} AS phenotype
           LEFT JOIN {cvterm} AS trait_name ON phenotype.attr_id = trait_name.cvterm_id
           LEFT JOIN {stock} AS germplasm_accession USING (stock_id)
           LEFT JOIN {phenotypeprop} AS year USING (phenotype_id)
           LEFT JOIN {phenotypeprop} AS location USING (phenotype_id)
           LEFT JOIN {phenotypeprop} AS replicate USING (phenotype_id)
         WHERE
           year.type_id = :year_cvterm
           AND location.type_id = :location_cvterm
           AND replicate.type_id = :replicate_cvterm
           AND trait_name.name || germplasm_accession.uniquename || year.value || location.value || replicate.value = :combination"
        ,
        array(
          ':year_cvterm'      => $arr_plantprop['year'],
          ':location_cvterm'  => $arr_plantprop['location'],
          ':replicate_cvterm' => $arr_plantprop['replicate'],
          // Trait name, Germplasm Accession, Year, Location and Replicate.
          ':combination'      => $parameter['combination'][$cols[0]] . $parameter['combination'][$cols[1]] . $parameter['combination'][$cols[3]] . $parameter['combination'][$cols[4]] . $parameter['combination'][$cols[5]],
        )
      );

      $columnsprop = ($result->rowCount() > 0) ? $result->fetchField() : $columnsprop;

      break;

    //
    // Define additional case here.

    //
    default:
      $columnsprop = $columnsprop_expectedcols;
  }


  return $columnsprop;
}


/**
 * API: TRAIT NAME
 * Manage property and operation pertaining to trait name.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - no_unit : Exclude unit from a trait name.
 *   - unit    : Extract the unit part from a trait name.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Array keys:
 *   - trait_name
 *
 * @return String $traitprop
 *   Unit or trait name.
 *
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_traitprop($property, $parameter) {
  $traitprop = null;

  switch($property) {
    //
    case 'no_unit':
      $traitprop = preg_replace("/\(.+\)/", '', $parameter['trait_name']);

      break;

    //
    case 'unit':
      preg_match("/.*\(([^)]*)\)/", $parameter['trait_name'], $match);
      $traitprop = (isset($match[1])) ? $match[1] : 'text';

      break;

    //
    // Define additional case here.

    // default.
  }


  return $traitprop;
}


/**
 * API: DATA TYPE
 * Manage property and operation pertaining to data types.
 * Default to:
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   - unit_type       : Define a set of standard units and expected data type of the value.
 *   - alphanumeric    : Letters, number combination.
 *   - trait           : String value strickly in Trait name (unit) format.
 *   - text            : Letters only.
 *   - number          : Numbers + 0.
 *   - number_no_zero  : Numbers > 0.
 *   - scale_1-5       : 1,2,3,4 and 5.
 *   - scale_1-10      : 1 - 10.
 *   - y/n/?           : y, yes, n, no, unsure, ?.
 *   - date            : YYYY-MM-DD.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - value
 *
 * @return Boolean $datatypeprop
 *   Default to TRUE as value passed all conditions and FALSE when proven otherwise.
 *
 * @see analyzedphenotypes_validator_value() in validators.inc.
 */
function analyzedphenotypes_datatypeprop($property, $parameter = null) {
  $datatypeprop = TRUE;

  $value = $parameter['value'];

  switch($property) {
    //
    case 'unit_type':
      // unit, description, data type.
      // Create a case for custom data type that will be used to validate value of such type.
      $datatypeprop = array(
        'g' => array('name' => 'grams', 'type' => 'number'),
        'kg' => array('name' => 'kilograms', 'type' => 'number'),
        'cm'  => array('name' => 'centimeters', 'type' => 'number'),
        'mg'  => array('name' => 'milligrams', 'type' => 'number'),
        'mM'  => array('name' => 'milimolar', 'type' => 'number'),
        'days' => array('name' => 'days', 'type' => 'number_no_zero'),
        'days' => array('name' => 'days', 'type' => 'number_no_zero'),
        'days' => array('name' => 'days', 'type' => 'number_no_zero'),
        'y/n/?' => array('name' => 'Yes (y), No (n) or Unsure (?)', 'type' => 'y/n/?'),
        'count'  => array('name' => 'count', 'type' => 'number'),
        'colour'  => array('name' => 'color', 'type' => 'text'),
        'comments' => array('name' => 'comments', 'type' => 'text'),
        'scale 1-5' => array('name' => 'scale 1-5', 'type' => 'scale_1-5'),
        'scale 1-10' => array('name' => 'scale 1-10', 'type' => 'scale_1-10'),
        'percent' => array('name' => 'percentage', 'type' => 'number'),
        'average count' => array('name' => 'average count', 'type' => 'number'),
      );

      break;

    //
    case 'alphanumeric':
      // Letters, numbers and any 0 or more characters.
      if (preg_match('/[a-z0-9.*]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'trait':
      // Alphanumeric chars followed by ( letters and ) - Trait name (unit).
      if (preg_match('/\A[^()]+\s*\({1}[^)(]+\)\z/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    case 'text':
      // Letters.
      if (preg_match('/[a-z]/i', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number':
      // Numbers including 0.
      if (!is_numeric($value)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'four_digit_year':
      // Numbers, no 0.
      if ($value < 1900 && $value > date('Y')) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'number_no_zero':
      // Numbers, no 0.
      if (preg_match('/[1-9]/', $value) !== 1) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale_1-5':
      $data = (int)$value;
      $min = 1;
      $max = 5;

      if ($value < $min || $value > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'scale_1-10':
      $data = (int)$value;
      $min = 1;
      $max = 10;

      if ($value < $min || $value > $max) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'y/n/?':
      $enum = array('y', 'yes', 'n', 'no', 'unsure', '?');
      if (!in_array($value, $enum)) {
        $datatypeprop = FALSE;
      }

      break;

    //
    case 'date':
      // YYYY-MM-DD format.
      if (preg_match('/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $value, $matches)) {
        $year  = $matches[1];
        $month = $matches[2];
        $day   = $matches[3];

        if ($month >= 1 AND $month <= 12 AND $day >= 1 AND $day <= 31 AND $year > 1900) {
          $today = new DateTime();
          $date = DateTime::createFromFormat('Y-m-d', $value);

          if ($date <= $today) {
            $datatypeprop = FALSE;
          }
        }
      }

      break;

    //
    // Define additional case here.

    // default:
  }


  return $datatypeprop;
}


///////


/**
 * API: USER DATA FILE
 * Manage property and operation pertaining to user data file (tsv or txt).
 * Default to: Return valid file extensions.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - get_contents : Get the tab-delimeted contents of file.
 *   - get_columns  : Return row # 0 or the columns row of file.
 *   - default      : Valid file extensions.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - data_file
 *
 * @return $datafileprop
 *   Contents fo file as Text stream or Array of file extensions/columns.
 *
 * @see analyzedphenotypes_dragdrop() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_describe() in admin.form.inc.
 * @see analyzedphenotypes_validator_datafile() in validators.inc.
 * @see analyzedphenotypes_validator_tsv_data() in validators.inc.
 */
function analyzedphenotypes_datafileprop($property = null, $parameter = null) {
  $datafileprop = null;

  switch($property) {
    //
    case 'get_contents':
      $data_file = $parameter['data_file'];

      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $datafileprop = $datafile_contents;
      }

      break;

    //
    case 'get_columns':
      $data_file = $parameter['data_file'];

      $source = drupal_realpath($data_file->uri);
      $datafile_contents = file_get_contents($source);

      if ($datafile_contents) {
        $rows = explode("\n", $datafile_contents);
        $columns_row = $rows[0];
        unset($rows);

        if (!empty($columns_row)) {
          $datafileprop = str_getcsv($columns_row, "\t");
        }
      }

      break;

    case 'make_permanent':
      $data_file = $parameter['data_file'];

      if ($data_file) {
        $data_file->status = FILE_STATUS_PERMANENT;
        file_save($data_file);

        // Also, point out that we are using it ;-)
        // Note, the file_usage_add() function expects a numerical unique id which we don't have.
        // We have gotten around this by using the uid concatenated with the timestamp using
        // the assumption that a single user cannot upload more than one phenotype file within a second.
        file_usage_add($data_file, 'analyzedphenotypes', 'analyzedphenotypes-file', $data_file->uid . $data_file->timestamp);
      }

      break;

    //
    // Define additional case here.

    //
    default:
      $datafileprop = array(
        'tsv' => 'Tab Separated Values',
        'txt' => 'Text File',
      );
  }


  return $datafileprop;
}


/**
 * API: TRIPAL JOB
 * Manage property and operation pertaining to Tripal Job requests.
 * Default to: Return error limit and report frequency settings.
 * Dependencies:
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - new_job         : Register a new Tripal Job.
 *   - get_job         : Get job details.
 *   - update_progress : Update tripal job progres in tripal_jobs table.
 *   - write_progress  : Write percentage complete value/validation result to a temporary file used by progress bar/validation result window.
 *   - read_progress   : Read values in temporary file by write_progress instance.
 *   - tripal_fail     : Standard tripal error terminal reporting/messaging.
 * @param Array $parameter
 *   An associtive array that forms as arguments to an operation.
 *   Default to null. Array keys:
 *   - description
 *   - callback
 *   - dataset
 *   - job_id
 *   - progress
 *   - source (filenames used to write and read: source=jobprogress for standard progress bar and source=validateresult for validation result)
 *   - message
 *   - param
 * @return $tripaljobprop
 *
 * @see analyzedphenotypes_validate_tsv_file() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in validators.inc.
 * @see analyzedphenotypes_data_json() in .module.
 * @see analyzedphenotypes_file_insert() in admin.form.inc.
 */
function analyzedphenotypes_tripaljobprop($property = null, $parameter = null) {
  $tripaljobprop = null;

  switch($property) {
    //
    case 'new_job':
      global $user;

      $job_id = tripal_add_job(
        $parameter['description'], 'analyzedphenotypes', $parameter['callback'],
        array(serialize($parameter['dataset'])),
        $user->uid
      );

      if ($job_id > 0) {
        $tripaljobprop = $job_id;
      }

      break;

    //
    case 'get_job':
      global $user;

      $job = tripal_get_job($parameter['job_id']);

      if ($job->job_id > 0 && $job->uid == $user->uid && $job->modulename == 'analyzedphenotypes') {
        $tripaljobprop = array(
          'job_id' => $job->job_id,
          'status' => $job->status,
          'uid' => $job->uid,
          'progress' => $job->progress,
        );
      }

      break;

    //
    case 'update_progress':
      $tripaljobprop = dp_update('tripal_jobs')
        ->fields(array('progress' => $parameter['progress']))
        ->condition('job_id', $parameter['job_id'], '=')
        ->execution();

      break;

    //
    case 'write_progress':
      $file = file_directory_temp() . '/ap_' . $parameter['source'] . '_tripaljob_' . $parameter['job_id'] . '.txt';
      $tripaljobprop = file_unmanaged_save_data($parameter['progress'], $file, FILE_EXISTS_REPLACE);

      break;

    //
    case 'read_progress':
      $file = file_directory_temp() . '/ap_' . $parameter['source'] . '_tripaljob_' . $parameter['job_id'] . '.txt';
      $tripaljobprop = file_get_contents($file);

      break;

    //
    case 'tripal_fail':
      $ap = 'analyzedphenotypes';
      $true_print = array('print' => TRUE);

      tripal_report_error($ap, TRIPAL_CRITICAL,
        "\n" . $parameter['message'] . ' [VALUE: @param]' . "\n",
        array('@param' => $parameter['param']),
        $true_print
      );

      tripal_report_error($ap, TRIPAL_CRITICAL,
        'Failed to load phenotypic data in Tripal Job #!job_id.',
        array('!job_id' => $parameter['job_id']),
        $true_print
      );

      break;

    //
    // Additional case here.

    default:
      // Settings used in creating an instance of Tripal Job.
      $tripaljobprop = array(
        'error_limit'       => 5,
        'report_frequency'  => 5,
      );
  }


  return $tripaljobprop;
}


/**
 * API: SYSTEM VARIABLE
 * Manage property and operation pertaining to system variable.
 * Default to: Return variables.
 * Dependencies: analyzedphenotypes_genusprop().
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - db_cv          : Variables to define database and cv to use.
 *   - plant_property : Variables to hold cvterm for plant properties: Location, Replicate, Year and Data Collector.
 *   - genus_ontology : Variables to hold ontology for each genus.
 *   - default        : All variables defined.
 *
 * @return Array $systemvars
 *   Array of variables names.
 *
 * @see analyzedphenotypes_admin_settings() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_save() in admin.form.inc.
 * @see analyzedphenotypes_ap_validators() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in admin.form.inc.
 * @see analyzedphenotypes_cvprop() in api.inc.
 */
function analyzedphenotypes_systemvars($property = null) {
  $systemvars = null;

  $basename = 'analyzedphenotypes_systemvar_';

  $vars_database = array(
    'ap_cv'        => $basename . 'cv',
    'ap_database'  => $basename . 'db',
    'ap_allow_new' => $basename . 'allow_new',
    // cvterm genus.
    'ap_genus'     => $basename . 'genus',
    // cvterm related used in ontology.
    'ap_related'   => $basename . 'related',
    // cvterm taken by eye.
    'ap_method'    => $basename . 'method',
  );

  $vars_plantprop = array(
    // Plant property cvterms.
    'ap_origin'    => $basename . 'origin',
    'ap_year'      => $basename . 'year',
    'ap_location'  => $basename . 'location',
    'ap_replicate' => $basename . 'replicate',
    'ap_collector' => $basename . 'collector',
  );

  $genus = analyzedphenotypes_genusprop();
  $vars_ontology = array();

  // Genus to ontology terms.
  foreach($genus as $g) {
    $g = strtolower(str_replace(' ', '_', $g));
    $vars_ontology['ap_' .  $g] = array(
      'var' => $basename . $g,
      'def' => $basename . '_def_' . $g,
    );
  }

  switch($property) {
    //
    case 'db_cv':
      $systemvars = $vars_database;

      break;

    //
    case 'plant_property':
      $systemvars = $vars_plantprop;

      break;

    //
    case 'genus_ontology':
      $systemvars = $vars_ontology;

      break;

    //
    // Additional case here.

    default:
      $systemvars = $vars_database + $vars_plantprop + $vars_ontology;
  }


  return $systemvars;
}


/**
 * API: SYSTEM VARIABLE
 * Manage property and operation pertaining to system variable.
 * Default to: Return variables.
 * Dependencies: analyzedphenotypes_genusprop().
 *               analyzedphenotypes default ontology API.
 *
 * @param String $property
 *   A value representing a property/operation requested (each property represents a case in the switch implementation):
 *   Default to null.
 *   - options  : Variables for system options (eg. allow new term).
 *   - cvdbon   : Controlled vocabulary, database and otology settings per genus.
 *   - terms    : CV/ontology terms location, replicate, method and etc.
 *   - default  : All variables defined.
 *
 * @return Array $systemvars
 *   Array of variables names.
 *
 * @see analyzedphenotypes_admin_settings() in admin.form.inc.
 * @see analyzedphenotypes_loader_form_save() in admin.form.inc.
 * @see analyzedphenotypes_ap_validators() in validators.inc.
 * @see analyzedphenotypes_save_tsv_data() in admin.form.inc.
 * @see analyzedphenotypes_cvprop() in api.inc.
 * @see hook_install() implementation in install.inc.
 */
function analyzedphenotypes_systemvariables($property = null, $parameter = null) {
  $systemvars = null;
  $basename = 'analyzedphenotypes_systemvar_';

  // System variable types.
  $var_type = array(
    // Allow new header to be added during upload.
    'options' => 'options',
    // Controlled vocabulary (cv), database (db) etc.
    'cvdbon'  => 'cvdbon',
    // Controlled vocabulary terms etc.
    'terms'   => 'terms',
  );

  // System variable names.
  $var_name = array(
    // To register additional system variable, add here.

    $var_type['options'] => array('allownew'),
    $var_type['cvdbon']  => array('cv', 'db', 'ontology'),
    $var_type['terms']   => array('genus', 'year', 'method', 'related', 'location', 'replicate', 'collector')
  );

  // Array to hold all system variables.
  $system_variables = array();

  // Construct system variables.
  // Variables are reference using ap_ . name combination.
  // eg. ap_genus, ap_method etc.
  foreach($var_name as $i => $var) {
    if ($i == $var_type['cvdbon']) {
      // Per genus : controlled vocabulary, database and ontology variables.
      $genus = analyzedphenotypes_genusprop();

      // Construct variables.
      // eg. LENS: cv - analyzedphenotypes_systemvar_lens_cv.
      //           db - analyzedphenotypes_systemvar_lens_db.
      //           ontology - analyzedphenotypes_systemvar_lens_ontology.
      foreach($genus as $g) {
        $g = strtolower(str_replace(' ', '_', $g));

        foreach($var as $v) {
          $system_variables[$i]['ap_' . $g][$v] = $basename . $g . '_' . $v;
        }
      }
    }
    else {
      // Other terms.
      // eg. TERM METHOD: analyzedphenotypes_systemvar_method.
      foreach($var as $v) {
        $system_variables[$i]['ap_' . $v] = $basename . $v;
      }
    }
  }

  // Handle system variables request.
  switch($property) {
    //
    case 'options':
      // Return variables in options category.
      // Allow new ...
      $systemvars = $system_variables[ $var_type['options'] ];

      break;

    //
    case 'cvdbon':
      // Return variables in cv, db and ontology category per genus.
      // lens_cv, lens_db ...
      $systemvars = $system_variables[ $var_type['cvdbon'] ];

      break;

    //
    case 'terms':
      // Return variables in terms category.
      // methods, related, genus ...
      $systemvars = $system_variables[ $var_type['terms'] ];

      break;

    //
    case 'genus_vars':
      // Fetch system configuration for a genus.
      $sysvars = $system_variables[ $var_type['cvdbon'] ];
      // Prepare genus term: remove whitespaces and case.
      $sysvar_key = 'ap_' . trim(strtolower(str_replace(' ', '_', $parameter['genus'])));
      $systemvars = $sysvars[$sysvar_key];

      break;

    //
    // Define additional case here.

    //
    default:
      // All variables.
      $systemvars = $system_variables;
  }


  return $systemvars;
}


///////


/**
 * FUNCTION CALLBACK: VALIDATION RESULT
 * Manage validation result window.
 */
function analyzedphenotypes_validationresult($property, $parameter) {
  switch($property) {
    //
    case 'data':
      $job_id = $parameter;

      $result = analyzedphenotypes_tripaljobprop('read_progress', array(
        'job_id' => $job_id,
        'source' => 'validateresult',
      ));

      $status = json_decode($result, TRUE);
      print theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-data-scope'));

      break;

    //
    case 'file':
      $status = $parameter;

      return theme('analyzedphenotypes_validator_report', array('status' => $status, 'scope' => 'ap-file-scope'));

      break;

    //
    // Define additional case here.

    // default:
  }
}


/**
 * FUNCTION CALLBACK: AJAX CALLBACK
 * Manage AJAX callback.
 */
function analyzedphenotypes_AJAX_callback($form, $form_state) {
  $case = $form_state['triggering_element']['#ajax']['case'];
  $element = 'ap_AJAX_wrapper' . '_' . $case;

  if (isset($form_state['triggering_element']['#ajax']['contain'])) {
    $contain = $form_state['triggering_element']['#ajax']['contain'];

    return $form[$contain][$element];
  }
  else {
    return $form[$element];
  }
}


/**
 * FUNCTION: COMPUTE STANDARD DEVIATION.
 * Credits to: https://www.mathsisfun.com/data/standard-deviation.html.
 * This user-land implementation follows the implementation quite strictly;
 * it does not attempt to improve the code or algorithm in any way. It will
 * raise a warning if you have fewer than 2 values in your array, just like
 * the extension does (although as an E_USER_WARNING, not E_WARNING).
 *
 * @param array $a
 * @param bool $sample [optional] Defaults to false
 * @return float|bool The standard deviation or false on error.
 */
function analyzedphenotypes_stats_standard_deviation(array $a, $sample = false) {
  $n = count($a);

  if ($n === 0) {
      return false;
  }

  if ($sample && $n === 1) {
      return false;
  }

  $mean = array_sum($a) / $n;
  $carry = 0.0;

  foreach ($a as $val) {
    $d = ((double) $val) - $mean;
    $carry += $d * $d;
  };

  if ($sample) {
     --$n;
  }

  return sqrt($carry / $n);
}


/**
 * FUNCTION CONSTRUCT SUMMARY TABLE.
 */
function analyzedphenotypes_table_values($rows, $column_row, $trait) {
  $c = analyzedphenotypes_columnsprop();
  $columns = array_keys($c);

  $index_trait = array_search($columns[0], $column_row);
  $index_value = array_search($columns[6], $column_row);
  $index_year  = array_search($columns[3], $column_row);
  $index_site  = array_search($columns[4], $column_row);

  $arr_siteyear = array();
  $arr_values = array();

  foreach($rows as $i => $line) {
    if ($i > 0 && !empty($line)) {
      $data = str_getcsv($line, "\t");
      $data = array_map('trim', $data);

      if ($data[$index_trait] == $trait) {
        $value = $data[$index_value];
        $is_number = analyzedphenotypes_datatypeprop('number', array(
          'value' => $value
        ));

        if ($is_number) {
          $site_year = $data[$index_site] . ' ' . $data[$index_year];

          if (in_array($site_year, $arr_siteyear)) {
            $arr_values[$site_year][] = $value;
          }
          else {
            $arr_siteyear[] = $site_year;
            $arr_values[$site_year][] = $value;
          }
        }
        else {
          return 0;
        }
      }
    }
  }

  sort($arr_siteyear);
  return array('siteyear' => $arr_siteyear, 'values' => $arr_values);
}


/**
 * FUNCTION SCALE PHOTO.
 */
function analyzedphenotypes_scale_photo($parameter) {
  $h = $parameter['height'];
  $w = $parameter['width'];
  $scale_to = $parameter['scale_to'];

  $ratio = ($w > $h) ? $scale_to / $w : $scale_to / $h;

  $new_w = round($w * $ratio);
  $new_h = round($h * $ratio);

  return array('height' => $new_h, 'width' => $new_w);
}


/**
* FUNCTION LOAD MATERIALIZED VIEW.
*/
function analyzedphenotypes_mview($property, $parameter = null) {
  $mview = null;

  switch($property) {
    case 'view_phenotypes':
      // Default terms location and year to 0 when not indicated.
      $term_id_location = (isset($parameter['term_id_location'])) ? $parameter['term_id_location'] : 0;
      $term_id_year     = (isset($parameter['term_id_year'])) ? $parameter['term_id_year'] : 0;

      $mview_query = sprintf("
        SELECT
          o.genus as organism_genus,
          trait.cvterm_id as trait_id,
          trait.name as trait_name,
          proj.project_id as project_id,
          proj.name as project_name,
          loc.value as location,
          yr.value as year,
          s.stock_id as germplasm_id,
          s.name as germplasm_name,
          avg( CAST(p.value as FLOAT) ) as mean
        FROM {phenotype} p
          LEFT JOIN {cvterm} trait ON trait.cvterm_id=p.attr_id
          LEFT JOIN {project} proj USING(project_id)
          LEFT JOIN {stock} s USING(stock_id)
          LEFT JOIN {organism} o ON o.organism_id=s.organism_id
          LEFT JOIN {phenotypeprop} loc ON loc.phenotype_id=p.phenotype_id AND loc.type_id = %d
          LEFT JOIN {phenotypeprop} yr ON yr.phenotype_id=p.phenotype_id AND yr.type_id = %d
          GROUP BY trait.cvterm_id, trait.name, proj.project_id, proj.name, loc.value, yr.value, s.stock_id, s.name, o.genus
      ", $term_id_location, $term_id_year);

      $mview_schema = array(
        'description' => 'Caches phenotypic data for easier retrieval of means. Data replicates are combined.',
        'table' => 'mview_phenotype',
        'fields' => array (
          'organism_genus' => array (
            'type' => 'varchar',
            'length' => '255',
            'not null' => TRUE,
          ),
          'trait_id' => array (
            'size' => 'big',
            'type' => 'int',
            'not null' => TRUE,
          ),
          'trait_name' => array (
            'type' => 'varchar',
            'length' => '255',
            'not null' => TRUE,
          ),
          'experiment_id' => array (
            'size' => 'big',
            'type' => 'int',
            'not null' => TRUE,
          ),
          'experiment_name' => array (
            'type' => 'text',
            'not null' => TRUE,
          ),
          'location' => array (
            'type' => 'text',
            'not null' => TRUE,
          ),
          'year' => array (
            'type' => 'text',
            'not null' => TRUE,
          ),
          'stock_id' => array (
            'size' => 'big',
            'type' => 'int',
            'not null' => TRUE,
          ),
          'stock_name' => array (
            'type' => 'varchar',
            'length' => '255',
            'not null' => TRUE,
          ),
          'mean' => array(
            'type' => 'float',
          ),
        ),

        'indexes' => array(),
      );

      // Return query and SQL.
      $mview = array(
        'query' => $mview_query,
        'schema' => $mview_schema,
      );

      break;

    //
    // Additional case here.
  }


  return $mview;
}


/// AP DATA DOWNLOADER.


/**
 * FUNCTION GENERATE FILE.
 * Create a tsv, csv and xlsx file.
 *
 * @param $filename
 *   String, destination filename.
 * @param $data_rows
 *   An array containing the headers (index 0) and data points to write to a file.
 * @param $file_type
 *   String, indicating the type of file (tsv, csv, xlsx).
 */
function analyzedphenotypes_writefile($filename, $data_rows, $file_type) {
  $filepath = variable_get('trpdownload_fullpath', '') . $filename;

  switch($file_type) {
    //
    case 'tsv':
    //
    case 'csv':
      $FILE = fopen($filepath, 'w') or die ('Unable to create file to write to...');

      foreach($data_rows as $row) {
        if ($file_type == 'tsv') {
          fputcsv($FILE, $row, "\t");
        }
        else {
          fputcsv($FILE, $row);
        }
      }

      fclose($FILE);

      break;

    //
    case 'xlsx':
      // Load spreadsheet writer library.
      $xlsx_writer = libraries_load('spreadsheet_writer');
      include_once $xlsx_writer['library path'] . '/'. $xlsx_writer['files'][0];

      $writer = new XLSXWriter();
      @$writer->writeSheet($data_rows);
      $writer->writeToFile($filepath);

      break;
  }
}


/**
 * FUNCTION GLOBAL FIELD NAMES
 * Create/Register field names for every form element in the download form and use
 * in various hooks involved in processing download request.
 *
 * @param
 *
 * @return
 *   An array of string, field names grouped by base name, cache name, field id and hidden field.
 */
function analyzedphenotypes_fieldnames() {
  // Field names.
  $fields = array('experiment', 'genus', 'species', 'traits', 'year', 'location', 'germplasmtype', 'germplasm',
                  'missingdata', 'filetype', 'averagerep', 'rfriendly', 'columnheaders', 'origin', 'value',
                  'replicate', 'collector');
  // Properties.
  $prop = array('base', 'field', 'cache', 'field-id', 'field_hidden');

  $fieldnames = array();

  // For each field, create a field name used for #id, cache, #hidden fieldname etc.
  foreach($fields as $f) {
    foreach($prop as $p) {
      $c = ($p == 'field-id') ? '-' : '_';
      // ap-fieldname-field-id.
      $value = ($p == 'base') ? $f : 'ap' . $c . $f . $c . $p;

      $fieldnames[$f][$p] = $value;
    }
  }


  return $fieldnames;
}


/**
 * FUNCTION POPULATE DOWNLOAD FORM FIELDS.
 * AJAX load values to each form field in the form.
 * Dependencies: analyzedphenotypes_fieldnames(), analyzedphenotypes_systemvariables().
 *
 * @param $field
 *   String, the target field element.
 *   - file : Fill/create a file with datapoints based on filters. File provided by Tripal Job.
 *   - field_project : Fill select project/experiment field (select field).
 *   - field_project_summary : Fill project/experiment summary count information/tip window (on mouse over to activate).
 *   - field_genus : Fill select genus field (select field).
 *   - field_species : Fill select species field (select field).
 *   - field_traits : Fill select traits field (checkbox group).
 *   - field_year : Fill select year field (checkbox group).
 *   - field_location : Fill select locaiton field (checkbox group).
 *   - field_germplasmtype : Fill select germplasm type field (checkbox group).
 *   - field_germpasm : Fill germplasm field/list (collapsible window).
 * @param $parameter
 *   An array containing form field values preceeding selected field.
 *   - experiment : experiment field.
 *   - genus : genuse field.
 *   - species : species field.
 *   - traits : traits field.
 *   - year : year field.
 *   - location : location field.
 *   - germplasmtype : germplasmtype field.
 *
 * @return
 *   An array of values based on field request.
 *
 * @see analyzedphenotypes_data_download in admin.form.inc.
 */
function analyzedphenotypes_populatefield($field, $parameter = null) {
  // Use this character to separate entries in a string.
  $delimiter = '~';
  $field_values = null;

  switch($field) {
    //
    case 'file':
      // Create export file.
      // In:
      //   .module - analyzedphenotypes_trpdownload_generate_file() : Tripal Download API create file.
      // Parameters:
      //   $parameter['experiment'], $parameter['genus'], $parameter['species'], $parameter['traits']
      //   $parameter['year'], $parameter['location'], $parameter['germplasmtype'].

      $fldname = analyzedphenotypes_fieldnames();

      // Match field to actual chado column names.
      // Limit rows to project, trait, year, location, germplasm type and germplasm.
      $column = array(
        $fldname['experiment']['base']    => 'project_id',
        $fldname['traits']['base']        => 'cvterm_id',
        $fldname['germplasmtype']['base'] => 't3.type_id',
        $fldname['year']['base']          => 't7.value',
        $fldname['location']['base']      => 't8.value',
        // As entered in the germplasm field.
        $fldname['germplasm']['base']     => "CONCAT(t3.name, ' (', t3.uniquename, ')')",
      );

      // Processed filter from tripal_download api.
      $filters = $parameter['filters'];

      $limit = array();
      foreach($column as $filter => $col) {
        $filter_value = $filters[$filter];

        // Convert filter so chado_query arguments (:placeholder = value).
        // Exclude filter with value = all.
        if ($filter_value != 'all') {
          $placeholder = ':' . $filter;

          $limit['limit'] .= ' AND ' . $col . ' IN (' . $placeholder . ')';
          $limit['args'][$placeholder] = explode($delimiter, $filter_value);
        }
      }

      $sql = sprintf("
        SELECT
          t2.project_id, t1.phenotype_id, t2.name AS %s, t3.name AS %s,
          STRING_AGG(t4.value, '') AS %s, t5.name AS %s, t1.value AS %s, t6.value AS %s,
          t7.value AS %s, t8.value AS %s, t9.value AS %s
        FROM
          {phenotype}                AS t1
          INNER JOIN {project}       AS t2 USING(project_id)
          INNER JOIN {stock}         AS t3 USING(stock_id)
          LEFT  JOIN {stockprop}     AS t4 ON t3.stock_id = t4.stock_id AND t4.type_id = :term_origin
          INNER JOIN {cvterm}        AS t5 ON t1.attr_id = t5.cvterm_id
          INNER JOIN {phenotypeprop} AS t6 USING(phenotype_id)
          INNER JOIN {phenotypeprop} AS t7 USING(phenotype_id)
          INNER JOIN {phenotypeprop} AS t8 USING(phenotype_id)
          INNER JOIN {phenotypeprop} AS t9 USING(phenotype_id)
        WHERE
          t6.type_id = :term_replicate AND t7.type_id = :term_year
          AND t8.type_id = :term_location AND t9.type_id = :term_collector
          %s
        GROUP BY
          t2.project_id, t1.phenotype_id, t2.name, t3.name, t5.name, t1.value,
          t6.value, t7.value, t8.value, t9.value
        ORDER BY t2.name, t3.name, t7.value, t8.value ASC",
          $fldname['experiment']['base'], $fldname['germplasm']['base'],
          $fldname['origin']['base'],     $fldname['traits']['base'],
          $fldname['value']['base'],      $fldname['replicate']['base'],
          $fldname['year']['base'],       $fldname['location']['base'],
          $fldname['collector']['base'],
          $limit['limit']);


      // Plant property (replicate, year, location, data collector) cvterms;
      $sysvars = analyzedphenotypes_systemvariables('terms');

      $options = chado_query($sql,
        // project, traits, year, locaiton, germplasm type and germplasm filters.
        $limit['args'] +
        array(
          // Plant property terms.
          ':term_replicate' => variable_get($sysvars['ap_replicate']),
          ':term_year'      => variable_get($sysvars['ap_year']),
          ':term_origin'    => variable_get($sysvars['ap_origin']),
          ':term_location'  => variable_get($sysvars['ap_location']),
          ':term_collector' => variable_get($sysvars['ap_collector']),
        ));

      // Return result.
      if (count($options) > 0) {
        $field_values = ($options->rowCount() > 0) ? $options->fetchAll() : $field_values;
      }

      break;

    //
    case 'field_project':
      // Fill field project.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter : null.

      $options = chado_query(
        "SELECT project_id, t1.name
         FROM {project} AS t1 INNER JOIN {phenotype} USING(project_id)
         GROUP BY project_id ORDER BY t1.name ASC"
      );

      if (count($options) > 0) {
        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_project_summary':
      // Fill field project summary count information.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter : null.

      $options = chado_query(
        "SELECT project_id, CONCAT('*' || COUNT(DISTINCT attr_id) || ' Traits, ' || COUNT(phenotype_id) || ' Data Points ')
         FROM {phenotype}
         GROUP BY project_id
         ORDER BY project_id ASC"
      );

      if ($options) {
        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_genus':
      // Fill field genus.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.

      $options = analyzedphenotypes_genusprop('project_genus', array(
        'project_id' => $parameter['project_id'],
      ));

      if ($options) {
        $field_values = array($options => $options);
      }

      break;

    //
    case 'field_species':
      // Fill field species.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.

      $options = chado_query(
        "SELECT species
         FROM {organism}
         WHERE genus = :genus
           AND organism_id
             IN (SELECT organism_id FROM {stock} INNER JOIN {phenotype} USING (stock_id) WHERE project_id = :project_id)
         GROUP BY species ORDER BY species ASC",
        array(
          ':genus' => $parameter['genus'],
          ':project_id' => $parameter['project_id'],
        )
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_traits':
      // Fill field traits.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.

      if (count($parameter['project_id']) > 1) {
        $options = chado_query(
          "SELECT t1.cvterm_id, t1.name
           FROM {cvterm} AS t1 INNER JOIN {phenotype} ON t1.cvterm_id = attr_id
           WHERE project_id IN (:project_id)
           GROUP BY t1.cvterm_id, t1.name
           ORDER BY t1.name ASC",
          array(
            ':project_id' => $parameter['project_id'],
          )
        );

        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }
      else {
        $options = chado_query(
          "SELECT cvterm_id, name FROM {cvterm} WHERE cvterm_id IN (SELECT attr_id FROM {phenotype}
           WHERE project_id = :project_id
             AND stock_id IN
               (SELECT stock_id FROM {stock} WHERE organism_id IN (SELECT organism_id from {organism}
                WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species))))",
          array(
            ':project_id' => $parameter['project_id'],
            ':genus' => $parameter['genus'],
            ':species' => $parameter['species'],
          )
        );

        $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;
      }

      break;

    //
    case 'field_year':
      // Fill field year.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);

      $options = chado_query(
        "SELECT value FROM {phenotypeprop} WHERE type_id = :year_property
         AND phenotype_id IN (SELECT phenotype_id FROM {phenotype}
           WHERE
             project_id IN (:project_id)
             AND attr_id IN (:traits)
             AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN
               (SELECT organism_id from {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)))
             )
         GROUP BY value ORDER BY value DESC",
        array(
          ':year_property' => $year_property,
          ':project_id' => $parameter['project_id'],
          ':traits' => $parameter['traits'],
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        )
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_location':
      // Fill field location.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);
      $location_property = variable_get($sysvars['ap_location']);

      if (isset($parameter['genus']) && isset($parameter['species'])) {
        $condition_species = 'AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN (
          SELECT organism_id FROM {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)
        ))';

        $args_species = array(
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        );
      }
      else {
        $condition_species = '';
        $args_species = array();
      }


      $sql = sprintf(
        "SELECT t1.value
         FROM
           {phenotype}
           INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
           INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
         WHERE
           project_id IN (:project_id)
           AND attr_id IN (:traits)
           AND t1.type_id = :location_property
           AND t2.type_id = :year_property
           AND t2.value IN (:year)
           %s ORDER BY t1.value ASC", $condition_species);

      $options = chado_query($sql,
        array(
          ':project_id' => $parameter['project_id'],
          ':traits'  => $parameter['traits'],
          ':year' => $parameter['year'],
          ':location_property' => $location_property,
          ':year_property' => $year_property,
        )
        + $args_species
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed(0, 0) : $field_values;

      break;

    //
    case 'field_germplasmtype':
      // Fill field germplasm type.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.
      //   $parameter['location'] : locations selected.

      $sysvars = analyzedphenotypes_systemvariables('terms');

      $year_property = variable_get($sysvars['ap_year']);
      $location_property = variable_get($sysvars['ap_location']);

      if (isset($parameter['genus']) && isset($parameter['species'])) {
        $condition_species = 'AND stock_id IN (SELECT stock_id FROM {stock} WHERE organism_id IN (
          SELECT organism_id FROM {organism} WHERE LOWER(genus) = LOWER(:genus) AND LOWER(species) = LOWER(:species)
          ))';

        $args_species = array(
          ':genus' => $parameter['genus'],
          ':species' => $parameter['species'],
        );
      }
      else {
        $condition_species = '';
        $args_species = array();
      }

      $sql = sprintf(
        "SELECT cvterm_id, t0.name FROM {cvterm} AS t0 INNER JOIN {stock} ON cvterm_id = type_id
         WHERE stock_id IN (
           SELECT stock_id
             FROM
               {phenotype}
               INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
               INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
             WHERE
               project_id IN (:project_id)
               AND attr_id IN (:traits)
               AND t1.type_id = :location_property
               AND t1.value IN (:location)
               AND t2.type_id = :year_property
               AND t2.value IN (:year)
               %s)", $condition_species);

      $options = chado_query($sql,
        array(
          ':project_id' => $parameter['project_id'],
          ':traits' => $parameter['traits'],
          ':year_property' => $year_property,
          ':year' => $parameter['year'],
          ':location_property' => $location_property,
          ':location' => $parameter['location'],
        )
        + $args_species
      );

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;

      break;

    //
    case 'field_germplasm':
      // List germplasm found base on the filters selected.
      // In:
      //   .admin.form - analyzedphenotypes_data_download() : Data download form.
      // Parameters:
      //   $parameter['project_id'] : experiment/project selected.
      //   $parameter['genus'] : genus selected.
      //   $parameter['species'] : species selected.
      //   $parameter['traits'] : traits selected.
      //   $parameter['year'] : years selected.
      //   $parameter['location'] : locations selected.

      if (isset($parameter['genus'])) {
        $sysvars = analyzedphenotypes_systemvariables('terms');

        $var_value = array(
          ':year' => variable_get($sysvars['ap_year']),
          ':location' => variable_get($sysvars['ap_location']),
        );

        $fields = array(
          'plantprop' => array(
            ':project'  => array('operator' => 'IN', 'field' => 'project_id', 'value' => $parameter['project_id'], 'type' => ''),
            ':traits'   => array('operator' => 'IN', 'field' => 'attr_id',    'value' => $parameter['traits'],     'type' => ''),
            ':year'     => array('operator' => 'IN', 'field' => 't1.value',   'value' => $parameter['year'],       'type' => 't1.type_id'),
            ':location' => array('operator' => 'IN', 'field' => 't2.value',   'value' => $parameter['location'],   'type' => 't2.type_id'),
          ),
          'organism' => array(
            ':genus'    => array('operator' => '=',  'field' => 'genus',      'value' => $parameter['genus'],       'type' => ''),
            ':species'  => array('operator' => '=',  'field' => 'species',    'value' => $parameter['species'],     'type' => ''),
          ),
        );

        $condition = array();
        $where_clause = array();
        $args = array();

        foreach($fields as $field => $prop) {
          foreach($prop as $col => $value) {
            if (isset($value['value']) && !empty($value['value'])) {
              if ($value['type']) {
                $p = $col . '_property';
                $condition[$field][] = $value['type'] . ' = ' . $p;
                $args[$p] = $var_value[$col];
              }

              $v = ($value['operator'] == 'IN') ? ' (' . $col . ')' : $col;
              $condition[$field][] = $value['field'] . ' ' . $value['operator']  .  $v;

              $args[$col] = $value['value'];
            }
          }

          $where_clause[$field] = implode(' AND ', $condition[$field]);
        }

        $sql1 = "SELECT stock_id, CONCAT(name, ' (', uniquename, ')') AS germplasm FROM {stock} WHERE ";

        $sql2 = sprintf("
          stock_id IN (
            SELECT stock_id FROM {phenotype}
              INNER JOIN {phenotypeprop} AS t1 USING (phenotype_id)
              INNER JOIN {phenotypeprop} AS t2 USING (phenotype_id)
            WHERE %s
        ) ", $where_clause['plantprop']);

        $sql3 = sprintf(" AND organism_id IN (SELECT organism_id FROM {organism} WHERE %s) ", $where_clause['organism']);
        $sql4 = " AND type_id IN (:germplasmtype) ";
        $sql5 = " GROUP BY stock_id, name, uniquename ORDER BY name ASC";

        $sql = $sql1 . $sql2;

        if (!empty($where_clause['organism'])) {
          $sql .= $sql3;
        }

        if (!empty($parameter['germplasmtype'])) {
          $sql .= $sql4;
          $args[':germplasmtype'] = $parameter['germplasmtype'];
        }

        $sql .= $sql5;
        $options = chado_query($sql, $args);
      }
      else {
        $options = chado_query(
          "SELECT t1.stock_id, CONCAT(t1.name, ' (', t1.uniquename, ')') AS germplasm
          FROM {stock} AS t1 INNER JOIN {phenotype} USING(stock_id) WHERE project_id IN (:project_id)
          GROUP BY t1.stock_id, t1.name, t1.uniquename ORDER BY t1.name ASC",
          array(
            ':project_id' => $parameter['project_id'],
          )
        );
      }

      $field_values = ($options->rowCount() > 0) ? $options->fetchAllKeyed() : $field_values;

      break;
  }


  return $field_values;
}


/**
 * FUNCTION CACHE FORM VALUE.
 * Form field values are save (cached) when nanipulating fields. Values are then compared
 * to current selection to predict if new value had been selected or form retained same
 * values in each AJAX call. The form state can then be used wheather to process a request
 * or just reload values/selections.
 *
 * @param $value
 *   Value entered or selection made to field element.
 *
 * @return
 *   An array, to register a value in Drupal Form API.
 */
function analyzedphenotypes_cachevalue($value) {
  if (is_array($value)) {
    $value = implode('_', $value);
  }

  return array(
    '#type' => 'value',
    '#value' => $value,
  );
}


// AUTO COMPLETE FORM FIELD - ADDED BY AJAX REQUEST.


/**
 * FUNCTION CALLBACK: AJAX add and remove form field.
 * Dependencies: analyzedphenotypes_autofieldprop().
 *
 * $param $command
 *   String, add (add) or remove (rem) to add and remove autocomplete from field element.
 * $param $element
 *   String, id attribute of element where to insert field in case of command add and
 *   id attribute of field to delete in case of command rem.
 * @param $default_value
 *   String, default value of autocomplete field. base64 encoding used.
 *
 * @return
 *   Rendered autocomplete form element.
 *
 * @see
 *   hook_menu().
 */
function analyzedphenotypes_autofield_callback($command, $element, $default_value = null) {
  // Add or remove autofield command.
  $cmd = trim($command);
  $elm = trim($element);

  if (($cmd == 'add' || $cmd == 'rem') && !empty($elm)) {
    // Add or remove autofield command.
    $element_id = '#' . $elm;

    if ($cmd == 'add') {
      // Create a from field api and append to element wrapper.
      $default_value = base64_decode($default_value);
      $field_api = analyzedphenotypes_autofieldprop($default_value);
      $field_htm = drupal_render($field_api);

      $AJAX_cmd = ajax_command_prepend($element_id, $field_htm);
    }
    else {
      // Given a reference id to an element, remove if from DOM.
      $AJAX_cmd = ajax_command_remove($element_id);
    }

    return array(
      '#type' => 'ajax',
      '#commands' => array($AJAX_cmd),
    );
  }
  else {
    // Not valid command. No element. Do nothing.
    return FALSE;
  }
}


/**
 * FUNCTION: Define a autocomplete form field. Button set add and remove rendered ajacent to each element.
 * Dependencies:
 *
 * @param $default_value
 *   Default to: Type Germplasm/Stock Name.
 *   Initial value of autocomplete form field (#value key of the form field API).
 *
 * @return array
 *   Array, form autocomplete form field API array.
 *
 * @see
 *   analyzedphenotypes_autofield_callback() in api.inc.
 *   analyzedphenotypes_data_download() in admin.form.inc.
 */
function analyzedphenotypes_autofieldprop($default_value) {
  // Menu item that will handle AJAX request.
  $autofield_menu = 'download/autofield/';
  // Id each field added. This id will be use to reference an element.
  $autofield_id = uniqid();
  // Add and remove command - argument to menu item.
  $autofield_rem  = 'rem';
  $autofield_add  = 'add';
  // Make an AJAX link.
  $autofield_attr = array(
    'attributes' => array(
      'class' => array('use-ajax', 'ap-autofield-germplasm-control'),
      'title' => 'Click to Add (+) or Remove (x) Germplasm',
    )
  );

  $emptyfield_default_value = 'Type Germplasm/Stock Name';

  // l(text, url + element, attributes);
  // Use id #ap-autofield in the main form where field will be inserted.
  // Remove does not require a default value - &.
  $autofield_btns = l('Rem', $autofield_menu . $autofield_rem . '/ap-autofield-wrapper-' . $autofield_id . '/&', $autofield_attr)
                  . l('Add', $autofield_menu . $autofield_add . '/ap-autofield/' . base64_encode($emptyfield_default_value), $autofield_attr);

  // Define field API.
  $default_value = ($default_value) ? $default_value : $emptyfield_default_value;
  $autofield_fld['ap_autofield_' . $autofield_id] = array(
    '#type' => 'textfield',
    '#value' => $default_value,
    '#theme_wrappers' => array(),
    // Wrap this field.
    '#prefix' => '<div id="ap-autofield-wrapper-' . $autofield_id . '" class="ap-autofield-wrapper ap-autofield-AJAX-off">',
    // Buttons.
    '#suffix' => '<span>' . $autofield_btns . '</span></div>',
    //
    '#attributes' => array('class' => array('ap-autofield-field')),
    '#id' => 'ap-autofield-' . $autofield_id,
  );


  return $autofield_fld;
}