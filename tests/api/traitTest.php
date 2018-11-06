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
   * Data Provider
   */
  public function provideTraits() {
    $faker = Factory::create();
    $traits = [];

    // Prepare our necessary records.
    $organism = factory('chado.organism')->create();

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
