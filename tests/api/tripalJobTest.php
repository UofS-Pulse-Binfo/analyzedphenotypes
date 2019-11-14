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
  public function testCreateTripalJob() {
    $faker = Factory::create();

    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");

    // Test job created and job_id returned.
    $args = [
      'description' => $faker->sentence(7,TRUE),
      'dataset' => ['arg1' => 1, 'arg2' => 2, 'arg3' => [3,4]],
      'callback' => 'ap_create_tripaljob',
    ];
    $options = ['return_jobid' => TRUE];
    $job_id = ap_create_tripaljob($args, $options);
    $this->assertNotFalse($job_id,
      'Tripal job was not created successfully but should have been.');
    $this->assertGreaterThan(0,$job_id,
      'The job_id should have been returned and was not.');

    // Test job created and true returned.
    $options = [];
    $success = ap_create_tripaljob($args, $options);
    $this->assertTRUE($success,
      'Tripal job was not created successfully but should have been.');

    // Test job not created when callback doesn't exist.
    $args = [
      'description' => $faker->sentence(7,TRUE),
      'dataset' => ['arg1' => 1, 'arg2' => 2, 'arg3' => [3,4]],
      'callback' => 'test_callback_doesnt_exist',
    ];
    $options = ['return_jobid' => TRUE];
    $job_id = ap_create_tripaljob($args, $options);
    $this->assertFalse($job_id,
      'Tripal job was created successfully but should NOT have been.');

    putenv("TRIPAL_SUPPRESS_ERRORS");
  }

  /**
   * Test Retrieval of Tripal job.
   * @group tripal-jobs-api
   */
  public function testGetTripalJob() {
    $faker = Factory::create();

    putenv("TRIPAL_SUPPRESS_ERRORS=TRUE");

    $args = [
      'description' => $faker->sentence(7,TRUE),
      'dataset' => ['arg1' => 1, 'arg2' => 2, 'arg3' => [3,4]],
      'callback' => 'ap_create_tripaljob',
    ];
    $options = ['return_jobid' => TRUE];
    $job_id = ap_create_tripaljob($args, $options);

    // Test job retrieval when it should work.
    $job = ap_get_tripaljob($job_id);
    $this->assertNotFalse($job);
    $this->assertEquals($job_id, $job['job_id'],
      'The job returned was not the one we asked for.');

    // Test job retrieval when it should fail.
    $job = ap_get_tripaljob(0);
    $this->assertFalse($job);

    putenv("TRIPAL_SUPPRESS_ERRORS");
  }

  /**
   * Test Retrieval of Tripal job progress.
   * @group tripal-jobs-api
   */
  public function testGetProgressTripalJob() {
    $faker = Factory::create();

    $args = [
      'description' => $faker->sentence(7,TRUE),
      'dataset' => ['arg1' => 1, 'arg2' => 2, 'arg3' => [3,4]],
      'callback' => 'ap_create_tripaljob',
    ];
    $options = ['return_jobid' => TRUE];
    $job_id = ap_create_tripaljob($args, $options);

    // Test updating progress.
    $success = ap_update_tripaljob_progress($job_id, 50);
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

    // Test we cannot read the progress when the job_id doesn't exist.
    $progress = ap_read_tripaljob_progress(0, $source);
    $this->assertFalse($progress,
      'Read progress from a tracker file when we had no job???');

  }

}
