<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class fileUploadTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test uploading a simple file with a single trait/method/unit combination.
   */
  public function testSingleTrait() {

    // Load the file.
    $traits_in_file = array();
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'dolor sit amet',
      'unit' => 'metris',
    );
    $info = \APFileUploadHelper::loadFile('SHORT-TestData-1trait1method.tsv', $traits_in_file);

    // Retrieve the phenotype records.
    // There should be 45 records total.
    $num_records = chado_query(
      'SELECT count(*) FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchField();
    $this->assertEquals(45, $num_records,
      "The number of records inserted does not match what we expected.");

    // Check that there is only a single trait.
    $records = chado_query(
      'SELECT DISTINCT attr_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of traits we expected.");
    $this->assertEquals($info['traits']['Lorem ipsum'], $records[0]->attr_id,
      "We did not get the trait cvterm_id we expected.");

    // Check that there is only a single method.
    $records = chado_query(
      'SELECT DISTINCT assay_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of methods we expected.");
    $this->assertEquals($info['methods']['Lorem ipsum'], $records[0]->assay_id,
      "We did not get the method cvterm_id we expected.");

    // Check that there is a single unit.
    $records = chado_query(
      'SELECT DISTINCT unit_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of units we expected.");
    $this->assertEquals($info['units']['Lorem ipsum'], $records[0]->unit_id,
      "We did not get the unit cvterm_id we expected.");
  }

  /**
   * Test uploading a simple file with a single trait/method/unit combination.
   * @group lacey-wip
   */
  public function testTwoMethodsPerTrait() {

    // Load the file.
    $traits_in_file = array();
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'dolor sit amet',
      'unit' => 'metris',
    );
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'vitae aliquet arcu',
      'unit' => 'metris',
    );
    $info = \APFileUploadHelper::loadFile('SHORT-TestData-1trait2methods.tsv', $traits_in_file);

    // Retrieve the phenotype records.
    // There should be 90 records total.
    $num_records = chado_query(
      'SELECT count(*) FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchField();
    $this->assertEquals(90, $num_records,
      "The number of records inserted does not match what we expected.");

    // Check that there is only a single trait.
    $records = chado_query(
      'SELECT DISTINCT attr_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of traits we expected.");
    $this->assertEquals($info['traits']['Lorem ipsum'], $records[0]->attr_id,
      "We did not get the trait cvterm_id we expected.");

    // Check that there is only a single method.
    $records = chado_query(
      'SELECT DISTINCT assay_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(2, sizeof($records),
      "We did not get the number of methods we expected.");

    // Check that there is a single unit.
    $records = chado_query(
      'SELECT DISTINCT unit_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of units we expected.");
  }
}
