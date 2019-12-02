/**
 * @file
 * Contains scripts to manage STAGE 01 - Upload File.
 */

(function($) {
  Drupal.behaviors.stage01 = {
    attach: function (context, settings) {
      // Add event listener to project autocomplete search field.
      $('#ap-project-select-field')
      .focus(function(e) {
        // Field gets focused.
        if (e.target.value) {
          // Select value previously entered, ready for new entry.
          $(this).select();
        }
      })
      .focusout(function(e) {
        // Clicks on autocomplete suggestions.
        var projectName = e.target.value;

        if (projectName && Drupal.settings.analyzedphenotypes.vars.project_genus) {
          $.get(Drupal.settings.analyzedphenotypes.vars.project_genus + encodeURI(projectName), null, getProjectGenus);
        }
        else {
          getProjectGenus('');
        }
      });


      /**
       * Funciton Callback, process get response.
      */
      function getProjectGenus(response) {
        if (response.genus) {
          // Select the genus. Then disable it.
          $('#ap-genus-select-field').val(response.genus)
        }
        else {
          // Set to default in case it was altered previously.
          $('#ap-genus-select-field').val('- Select -');
        }
      }
      //
} }; }(jQuery));
