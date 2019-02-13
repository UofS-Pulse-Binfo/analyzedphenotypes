<?php
/**
 * @file
 *
 * API: ONTOLOGY.
 * Manage property and operation pertaining to data ontology information.
 *
 * Contains Default Controlled Vocabulary terms.
 * Source: TRIPAL CORE - http://tripal.info
 *         ONTOBEE - http://www.ontobee.org
 */



/**
 * DATA SOURCE
 * Define controlled vocabulary terms required by this system.
 *
 * @return
 *   An array, where the key is the controlled vocabulary name and value contains
 *   definition and subsequent controlled vocabulary terms.
 */
function ap_define_defaultontology() {
  $ontology = array();

  // Array to associate each term to predefine terms.
  // Predefined Terms => Matching name in terms array definition below (case-sensitive).
  $map_term = array(
    'genus'     => 'genus',
    'year'      => 'Year',
    'method'    => 'method',
    'unit'      => 'unit',
    'related'   => 'related',
    'location'  => 'location',
    'replicate' => 'replicate',
    'collector' => 'Collected By',
  );

  // YEAR - TRIPAL PUB, TRIPAL CORE:
  $ontology['tripal_pub'] = array(
    'name' => 'tripal_pub',
    'definition' => 'Tripal Publication Ontology. A temporary ontology until a more formal appropriate ontology to be identified.',
    'all terms' => FALSE,

    'terms' => array(
      array(
        'id'     => 'TPUB:0000059',
        'name'     => 'Year',
        'definition' => 'The year the work was published. This should be a 4 digit year.',
      ),

      // Additional terms here.
    ),
  );

  // GENUS - TAXONOMIC RANK, TRIPAL CORE:
  $ontology['taxonomic_rank'] = array(
    'name' => 'taxonomic_rank',
    'definition' => 'A vocabulary of taxonomic ranks (species, family, phylum, etc).',
    'all terms' => FALSE,

    'terms' => array(
      array(
        'id'    => 'TAXRANK:0000005',
        'name'     => 'genus',
        'definition' => 'The genus.',
      ),

      // Additional terms here.
    ),
  );

  // RELATED - SYNONYM TYPE, TRIPAL CORE:
  $ontology['synonym_type'] = array(
    'name' => 'synonym_type',
    'definition' => 'A local vocabulary added for synonynm types.',
    'all terms' => FALSE,

    'terms' => array(
      array(
        'id'     => 'internal:related',
        'name'     => 'related',
        'definition' => 'Is related to.',
      ),

      // Additional terms here.
    )
  );

  // METHOD => UNIT - UNIT
  $ontology['uo'] = array(
    'name' => 'uo',
    'definition' => 'Units of Measurement Ontology.',
    'all terms' => FALSE,

    'terms' => array(
      array(
        'id'     => 'UO:0000000',
        'name'     => 'unit',
        'definition' => '',
      ),

      // Additional terms here.
    )
  );

  // TRAIT => METHOD, LOCATION, REPLICATE, COLLECTED BY - NCIT:
  $ontology['NCIT'] = array(
    'name' => 'NCIT',
    'definition' => 'The NCIT OBO Edition project aims to increase integration of the NCIt with OBO Library ontologies
      NCIt is a reference terminology that includes broad coverage of the cancer domain, including cancer related diseases,
      findings and abnormalities. NCIt OBO Edition releases should be considered experimental.',
    'all terms' => FALSE,

    'terms' => array(
      array(
        'id'     => 'NCIT:C71460',
        'name'     => 'method',
        'definition' => 'A means, manner of procedure, or systematic course of action that have to be performed in order
          to accomplish a particular goal.',
      ),
      array(
        'id' => 'NCIT:C25341',
        'name' => 'location',
        'definition' => 'A position, site, or point in space where something can be found.',
      ),
      array(
        'id'     => 'NCIT:C28038',
        'name'     => 'replicate',
        'definition' => 'A role played by a biological sample in the context of an experiment where the intent is that
          biological or technical variation is measured.'
      ),
      array(
        'id'     => 'NCIT:C45262',
        'name'     => 'Collected By',
        'definition' => 'Indicates the person, group, or institution who performed the collection act.',
      ),

      // Additional terms here.
    ),
  );


  return array(
    'terms' => $ontology,
    'map_term' => $map_term,
  );
}


