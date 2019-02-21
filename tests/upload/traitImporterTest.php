<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class traitImporterTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Tests loading of the Trait Importer.
   * @group upload
   * @group importer
   */
  public function testTraitImporter() {
    $faker = Factory::create();

    // Load our importer into scope.
    module_load_include('inc', 'analyzedphenotypes', 'includes/TripalImporter/TraitImporter');

    $organism = $info['organism'] = factory('chado.organism')->create([
      'genus' => $faker->unique->word . uniqid(),
    ]);

    // Configure Module for this organism
    // Set the line in the config page for the fake organism.
    // Variables are analyzedphenotypes_systemvar_[genus]_[cv/db/ontology]
    $trait_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_cv', $trait_cv->cv_id);
    $method_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_method', $method_cv->cv_id);
    $unit_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_unit', $unit_cv->cv_id);
    $trait_db = $info['trait_db'] = factory('chado.db')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_db', $trait_db->db_id);

    // Create an array of arguments we'll use for testing our importer.
    $run_args = ['genus' => $organism->genus];
    $file = ['file_local' => __DIR__ . '/../example_files/AnalyzedPhenotypes-Traits.tsv'];

    // Create a new instance of our importer.
    $importer = new \TraitImporter();
    $importer->create($run_args, $file);

    // Before we run our loader we must let the TripalImporter prepare the
    // files for us.
    $importer->prepareFiles();
    $importer->run();

    $FILE = fopen($file['file_local'], 'r');
    $linenum = 0;
    while (($data = fgetcsv($FILE, 100000, "\t")) !== FALSE) {
      if ($linenum !== 0) {

	// Check trait inserted in correct CV.
        $trait_name = $data[0];
        $exists = chado_select_record('cvterm', array('cvterm_id'), array('cv_id' => $trait_cv->cv_id, 'name' => $trait_name));
	$this->assertNotEmpty($exists,
	  "Unable to find trait, $trait_name, in the expected CV."); 

	// Check method inserted in correct CV.
        $method_name = $data[2];
	$exists = chado_select_record('cvterm', array('cvterm_id'), array('cv_id' => $method_cv->cv_id, 'name' => $method_name));
	$this->assertNotEmpty($exists,
	  "Unable to find the method, $method_name, in the expected CV.");

	// Check unit inserted in correct CV.
        $unit_name = $data[4];
        $exists = chado_select_record('cvterm', array('cvterm_id'), array('cv_id' => $unit_cv->cv_id, 'name' => $unit_name));
	$this->assertNotEmpty($exists,
	  "Unable to find method, $unit_name, in the expected CV.");
        $unit_id = $exists[0]->cvterm_id;

	// Check type was set properly.
        $type = $data[5];
        $exists = chado_select_record('cvtermprop', array('cvtermprop_id'), array('cvterm_id' => $unit_id, 'value' => $type));
        $this->assertNotEmpty($exists,
          "Unable to find the expected type, $type.");
      }
      $linenum++;
    }
    fclose($FILE);
  }
}
