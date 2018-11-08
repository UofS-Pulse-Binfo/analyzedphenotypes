<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class dataIntegrityTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Basic test example.
   * Tests must begin with the word "test".
   * See https://phpunit.readthedocs.io/en/latest/ for more information.
   */
  public function testUploadDataIntegrity() {

    $info = $this->loadFile('AnalyzedPhenotypes-TestData-1trait3loc2yr3rep.txt');
    // @debug print "Info: ".print_r($info,TRUE)."\n";

    // Retrieve the phenotype records.
    // There should be X records total.
    $num_records = chado_query(
      'SELECT count(*) FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchField();
    $this->assertEquals(270, $num_records,
      "The nunmber of records inserted does not match what we expected.");

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
         'stock_uniquename' => $line[1],
         'year' => $line[3],
         'location' => $line[4],
         'replicate' => $line[5],
         'value' => $line[6],
         'data_collector' => $line[7],
        ];

        // Try to pull the expected record out of the db.
        // Unique Combination (for given test file):
        //  trait, stock, project,location, year, replicate.

        // Compile the values to search on.
        $values = [];
        $values[':trait_id'] = $info['traits'][ $file['trait_name'] ];
        $values[':method_id'] = $info['methods'][ $file['trait_name'] ];
        $values[':unit_id'] = $info['units'][ $file['trait_name'] ];
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
          "There trait/stock/project/value combination in the following line was not saved to the chado phenotype table.\n"
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
  }

  /**
   * HELPER: Load Data in specified file.
   *
   * @param $file
   *   The full path to the file.
   */
  function loadFile($file) {
    $faker = Factory::create();
    $info = [];

    $path = drupal_get_path('module', 'analyzedphenotypes');
    $file = DRUPAL_ROOT . '/' . $path . '/tests/example_files/' . $file;
    $info['file'] = $file;

    $project = $info['project'] = factory('chado.project')->create();
    $organism = $info['organism'] = factory('chado.organism')->create();

    // Configure Module for this organism
    // Set the line in the config page for the fake organism.
    // Variables are analyzedphenotypes_systemvar_[genus]_[cv/db/ontology]
    $trait_cv = $info['trait_cv'] = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_cv', $trait_cv->cv_id);
    $trait_db = $info['trait_db'] = factory('chado.db')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_db', $trait_db->db_id);

    // Attach organism to the project.
    chado_insert_record('projectprop', [
      'project_id' => $project->project_id,
      'type_id' => ['name' => 'genus', 'cv_id' => ['name' => 'taxonomic_rank']],
      'value' => $organism->genus,
    ]);

    // Add our file to the managed file system.
    $fakefilename = 'temporary://'.$faker->uuid();
    file_unmanaged_copy($file, $fakefilename);
    $managed_file = new \stdClass();
    $managed_file->fid = NULL;
    $managed_file->uri = $fakefilename;
    $managed_file->filename = drupal_basename($fakefilename);
    $managed_file->filemime = file_get_mimetype($fakefilename);
    $managed_file->uid = 1;
    $managed_file->status = FILE_STATUS_PERMANENT;
    file_save($managed_file);
    $data_file_fid = $managed_file->fid;
    // @debug print "FID: $data_file_fid.\n";

    // Set/Create Trait IDs for our file.
    $trait_name = 'Lorem ipsum';
    $results = ap_insert_trait([
      'name' => $trait_name,
      'definition' => $faker->sentences(2, true),
      'method_title' => $faker->words(2, true),
      'method' => $faker->sentences(5, true),
      'unit' => $faker->word(true),
      'genus' => $organism->genus,
    ]);

    // @debug print_r($results);

    $trait_cvterm_ids = [ $trait_name => $results['trait']->cvterm_id ];
    $info['traits'] = $trait_cvterm_ids;
    $method_cvterm_ids = [ $trait_name => $results['method']->cvterm_id ];
    $info['methods'] = $method_cvterm_ids;
    $unit_cvterm_ids = [ $trait_name => $results['unit']->cvterm_id ];
    $info['units'] = $unit_cvterm_ids;
    $info['trait_details'] = [ $results ];

    // Ensure the germplasm exists.
    $info['stocks'] = [];
    $stock_type = factory('chado.cvterm')->create();
    for ($i=1; $i <= 15; $i++) {
      $stock = [
        'name' => 'GERM'.$i,
        'uniquename' => 'ID:'.$i,
        'type_id' => $stock_type->cvterm_id,
        'organism_id' => $organism->organism_id,
      ];
      $stock = chado_insert_record(
        'stock',
        $stock
      );
      $info['stocks'][ 'ID:'.$i ] = $stock['stock_id'];
    }

    // Load the data
    ob_start();
    $dataset = [
      'project_name' => $project->name,
      'project_genus' => $organism->genus,
      'data_file' => $data_file_fid,
      'trait_cvterm' => $trait_cvterm_ids,
      'method_cvterm' => $method_cvterm_ids,
      'unit_cvterm' => $unit_cvterm_ids];
    analyzedphenotypes_save_tsv_data(serialize($dataset), 999999999);
    ob_end_clean();

    // Return any fake variables to the parent function.
    return $info;

  }

}
