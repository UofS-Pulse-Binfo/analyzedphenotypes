<?php
/**
 * @file
 *
 * API: COLUMN HEADERS/TRAITS.
 * Manage property and operation pertaining column headers.
 */
 
 
 
/**
 * DATA SOURCE
 * Prepare column headers used in forms, validators and data exporter.
 *
 * @param $set
 *   An array with the following keys:
 *   - columns_datatype : create a set of column headers used as expected column headers in data loader.
 *   - columns_combination : create a set of column headers used to verify the uniqueness of datapoints.
 *   - columns_downloadable : create a set of column headers used to pre-fill header picker in data exporter form.
 *   - columns_downloadable_tofield : create a set of column headers used in data exporter and relate to form field names.
 *
 * @return
 *  An array, a set of column headers.
 */
function ap_create_columnheaders($set) {
  // STANDARD COLUMN HEADERS EXPECTED IN A FILE.
  $cols = array(
    'Trait Name',                 // #0
    'Germplasm Accession',        // #1
    'Germplasm Name',             // #2
    'Year',                       // #3
    'Location',                   // #4
    'Replicate',                  // #5
    'Value',                      // #6
    'Data Collector',             // #7

    // Define column header here...
  );

  $columnheaders = null;

  // Create sets, define type and map to column headers.

  if ($set == 'columns_datatype') {
    // Column headers expected in a tsv file and expected data type each is measuring.
    // @see dataype.api for handling of each data type.
    $columnheaders = array(
      $cols[0] => 'trait',        // Trait name - trait type.
      $cols[1] => 'alphanumeric', // Germplasm Accession name - text and numbers.
      $cols[2] => 'alphenumeric', // Gemplasm Name - text and numbers.
      $cols[3] => 'year',         // Yer - year YYYYY.
      $cols[4] => 'text',         // Location - text.
      $cols[5] => 'number',       // Replicate - number.
      $cols[6] => '',             // Value - to be determined by unit in trait name.
      $cols[7] => 'text',         // Data Collector - text.
    );
  }
  elseif ($set == 'columns_combination') {
    // Combination of columns used to test uniquness of a row.
    // Trait Name + Germplasm Accession + Year + Location + Replicate.
    $columnheaders = array(
      $cols[0],                   // Trait Name.
      $cols[1],                   // Germplasm Accession.
      $cols[3],                   // Year.
      $cols[4],                   // Location.
      $cols[5],                   // Replicate.
    );
  }
  elseif ($set == 'columns_downloadable' || $set == 'columns_downloadable_tofield') {
    // Downloadable column header used to pre-populate column header picker
    // for data downloader.

    // Available column headers as shown in column headers picker in download form.
    $downloadable_columns = array(
      'Experiment',               // Extra : Experiment.
      $cols[0],                   // Trait Name.
      $cols[2],                   // Germplasm Name.
      $cols[3],                   // Year.
      $cols[4],                   // Location.
      $cols[5],                   // Replicate.
      $cols[7],                   // Data Collector.
      'Country of Origin',        // Extra : Country of Origin.
    );

    if ($set == 'columns_downloadable') {
      // Populate header picker.
      $columnheaders = $downloadable_columns;
    }
    else {
      // Field names used by Data download form that relates to a column header.
      // @TODO: change when field name api is ready.
      $fieldnames = ap_construct_download_fieldnames();

      $columnheaders = array(
        $downloadable_columns[0] => $fieldnames['experiment']['base'],
        $downloadable_columns[1] => $fieldnames['traits']['base'],
        $downloadable_columns[2] => $fieldnames['germplasm']['base'],
        $downloadable_columns[3] => $fieldnames['year']['base'],
        $downloadable_columns[4] => $fieldnames['location']['base'],
        $downloadable_columns[5] => $fieldnames['replicate']['base'],
        $downloadable_columns[6] => $fieldnames['collector']['base'],
        $downloadable_columns[7] => $fieldnames['origin']['base']
      );
    }
  }


  return $columnheaders;
}


/**
 * FUNCTION:
 * Test combnination of column headers against matching column headers from a data point/row to see if it
 * exsits in chado.phenotype and/or chado.phenotypeprop tables.
 * Dependencies: ap_create_columnheaders().
 *
 * @param $datarow
 *   An array, a single row of data from a tsv data sheet. Data/cell filtered to only include
 *   cells matching column headers in column combnination set.
 *
 * @return
 *   Record id number when match found and null when no match found.
 */
function ap_match_columncombination($datarow) {
  // Fetch combination of columns to test uniqueness of record.
  $combination_columns = ap_create_columnheaders('combination_columns');

  $sysvar = ap_get_variablenames(
    array('variablename' => 'varset'),
    array('set' => 'terms')
  );

  foreach($sysvar as $i => $var) {
    $sysvar_terms[$i] = variable_get($var);
  }

  $result = chado_query(
    "SELECT phenotype.phenotype_id
     FROM
       {phenotype} AS phenotype
       LEFT JOIN {cvterm}        AS trait_name ON phenotype.attr_id = trait_name.cvterm_id
       LEFT JOIN {stock}         AS germplasm_accession USING (stock_id)
       LEFT JOIN {phenotypeprop} AS year USING (phenotype_id)
       LEFT JOIN {phenotypeprop} AS location USING (phenotype_id)
       LEFT JOIN {phenotypeprop} AS replicate USING (phenotype_id)
     WHERE
       year.type_id = :year_cvterm
       AND location.type_id  = :location_cvterm
       AND replicate.type_id = :replicate_cvterm
       
       AND trait_name.name = :trait
       AND germplasm_accession.uniquename = :uniquename
       AND year.value = :year 
       AND location.value = :location
       AND replicate.value = :rep"
    ,
    array(
      ':year_cvterm'      => $sysvar_terms['ap_year'],
      ':location_cvterm'  => $sysvar_terms['ap_location'],
      ':replicate_cvterm' => $sysvar_terms['ap_replicate'],

      // Trait name, Germplasm Accession, Year, Location and Replicate.
      ':trait'            => $datarow[ $combination_columns[0] ], 
      ':uniquename'       => $datarow[ $combination_columns[1] ],
      ':year'             => $datarow[ $combination_columns[2] ],
      ':location'         => $datarow[ $combination_columns[3] ],
      ':rep'              => $datarow[ $combination_columns[4] ],
    )
  );


  return ($result->rowCount()) ? $result->fetchField() : null;
}


/**
 * FUNCTION:
 * Convert column header to R-Friendly version.
 *
 * @param $columnheader
 *   A string, column header or value in Trait Name column.
 *
 * @return
 *   A string in R compatible format.
 */
function ap_convert_rcolumnheader($columnheader) {
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
  $selected_trait = trim(strtolower($columnheader));
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
  return ucfirst(implode('.', $rfriendly));
}
