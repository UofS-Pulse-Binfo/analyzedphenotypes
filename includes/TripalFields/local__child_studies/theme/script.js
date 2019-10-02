/**
 * @file
 * Manage child studies field behavior.
 */
(function ($) {
Drupal.behaviors.ChildStudies = {
  attach: function(context, settings) {
  // Load tiles, with grace!
  $('#ap-field-child-studies-wrapper').fadeIn(600);

  // Reference all tiles.
  var traitWrapper = $('#ap-field-child-studies-wrapper ul.trait-list > li');

  // Set the common height.
  var maxHeight = getMaxHeight();
  traitWrapper.css('height', maxHeight);


  // Highlight similar stocks.
  $('#ap-field-child-studies-wrapper ul.trait-list > li div')
    .mouseover(function() {
      var stock = $(this).text();

      traitWrapper.find('a').each(function() {
        if ($(this).text() == stock) {
          $(this).parent().css('border-width', '2px');
        }
      });
  }).mouseout(function() {
    traitWrapper.find('div').css('border-width', '1px');
  });


  /**
   * Function, calculate the maximum height based on all the tiles.
   * The largest of all will become the base height across all tiles.
   */
  function getMaxHeight() {
    var maxHeight = 0;

    traitWrapper.each(function() {
      var liHeight = Math.floor($(this).height());

      if (liHeight > maxHeight) {
        maxHeight = liHeight;
      }
    });

    return maxHeight + 10;
  }

}}}(jQuery));