/**
 * FUNCTION:
 * Insert into chado.cvterm or test existence of a cvterm.
 * and return cvterm id to be set as default value in configuration page.
 * Dependencies: ap_define_defaultontology().
 *
 * @return
 *   An array, controlled vocabulary terms cvterm id.
 *
 * @see hook_install() implementation in install.inc.
 */
function ap_load_defaultontology() {
  $ontology = ap_define_defaultontology();

  // This array to hold default controlled vocabulary terms used in this system.
  $ontology_load = array();

  // Each controlled vocabulary - cv.
  foreach($ontology['terms'] as $abbrev => $details) {
    // Each term in a Controlled Vocabulary cv.
    foreach($details['terms'] as $term) {
      // Fetch the term, see if previously added.
      $a = array(
        'name' => $term['name'],
        'cv_id' => array('name' => $details['name']),
      );

      if (function_exists('chado_get_cvterm')) {
        $t = chado_get_cvterm($a);
      }
      else {
        $t = tripal_get_cvterm($a);
      }

      if (!$t) {
        // Term not in chado.cvterm. Insert!
        // #1. Controlled vocabulary.
        if (function_exists('chado_insert_cv')) {
          $ins_cv = chado_insert_cv($details['name'], $details['definition']);
        }
        else {
          $ins_cv = tripal_insert_cv($details['name'], $details['definition']);
        }

        if (!$ins_cv) {
          // Failed to insert cv.
          tripal_report_error('Analyzed Phenotypes', TRIPAL_CRITICAL, 'Chado/Tripal failed to insert cv ontology', array(), array('print' => TRUE));
        }

        // #2. Terms.
        $term['cv_name'] = $details['name'];
        $ins_cvterm = tripal_insert_cvterm($term);
      }

      // CV TERM NAME.
      $name = ($t) ? $t->name : $ins_cvterm->name;
      // Match predefined variable to cvterm saved.
      $map_term = array_search($name, $ontology['map_term']);

      // Save term to be used as configuration page default settings.
      // eg. ap_location = 1231.
      $ontology_load[$map_term] = ($t) ? $t->cvterm_id : $ins_cvterm->cvterm_id;

    }
  }


  return $ontology_load;
}


/**
 * FUNCTION:
 * Set ontology relationship of a trait in chado.cvterm_relationship.
 * Dependencies: ap_get_variablenames() - systermvars.api.
 *
 * @param $identifiers
 *   An array with the following keys:
 *   - trait_cvterm_id : cvterm id number to which the ontology will be associated to.
 *   and - ontology_cvterm_id : cvterm id number of ontology term that will be associated to a cvterm_id.
 *
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *   - replace : boolean true or false, when set to true attempt to replace existing ontolgy.
 */
function ap_set_ontology($identifiers, $options = array('replace' => FALSE)) {
  $sysvar = ap_get_variablenames(
    array('variablename' => 'related'),
    array('set' => 'terms')
  );

  $sysvar_related = variable_get($sysvar);

  if ($sysvar_related > 0 && $identifiers['trait_cvterm_id'] > 0 && $identifiers['ontology_cvterm_id'] > 0) {
    $values = array(
      'type_id' => $sysvar_related,
      'object_id' => $identifiers['trait_cvterm_id'],
      'subject_id'  => $identifiers['ontology_cvterm_id'],
    );

    // Check if there existed any ontology for a trait.
    $umatch = array_slice($values, 0, 2);
    $on = chado_generate_var('cvterm_relationship', $umatch);

    if ($on && $options['replace']) {
      // When implied in otions array, replace previously set ontology.

      // Found an ontology term associated to a trait and
      // replace existing value.

      // Alter the subject_id part to new ontology.
      $uvalue = array_slice($values, 2, 1);
      chado_update_record('cvterm_relationship', $umatch, $uvalue);
    }
    elseif (!$on) {
      // Create trait - ontology relationship.
      // No existing ontology information for this trait.
      chado_insert_record('cvterm_relationship', $values);
    }
  }
  else {
    // Failed to update or set ontology term.
    tripal_report_error('Analyzed Phenotypes', TRIPAL_CRITICAL, 'Chado/Tripal failed to update or set ontology term', array(), array('print' => TRUE));
  }
}


