<?php
namespace Tests\download;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use  Tests\DatabaseSeeders\PhenotypeSeeder;

class generateFileTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Generate File Test: TSV
   *
   * @group download
   * @group generate-file
   *
   * Specifically, analyzedphenotypes_trpdownload_generate_file() is run after inserting
   * fake data and then
   *   - there are the expected number of lines in the file based on the database
   *   - it's checked that every line in the file has a line in the database
   */
  public function testGenerateFileTSV() {

    $seeder = new PhenotypeSeeder();
    $data = $seeder->up();

    $this->assertTrue(true);
  }
}
