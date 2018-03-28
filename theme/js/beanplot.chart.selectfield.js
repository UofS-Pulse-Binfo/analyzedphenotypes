(function($) {
  Drupal.behaviors.analyzedPhenotypesBeanplotSelectField = {
    attach: function (context, settings) {
    var notProjectTrait = $('#cache-notproject_traits');
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
      var t = notProjectTrait.val().split('#');

      fldTrait.find('option').each(function() {
        // Trait is not a specific to an experiment.
        var c = (t.indexOf($(this).attr('value')) > 0) ? '#CCCCCC' : 'black';
        $(this).css('color', c);
      });

      // Set the default experiment when trait field changed.
      var fldDefaultExperiment = $('#cache_project_id');
      if (fldDefaultExperiment.val() > 0) {
        fldExperiment.find('option').each(function() {
          if ($(this).attr('value') > 0) {
            var sel = ($(this).attr('value') == fldDefaultExperiment.val())
              ? 'selected' : '';

            $(this).attr('selected', sel);
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
