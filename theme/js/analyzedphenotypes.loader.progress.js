/**
 * @file
 * Contains scripts to manage behaviours used in Drag and Drop.
 */

(function($) {
  Drupal.behaviors.apProgress = {
    attach: function (context, settings) {
      var settings = Drupal.settings.analyzedphenotypes.vars;

      var jobId = settings['job_id'];
      var pathJSON = settings['path_json'];
      var pathValidation = settings['path_validation'];

      pb = new Drupal.progressBar('trpdownloadProgressBar', function(percentage, message) {
        if (percentage == '100') {
          pb.stopMonitoring();

          $('#ap-validation-result-embed').once().load(pathValidation + jobId);
        }
      });


      pb.setProgress(0, '');
      $('.progress-pane').append(pb.element);

      pb.startMonitoring(pathJSON + jobId, 250);
    }
  };
}(jQuery));
