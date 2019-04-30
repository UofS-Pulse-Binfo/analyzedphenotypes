<?php

use Faker\Factory;

/**
 * This class can be included at the top of your TripalTests Case
 * to facillitate testing of analyzedphenotpye file upload.
 */
class APFileUploadHelper {

  /**
   * HELPER: Load Data in specified file.
   *
   * @param $file
   *   The full path to the file.
   * @param $traits_in_file
   *   An array where each element defines trait_name, method_name and unit.
   */
  public static function loadFile($file, $traits_in_file) {
    $faker = Factory::create();
    $info = [];

    $path = drupal_get_path('module', 'analyzedphenotypes');
    $file = DRUPAL_ROOT . '/' . $path . '/tests/example_files/' . $file;
    $info['file'] = $file;

    $project = $info['project'] = factory('chado.project')->create([
      'name' => $faker->unique->word . ' ' . uniqid(),
    ]);
    $organism = $info['organism'] = factory('chado.organism')->create([
      'genus' => $faker->unique->word . uniqid(),
    ]);

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
    $trait_cvterm_ids = array();
    $method_cvterm_ids = array();
    $unit_cvterm_ids = array();
    $info['trait_details'] = array();
    foreach ($traits_in_file as $d) {
      $results = ap_insert_trait([
        'name' => $d['trait_name'],
        'description' => $faker->sentences(2, true),
        'method_title' => $d['method_name'],
        'method' => $faker->sentences(5, true),
        'unit' => $d['unit'],
        'genus' => $organism->genus,
        'type' => $d['type'],
      ]);

      // @debug print_r($results);

      $trait_cvterm_ids[ $d['trait_name'] ] = $results['trait']->cvterm_id;
      $method_cvterm_ids[ $d['method_name'] ] = $results['method']->cvterm_id;
      $unit_cvterm_ids[ $d['unit'] ] = $results['unit']->cvterm_id;
      $info['trait_details'][] = $results;
    }

    $info['traits'] = $trait_cvterm_ids;
    $info['methods'] = $method_cvterm_ids;
    $info['units'] = $unit_cvterm_ids;

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
      'unit_cvterm' => $unit_cvterm_ids,
    ];
    analyzedphenotypes_save_tsv_data(serialize($dataset), 999999999);
    ob_end_clean();

    // Return any fake variables to the parent function.
    return $info;

  }
}
