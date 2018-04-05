(function($) {
  Drupal.behaviors.analyzedPhenotypesBeanplotSelectField = {
    attach: function (context, settings) {
    var projectTrait = $('#cache-project_traits');
    var fldTrait = $('#ap-trait-distribution-trait-field');
    var fldExperiment = $('#ap-trait-distribution-experiment-field');

    var experimentChanged = 0;

    fldExperiment.once(function() {
      $(this).change(function() {
        experimentChanged = 1;
      });
    });

    // Option selected. Gray-out traits not related to a project.
    // This will cue user what traits available in an experiment,
    // however the grayed option is still selectable in case user
    // selects it.

    $(document)
    .ajaxComplete(function() {
      var t = projectTrait.val().split('#');

      fldTrait.find('option').each(function() {
        // Trait is not a specific to an experiment.
        var v = $(this).attr('value');

        if (v > 0) {
          var found = 0;
          for(var i = 0; i < t.length; i++) {
            if (t[i] == v) {
              found = 1;

              break;
            }
          }

          var c = (found) ? 'black' : 'gray';

          $(this).once(function() {
            var txt = $(this).text();
            var nTxt = (c == 'black') ? '* ' : '';

            $(this)
              .css('color', c)
              .prepend(nTxt);
          });
        }
      });

      // Set the default experiment when trait field changed.
      var fldDefaultExperiment = $('#cache_project_id');
      if (fldDefaultExperiment.val() > 0) {
        fldExperiment.find('option').each(function() {
          if ($(this).attr('value') > 0) {
            var sel = ($(this).attr('value') == fldDefaultExperiment.val())
              ? 'selected' : '';

            $(this).once(function() {
              $(this).attr('selected', sel)
              .prepend(function() {
                return (sel == 'selected') ? '* ' : '';
              });
            });
          }
        });
      }
    })
    .ajaxStop(function() {
      if (experimentChanged) {
        $('#ap-trait-distribution-trait-field option:first-child')
          .attr('selected', 'selected');
      }

      experimentChanged = 0;
    });
} }; })(jQuery);
