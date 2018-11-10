<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;
use Faker\Factory;

class PhenotypeSeeder extends Seeder {
  /**
   * Seeds the database with users.
   *
   * @return void
   */
  public function up($num_traits = 3, $num_methods_per_trait = 1) {
    $faker = Factory::create();

    // Create Project.
    $project = $info['project'] = factory('chado.project')->create();
    $organism = $info['organism'] = factory('chado.organism')->create();

    // Configure Module for this organism.
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

    // Create Stocks (between 50-100 of them).
    $stocks = factory('chado.stock', rand(50,100))->create();

    // Create traits.
    $traits = [];
    for($i=1; $i<=$num_traits; $i++) {

      $trait_name = $faker->name();
      $trait_description = $faker->sentences(2, true);

      //for($i=1; $i<=$num_methods_per_trait; $i++) {
        $traits[] = ap_insert_trait([
          'name' => $trait_name,
          'definition' => $trait_description,
          'method_title' => $faker->words(2, true),
          'method' => $faker->sentences(5, true),
          'unit' => $faker->word(true),
          'genus' => $organism->genus,
        ]);
      //}
    }

    // For each trait...
    foreach ($traits as $trait_details) {

      // Generate specifics global to the trait.
      $min = rand(10,500);
      $max = $min + rand(100,500);
      $num_years = rand(2,5);
      $num_sites = rand(1,4);

      // For each year...
      for ($yrI=1; $yrI<=$num_years; $yrI++) {
        $year = $faker->year();

        // For each site...
        for ($siteI=1; $siteI<=$num_sites; $siteI++) {
          $site = $faker->city() . ', ' . $faker->country();

          // $mean = rand($min, $max);

          // For each of 3 reps...
          for ($rep=1; $rep<=3; $rep++) {

            // For each germplasm assayed...
            foreach ($stocks as $stock) {

              // Use sections to simulate a bell curve.
              // By having more sections near to the mean, we ensure more
              // values will fall near to the mean.
              // @todo implemement

              $value = rand($min, $max);

              $phenotype_data = [
                'uniquename' => uniqid(),
                'project_id' => $project->project_id,
                'stock_id' => $stock->stock_id,
                'attr_id' => $trait_details['trait']->cvterm_id,
                'assay_id' => $trait_details['method']->cvterm_id,
                'unit_id' => $trait_details['unit']->cvterm_id,
                'value' => $value,
              ];
              $phenotype_id = db_insert('chado.phenotype')
                ->fields($phenotype_data)
                ->execute();

              // Location:
              $values = [
                'value' => $site,
                'type_id' => $prop_types['location'],
                'phenotype_id' => $phenotype_id,
              ];
              chado_insert_record('phenotypeprop', $values);

              // Year:
              $values = [
                'value' => $year,
                'type_id' => $prop_types['year'],
                'phenotype_id' => $phenotype_id,
              ];
              chado_insert_record('phenotypeprop', $values);

              // Check the replicate:
              $values = [
                'value' => $rep,
                'type_id' => $prop_types['replicate'],
                'phenotype_id' => $phenotype_id,
              ];
              chado_insert_record('phenotypeprop', $values);

              // Check the data collector:
              $values = [
                'value' => $faker->name,
                'type_id' => $prop_types['collector'],
                'phenotype_id' => $phenotype_id,
              ];
              chado_insert_record('phenotypeprop', $values);

              $phenotypes[] = [
                'project' => $project,
                'stock' => $stock,
                'trait' => $trait_details['trait'],
                'method' => $trait_details['method'],
                'unit' => $trait_details['unit'],
                'phenotype_id' => $phenotype_id,
                'phenotype' => $phenotype_data,
                'site' => $site,
                'year' => $year,
                'replicate' => $rep,
                'data_collector' => $values['value'],
              ];

            } // stock...
          } // replicate...
        } // site...
      } // year...
    } // trait...

    return $phenotypes;
  }
}
