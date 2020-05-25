<?php

namespace Tests\DatabaseSeeders;

use StatonLab\TripalTestSuite\Database\Seeder;
use StatonLab\TripalTestSuite\Database\Factory;

class Massive2500MillionPhenotypeSeeder extends Seeder
{
    /**
     * Seeds the database with 2.5 Billion phenotype records. Specifically,
     *  - 1000 experiments
     *  - 50 traits
     *  - 10 site-years
     *  - 5000 germplasm
     * Each experiments consists of the same 50 traits measuring the same
     * germplasm in the same site-years. The actual measurements are different.
     *
     * @return void
     */
    public function up() {
      $faker = \Faker\Factory::create();
      print "\n";

      // --------------------------------------------------
      // Some of our Required variables.
      // --    5 Species.
      $values = [
        [
          'abbreviation' => 'T. databasica',
          'genus' => 'Tripalus',
          'species' => 'databasica',
          'common_name' => 'Cultivated Tripal'
        ],
        [
          'abbreviation' => 'T. ferox',
          'genus' => 'Tripalus',
          'species' => 'ferox',
          'common_name' => 'Wild Tripal'
        ],
        [
          'abbreviation' => 'T. marveleria',
          'genus' => 'Tripalus',
          'species' => 'marveleria',
          'common_name' => 'Marvelous Tripal'
        ],
        [
          'abbreviation' => 'T. fascinatus',
          'genus' => 'Tripalus',
          'species' => 'fascinatus',
          'common_name' => 'Fascinating Tripal'
        ],
        [
          'abbreviation' => 'T. amazeus',
          'genus' => 'Tripalus',
          'species' => 'amazeus',
          'common_name' => 'Amazing Tripal'
        ]
      ];
      $all_organism = [];
      foreach($values as $organism) {
        $match = [
          'genus' => $organism['genus'],
          'species' => $organism['species'],
        ];
        $exists = chado_select_record('organism', ['organism_id'], $match);
        if ($exists) {
          $all_organism[] = $exists[0]->organism_id;
          chado_update_record('organism', $match, $organism);
        }
        else {
          $r = chado_insert_record('organism', $organism);
          $all_organism[] = $r['organism_id'];
        }
      }
      print "CREATED " . count($all_organism) . " Tripalus species.\n";
      // -- Germplasm Type.
      //   We choose the type_ids of stock-based Tripal Content Types.
      $germplasm_type_id = db_query("SELECT type_id FROM {chado_bundle}
        WHERE data_table = 'stock' AND type_column = 'type_id' LIMIT 1")->FetchField();
      // -- Retrieve the types that should be used for each property.
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

      // --------------------------------------------------
      // We generate the parts of the phenotype.
      $num_experiments = 10;
      $num_germplasm = 25;
      $num_traits = 3;
      // -- 1000 Experiments.
      $records = factory('chado.project', $num_experiments)->create();
      $experiment_ids = [];
      foreach ($records as $r) {
        $experiment_ids[] = $r->project_id;
      }
      print "CREATED " . count($experiment_ids) . " experiments.\n";

      // -- 1000 Germplasm.
      $germplasm_ids = [];
      foreach ($all_organism as $organism_id) {
        $records = factory('chado.stock', $num_germplasm)->create([
          'organism_id' => $organism_id,
          'type_id' => $germplasm_type_id,
        ]);
        foreach ($records as $r) {
          $germplasm_ids[] = $r->stock_id;
        }
      }
      print "CREATED " . count($germplasm_ids) . " germplasm.\n";

      // --   20 Traits.
      $traits = [];
      for ($i=0; $i<$num_traits; $i++) {
        $trait = ap_insert_trait([
          'genus' => 'Tripalus',
          'name' => $faker->words(3, TRUE),
          'description' => $faker->sentences(2, TRUE),
          'method_title' => $faker->words(3, TRUE),
          'method' => $faker->sentences(5, TRUE),
          'unit' => $faker->word(),
          'type' => 'quantitative',
        ]);
        if ($trait) { $traits[] = $trait; }
      }
      print "CREATED " . count($traits) . " traits.\n";

      // --   10 Site-Years.
      $siteyrs = [];
      foreach ([1950, 1951] as $year) {
        for($i=0; $i<5; $i++) {
          $siteyrs[] = [$year, $faker->city()];
        }
      }
      print "CREATED " . count($siteyrs) . " site years.\n";

      // --------------------------------------------------
      // Now, we finally generate the phenotypes!
      $phenotypes = 0;
      foreach ($experiment_ids as $experiment_id) {         // Experiments
        foreach ($traits as $trait_details) {               // Traits
          // Generate specifics global to the trait.
          $min = rand(10,500);
          $max = $min + rand(100,500);

          foreach ($germplasm_ids as $germplasm_id) {       // Germplasm
            foreach ($siteyrs as $siteyr) {                 // Site Years
              for ($rep=0; $rep<3; $rep++) {                // Replications

                $value = rand($min, $max);

                $phenotype_data = [
                  'uniquename' => uniqid(),
                  'project_id' => $experiment_id,
                  'stock_id' => $germplasm_id,
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
                  'value' => $siteyr[1],
                  'type_id' => $prop_types['location'],
                  'phenotype_id' => $phenotype_id,
                ];
                chado_insert_record('phenotypeprop', $values);

                // Year:
                $values = [
                  'value' => $siteyr[0],
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

                $phenotypes++;
              }
            }
          }
        }
      }
      print "CREATED " . $phenotypes . " phenotypes.\n";
    }
}