/**
 * FUNCTION:
 * Create a list of cvterm terms as ontology suggestions to
 * crop ontology field in Stage 3 - Describe Trait of upload.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *               ap_get_cvterm() - cv.api.
 *
 * @param $identifiers.
 *   An array with the following keys:
 *     - trait : string, trait name or column header.
 *     - genus : string, genus.
 * @param $options
 *   An array to be used as additional options to be performed to
 *   returned result. Keys:
 *     - limitrows : integer, number of records/row to return. Default to 10 recrods.
 *
 *  @return
 *    An array, containing the cvterm_id and name of term as ontology.
 */
function ap_suggest_ontology($identifiers, $options = array('limitrows' => 10)) {
  $sysvar = ap_get_variablenames(
    array('variablename' => $identifiers['genus']),
    array('set' => 'cvdbon', 'suffix' => 'ontology')
  );

  $sysvar_genus_ontology = variable_get($sysvar);

  if ($sysvar_genus_ontology) {
    // Prepare suggestions only when ontology configuration is set for a genus.

    // Fetch all terms in a controlled vocabulary (cv).
    // Returm cvterm id and name.
    $ontology_terms = ap_get_cvterm(
      array('cv_id' => $sysvar_genus_ontology),
      array('dataset' => 'cvtermidname')
    );

    // Each term, break it to individual words, each word will
    // become a keyword that will be searched in the trait name.
    $ctr = 0;
    foreach($ontology_terms as $term) {
      $keys = explode(' ', trim(strtolower($term['name'])));

      // Compare each word.
      foreach($keys as $key) {
        if ($ctr >= $options['limitrows']) {
          break;
        }
        else {
          // A keyword is being used in the trait name, suggest the cvterm.
          // Skip keys that are just 2 chars.
          if (strlen($key) > 2 && stristr($identifiers['trait'], $key)) {
            $suggest_ontology[ $term['cvterm_id'] ]
              = array('name' => $term['name']);

            $ctr++;
          }
        }
      }
    }
  }


  return (isset($suggest_ontology)) ? $suggest_ontology : null;
}


/**
 * FUNCTION:
 * Match an ontology name.
 * Dependencies: ap_get_variablenames() - systemvars.api.
 *               ap_get_cvterm() - cv.api.
 *
 * @param $genus
 *   A string value, genus.
 * @param $trait
 *   A string, ontology name.
 *
 * @return
 *   An array, containing cvterm_id and name.
 */
function ap_match_ontology($genus, $ontology) {
  $sysvar = ap_get_variablenames(
    array('variablename' => $genus),
    array('set' => 'cvdbon', 'suffix' => 'ontology')
  );

  $sysvar_genus_ontology = variable_get($sysvar);

  // Find a cvterm by name, limit search in ontology configuration (cv)
  // and return the cvterm id and name.
  $ontology_match = ap_get_cvterm(
    array('name' => $ontology, 'genus' => $sysvar_genus_ontology),
    array('dataset' => 'cvtermidname')
  );

  $ontology_match = reset($ontology_match);


  return ($ontology_match)
    ? array($ontology_match['cvterm_id'] => $ontology_match['name'])
    : null;
}
