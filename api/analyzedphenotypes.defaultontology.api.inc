<?php
/**
 * @file
 * Contains Default Controlled Vocabulary terms.
 * Source: TRIPAL CORE - http://tripal.info
 *         ONTOBEE - http://www.ontobee.org
 */

/**
 * FUNCTION: Define controlled vocabulary terms required by this system.
 *
 * @return
 *   An array, where the key is the controlled vocabulary name and value contains
 *   definition and subsequent controlled vocabulary terms.
 *
 * @see analyzedphenotypes_defaultontologyload() in file.
 */
function analyzedphenotypes_defaultontologydefine() {
  $ontology = array();

  // Array to associate each term to predefine terms.
  // Predefined Terms => Matching name in terms array definition below (case-sensitive).
  $map_term = array(
    'genus'     => 'genus',
    'year'      => 'Year',
    'method'    => 'method',
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

  // METHOD, LOCATION, REPLICATE, COLLECTED BY - NCIT:
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
 * FUNCTION: Insert into chado.cvterm or test existence of a cvterm.
 * and return cvterm id to be set as default value in configuration page.
 *
 * Dependencies: analyzedphenotypes_defaultontologydefine().
 *
 * @return
 *   An array, controlled vocabulary terms cvterm id.
 *
 * @see analyzedphenotypes_defaultontologydefine() in file.
 * @see hook_install() implementation in install.inc.
 */
function analyzedphenotypes_defaultontologyload() {
  $ontology = analyzedphenotypes_defaultontologydefine();

  // This array to hold default controlled vocabulary terms used in this system.
  $ontology_load = array();

  // Each controlled vocabulary - cv.
  foreach($ontology['terms'] as $abbrev => $details) {
    // Each term in a Controlled Vocabulary cv.
    foreach($details['terms'] as $term) {
      // Fetch the term, see if previously added.
      $t = tripal_get_cvterm(array(
        'name' => $term['name'],
        'cv_id' => array('name' => $details['name']),
      ));

      if (!$t) {
        // Term not in chado.cvterm. Insert!
        // #1. Controlled vocabulary.
        $ins_cv = tripal_insert_cv($details['name'], $details['definition']);

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
