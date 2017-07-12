/**
 * @file
 * Contains scripts to manage behaviours used in Tripal Download Progress Bar.
 */

(function($) {
  Drupal.behaviors.apProgress = {
    attach: function (context, settings) {
      //
      var vrContainerId = '#ap-validation-result-embed';

      var vrContainerId = '#ap-validation-result-embed';

      var jobId;
      var jobIdField = $('#ap-job-id-hidden-field');

      var DrupalProgressBar = new Drupal.progressBar('trpdownloadProgressBar', progressBarResult);
      $('.progress-pane').once().append(DrupalProgressBar.element);


      var begin = 0;
      beginListener();


      function jobIdFieldListener() {
        if(!begin) {
          return;
        }

        jobId = jobIdField.val();
        if (jobId > 0) {
          jobIdField.val('0');
          progressBar();

          begin = 0;
        }

        setInterval(jobIdFieldListener, 1000);
      }


      function beginListener() {
        begin = 1;
        jobIdFieldListener();
      }


      function progressBar() {
        if (jobId > 0) {
          var pathJSON = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/json/validate_jobstatus/';

          DrupalProgressBar.setProgress(0, 'Validating Data...');
          DrupalProgressBar.startMonitoring(pathJSON + jobId, 500);
        }
      }


      function progressBarResult(percentage, message, pb) {
        var pathVAL  = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/validation_result/data/';

        if ((percentage == '100' || message == 'Error') && jobId > 0) {
          $(vrContainerId).load(pathVAL + jobId, function() {
            if ($('#ap-validator-failed').length > 0) {
              $('#ap-dnd-container').show();
              DrupalProgressBar.setProgress(0, 'Validating Data...');
              beginListener();
            }
            else {
              $('.ap-tsv-file-form-element span').remove();
            }
          });

          jobId = 0;
          pb.stopMonitoring();
        }
      }


      function resetJobIdField() {
        if (jobIdField.val() > 0) {
          jobIdField.val(0);
        }
      }


      //
} }; }(jQuery));




/*

var jobId;
      var jobIdField = $('#ap-job-id-hidden-field');
      var DrupalProgressBar = new Drupal.progressBar('trpdownloadProgressBar', progressBarResult);

      var begin;
      beginListener();

      function jobIdFieldListener() {
        if(!begin) {
          return;
        }

        jobId = jobIdField.val();
        if (jobId > 0) {
          progressBar();
        }

        setTimeout(jobIdFieldListener, 1000);
      }

      function pauseListener() {
        begin = 0;
      }

      function beginListener() {
        begin = 1;
        jobIdFieldListener();
      }

      function progressBar() {
        if (jobId > 0) {
          resetJobIdField();

          var pathJSON = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/json/validate_jobstatus/';

          DrupalProgressBar.setProgress(0, 'Waiting...');
          $('.progress-pane').once().append(DrupalProgressBar.element);

          DrupalProgressBar.startMonitoring(pathJSON + jobId, 500);
        }
      }

      function progressBarResult(percentage, message, pb) {
        var pathVAL  = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/validation_result/data/';

        if ((percentage == '100' || message == 'Error') && jobId > 0) {
          $('#ap-validation-result-embed').once().load(pathVAL + jobId, function() {
            if ($('#ap-validator-failed').length > 0) {
              DrupalProgressBar.setProgress(0, 'Waiting...');
              $('#ap-dnd-container').show();
              beginListener();
            }
            else {
              $('.ap-tsv-file-form-element span').remove();
            }
          });

          jobId = 0;
          resetJobIdField();
          pb.stopMonitoring();
        }
      }

      function resetJobIdField() {
        if (jobIdField.val() > 0) {
          jobIdField.val(0);
          pauseListener();
        }
      }

    //////////////////////////////////////////////////////////////////////////////////


    var vrContainerId = '#ap-validation-result-embed';

      var jobId;
      var jobIdField = $('#ap-job-id-hidden-field');

      var DrupalProgressBar = new Drupal.progressBar('trpdownloadProgressBar', progressBarResult);
      $('.progress-pane').once().append(DrupalProgressBar.element);


      var begin = 0;
      beginListener();


      function jobIdFieldListener() {
        if(!begin) {
          return;
        }

        jobId = jobIdField.val();
        if (jobId > 0) {
          progressBar();
        }

        setTimeout(jobIdFieldListener, 1000);
      }


      function pauseListener() {
        begin = 0;
      }


      function beginListener() {
        begin = 1;
        jobIdFieldListener();
      }


      function progressBar() {
        if (jobId > 0) {
          resetJobIdField();

          var pathJSON = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/json/validate_jobstatus/';

          DrupalProgressBar.setProgress(0, 'Validating Data...');
          DrupalProgressBar.startMonitoring(pathJSON + jobId, 500);
        }
      }


      function progressBarResult(percentage, message, pb) {
        var pathVAL  = '/dev/reynold/admin/tripal/extension/analyzedphenotypes/validation_result/data/';

        if ((percentage == '100' || message == 'Error') && jobId > 0) {
          $(vrContainerId).load(pathVAL + jobId, function() {
            if ($('#ap-validator-failed').length > 0) {
              $('#ap-dnd-container').show();
              beginListener();
            }
            else {
              $('.ap-tsv-file-form-element span').remove();
            }
          });

          jobId = 0;
          //resetJobIdField();
          pb.stopMonitoring();
        }
      }


      function resetJobIdField() {
        if (jobIdField.val() > 0) {
          jobIdField.val(0);
          pauseListener();
        }
      }


      */
