<?php
namespace Tests\api;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;
use Faker\Factory;

class tripalJobTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test Tripal Jobs API.
   * @group tripal-jobs-api
   */
  public function testTripalJob() {
    $faker = Factory::create();

    // Test job creation:
    $args = [
      'description' => $faker->sentence(7,TRUE),
      'dataset' => ['arg1' => 1, 'arg2' => 2, 'arg3' => [3,4]],
      'callback' => 'ap_create_tripaljob',
    ];
    $options = ['return_jobid' => TRUE];
    $job_id = ap_create_tripaljob($args, $options);
    $this->assertNotFalse($job_id,
      'Tripal job was not created successfully.');

    // Test job retrieval.
    $job = ap_get_tripaljob($job_id);
    $this->assertNotFalse($job);
    $this->assertEquals($job_id, $job['job_id'],
      'The job returned was not the one we asked for.');

    // Test updating progress.
    $success = ap_update_tripaljob_progress($job_id, 50);
    print_r($success);
    $this->assertNotFalse($success,
      'Unable to update the progress of the job.');
    $job = ap_get_tripaljob($job_id);
    $this->assertEquals(50, $job['progress'],
      'The job progress was not what we expected.');

    // The following functions manage display of progress bar.
    $source = 'jobprogress';
    $success = ap_write_tripaljob_progress($job_id, 75, $source);
    $this->assertNotFalse($success,
      'Unable to write the progress of the job to a tracker file.');
    $progress = ap_read_tripaljob_progress($job_id, $source);
    $this->assertNotFalse($progress,
      'Unable to read the progress of the job from the tracker file.');
    $this->assertEquals(75, $progress,
      'The job progress was not what we expected.');

  }
}
