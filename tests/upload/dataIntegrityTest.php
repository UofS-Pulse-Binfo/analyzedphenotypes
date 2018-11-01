<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class dataIntegrityTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  // use DBTransaction;

  /**
   * Basic test example.
   * Tests must begin with the word "test".
   * See https://phpunit.readthedocs.io/en/latest/ for more information.
   */
  public function testUploadDataIntegrity() {

    $this->loadFile('AnalyzedPhenotypes-TestData-1trait3loc2yr3rep.txt');
    $this->assertTrue(true);

  }

  /**
   * HELPER: Load Data in specified file.
   *
   * @param $file
   *   The full path to the file.
   */
  function loadFile($file) {
    $faker = Factory::create();

    $path = drupal_get_path('module', 'analyzedphenotypes');
    $file = DRUPAL_ROOT . '/' . $path . '/tests/example_files/' . $file;

    $project = factory('chado.project')->create(); 
    $organism = factory('chado.organism')->create();
 
    // Configure Module for this organism
    // Set the line in the config page for the fake organism.
    // Variables are analyzedphenotypes_systemvar_[genus]_[cv/db/ontology]
    $trait_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_'.$organism->genus.'_cv', $trait_cv->cv_id);
    $trait_db = factory('chado.db')->create();
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
    $trait_name = 'Lorem ipsum (cm)';
    $cvterm = ap_insert_cvterm(
      array(
        'name' => $trait_name,
        'definition' => 'Lorem ipsum testing.',
        'genus' => $organism->genus,
      ),
      array('return_inserted_id' => TRUE)
    );
    $trait_cvterm_ids = [ $trait_name => $cvterm ];

    // Ensure the germplasm exists.
    $stock_type = factory('chado.cvterm')->create(); 
    for ($i=1; $i <= 15; $i++) {
      $stock = [
        'name' => 'GERM'.$i,
        'uniquename' => 'ID:'.$i,
        'type_id' => $stock_type->cvterm_id,
        'organism_id' => $organism->organism_id,
      ];
      chado_insert_record(
        'stock',
        $stock
      );
    }

    // Load the data
    $dataset = [
      'project_name' => $project->name,
      'project_genus' => $organism->genus,
      'data_file' => $data_file_fid,
      'trait_cvterm' => $trait_cvterm_ids];
    analyzedphenotypes_save_tsv_data(serialize($dataset), 999999999);

    // Return any fake variables to the parent function.    
    return [
      'project' => $project,
      'organism' => $organism,
    ];
  }

}
