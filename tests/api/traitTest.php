<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class traitTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test ap_insert_trait().
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApInsertTrait($trait_details, $expected_pass) {

    $trait = ap_insert_trait($trait_details);

    $this->assertNotEmpty($trait);

    // Check that each cvterm for the trait was created successfully.
    foreach (['trait','method','unit'] as $part) {
      $this->assertArrayHasKey($part, $trait);
      $this->assertObjectHasAttribute('cvterm_id', $trait[$part]);
    }

    // Check Trait -> Method relationship.
    $trait_rels = chado_generate_var(
      'cvterm_relationship',
      ['subject_id' => $trait['trait']->cvterm_id],
      ['return_array' => TRUE]
    );
    $this->assertEquals(1,sizeof($trait_rels));
    // - Type = NCIT:method
    $this->assertEquals('method', $trait_rels[0]->type_id->name);
    $this->assertEquals('NCIT', $trait_rels[0]->type_id->cv_id->name);
    // - Object = Method.
    $this->assertEquals($trait['method']->cvterm_id, $trait_rels[0]->object_id->cvterm_id);

    // Check Method -> Unit relationship.
    $method_rels = chado_generate_var(
      'cvterm_relationship',
      ['subject_id' => $trait['method']->cvterm_id],
      ['return_array' => TRUE]
    );
    $this->assertEquals(1,sizeof($method_rels));
    // - Type = NCIT:method
    $this->assertEquals('unit', $method_rels[0]->type_id->name);
    $this->assertEquals('uo', $method_rels[0]->type_id->cv_id->name);
    // - Object = Method.
    $this->assertEquals($trait['unit']->cvterm_id, $method_rels[0]->object_id->cvterm_id);
  }

  /**
   * Test ap_get_trait().
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetTrait($trait_details, $expected_pass) {

    // Supress tripal errors
    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");

    // Set the line in the config page for the fake organism.
    // Variables are analyzedphenotypes_systemvar_[genus]_[cv/db/ontology]
    $genus = 'Tripalus-AP-TESTSUITE';
    $trait_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_cv', $trait_cv->cv_id);
    $trait_db = factory('chado.db')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_db', $trait_db->db_id);

    // Try to retrieve a trait that doesn't yet exist.
    $trait = ap_get_trait($genus, $trait_details['name']);
    $this->assertFalse($trait,
      "Tried to get a trait which doesn't exist.");

    // Try to retrieve a trait that we just inserted.
    ap_insert_trait($trait_details);
    $trait = ap_get_trait($genus, $trait_details['name']);
    $this->assertNotFalse($trait);

    // Check that we have the basics of a trait.
    $this->assertNotEquals(0,$trait->cvterm_id, 'There was no cvterm_id');
    $this->assertEquals($trait_details['name'],$trait->name,
      'The retrieved trait name did not match.');
    $this->assertEquals($trait_details['description'],$trait->definition,
      'The retrieved trait definition did not match.');

    // Remove the configuration.
    variable_del('analyzedphenotypes_systemvar_'.$genus.'_cv');
    variable_del('analyzedphenotypes_systemvar_'.$genus.'_db');

    // Clean the buffer and unset tripal errors suppression
    putenv("TRIPAL_SUPPRESS_ERRORS");

  }

  /**
   * Test ap_get_method().
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetMethod($trait_details, $expected_pass) {
    $genus = 'Tripalus-AP-TESTSUITE';

    $this->configureApForOrganism();

    // Try to retrieve a method that doesn't yet exist.
    $method = ap_get_method($genus, $trait_details['method_title']);
    $this->assertFalse($method,
      "Tried to get a method which doesn't exist.");

    // Try to retrieve a method that we just inserted.
    $trait = ap_insert_trait($trait_details);
    $this->assertNotFalse($trait, 'not even the trait!?!?');
    $method = ap_get_method($genus, $trait_details['method_title']);
    $this->assertNotFalse($method,
      "Unable to retrieve a method we just inserted.");

    // Check that we have the basics of a method.
    $this->assertNotEquals(0,$method->cvterm_id, 'There was no cvterm_id');
    $this->assertEquals($trait_details['method_title'],$method->name,
      'The retrieved trait name did not match.');
    $this->assertEquals($trait_details['method'],$method->definition,
      'The retrieved trait definition did not match.');

  }

  /**
   * Test ap_get_trait_methods.
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetTraitMethods($trait_details, $expected_pass) {
    $genus = 'Tripalus-AP-TESTSUITE';

    $this->configureApForOrganism();

    // Try to retrieve methods for a trait that doesn't yet exist.
    $methods = ap_get_trait_methods($genus, $trait_details['name']);
    $this->assertFalse($methods,
      "Tried to get methods for a trait which doesn't exist.");

    // Try to retrieve methods for a trait that we just inserted.
    $trait = ap_insert_trait($trait_details);
    $this->assertNotFalse($trait, 'not even the trait!?!?');
    $methods = ap_get_trait_methods($genus, $trait_details['name']);
    $this->assertNotFalse($methods,
      "Unable to retrieve methods for the trait we just inserted.");
    $this->assertEquals(1, sizeof($methods), 'There should only be one method.');
  }

  /**
   * Test ap_get_method_units.
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetMethodUnits($trait_details, $expected_pass) {
    $genus = 'Tripalus-AP-TESTSUITE';

    $this->configureApForOrganism();

    // Try to retrieve methods for a trait that doesn't yet exist.
    $units = ap_get_trait_methods($genus, $trait_details['method']);
    $this->assertFalse($units,
      "Tried to get units for a method which doesn't exist.");

    // Try to retrieve methods for a trait that we just inserted.
    $trait = ap_insert_trait($trait_details);
    $this->assertNotFalse($trait, 'not even the trait!?!?');
    $units = ap_get_method_units($genus, $trait_details['method']);
    $this->assertNotFalse($units,
      "Unable to retrieve units for the trait we just inserted.");
    $this->assertEquals(1, sizeof($units), 'There should only be one unit.');
  }

  /**
   * Test ap_get_method_id.
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetMethodID($trait_details, $expected_pass) {
    $genus = 'Tripalus-AP-TESTSUITE';

    $this->configureApForOrganism();

    // Try to retrieve methods for a trait that doesn't yet exist.
    $id = ap_get_method_id($genus, $trait_details['method']);
    $this->assertFalse($id,
      "Tried to get method id for a method which doesn't exist.");

    // Try to retrieve methods for a trait that we just inserted.
    $trait = ap_insert_trait($trait_details);
    $this->assertNotFalse($trait, 'not even the trait!?!?');
    $id = ap_get_method_id($genus, $trait_details['method']);
    $this->assertNotFalse($id,
      "Unable to retrieve  method id for the method we just inserted.");
  }

  /**
   * Test ap_get_unit_id.
   *
   * @group api
   * @group trait
   *
   * @dataProvider provideTraits
   */
  public function testApGetUnitID($trait_details, $expected_pass) {
    $genus = 'Tripalus-AP-TESTSUITE';

    $this->configureApForOrganism();

    // Try to retrieve units for a trait that doesn't yet exist.
    $id = ap_get_unit_id($genus, $trait_details['unit']);
    $this->assertFalse($id,
      "Tried to get unit id for a unit which doesn't exist.");

    // Try to retrieve units for a trait that we just inserted.
    $trait = ap_insert_trait($trait_details);
    $this->assertNotFalse($trait, 'not even the trait!?!?');
    $id = ap_get_unit_id($genus, $trait_details['unit']);
    $this->assertNotFalse($id,
      "Unable to retrieve  unit id for the unit we just inserted.");
  }

  /**
   * Configure the system variables for Tripalus-AP-TESTSUITE.
   */
  public function configureApForOrganism() {
    $trait_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_cv', $trait_cv->cv_id);
    $method_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_method', $method_cv->cv_id);
    $unit_cv = factory('chado.cv')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_unit', $unit_cv->cv_id);
    $trait_db = factory('chado.db')->create();
    variable_set('analyzedphenotypes_systemvar_tripalus-ap-testsuite_db', $trait_db->db_id);
  }

  /**
   * Data Provider
   */
  public function provideTraits() {
    $faker = Factory::create();
    $traits = [];

    // Prepare our necessary records.
    $organism = factory('chado.organism')->create(['genus' => 'Tripalus-AP-TESTSUITE']);

    // Non-existent, should work.
    $traits[] = [
      'trait_details' => [
        'genus' => $organism->genus,
        'name' => $faker->words(3, true),
        'description' => $faker->sentences(2, true),
        'method_title' => $faker->words(2, true),
        'method' => $faker->sentences(5, true),
        'unit' => $faker->word(),
      ],
      'expected_pass' => true,
    ];

    return $traits;
  }
}
