<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Tests\DatabaseSeeders\PhenotypeSeeder;
use Faker\Factory;
require_once 'APFileUploadHelper.php';

class dataIntegrityTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Data Integrity test.
   * This checks that the data is loaded into the chado tables correctly.
   *
   * @group seeder
   * @group data-integrity
   *
   * Specifically, analyzedphenotypes_save_tsv_data() is run with fake parameters
   * and an example file and then
   *   - it's checked that every fake phenotype became a record in chado.phenotype
   *   - each record in chado.phenotype should be linked to the
   *       - trait cvterm via phenotype.attr_id
   *       - method cvterm via phenotype.assay_id
   *       - unit cvterm via phenotype.unit_id
   *       - project via phenotype.project_id
   *       - germplasm assayed via phenotype.stock_id
   *   - each record in chado.phenotype should have the following properties
   *       - location
   *       - year
   *       - replicate
   *       - data collector
   *     using terms chosen by the administrator.
   */
  public function testPhenotypeSeederDataIntegrity() {

    $seeder = new PhenotypeSeeder();
    $data = $seeder->up();

    /* DEBUG
    $debug_query =  "
      SELECT json_build_object(
        'phenotype_id', p.phenotype_id,
        'trait_id', p.attr_id,
        'method_id', p.assay_id,
        'unit_id', p.unit_id,
        'stock_id', p.stock_id,
        'project_id', p.project_id,
        'value', p.value,
        'properties', array_to_json(array_agg(props.json))
      ) as json
      FROM chado.phenotype p
        LEFT JOIN (
        SELECT row_to_json(prop) as json, phenotype_id
        FROM chado.phenotypeprop prop
      ) props ON props.phenotype_id=p.phenotype_id
      GROUP BY p.phenotype_id, p.attr_id, p.stock_id, p.project_id, p.value";
    $results = chado_query($debug_query)->fetchAll();
    print "Database Records (JSON):\n";
    foreach($results as $result) {
      $noSlahes = str_replace('\\', '', $result->json);
      $jsonObject = json_decode($noSlahes);
      print "------------------------------\n";
      print print_r($jsonObject, TRUE)."\n";
    }

    */

    $single_phenotype_query =  "
      SELECT
        p.phenotype_id as phenotype_id,
        p.attr_id as trait_id,
        p.assay_id as method_id,
        p.unit_id as unit_id,
        p.stock_id as stock_id,
        p.project_id as project_id,
        p.value as value,
        count(props.*) as properties
      FROM chado.phenotype p
        LEFT JOIN chado.phenotypeprop props ON props.phenotype_id=p.phenotype_id
      WHERE p.phenotype_id=:id
      GROUP BY p.phenotype_id, p.attr_id, p.stock_id, p.project_id, p.value";


    foreach ($data as $expected) {

      // @debug print_r($expected);

      // Select from the database as JSON.
      $db_result = chado_query(
          $single_phenotype_query,
          [':id' => $expected['phenotype_id']])->fetchObject();
      // @debug print str_repeat('-',50) . print_r($db_result, TRUE)."\n";

      $this->assertEquals($expected['trait']->cvterm_id, $db_result->trait_id,
        "The Trait ID was not what we expected.");

      $this->assertEquals($expected['method']->cvterm_id, $db_result->method_id,
        "The Method ID was not what we expected.");

      $this->assertEquals($expected['unit']->cvterm_id, $db_result->unit_id,
        "The Unit ID was not what we expected.");

      $this->assertEquals($expected['project']->project_id, $db_result->project_id,
        "The Project ID was not what we expected.");

      $this->assertEquals($expected['stock']->stock_id, $db_result->stock_id,
        "The Stock ID was not what we expected.");

      $this->assertEquals($expected['phenotype']['value'], $db_result->value,
        "The Phenotypic Value was not what we expected.");

      $this->assertEquals(4, $db_result->properties,
        "There should be 4 properties (location, year, replicate and data collector) for each phenotype.");
    }

    $this->assertNotEmpty($data);

    // Remove configuration.
    $genus = $data[0]['organism']->genus;
    variable_del('analyzedphenotypes_systemvar_'.$genus.'_cv');
    variable_del('analyzedphenotypes_systemvar_'.$genus.'_db');
  }

  /**
   * Data Integrity test.
   * This checks that the data is loaded into the chado tables correctly.
   *
   * @group data-integrity
   * @group upload
   * @group lacey-wip
   *
   * Specifically, analyzedphenotypes_save_tsv_data() is run with fake parameters
   * and an example file and then
   *   - it's checked that every line of the file became a record in chado.phenotype
   *   - each record in chado.phenotype should be linked to the
   *       - trait cvterm via phenotype.attr_id
   *       - method cvterm via phenotype.assay_id
   *       - unit cvterm via phenotype.unit_id
   *       - project via phenotype.project_id
   *       - germplasm assayed via phenotype.stock_id
   *   - each record in chado.phenotype should have the following properties
   *       - location
   *       - year
   *       - replicate
   *       - data collector
   *     using terms chosen by the administrator.
   */
  public function testUploadDataIntegrity() {

    $traits_in_file = array();
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'dolor sit amet',
      'unit' => 'metris',
    );
    $info = \APFileUploadHelper::loadFile('TestData-1trait3loc2yr3rep.tsv', $traits_in_file);
    // @debug print "Info: ".print_r($info,TRUE)."\n";

    // Retrieve the phenotype records.
    // There should be X records total.
    $num_records = chado_query(
      'SELECT count(*) FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchField();
    $this->assertEquals(270, $num_records,
      "The number of records inserted does not match what we expected.");

    // Retrieve the types that should be used for each property.
    $sysvars = ap_get_variablenames(
      array('variablename' => 'varset'),
      array('set' => 'terms')
    );
    $prop_types = [
      'location' => variable_get($sysvars['ap_location']),
      'year' => variable_get($sysvars['ap_year']),
      'replicate' => variable_get($sysvars['ap_replicate']),
      'collector' => variable_get($sysvars['ap_collector']),
    ];
    // @debug print "Property types" . print_r($prop_types,TRUE) . "\n";

    // For every line of the file, check the data was loaded properly.
    if (($handle = fopen($info['file'], "r")) !== FALSE) {

      // Remove header.
      fgets($handle);

      $line_num = 0;
      while (($line = fgetcsv($handle, 1000, "\t")) !== FALSE) {
        $line_num++;
        //@debug print "Line: ".print_r($line,TRUE)."\n";

        // Grab values to test from file.
        $file = [
         'trait_name' => $line[0],
         'method_name' => $line[1],
         'unit' => $line[2],
         'stock_uniquename' => $line[3],
         'year' => $line[5],
         'location' => $line[6],
         'replicate' => $line[7],
         'value' => $line[8],
         'data_collector' => $line[9],
        ];

        // Try to pull the expected record out of the db.
        // Unique Combination (for given test file):
        //  trait, stock, project,location, year, replicate.

        // Compile the values to search on.
        $values = [];
        $values[':trait_id'] = $info['traits'][ $file['trait_name'] ];
        $values[':method_id'] = $info['methods'][ $file['method_name'] ];
        $values[':unit_id'] = $info['units'][ $file['unit'] ];
        $values[':stock_id'] = $info['stocks'][ $file['stock_uniquename'] ];
        $values[':project_id'] = $info['project']->project_id;
        $values[':value'] = $file['value'];

        // Check trait/stock/project combination exists
        $phenotype_records = chado_query(
          'SELECT phenotype_id FROM {phenotype} WHERE
              attr_id=:trait_id AND
              assay_id = :method_id AND
              unit_id = :unit_id AND
              stock_id=:stock_id AND
              project_id=:project_id AND
              value=:value',
          $values)->fetchAll();

        $this->assertNotEmpty($phenotype_records,
          "The trait/stock/project/value combination in the following line was not saved to the chado phenotype table.\n"
          ."  Failed Line ($line_num) : '"
          .implode("\t",$file)."'\n"
          ."  Selected Values: ".print_r($values,TRUE)."\n");

        // FOR THIS TRAIT/STOCK/PROJECT COMBINATION:
        if (is_array($phenotype_records)) {

          // Process the ids for easy restriction of results.
          $phenotype_ids = [];
          foreach ($phenotype_records as $r) {
            $phenotype_ids[] = $r->phenotype_id;
          }

          // Retrieve the phenotype_id for the specific value
          $phenotype_id = chado_query(
            'SELECT phenotype_id FROM {phenotype} WHERE value=:value AND phenotype_id IN (:ids)',
            array(':value' => $file['value'], ':ids' => $phenotype_ids))->fetchField();

            /* DEBUG
            print "------------------------------\n";
            print "Line ($line_num) : '" . implode("\t",$file)."'\n";

            $debug_query =  "
              SELECT json_build_object(
                'phenotype_id', p.phenotype_id,
                'trait_id', p.attr_id,
                'method_id', p.assay_id,
                'unit_id', p.unit_id,
                'stock_id', p.stock_id,
                'project_id', p.project_id,
                'value', p.value,
                'properties', array_to_json(array_agg(props.json))
              ) as json
              FROM chado.phenotype p
                LEFT JOIN (
                SELECT row_to_json(prop) as json, phenotype_id
                FROM chado.phenotypeprop prop
              ) props ON props.phenotype_id=p.phenotype_id
              WHERE p.phenotype_id IN (:ids)
              GROUP BY p.phenotype_id, p.attr_id, p.stock_id, p.project_id, p.value";
            $results = chado_query($debug_query, array(':ids' => $phenotype_ids))->fetchAll();
            print "Database Records (JSON):\n";
            foreach($results as $result) {
              $noSlahes = str_replace('\\', '', $result->json);
              $jsonObject = json_decode($noSlahes);
              print print_r($jsonObject, TRUE)."\n";
            }

            */

          // Check the location:
          $values = [
            ':value' => $file['location'],
            ':proptype' => $prop_types['location'],
            ':ids' => $phenotype_ids
          ];
          $present = chado_query(
            'SELECT count(*) FROM {phenotypeprop} WHERE value=:value AND type_id=:proptype AND phenotype_id IN (:ids)',
            $values)->fetchField();
          $this->assertGreaterThan(0, $present,
            "The location, '".$file['location']."' was not recorded for the expected trait/stock/project/value combination.\n"
            ."  Failed Line ($line_num) : '" . implode("\t",$file)."'\n"
            ."  Selected Values: ".print_r($values,TRUE)."\n"
          );

          // Check the year:
          $values = [
            ':value' => $file['year'],
            ':proptype' => $prop_types['year'],
            ':ids' => $phenotype_ids
          ];
          $present = chado_query(
            'SELECT count(*) FROM {phenotypeprop} WHERE value=:value AND type_id=:proptype AND phenotype_id IN (:ids)',
            $values)->fetchField();
          $this->assertGreaterThan(0, $present,
            "The year, '".$file['year']."' was not recorded for the expected trait/stock/project/value combination.\n"
            ."  Failed Line ($line_num) : '" . implode("\t",$file)."'\n"
            ."  Selected Values: ".print_r($values,TRUE)."\n"
          );

          // Check the replicate:
          $values = [
            ':value' => $file['replicate'],
            ':proptype' => $prop_types['replicate'],
            ':ids' => $phenotype_ids
          ];
          $present = chado_query(
            'SELECT count(*) FROM {phenotypeprop} WHERE value=:value AND type_id=:proptype AND phenotype_id IN (:ids)',
            $values)->fetchField();
          $this->assertGreaterThan(0, $present,
            "The replicate, '".$file['replicate']."' was not recorded for the expected trait/stock/project/value combination.\n"
            ."  Failed Line ($line_num) : '" . implode("\t",$file)."'\n"
            ."  Selected Values: ".print_r($values,TRUE)."\n"
          );

          // Check the data collector:
          $values = [
            ':value' => $file['data_collector'],
            ':proptype' => $prop_types['collector'],
            ':ids' => $phenotype_ids
          ];
          $present = chado_query(
            'SELECT count(*) FROM {phenotypeprop} WHERE value=:value AND type_id=:proptype AND phenotype_id IN (:ids)',
            $values)->fetchField();
          $this->assertGreaterThan(0, $present,
            "The replicate, '".$file['data_collector']."' was not recorded for the expected trait/stock/project/value combination.\n"
            ."  Failed Line ($line_num) : '" . implode("\t",$file)."'\n"
            ."  Selected Values: ".print_r($values,TRUE)."\n"
          );
        }
      }
    }
    else {
      $this->fail('Unable to read example file.');
    }

    // Remove configuration.
    variable_del('analyzedphenotypes_systemvar_'.$info['organism']->genus.'_cv');
    variable_del('analyzedphenotypes_systemvar_'.$info['organism']->genus.'_db');
  }



}
