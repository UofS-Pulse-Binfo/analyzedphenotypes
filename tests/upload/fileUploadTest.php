<?php
namespace Tests\upload;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class fileUploadTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test uploading a simple file with a single trait/method/unit combination.
   * @group upload
   * @group file-tests
   */
  public function testSingleTrait() {

    // Load the file.
    $traits_in_file = array();
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'dolor sit amet',
      'unit' => 'metris',
      'type' => 'quantitative',
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
    $this->assertEquals($info['methods']['dolor sit amet'], $records[0]->assay_id,
      "We did not get the method cvterm_id we expected.");

    // Check that there is a single unit.
    $records = chado_query(
      'SELECT DISTINCT unit_id FROM {phenotype} WHERE project_id=:project',
      array(':project' => $info['project']->project_id))->fetchAll();
    $this->assertEquals(1, sizeof($records),
      "We did not get the number of units we expected.");
    $this->assertEquals($info['units']['metris'], $records[0]->unit_id,
      "We did not get the unit cvterm_id we expected.");

    // Test the method is of the correct type: qualitative or quantitative.
    $type = chado_query("SELECT value FROM {cvtermprop} WHERE cvterm_id=:id AND type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='additionalType')",
      array(':id' => $info['units']['metris']))->fetchField();
    $this->assertEquals('quantitative',$type,
      "The unit metris was not quantitative.");
  }

  /**
   * Test uploading a simple file with a single trait/method/unit combination.
   * @group upload
   * @group file-tests
   */
  public function testTwoMethodsPerTrait() {

    // Load the file.
    $traits_in_file = array();
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'dolor sit amet',
      'unit' => 'metris',
      'type' => 'qualitative',
    );
    $traits_in_file[] = array(
      'trait_name' => 'Lorem ipsum',
      'method_name' =>'vitae aliquet arcu',
      'unit' => 'metris',
      'type' => 'qualitative',
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

    // Check that there are two methods.
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

    // Test the method is of the correct type: qualitative or quantitative.
    $type = chado_query("SELECT value FROM {cvtermprop} WHERE cvterm_id=:id AND type_id IN (SELECT cvterm_id FROM {cvterm} WHERE name='additionalType')",
      array(':id' => $info['units']['metris']))->fetchField();
    $this->assertEquals('qualitative',$type,
      "The unit metris was not qualitative.");

  }
}
